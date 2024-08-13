#ifndef CONSENSUS_H
#define CONSENSUS_H
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <omp.h>

#include <igraph/igraph.h>
#include <libleidenalg/GraphHelper.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>
#include <libleidenalg/ModularityVertexPartition.h>


class Consensus {
    public:
        Consensus(std::string edgelist, double threshold, std::string output_file, std::string log_file, int log_level) : edgelist(edgelist), threshold(threshold), output_file(output_file), log_file(log_file), log_level(log_level), num_calls_to_log_write(0) {
            // constructor for strict consensus
            if(this->log_level > 0) {
                this->start_time = std::chrono::steady_clock::now();
                this->log_file_handle.open(this->log_file);
            }
        };
        Consensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : edgelist(edgelist), partition_file(partition_file), final_algorithm(final_algorithm), threshold(threshold), final_resolution(final_resolution), num_partitions(num_partitions), num_processors(num_processors), output_file(output_file), log_file(log_file), log_level(log_level), num_calls_to_log_write(0) {
            if(this->log_level > 0) {
                this->start_time = std::chrono::steady_clock::now();
                this->log_file_handle.open(this->log_file);
            }

            std::ifstream partition_file_handle(this->partition_file);
            std::string current_algorithm;
            double current_weight;
            double current_clustering_parameter;
            while(partition_file_handle >> current_algorithm >> current_weight >> current_clustering_parameter) {
                this->algorithm_vector.push_back(current_algorithm);
                this->weight_vector.push_back(current_weight);
                this->clustering_parameter_vector.push_back(current_clustering_parameter);
            }
            partition_file_handle.close();

            if(this->num_partitions != this->algorithm_vector.size()) {
                throw std::invalid_argument("--partition does not equal the number of algorithms provided is the algorithm file");
            }
        };
        Consensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level, bool voting_flag) : edgelist(edgelist), partition_file(partition_file), final_algorithm(final_algorithm), threshold(threshold), final_resolution(final_resolution), num_partitions(num_partitions), num_processors(num_processors), output_file(output_file), log_file(log_file), log_level(log_level), num_calls_to_log_write(0) {
            // ensemble consensus
            if(this->log_level > 0) {
                this->start_time = std::chrono::steady_clock::now();
                this->log_file_handle.open(this->log_file);
            }

            std::ifstream partition_file_handle(this->partition_file);
            std::string current_algorithm;
            double current_weight;
            double current_clustering_parameter;
            while(partition_file_handle >> current_algorithm >> current_weight >> current_clustering_parameter) {
                this->algorithm_vector.push_back(current_algorithm);
                this->weight_vector.push_back(current_weight);
                this->clustering_parameter_vector.push_back(current_clustering_parameter);
            }
            partition_file_handle.close();

            if(this->num_partitions != this->algorithm_vector.size()) {
                throw std::invalid_argument("--partition does not equal the number of algorithms provided is the algorithm file");
            }
            this->voting_flag = voting_flag;
        };
        virtual int main() = 0;
        int WriteToLogFile(std::string message, int message_type);
        void WritePartitionMap(std::map<int, int>& final_partition);
        void WritePartitionMapWithTranslation(std::map<int, int>& final_partition, igraph_t* graph_ptr);
        void LoadIgraphFromFile(igraph_t* graph_ptr);
        void StartWorkers(igraph_t* graph);
        virtual ~Consensus() {
            if(this->log_level > 0) {
                this->log_file_handle.close();
            }
        }

        static inline std::map<int, int> GetConnectedComponents(igraph_t* graph_ptr) {
            std::map<int, int> partition;
            igraph_vector_int_t component_id_vector;
            igraph_vector_int_init(&component_id_vector, 0);
            igraph_vector_int_t membership_size_vector;
            igraph_vector_int_init(&membership_size_vector, 0);
            igraph_integer_t number_of_components;
            igraph_connected_components(graph_ptr, &component_id_vector, &membership_size_vector, &number_of_components, IGRAPH_WEAK);

            igraph_eit_t eit;
            igraph_eit_create(graph_ptr, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
            for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
                igraph_integer_t current_edge = IGRAPH_EIT_GET(eit);
                int from_node = IGRAPH_FROM(graph_ptr, current_edge);
                if(!partition.contains(from_node)) {
                    int from_component_id = VECTOR(component_id_vector)[from_node];
                    if(VECTOR(membership_size_vector)[from_component_id] > 1) {
                        partition[from_node] = from_component_id;
                    }
                }
                int to_node = IGRAPH_TO(graph_ptr, current_edge);
                if(!partition.contains(to_node)) {
                    int from_component_id = VECTOR(component_id_vector)[to_node];
                    if(VECTOR(membership_size_vector)[from_component_id] > 1) {
                        partition[to_node] = from_component_id;
                    }
                }

            }
            igraph_eit_destroy(&eit);
            igraph_vector_int_destroy(&component_id_vector);
            igraph_vector_int_destroy(&membership_size_vector);
            return partition;
        }


        static inline void RemoveEdgesBasedOnThreshold(igraph_t* graph, double current_threshold) {
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

        static inline void RunLouvainAndUpdatePartition(std::map<int, int>& partition_map, int seed, double resolution_value, igraph_t* graph) {
            igraph_vector_int_t membership;
            igraph_vector_int_init(&membership, 0);
            igraph_rng_seed(igraph_rng_default(), seed);
            igraph_community_multilevel(graph, 0, resolution_value, &membership, 0, 0);

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

        static inline void RunLeidenAndUpdatePartition(std::map<int, int>& partition_map, MutableVertexPartition* partition, int seed, igraph_t* graph, int num_iter= 2) {
            Optimiser o;
            o.set_rng_seed(seed);
            for(int i = 0; i < num_iter; i ++) {
                o.optimise_partition(partition);
            }
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

        static inline void SetIgraphAllEdgesWeight(igraph_t* graph, double weight) {
            igraph_eit_t eit;
            igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
            std::set<std::pair<int, int>> edge_set;
            for(; !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit)) {
                int current_edge = IGRAPH_EIT_GET(eit);
                SETEAN(graph, "weight", IGRAPH_EIT_GET(eit), weight);
            }
            igraph_eit_destroy(&eit);
        }

        static inline void ClusterWorker(std::string edgelist, std::vector<std::string>& algorithm_vector, std::vector<double>& clustering_parameter_vector, igraph_t* graph_ptr) {
            while(true) {
                std::unique_lock<std::mutex> num_partition_lock{Consensus::num_partition_index_mutex};
                int current_index = Consensus::num_partition_index_queue.front();
                Consensus::num_partition_index_queue.pop();
                num_partition_lock.unlock();
                if(current_index == -1) {
                    // done with work
                    return;
                }
                std::map<int, int> clustering = Consensus::GetCommunities(edgelist, algorithm_vector[current_index], current_index, clustering_parameter_vector[current_index], graph_ptr);
                std::map<int, std::vector<int>> cluster_to_nodes_map;
                if(Consensus::voting_flag) cluster_to_nodes_map = Consensus::GetClusterToNodeMap(clustering);
                {
                    std::lock_guard<std::mutex> done_being_clustered_guard(Consensus::done_being_clustered_mutex);
                    Consensus::done_being_clustered_clusterings.push(clustering);
                    if(Consensus::voting_flag) Consensus::done_being_clustered_cluster_to_nodes_map.push(cluster_to_nodes_map);
                }
            }
        }

        static inline std::map<int, std::vector<int>> GetClusterToNodeMap(std::map<int, int> clustering) {
            std::map<int, std::vector<int>> cluster_to_nodes_map;

            for(auto const& [node_id, cluster_id] : clustering) {
                cluster_to_nodes_map[cluster_id].push_back(node_id);
            }
            return cluster_to_nodes_map;
        }

        static inline std::map<int, int> GetCommunities(std::string edgelist, std::string algorithm, int seed, double clustering_parameter, igraph_t* graph_ptr) {
            // TO DO -- add system call to run CM if it is appended to the algorithm i.e. "leiden-cpm-cm"
            //bool cm_flag = algorithm.compare(algorithm.size()-3, 3, "-cm");
            //if(cm_flag) algorithm.erase(algorithm.size()-3, algorithm.size());

            std::map<int, int> partition_map;
            igraph_t graph;
            if(graph_ptr == nullptr) {
                FILE* edgelist_file = fopen(edgelist.c_str(), "r");
                igraph_set_attribute_table(&igraph_cattribute_table);
                igraph_read_graph_edgelist(&graph, edgelist_file, 0, false);
                fclose(edgelist_file);
                SetIgraphAllEdgesWeight(&graph, 1);

            } else {
                graph = *graph_ptr;
            }

            if(algorithm == "louvain") {
                RunLouvainAndUpdatePartition(partition_map, seed, clustering_parameter, &graph);
            } else if(algorithm == "leiden-cpm") {
                Graph leiden_graph(&graph);
                CPMVertexPartition partition(&leiden_graph, clustering_parameter);
                RunLeidenAndUpdatePartition(partition_map, &partition, seed, &graph);
            } else if(algorithm == "leiden-mod") {
                Graph leiden_graph(&graph);
                ModularityVertexPartition partition(&leiden_graph);
                RunLeidenAndUpdatePartition(partition_map, &partition, seed, &graph);
            } else {
                throw std::invalid_argument("GetCommunities(): Unsupported algorithm");
            }

            //if(cm_flag) {
            //    //system call to CM
            //}

            if(graph_ptr == nullptr) {
                igraph_destroy(&graph);
            }
            return partition_map;
        }

        static inline void SetIgraphEdgeWeightFromVertices(igraph_t* graph, int from_node, int to_node, double edge_weight) {
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

        static inline std::map<std::string, std::string> ReadClusteringFile(std::string clustering_file) {
            std::map<std::string, std::string> input_clustering;
            std::map<std::string, std::vector<std::string>> cluster_id_to_membership_map;
            std::string node_id;
            std::string cluster_id;
            std::ifstream clustering_file_handle(clustering_file);
            while(clustering_file_handle >> node_id >> cluster_id) {
                /* input_clustering[node_id] = cluster_id; */
                cluster_id_to_membership_map[cluster_id].push_back(node_id);
            }

            for(auto const& [cluster_id, membership_vector] : cluster_id_to_membership_map) {
                if(membership_vector.size() > 1) {
                    for(int i = 0; i < membership_vector.size(); i ++) {
                        input_clustering[membership_vector[i]] = cluster_id;
                    }
                }
            }

            return input_clustering;
        }

    protected:
        std::string edgelist;
        std::string partition_file;
        std::string final_algorithm;
        double threshold;
        double final_resolution;
        int num_partitions;
        int num_processors;
        std::string output_file;
        std::string log_file;
        std::chrono::steady_clock::time_point start_time;
        std::ofstream log_file_handle;
        int log_level;
        std::vector<std::string> algorithm_vector;
        std::vector<double> weight_vector;
        std::vector<double> clustering_parameter_vector;
        int num_calls_to_log_write;
        static inline bool voting_flag = false;
        static inline std::mutex num_partition_index_mutex;
        static inline std::queue<int> num_partition_index_queue;
        static inline std::mutex done_being_clustered_mutex;
        static inline std::queue<std::map<int, int>> done_being_clustered_clusterings;
        static inline std::queue<std::map<int, std::vector<int>>> done_being_clustered_cluster_to_nodes_map;
};

#endif
