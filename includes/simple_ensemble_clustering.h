#ifndef SIMPLE_ENSEMBLE_CLUSTERING_H
#define SIMPLE_ENSEMBLE_CLUSTERING_H

#include "consensus.h"

class SimpleEnsembleClustering : public Consensus {
    public:
        SimpleEnsembleClustering(std::string edgelist, float threshold, std::vector<std::string> clustering_files, std::vector<std::string> clustering_weights, std::string output_file, std::string log_file, int log_level) : Consensus(edgelist, threshold, output_file, log_file, log_level), clustering_files(clustering_files), clustering_weights(clustering_weights) {
        };
        int main();
    private:
        std::vector<std::string> clustering_files;
        std::vector<std::string> clustering_weights;
};

#endif
