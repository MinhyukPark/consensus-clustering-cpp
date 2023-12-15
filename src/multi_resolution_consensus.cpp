#include "multi_resolution_consensus.h"

bool MultiResolutionConsensus::CheckConvergence(igraph_t* lhs_graph_ptr, igraph_t* rhs_graph_ptr, int iter_count) {
    if(iter_count == 0) {
        // It's the first iteration
        // Continuing without checking convergence
        return false;
    }
    igraph_bool_t is_identical;
    igraph_is_same_graph(lhs_graph_ptr, rhs_graph_ptr, &is_identical);
    if(is_identical) {
        this->WriteToLogFile("Graphs identical", 1);
    } else {
        this->WriteToLogFile("Graphs not identical" , 1);
    }
    return is_identical;
}

int MultiResolutionConsensus::main() {
    this->WriteToLogFile("Loading the initial graph" , 1);
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    this->LoadIgraphFromFile(&graph);
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
    while (!MultiResolutionConsensus::CheckConvergence(&graph, &next_graph, iter_count) || iter_count > max_iter) {
        if(iter_count != 0) {
            igraph_destroy(&graph);
            igraph_copy(&graph, &next_graph);
        }
        this->WriteToLogFile("Staring iteration: " + std::to_string(iter_count), 1);
        this->WriteToLogFile("Starting to copy the intermediate graphs", 1);
        if(iter_count != 0) {
            igraph_destroy(&next_graph);
        }
        igraph_copy(&next_graph, &graph);
        this->WriteToLogFile("Finshed copying the intermediate graphs", 1);
        this->WriteToLogFile("Starting to set the edge weight for the intermediate graph", 1);
        Consensus::SetIgraphAllEdgesWeight(&next_graph, 0);
        this->WriteToLogFile("Finsihed setting the edge weight for the intermediate graph", 1);

        std::vector<std::map<int, int>> results;
        this->WriteToLogFile("Starting workers" , 1);
        this->StartWorkers(&graph);
        while(!Consensus::done_being_clustered_clusterings.empty()) {
            results.push_back(Consensus::done_being_clustered_clusterings.front());
            Consensus::done_being_clustered_clusterings.pop();
        }

        this->WriteToLogFile("Got results back from workers" , 1);

        for(int i = 0; i < this->num_partitions; i++) {
            this->WriteToLogFile("Starting to incorate results from worker: " + std::to_string(i) , 1);
            std::map<int, int> current_partition = results.at(i);
            igraph_eit_t eit;
            igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
            for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
                igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
                int from_node = IGRAPH_FROM(&graph, current_edge);
                int to_node = IGRAPH_TO(&graph, current_edge);

                igraph_integer_t next_graph_current_edge;
                igraph_get_eid(&next_graph, &next_graph_current_edge, from_node, to_node, false, false);

                igraph_real_t graph_edge_weight = EAN(&graph, "weight", current_edge);
                if(graph_edge_weight != 0 && graph_edge_weight != max_weight) {
                    if(current_partition.at(from_node) == current_partition.at(to_node)) {
                        igraph_real_t next_graph_edge_weight = EAN(&next_graph, "weight", next_graph_current_edge);
                        next_graph_edge_weight += (1 * (this->weight_vector[i]));
                        SETEAN(&next_graph, "weight", next_graph_current_edge, next_graph_edge_weight);
                    }
                } else {
                    SETEAN(&next_graph, "weight", next_graph_current_edge, graph_edge_weight);
                }
            }
            igraph_eit_destroy(&eit);
            this->WriteToLogFile("Finished incorpating results from worker: " + std::to_string(i) , 1);
        }
        this->WriteToLogFile("Started removing edges from the intermediate graph" , 1);
        Consensus::RemoveEdgesBasedOnThreshold(&next_graph, this->threshold * max_weight);
        this->WriteToLogFile("Finished removing edges from the intermediate graph" , 1);
        iter_count ++;
    }

    this->WriteToLogFile("Simple consensus took " + std::to_string(iter_count) + " iterations", 1);

    this->WriteToLogFile("Started the final connected components run" , 1);
    std::map<int, int> final_partition = Consensus::GetConnectedComponents(&next_graph);
    this->WriteToLogFile("Finished the final connected components run" , 1);
    igraph_destroy(&graph);
    if(iter_count != 0) {
        igraph_destroy(&next_graph);
    }

    this->WriteToLogFile("Started writing to the output clustering file" , 1);
    this->WritePartitionMap(final_partition);
    this->WriteToLogFile("Finished writing to the output clustering file" , 1);


    return 0;
}
