#ifndef ENSEMBLE_CONSENSUS_H
#define ENSEMBLE_CONSENSUS_H

#include "consensus.h"

class EnsembleConsensus : public Consensus {
    public:
        EnsembleConsensus(std::string edgelist,
                          std::string partition_file,
                          std::string final_algorithm,
                          double threshold,
                          double final_resolution,
                          double delta,
                          int max_iter,
                          int num_partitions,
                          int num_processors,
                          std::string output_file,
                          std::string log_file,
                          int log_level, 
                          bool voting_flag,
                          bool delta_convergence_flag, 
                          bool final_clustering_flag) : 
                          Consensus(edgelist, partition_file, final_algorithm, threshold, final_resolution, num_partitions, num_processors, output_file, log_file, log_level, voting_flag), 
                          delta(delta), 
                          max_iter(max_iter), 
                          delta_convergence_flag(delta_convergence_flag), 
                          final_clustering_flag(final_clustering_flag) {
        };
        int main();

        bool CheckConvergence(igraph_t* lhs_graph_ptr, igraph_t* rhs_graph_ptr, double max_weight, int iter_count);
        bool CheckConvergence(igraph_t* graph_ptr, double max_weight, int iter_count);
        bool CheckConvergence(igraph_t* lhs_graph_ptr, igraph_t* rhs_graph_ptr, int iter_count);

    private:
        double delta;
        int max_iter;
        bool delta_convergence_flag; // N for when the graph is the same between iterations Y for when 100-delta percent of edges have coverged to 0 or 1.
        bool final_clustering_flag; // Y for connected components N for using last method and parameters in p
};
#endif
