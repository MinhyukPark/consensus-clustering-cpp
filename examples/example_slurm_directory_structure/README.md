# Example SLURM based directory structure

This is a full example assuming that one is working on a module based SLURM cluster, with specific parameters for SBATCH set as the ones at the Illinois Campus Cluster.

An example directory structure is shown in this folder where the following command would work for running consensus clustering on an input edgelist. The only changes required are adding where the github repository was cloned and the location of the edgelist.
```
sbatch ./scripts/submit_job.sbatch
```

