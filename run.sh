#!/bin/bash

set -e

# if [ $# -eq 0 ]; then
# gcc -o main serijski/main.c
# ./main > serial.txt
# elif  [ $1 == "cuda" ]; then
# nvcc -o main cuda/mainCuda.cu
# ./main > paralel_cuda.txt
# fi


gcc serijski/main.c -o serial
srun --reservation=fri ./serial > serial.txt

module load CUDA/10.1.243-GCC-8.3.0
nvcc cuda/mainCUDA.cu -O2 -o ./mainCUDA
srun --reservation=fri -G1 -n1 ./mainCUDA > cuda.txt

# module load OpenMPI/4.1.0-GCC-10.2.0 
# mpicc openMPI/mainMPI.c -o mpi
# srun --reservation=fri --constraint=AMD  --mpi=pmix -n2 -N1 ./mpi
# srun --reservation=fri --constraint=AMD  --mpi=pmix -n2 -N1 ./mpi > mpi.txt

diff serial.txt cuda.txt
