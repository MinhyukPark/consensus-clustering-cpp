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


### Threshold consensus
This implementation of the Threshold Consensus runs a clustering algorithm $n_p$ times with different random seeds in a single iteration and only keeps the edges that appear in at least $\tau$ proportion of the partitions. When $\tau=1$, this is equivalent to *strict* consensus.
```
Usage: consensus-clustering [--help] [--version] --edgelist VAR [--threshold VAR] [--algorithm VAR] [--resolution VAR] [--partitions VAR] [--num-processors VAR] --output-file VAR --log-file VAR [--log-level VAR]

Optional arguments:
  -h, --help        shows help message and exits
  -v, --version     prints version information and exits
  --edgelist        Network edge-list file [required]
  --threshold       Threshold value [default: 1]
  --algorithm       Clustering algorithm (leiden-cpm, leiden-mod, louvain)
  --resolution      Resolution value for leiden-cpm [default: 0.01]
  --partitions      Number of partitions in consensus clustering [default: 10]
  --num-processors  Number of processors [default: 1]
  --output-file     Output clustering file [required]
  --log-file        Output log file [required]
  --log-level       Log level where 0 = silent, 1 = info, 2 = verbose [default: 1]
```

---
# BELOW IS NOT MODIFIED YET ONLY THRESHOLD CONSENSUS HAS BEEN IMPLEMENTED

### Simple consensus
See description in the report. This does not yet support IKC and strict consensus in the final step and the algorithm list should be set manually in the code. The best-scoring selection in the final step is also not included yet, and the first algorithm in the list will be used to create the final partition.
```
$ python3 simple_consensus.py -n <edge-list> -t <threshold> -r <resolution-value> -p <number-partitions>
```
**Arguments**
```
 -n,  --edgelist           input network edge-list
 -t,  --threshold          threshold value
 -a,  --algorithm          clustering algorithm (leiden-cpm, leiden-mod, louvain)
 -r,  --resolution         resolution value for leiden-cpm
 -p,  --partitions         number of partitions used in consensus clustering
 -d,  --delta              convergence parameter
 -p,  --maxiter            maximum number of iterations
```
The script `evaluate_partitions.py` can be used to evaluate the output partition in terms of cluster statistics and modularity:
```
$ python3 evaluate_partitions.py -n <edge-list> -m <partition-membership>
```

