#include "simple_ensemble_clustering.h"

int SimpleEnsembleClustering::main() {
    this->WriteToLogFile("Loading clustering files" , 1);
    std::vector<std::map<std::string, std::string>> input_clusterings;
    for(int i = 0; i < this->clustering_files.size(); i ++) {
        input_clusterings.push_back(Consensus::ReadClusteringFile(this->clustering_files[i]));
    }

    this->WriteToLogFile("Loading clustering weights" , 1);
    std::vector<float> float_custering_weights;
    for(int i = 0; i < this->clustering_weights.size(); i++) {
        float_custering_weights.push_back(std::stof(this->clustering_weights[i]));
    }

    this->WriteToLogFile("Loading the final graph" , 1);
    FILE* edgelist_file = fopen(this->edgelist.c_str(), "r");
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_read_graph_ncol(&graph, edgelist_file, NULL, true, IGRAPH_ADD_WEIGHTS_YES, IGRAPH_UNDIRECTED);
    fclose(edgelist_file);
    this->WriteToLogFile("Finished loading the final graph", 1);


    this->WriteToLogFile("Started setting the default edge weights for the final graph", 1);
    Consensus::SetIgraphAllEdgesWeight(&graph, 0);
    this->WriteToLogFile("Finished setting the default edge weights for the final graph", 1);

    this->WriteToLogFile("Started adding to edge weights for the final graph", 1);
    igraph_eit_t eit;
    igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    for(int i = 0; i < this->clustering_files.size(); i++) {
        std::map<std::string, std::string> current_partition = input_clusterings[i];
        IGRAPH_EIT_RESET(eit);
        for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&graph, current_edge);
            int to_node = IGRAPH_TO(&graph, current_edge);
            std::string from_node_id = VAS(&graph, "name", from_node);
            std::string to_node_id = VAS(&graph, "name", to_node);
            if(current_partition.contains(from_node_id) && current_partition.contains(to_node_id)) {
                if(current_partition.at(from_node_id) == current_partition.at(to_node_id)) {
                    igraph_real_t current_edge_weight = EAN(&graph, "weight", IGRAPH_EIT_GET(eit));
                    SETEAN(&graph, "weight", IGRAPH_EIT_GET(eit), current_edge_weight + (1 * float_custering_weights[i]));
                }
            }
        }
    }

    /* BEGIN min proposal */
    IGRAPH_EIT_RESET(eit);
    std::map<igraph_integer_t, float> edge_to_total_weight_map;
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
        int from_node = IGRAPH_FROM(&graph, current_edge);
        int to_node = IGRAPH_TO(&graph, current_edge);
        std::string from_node_id = VAS(&graph, "name", from_node);
        std::string to_node_id = VAS(&graph, "name", to_node);
        float current_weight_sum = 0.0;
        for(int i = 0; i < this->clustering_files.size(); i++) {
            std::map<std::string, std::string> current_partition = input_clusterings[i];
            if(current_partition.contains(from_node_id) && current_partition.contains(to_node_id)) {
                current_weight_sum += float_custering_weights[i];
            }
        }
        edge_to_total_weight_map[current_edge] = current_weight_sum;
    }
    IGRAPH_EIT_RESET(eit);
    for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
        igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
        int from_node = IGRAPH_FROM(&graph, current_edge);
        int to_node = IGRAPH_TO(&graph, current_edge);
        std::string from_node_id = VAS(&graph, "name", from_node);
        std::string to_node_id = VAS(&graph, "name", to_node);
        std::cerr << "edge_to_total_weight_map[" << from_node_id << "-" <<to_node_id << "] " << edge_to_total_weight_map[current_edge] << std::endl;
        /* igraph_integer_t current_edge = IGRAPH_EIT_GET(eit); */
        igraph_real_t current_edge_weight = EAN(&graph, "weight", IGRAPH_EIT_GET(eit));
        if(edge_to_total_weight_map[current_edge] > 0) {
            std::cerr << "before normalize edge weight " << from_node_id << "-" <<to_node_id << " " << current_edge_weight << std::endl;
            SETEAN(&graph, "weight", IGRAPH_EIT_GET(eit), current_edge_weight / edge_to_total_weight_map[current_edge]);
        }
    }
    igraph_eit_destroy(&eit);
    /* END min proposal */

    this->WriteToLogFile("Finsished adding to edge weights for the final graph" , 1);

    this->WriteToLogFile("Started removing edges from the final graph", 1);
    Consensus::RemoveEdgesBasedOnThreshold(&graph, this->threshold);
    this->WriteToLogFile("Finished removing edges from the final graph", 1);

    this->WriteToLogFile("Started the final connected components run", 1);
    std::map<int, int> final_partition = Consensus::GetConnectedComponents(&graph);
    this->WriteToLogFile("Finished the final connected components run", 1);

    this->WriteToLogFile("Started writing to the output clustering file", 1);
    this->WritePartitionMapWithTranslation(final_partition, &graph);
    this->WriteToLogFile("Finished writing to the output clustering file", 1);

    igraph_destroy(&graph);
    return 0;
}
