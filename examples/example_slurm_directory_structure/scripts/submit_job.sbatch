#!/bin/bash
#SBATCH --time=04:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=16
#SBATCH --job-name=MissionControl
#SBATCH --partition=secondary
#SBATCH --mem=64GB


/bin/bash ./scripts/batch_script.run
