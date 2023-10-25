#ifndef SIMPLE_CONSENSUS_H
#define SIMPLE_CONSENSUS_H

#include "consensus.h"

class SimpleConsensus : public Consensus {
    public:
        SimpleConsensus(std::string edgelist, std::string partition_file, std::string final_algorithm, double threshold, double final_resolution, double delta, int max_iter, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : Consensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, num_partitions, num_processors, output_file, log_file, log_level), delta(delta), max_iter(max_iter)  {
        };
        int main();

        bool CheckConvergence(igraph_t* graph_ptr, double max_weight, int iter_count);

    private:
        double delta;
        int max_iter;
};
#endif
