#include "consensus.h"

/*
 * message type here is 1 for INFO, 2 for DEBUG, and -1 for ERROR
 */

void Consensus::LoadIgraphFromFile(igraph_t* graph_ptr) {
    FILE* edgelist_file = fopen(this->edgelist.c_str(), "r");
    igraph_read_graph_edgelist(graph_ptr, edgelist_file, 0, false);
    fclose(edgelist_file);
    bool remove_parallel_edges = true;
    bool remove_self_loops = true;
    igraph_simplify(graph_ptr, remove_parallel_edges, remove_self_loops, NULL);
}

void Consensus::StartWorkers(igraph_t* graph_ptr) {
    for(int i = 0; i < this->num_partitions; i ++) {
        Consensus::num_partition_index_queue.push(i);
    }
    for(int i = 0; i < this->num_processors; i ++) {
        Consensus::num_partition_index_queue.push(-1);
    }

    std::vector<std::thread> thread_vector;
    for(int i = 0; i < this->num_processors; i ++) {
        thread_vector.push_back(std::thread(Consensus::ClusterWorker, this->edgelist, std::ref(this->algorithm_vector), std::ref(this->clustering_parameter_vector), graph_ptr));
    }

    for(int i = 0; i < this->num_processors; i ++) {
        thread_vector[i].join();
    }
}
int Consensus::WriteToLogFile(std::string message, int message_type) {
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
        if(this->num_calls_to_log_write % 10 == 0) {
            std::flush(this->log_file_handle);
        }
        this->num_calls_to_log_write ++;
    }
    return 0;
}

void Consensus::WritePartitionMap(std::map<int, int>& final_partition) {
    std::ofstream clustering_output(this->output_file);
    for(auto const& [node_id, cluster_id]: final_partition) {
        clustering_output << node_id << " " << cluster_id << '\n';
    }
    clustering_output.close();
}

void Consensus::WritePartitionMapWithTranslation(std::map<int, int>& final_partition, igraph_t* graph_ptr) {
    std::ofstream clustering_output(this->output_file);
    for(auto const& [node_id, cluster_id]: final_partition) {
        clustering_output << VAS(graph_ptr, "name", node_id) << " " << cluster_id << '\n';
    }
    clustering_output.close();
}
