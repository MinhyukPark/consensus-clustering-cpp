#include "consensus.h"

void Consensus::write_partition_map(std::map<int, int>& final_partition) {
    std::ofstream clustering_output(this->output_file);
    for(auto const& [node_id, cluster_id]: final_partition) {
        clustering_output << node_id << " " << cluster_id << '\n';
    }
    clustering_output.close();
}

void Consensus::remove_edges_based_on_threshold(igraph_t* graph, double current_threshold) {
    igraph_vector_int_t edges_to_remove;
    igraph_vector_int_init(&edges_to_remove, 0);
    igraph_eit_t eit;
    igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        igraph_real_t current_edge_weight = EAN(graph, "weight", IGRAPH_EIT_GET(eit));
        if(current_edge_weight < current_threshold) {
            igraph_vector_int_push_back(&edges_to_remove, IGRAPH_EIT_GET(eit));
        }
    }
    igraph_es_t es;
    igraph_es_vector_copy(&es, &edges_to_remove);
    igraph_delete_edges(graph, es);
    igraph_eit_destroy(&eit);
    igraph_es_destroy(&es);
    igraph_vector_int_destroy(&edges_to_remove);
}

void run_louvain_and_update_partition(std::map<int, int>& partition_map, int seed, igraph_t* graph) {
    igraph_vector_int_t membership;
    igraph_vector_int_init(&membership, 0);
    igraph_rng_seed(igraph_rng_default(), seed);
    igraph_community_multilevel(graph, 0, 1, &membership, 0, 0);

    igraph_eit_t eit;
    igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    std::set<int> visited;
    for (; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
        int from_node = IGRAPH_FROM(graph, current_edge);
        int to_node = IGRAPH_TO(graph, current_edge);
        if(!visited.contains(from_node)) {
            visited.insert(from_node);
            partition_map[from_node] = VECTOR(membership)[from_node];
        }
        if(!visited.contains(to_node)) {
            visited.insert(to_node);
            partition_map[to_node] = VECTOR(membership)[to_node];
        }
    }
    igraph_eit_destroy(&eit);
    igraph_vector_int_destroy(&membership);
}

void run_leiden_and_update_partition(std::map<int, int>& partition_map, MutableVertexPartition* partition, igraph_t* graph) {
    Optimiser o;
    o.optimise_partition(partition);
    igraph_eit_t eit;
    igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    std::set<int> visited;
    for (; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
        int from_node = IGRAPH_FROM(graph, current_edge);
        int to_node = IGRAPH_TO(graph, current_edge);
        if(!visited.contains(from_node)) {
            visited.insert(from_node);
            partition_map[from_node] = partition->membership(from_node);
        }
        if(!visited.contains(to_node)) {
            visited.insert(to_node);
            partition_map[to_node] = partition->membership(to_node);
        }
    }
    igraph_eit_destroy(&eit);
}

void set_igraph_all_edges_weight(igraph_t* graph, double weight) {
    igraph_eit_t eit;
    igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        SETEAN(graph, "weight", IGRAPH_EIT_GET(eit), 1);
    }
    igraph_eit_destroy(&eit);
}

std::map<int, int> get_communities(std::string edgelist, std::string algorithm, int seed, double clustering_parameter, igraph_t* graph_ptr) {
    std::map<int, int> partition_map;
    igraph_t graph;
    if(graph_ptr == NULL) {
        FILE* edgelist_file = fopen(edgelist.c_str(), "r");
        igraph_set_attribute_table(&igraph_cattribute_table);
        igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
        fclose(edgelist_file);
        set_igraph_all_edges_weight(&graph, 1);

    } else {
        graph = *graph_ptr;
    }

    if(algorithm == "louvain") {
        run_louvain_and_update_partition(partition_map, seed, &graph);
    } else if(algorithm == "leiden-cpm") {
        Graph leiden_graph(&graph);
        CPMVertexPartition partition(&leiden_graph, clustering_parameter);
        run_leiden_and_update_partition(partition_map, &partition, &graph);
    } else if(algorithm == "leiden-mod") {
        Graph leiden_graph(&graph);
        ModularityVertexPartition partition(&leiden_graph);
        run_leiden_and_update_partition(partition_map, &partition, &graph);
    } else {
        throw std::invalid_argument("get_communities(): Unsupported algorithm");
    }

    if(graph_ptr == NULL) {
        igraph_destroy(&graph);
    }
    return partition_map;
}

/*
 * message type here is 1 for INFO, 2 for DEBUG, and -1 for ERROR
 */
