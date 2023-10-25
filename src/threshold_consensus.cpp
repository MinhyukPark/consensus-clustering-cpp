#include "threshold_consensus.h"

int ThresholdConsensus::main() {
    std::vector<std::map<int, int>> results;
    this->WriteToLogFile("Starting workers" , 1);
    for(int i = 0; i < this->num_partitions; i ++) {
        Consensus::num_partition_index_queue.push(i);
    }
    for(int i = 0; i < this->num_processors; i ++) {
        Consensus::num_partition_index_queue.push(-1);
    }

    std::vector<std::thread> thread_vector;
    for(int i = 0; i < this->num_processors; i ++) {
        thread_vector.push_back(std::thread(Consensus::ClusterWorker, this->edgelist, std::ref(this->algorithm_vector), std::ref(this->clustering_parameter_vector), nullptr));
    }

    for(int i = 0; i < this->num_processors; i ++) {
        thread_vector[i].join();
    }

    while(!Consensus::done_being_clustered_clusterings.empty()) {
        results.push_back(Consensus::done_being_clustered_clusterings.front());
        Consensus::done_being_clustered_clusterings.pop();
    }

    this->WriteToLogFile("Got results back from workers" , 1);


    this->WriteToLogFile("Loading the final graph" , 1);
    FILE* edgelist_file = fopen(this->edgelist.c_str(), "r");
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
    fclose(edgelist_file);
    this->WriteToLogFile("Finished loading the final graph" , 1);


    this->WriteToLogFile("Started setting the default edge weights for the final graph" , 1);
    Consensus::SetIgraphAllEdgesWeight(&graph, 1.0);
    this->WriteToLogFile("Finished setting the default edge weights for the final graph" , 1);

    this->WriteToLogFile("Started subtracting from edge weights for the final graph" , 1);
    igraph_eit_t eit;
    igraph_eit_create(&graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
    for(int i = 0; i < this->num_partitions; i++) {
        std::map<int,int> current_partition = results[i];
        IGRAPH_EIT_RESET(eit);
        for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
            igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
            int from_node = IGRAPH_FROM(&graph, current_edge);
            int to_node = IGRAPH_TO(&graph, current_edge);
            if(!current_partition.contains(from_node) || !current_partition.contains(to_node) || current_partition[from_node] != current_partition[to_node]) {
                igraph_real_t current_edge_weight = EAN(&graph, "weight", IGRAPH_EIT_GET(eit));
                SETEAN(&graph, "weight", IGRAPH_EIT_GET(eit), current_edge_weight - ((double)1/this->num_partitions));
            }
        }
    }
    igraph_eit_destroy(&eit);
    this->WriteToLogFile("Finsished subtracting from edge weights for the final graph" , 1);

    this->WriteToLogFile("Started removing edges from the final graph" , 1);
    Consensus::RemoveEdgesBasedOnThreshold(&graph, this->threshold);
    this->WriteToLogFile("Finished removing edges from the final graph" , 1);

    this->WriteToLogFile("Started the final clustering run" , 1);
    std::map<int, int> final_partition = Consensus::GetCommunities("", this->final_algorithm, 0, this->final_resolution, &graph);
    this->WriteToLogFile("Finished the final clustering run" , 1);
    igraph_destroy(&graph);

    this->WriteToLogFile("Started writing to the output clustering file" , 1);
    this->WritePartitionMap(final_partition);
    this->WriteToLogFile("Finished writing to the output clustering file" , 1);

    return 0;
}
