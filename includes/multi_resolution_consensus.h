#ifndef MULTI_RESOLUTION_CONSENSUS_H
#define MULTI_RESOLUTION_CONSENSUS_H

#include "consensus.h"

class MultiResolutionConsensus : public Consensus {
    public:
        MultiResolutionConsensus(std::string edgelist, std::string partition_file, double threshold, double delta, int max_iter, int num_partitions, int num_processors, std::string output_file, std::string log_file, int log_level) : Consensus(edgelist, partition_file, "", threshold, -1, num_partitions, num_processors, output_file, log_file, log_level), delta(delta), max_iter(max_iter)  {
        };
        int main();

        bool CheckConvergence(igraph_t* lhs_graph_ptr, igraph_t* rhs_graph_ptr, int iter_count);

    private:
        double delta;
        int max_iter;
};
#endif