int Consensus::write_to_log_file(std::string message, int message_type) {
    if(this->log_level > 0) {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::string log_message_prefix;
        if(message_type == 1) {
            log_message_prefix = "[INFO]";
        } else if(message_type == 2) {
            log_message_prefix = "[DEBUG]";
        } else if(message_type == -1) {
            log_message_prefix = "[ERROR]";
        }
        auto days_elapsed = std::chrono::duration_cast<std::chrono::days>(now - this->start_time);
        auto hours_elapsed = std::chrono::duration_cast<std::chrono::hours>(now - this->start_time - days_elapsed);
        auto minutes_elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - this->start_time - days_elapsed - hours_elapsed);
        auto seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - this->start_time - days_elapsed - hours_elapsed - minutes_elapsed);
        auto total_seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - this->start_time);
        log_message_prefix += "[";
        log_message_prefix += std::to_string(days_elapsed.count());
        log_message_prefix += "-";
        log_message_prefix += std::to_string(hours_elapsed.count());
        log_message_prefix += ":";
        log_message_prefix += std::to_string(minutes_elapsed.count());
        log_message_prefix += ":";
        log_message_prefix += std::to_string(seconds_elapsed.count());
        log_message_prefix += "]";

        log_message_prefix += "(t=";
        log_message_prefix += std::to_string(total_seconds_elapsed.count());
        log_message_prefix += "s)";
        this->log_file_handle << log_message_prefix << " " << message << '\n';
    }
    return 0;
}

int ThresholdConsensus::main() {
    std::vector<std::map<int, int>> results(this->num_partitions);
    this->write_to_log_file("Starting workers" , 1);
    this->start_workers(results, NULL);
    this->write_to_log_file("Got results back from workers" , 1);


    this->write_to_log_file("Loading the final graph" , 1);
    FILE* edgelist_file = fopen(this->edgelist.c_str(), "r");
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
    fclose(edgelist_file);
    this->write_to_log_file("Finished loading the final graph" , 1);

    igraph_eit_t eit;
    igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

    this->write_to_log_file("Started setting the default edge weights for the final graph" , 1);
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        SETEAN(&graph, "weight", IGRAPH_EIT_GET(eit), 1);
    }
    this->write_to_log_file("Finished setting the default edge weights for the final graph" , 1);

    this->write_to_log_file("Started subtracting from edge weights for the final graph" , 1);
    for(int i = 0; i < this->num_partitions; i++) {
        std::map<int,int> current_partition = results[i];
        IGRAPH_EIT_RESET(eit);
        for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&graph, current_edge);
            int to_node = IGRAPH_TO(&graph, current_edge);
            if(current_partition[from_node] != current_partition[to_node]) {
                igraph_real_t current_edge_weight = EAN(&graph, "weight", IGRAPH_EIT_GET(eit));
                SETEAN(&graph, "weight", IGRAPH_EIT_GET(eit), current_edge_weight - ((double)1/this->num_partitions));
            }
        }
    }
    this->write_to_log_file("Finsished subtracting from edge weights for the final graph" , 1);

    this->write_to_log_file("Started removing edges from the final graph" , 1);
    this->remove_edges_based_on_threshold(&graph, this->threshold);
    this->write_to_log_file("Finished removing edges from the final graph" , 1);

    this->write_to_log_file("Started the final clustering run" , 1);
    std::map<int, int> final_partition = get_communities("", this->final_algorithm, 0, this->final_resolution, &graph);
    this->write_to_log_file("Finished the final clustering run" , 1);
    igraph_destroy(&graph);

    this->write_to_log_file("Started writing to the output clustering file" , 1);
    this->write_partition_map(final_partition);
    this->write_to_log_file("Finished writing to the output clustering file" , 1);

    return 0;
}

bool SimpleConsensus::check_convergence(igraph_t* graph_ptr, double max_weight, int iter_count) {
    this->write_to_log_file("Starting convergence check", 1);
    if(iter_count == 0) {
        this->write_to_log_file("It's the first iteration. Continuing without checking convergence.", 1);
        return false;
    }
    int count = 0;
    int num_edges = 0;
    igraph_eit_t eit;
    igraph_eit_create(graph_ptr, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        igraph_real_t current_edge_weight = IGRAPH_EIT_GET(eit);
        if (current_edge_weight != 0 && current_edge_weight != max_weight) {
            count ++;
        }
        num_edges ++ ;
    }
    this->write_to_log_file("Finished convergence check", 1);
    igraph_eit_destroy(&eit);
    return count <= this->delta * num_edges;
}

