#include "consensus.h"

std::map<int, int> get_communities(std::string edgelist, std::string algorithm, int seed, double resolution) {
    std::map<int, int> partition_map;

    FILE* edgelist_file = fopen(edgelist.c_str(), "r");
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
    fclose(edgelist_file);

    igraph_eit_t eit;
    igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        SETEAN(&graph, "weight", IGRAPH_EIT_GET(eit), 1);
    }
    if(algorithm == "louvain") {
        igraph_vector_int_t membership;
        igraph_vector_int_init(&membership, 0);
        igraph_rng_seed(igraph_rng_default(), seed);
        igraph_community_multilevel(&graph, 0, 1, &membership, 0, 0);

        IGRAPH_EIT_RESET(eit);
        igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
        std::set<int> visited;
        for (; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&graph, current_edge);
            int to_node = IGRAPH_TO(&graph, current_edge);
            if(!visited.contains(from_node)) {
                visited.insert(from_node);
                partition_map[from_node] = VECTOR(membership)[from_node];
            }
            if(!visited.contains(to_node)) {
                visited.insert(to_node);
                partition_map[to_node] = VECTOR(membership)[to_node];
            }
        }

    } else if(algorithm == "leiden-cpm") {
        Graph leiden_graph(&graph);
        CPMVertexPartition partition(&leiden_graph, resolution);
        Optimiser o;
        o.optimise_partition(&partition);

        IGRAPH_EIT_RESET(eit);
        igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
        std::set<int> visited;
        for (; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&graph, current_edge);
            int to_node = IGRAPH_TO(&graph, current_edge);
            if(!visited.contains(from_node)) {
                visited.insert(from_node);
                partition_map[from_node] = partition.membership(from_node);
            }
            if(!visited.contains(to_node)) {
                visited.insert(to_node);
                partition_map[to_node] = partition.membership(to_node);
            }
        }
    } else if(algorithm == "leiden-mod") {
        Graph leiden_graph(&graph);
        ModularityVertexPartition partition(&leiden_graph);
        Optimiser o;
        o.optimise_partition(&partition);

        IGRAPH_EIT_RESET(eit);
        igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
        std::set<int> visited;
        for (; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&graph, current_edge);
            int to_node = IGRAPH_TO(&graph, current_edge);
            if(!visited.contains(from_node)) {
                visited.insert(from_node);
                partition_map[from_node] = partition.membership(from_node);
            }
            if(!visited.contains(to_node)) {
                visited.insert(to_node);
                partition_map[to_node] = partition.membership(to_node);
            }
        }
    } else {
        throw std::invalid_argument("get_communities(): Unsupported algorithm");
    }

    // try running leiden

    igraph_destroy(&graph);
    return partition_map;
}

int Consensus::main() {
    std::vector<std::map<int, int>> results(this->num_partitions);
    #pragma omp parallel for num_threads(this->num_processors)
    for(int i = 0; i < this->num_partitions; i++) {
        results[i] = get_communities(this->edgelist, this->algorithm, i, this->resolution);
    }

    for(int i = 0; i < this->num_partitions; i++) {
        std::cout << "partition " << i << "results" << std::endl;
        for(auto const& [key, val]: results[i]) {
            std::cout << "node " << key << ": " << val  << std::endl;
        }
    }

    /* igraph_t g; */
    /* igraph_famous(&g, "Zachary"); */

    /* Graph graph(&g); */
    /* CPMVertexPartition part(&graph, 0.05); */
    /* Optimiser o; */
    /* o.optimise_partition(&part); */
    /* std::cout << "Node Community" << std::endl; */
    /* for(int i = 0; i < graph.vcount(); i++) { */
    /*     std::cout << i << " " << part.membership(i) << std::endl; */
    /* } */
    /* igraph_destroy(&g); */

    return 0;
}
