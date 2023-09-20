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
        Consensus(std::string edgelist, std::string algorithm, double threshold, double resolution, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : edgelist(edgelist), algorithm(algorithm), threshold(threshold), resolution(resolution), num_partitions(num_partitions), num_processors(num_processors), output_file(output_file), log_file(log_file), log_level(log_level) {
            if(this->log_level > 0) {
                this->start_time = std::chrono::steady_clock::now();
                this->log_file_handle.open(this->log_file);
            }
        };
        int write_to_log_file(std::string message, int message_type);
        int main();
        ~Consensus() {
            if(this->log_level > 0) {
                this->log_file_handle.close();
            }
        }
    private:
        std::string edgelist;
        std::string algorithm;
        double threshold;
        double resolution;
        int num_partitions;
        int num_processors;
        std::string output_file;
        std::string log_file;
        std::chrono::steady_clock::time_point start_time;
        std::ofstream log_file_handle;
        int log_level;
};
#endif
