#ifndef THRESHOLD_CONSENSUS_H
#define THRESHOLD_CONSENSUS_H

#include "consensus.h"

class ThresholdConsensus : public Consensus {
    public:
        ThresholdConsensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : Consensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, num_partitions, num_processors, output_file, log_file, log_level) {
        };
        int main();
};

#endif
