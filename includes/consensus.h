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
#include <thread>
#include <omp.h>

#include <igraph/igraph.h>
#include <libleidenalg/GraphHelper.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>
#include <libleidenalg/ModularityVertexPartition.h>


class Consensus {
    public:
        Consensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : edgelist(edgelist), partition_file(partition_file), final_algorithm(final_algorithm), threshold(threshold), final_resolution(final_resolution), num_partitions(num_partitions), num_processors(num_processors), output_file(output_file), log_file(log_file), log_level(log_level) {
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
        virtual int main() = 0;
        int write_to_log_file(std::string message, int message_type);
        virtual ~Consensus() {
            if(this->log_level > 0) {
                this->log_file_handle.close();
            }
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
};

class ThresholdConsensus : public Consensus {
    public:
        ThresholdConsensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : Consensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, num_partitions, num_processors, output_file, log_file, log_level) {
        };
        int main();
};

class SimpleConsensus : public Consensus {
    public:
        SimpleConsensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, double delta, int max_iter, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : Consensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, num_partitions, num_processors, output_file, log_file, log_level), delta(delta), max_iter(max_iter)  {
        };
        bool check_convergence(igraph_t* graph_ptr, int iter_count);
        int main();
    private:
        double delta;
        int max_iter;
};
#endif
