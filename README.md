# Consensus clustering

This respository includes Simple Consensus (SC) and Threshold Consensus (TC). The algorithms and the codes use many ideas from the Fast Consensus Clustering software available at https://github.com/kaiser-dan/fastconsensus and the paper [Fast consensus clustering in complex networks](https://arxiv.org/pdf/1902.04014.pdf), Phys. Rev. E., 2019.

## Environment setup
The project uses gcc/11.2.0 and cmake/3.26.3 for now. On a module based cluster system, one can simply do the following commands to get the right environment for building and running.
```
module load gcc/11.2.0
module load cmake/3.26.3
```

## How to build
A one time setup is needed with the [setup.sh](setup.sh) script. This script builds the igraph and libleidenalg libraries.
```
./setup.sh
```

Once the one time setup is complete, all one has to do is run the script provided in [easy_build_and_compile.sh](easy_build_and_compile.sh).
```
./easy_build_and_compile.sh
```

### How to run the subcommands
This repostiory contains both simple consensus and threshold consensus which are enabled by their subcommand flags as follows. Each subcommand is described in further detail in the following sections.
```
Usage: consensus-clustering [--help] [--version] {simple,threshold}

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits

Subcommands:
  simple        Simple consensus algorithm
  threshold     Threshold consensus algorithm (set threshold to 1 for strict consensus)
```

### Partition file example
Both simple and threshold consensus takes in a list of algorithms and their parameters in order to run the respective consensus algorithms. The following file format can be used for the `--partition-file` parameters. The first column is the algorithm, the second column is the weight, and the third column is the clustering parameter. This clustering parameter would be resolution value for leiden-cpm for example. This column is ignored for louvain and leiden-mod so using -1 suffices for these methods. The weight column is also ignored when doing simple consensus so any numeric value suffices there.
```
louvain 1 -1
louvain 1 -1
louvain 1 -1
louvain 1 -1
louvain 1 -1
leiden-cpm 1 0.2
leiden-cpm 1 0.2
leiden-cpm 1 0.2
leiden-cpm 1 0.2
leiden-cpm 1 0.2
```

### Threshold consensus
This implementation of the Threshold Consensus runs a clustering algorithm $n_p$ times with different random seeds in a single iteration and only keeps the edges that appear in at least $\tau$ proportion of the partitions. When $\tau=1$, this is equivalent to *strict* consensus.
```
Usage: consensus-clustering threshold [--help] [--version] --edgelist VAR [--threshold VAR] --partition-file VAR [--final-algorithm VAR] [--final-resolution VAR] [--partitions VAR] [--num-processors VAR] --output-file VAR --log-file VAR [--log-level VAR]

Threshold consensus algorithm (set threshold to 1 for strict consensus)

Optional arguments:
  -h, --help          shows help message and exits
  -v, --version       prints version information and exits
  --edgelist          Network edge-list file [required]
  --threshold         Threshold value [default: 1]
  --partition-file    Clustering partition file where the first column is one of (leiden-cpm, leiden-mod, louvain), second column is its weight, and the third column is the clustering method parameter (resolution value for leiden-cpm, ignored for leiden-mod and louvain. One can put -1 here in these cases). [required]
  --final-algorithm   Final clustering algorithm to be used (leiden-cpm, leiden-mod, louvain)
  --final-resolution  Resolution value for the final run. Only used if --final-algorithm is leiden-cpm [default: 0.01]
  --partitions        Number of partitions in consensus clustering [default: 10]
  --num-processors    Number of processors [default: 1]
  --output-file       Output clustering file [required]
  --log-file          Output log file [required]
  --log-level         Log level where 0 = silent, 1 = info, 2 = verbose [default: 1]
```


### Simple consensus
See description in the report. This does not yet support IKC and strict consensus in the final step. The best-scoring selection in the final step is also not included yet.
```
Usage: consensus-clustering simple [--help] [--version] --edgelist VAR [--threshold VAR] --partition-file VAR [--final-algorithm VAR] [--final-resolution VAR] [--delta VAR] [--max-iter VAR] [--partitions VAR] [--num-processors VAR] --output-file VAR --log-file VAR [--log-level VAR]

Simple consensus algorithm

Optional arguments:
  -h, --help          shows help message and exits
  -v, --version       prints version information and exits
  --edgelist          Network edge-list file [required]
  --threshold         Threshold value [default: 1]
  --partition-file    Clustering partition file where the first column is one of (leiden-cpm, leiden-mod, louvain), second column is its weight, and the third column is the clustering method parameter (resolution value for leiden-cpm, ignored for leiden-mod and louvain. One can put -1 here in these cases). [required]
  --final-algorithm   Final clustering algorithm to be used (leiden-cpm, leiden-mod, louvain)
  --final-resolution  Resolution value for the final run. Only used if --final-algorithm is leiden-cpm [default: 0.01]
  --delta             Convergence parameter [default: 0.02]
  --max-iter          Maximum number of iterations in simple consensus [default: 2]
  --partitions        Number of partitions in consensus clustering [default: 10]
  --num-processors    Number of processors [default: 1]
  --output-file       Output clustering file [required]
  --log-file          Output log file [required]
  --log-level         Log level where 0 = silent, 1 = info, 2 = verbose [default: 1]
```
