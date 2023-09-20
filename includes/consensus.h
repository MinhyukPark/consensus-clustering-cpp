#ifndef CONSENSUS_H
#define CONSENSUS_H
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <igraph/igraph.h>
#include <libleidenalg/GraphHelper.h>
#include <libleidenalg/Optimiser.h>
#include <libleidenalg/CPMVertexPartition.h>
#include <libleidenalg/ModularityVertexPartition.h>

class Consensus {
    public:
        Consensus(std::string edgelist, std::string algorithm, double threshold, double resolution, int num_partitions, int num_processors) : edgelist(edgelist), algorithm(algorithm), threshold(threshold), resolution(resolution), num_partitions(num_partitions), num_processors(num_processors) {
        };
        int main();
    private:
        std::string edgelist;
        std::string algorithm;
        double threshold;
        double resolution;
        int num_partitions;
        int num_processors;
};
#endif
