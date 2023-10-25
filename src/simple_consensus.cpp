#include "simple_consensus.h"
bool SimpleConsensus::CheckConvergence(igraph_t* graph_ptr, double max_weight, int iter_count) {
    if(iter_count == 0) {
        // It's the first iteration
        // Continuing without checking convergence
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
    igraph_eit_destroy(&eit);
    return count <= this->delta * num_edges;
}

int SimpleConsensus::main() {
    this->WriteToLogFile("Loading the initial graph" , 1);
    FILE* edgelist_file = fopen(this->edgelist.c_str(), "r");
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
    fclose(edgelist_file);
    this->WriteToLogFile("Finished loading the initial graph" , 1);
    this->WriteToLogFile("Started setting the default edge weights for the initial graph" , 1);
    Consensus::SetIgraphAllEdgesWeight(&graph, 1);
    this->WriteToLogFile("Finished setting the default edge weights for the initial graph" , 1);

    int iter_count = 0;
    double max_weight = 0;
    for (unsigned int i = 0; i < this->weight_vector.size(); i ++) {
        max_weight += this->weight_vector[i];
    }

    igraph_t next_graph;
    while (!SimpleConsensus::CheckConvergence(&next_graph, max_weight, iter_count) && iter_count < max_iter) {
        if(iter_count != 0) {
            igraph_destroy(&graph);
            igraph_copy(&graph, &next_graph);
        }
        iter_count ++;
        this->WriteToLogFile("Staring iteration: " + std::to_string(iter_count), 1);
        this->WriteToLogFile("Starting to copy the intermediate graphs", 1);
        igraph_copy(&next_graph, &graph);
        this->WriteToLogFile("Finshed copying the intermediate graphs", 1);
        this->WriteToLogFile("Starting to set the edge weight for the intermediate graph", 1);
        Consensus::SetIgraphAllEdgesWeight(&next_graph, 1);
        this->WriteToLogFile("Finsihed setting the edge weight for the intermediate graph", 1);

        std::vector<std::map<int, int>> results;
        this->WriteToLogFile("Starting workers" , 1);
        for(int i = 0; i < this->num_partitions; i ++) {
            Consensus::num_partition_index_queue.push(i);
        }
        for(int i = 0; i < this->num_partitions; i ++) {
            Consensus::num_partition_index_queue.push(-1);
        }

        std::vector<std::thread> thread_vector;
        for(int i = 0; i < this->num_processors; i ++) {
            thread_vector.push_back(std::thread(Consensus::ClusterWorker, this->edgelist, std::ref(this->algorithm_vector), std::ref(this->clustering_parameter_vector), &graph));
        }

        for(int i = 0; i < this->num_processors; i ++) {
            thread_vector[i].join();
        }
        while(!Consensus::done_being_clustered_clusterings.empty()) {
            results.push_back(Consensus::done_being_clustered_clusterings.front());
            Consensus::done_being_clustered_clusterings.pop();
        }

        this->WriteToLogFile("Got results back from workers" , 1);

        for(int i = 0; i < this->num_partitions; i++) {
            this->WriteToLogFile("Starting to incorate results from worker: " + std::to_string(i) , 1);
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
                SetIgraphEdgeWeightFromVertices(&next_graph, from_node, to_node, edge_weight);
            }
            igraph_eit_destroy(&eit);
            this->WriteToLogFile("Finished incorpating results from worker: " + std::to_string(i) , 1);
        }
        this->WriteToLogFile("Started removing edges from the intermediate graph" , 1);
        Consensus::RemoveEdgesBasedOnThreshold(&next_graph, this->threshold * max_weight);
        this->WriteToLogFile("Finished removing edges from the intermediate graph" , 1);
    }

    this->WriteToLogFile("Simple consensus took " + std::to_string(iter_count) + " iterations", 1);

    this->WriteToLogFile("Started the final clustering run" , 1);
    std::map<int, int> final_partition = GetCommunities("", this->final_algorithm, 0, this->final_resolution, &graph);
    this->WriteToLogFile("Finished the final clustering run" , 1);
    igraph_destroy(&graph);

    this->WriteToLogFile("Started writing to the output clustering file" , 1);
    this->WritePartitionMap(final_partition);
    this->WriteToLogFile("Finished writing to the output clustering file" , 1);


    return 0;
}