void set_igraph_edge_weight_from_vertices(igraph_t* graph, int from_node, int to_node, double edge_weight) {
    igraph_es_t graph_es;
    igraph_es_pairs_small(&graph_es, false, from_node, to_node, -1);
    igraph_eit_t graph_single_edge_eit;
    igraph_eit_create(graph, graph_es, &graph_single_edge_eit);
    for(; !IGRAPH_EIT_END(graph_single_edge_eit); IGRAPH_EIT_NEXT(graph_single_edge_eit)) {
        SETEAN(graph, "weight", IGRAPH_EIT_GET(graph_single_edge_eit), edge_weight);
    }
    igraph_eit_destroy(&graph_single_edge_eit);
    igraph_es_destroy(&graph_es);
}


void Consensus::start_workers(std::vector<std::map<int,int>>& results, igraph_t* graph_ptr) {
    omp_lock_t log_lock;
    omp_init_lock(&log_lock);
    #pragma omp parallel for num_threads(this->num_processors)
    for(int i = 0; i < this->num_partitions; i++) {
        results[i] = get_communities(this->edgelist, this->algorithm_vector[i], i, this->clustering_parameter_vector[i], graph_ptr);
        omp_set_lock(&log_lock);
        this->write_to_log_file("Worker " + std::to_string(i) + "finished", 1);
        omp_unset_lock(&log_lock);
    }
}


int SimpleConsensus::main() {
    this->write_to_log_file("Loading the initial graph" , 1);
    FILE* edgelist_file = fopen(this->edgelist.c_str(), "r");
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
    fclose(edgelist_file);
    this->write_to_log_file("Finished loading the initial graph" , 1);
    this->write_to_log_file("Started setting the default edge weights for the initial graph" , 1);
    set_igraph_all_edges_weight(&graph, 1);
    this->write_to_log_file("Finished setting the default edge weights for the initial graph" , 1);

    int iter_count = 0;
    double max_weight = 0;
    for (unsigned int i = 0; i < this->weight_vector.size(); i ++) {
        max_weight += this->weight_vector[i];
    }

    igraph_t next_graph;
    while (!this->check_convergence(&next_graph, max_weight, iter_count) && iter_count < max_iter) {
        if(iter_count != 0) {
            igraph_destroy(&graph);
            igraph_copy(&graph, &next_graph);
        }
        iter_count ++;
        this->write_to_log_file("Staring iteration: " + std::to_string(iter_count), 1);
        this->write_to_log_file("Starting to copy the intermediate graphs", 1);
        igraph_copy(&next_graph, &graph);
        this->write_to_log_file("Finshed copying the intermediate graphs", 1);
        this->write_to_log_file("Starting to set the edge weight for the intermediate graph", 1);
        set_igraph_all_edges_weight(&next_graph, 1);
        this->write_to_log_file("Finsihed setting the edge weight for the intermediate graph", 1);

        std::vector<std::map<int, int>> results(this->num_partitions);
        this->write_to_log_file("Starting workers" , 1);
        this->start_workers(results, &graph);
        this->write_to_log_file("Got results back from workers" , 1);

        for(int i = 0; i < this->num_partitions; i++) {
            this->write_to_log_file("Starting to incorate results from worker: " + std::to_string(i) , 1);
            std::map<int, int> current_partition = results[i];
            igraph_eit_t eit;
            igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
            for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
                igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
                int from_node = IGRAPH_FROM(&graph, current_edge);
                int to_node = IGRAPH_TO(&graph, current_edge);
                igraph_real_t edge_weight = EAN(&graph, "weight", current_edge);
                if(edge_weight != 0 && edge_weight != max_weight) {
                    if(current_partition[from_node] != current_partition[to_node]) {
                        edge_weight += (1 * (this->weight_vector[i]));
                    }
                }
                set_igraph_edge_weight_from_vertices(&next_graph, from_node, to_node, edge_weight);
            }
            igraph_eit_destroy(&eit);
            this->write_to_log_file("Finished incorpating results from worker: " + std::to_string(i) , 1);
        }
        this->write_to_log_file("Started removing edges from the intermediate graph" , 1);
        this->remove_edges_based_on_threshold(&next_graph, this->threshold * max_weight);
        this->write_to_log_file("Finished removing edges from the intermediate graph" , 1);
    }

    this->write_to_log_file("Simple consensus took " + std::to_string(iter_count) + " iterations", 1);

    this->write_to_log_file("Started the final clustering run" , 1);
    std::map<int, int> final_partition = get_communities("", this->final_algorithm, 0, this->final_resolution, &graph);
    this->write_to_log_file("Finished the final clustering run" , 1);
    igraph_destroy(&graph);

    this->write_to_log_file("Started writing to the output clustering file" , 1);
    this->write_partition_map(final_partition);
    this->write_to_log_file("Finished writing to the output clustering file" , 1);


    return 0;
}
