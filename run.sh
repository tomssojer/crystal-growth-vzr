#!/bin/bash

set -e

touch results.txt

# Serial algorithm
echo "Running serial algorithm" >> results.txt
gcc serijski/main.c -o run_serial
srun --reservation=fri ./run_serial >> results.txt

# openMP
echo "-----------------------------" >> results.txt
echo "Running openMP" >> results.txt

gcc -fopenmp -o run_mp OpenMp/mainMP.c -lm
for i in 1 2 4 8 16 32
do
    srun --reservation=fri --cpus-per-task=$i ./run_mp >> results.txt
done

# cuda
echo "-----------------------------" >> results.txt
echo "Running CUDA" >> results.txt

module load CUDA/10.1.243-GCC-8.3.0
nvcc cuda/mainCUDA.cu -O2 -o ./run_cuda
for i in 16 32 64 128 256 512 1024
do
    echo "CUDA - blocksize = $i" >> results.txt
    srun --reservation=fri -G1 -n1 ./run_cuda $i >> results.txt
done

# openMPI
echo "-----------------------------" >> results.txt
echo "Running MPI" >> results.txt

module load OpenMPI/4.1.0-GCC-10.2.0 
mpicc openMPI/mainMPI.c -o run_mpi

for i in 1 2; do
    for j in 1 4 16 32; do
        if [ $i != 2 ] || [ $j != 1 ]
        then
            srun --reservation=fri --constraint=AMD  --mpi=pmix -n$j -N$i ./run_mpi >> results.txt
        fi
    done
done
