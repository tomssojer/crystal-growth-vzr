#!/bin/bash

set -e

# gcc serijski/main.c -o serial
# srun --reservation=fri ./serial > serial.txt

# module load CUDA/10.1.243-GCC-8.3.0
# nvcc cuda/mainCUDA.cu -O2 -o ./mainCUDA
# srun --reservation=fri -G1 -n1 ./mainCUDA

module load OpenMPI/4.1.0-GCC-10.2.0 
mpicc openMPI/mainMPI.c -o mpi
srun --reservation=fri --constraint=AMD  --mpi=pmix -n2 -N1 ./mpi

# diff serial.txt mpi.txt
