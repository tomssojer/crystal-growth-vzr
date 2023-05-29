#!/bin/bash

set -e

if [ $# -lt 1 ]
then
    echo 1>&2 "$0: file name not provided"
    exit 2
fi

FILE=$1

# Load modules
module load CUDA/10.1.243-GCC-8.3.0
module load OpenMPI/4.1.0-GCC-10.2.0

# Compile programs
gcc serijski/main.c -o run_serial
gcc -fopenmp -o run_mp OpenMP/mainMP.c -lm
nvcc cuda/mainCUDA.cu -O2 -o run_cuda
mpicc openMPI/mainMPI.c -o run_row_mpi
mpicc openMPI/columnMPI.c -o run_col_mpi

echo -e "Starting simulation...\n\n" > $FILE

# Serial algorithm
echo -e "Running serial algorithm\n" >> $FILE
srun --reservation=fri ./run_serial >> $FILE

# openMP
echo -e "\n-----------------------------\nRunning openMP\n" >> $FILE

for i in 1 2 4 8 16 32
do
    echo "openMP - cpus per task = $i" >> $FILE
    srun --reservation=fri --cpus-per-task=$i ./run_mp >> $FILE
done

# cuda
echo -e "\n-----------------------------\nRunning CUDA\n" >> $FILE

for i in 16 32 64 128 256 512 1024
do
    echo "CUDA - blocksize = $i" >> $FILE
    srun --reservation=fri -G1 -n1 ./run_cuda $i >> $FILE
done

# openMPI by rows
echo -e "\n-----------------------------\nRunning MPI by rows\n" >> $FILE 

for i in 1 2; do
    for j in 1 4 16 32; do
        if [ $i != 2 ] || [ $j != 1 ]
        then
            echo "openMPI by rows - number of nodes = $i; number of processes = $j" >> $FILE
            srun --reservation=fri --constraint=AMD  --mpi=pmix -n$j -N$i ./run_row_mpi >> $FILE
        fi
    done
done

# openMPI by colums
echo -e "\n-----------------------------\nRunning MPI by columns\n" >> $FILE

for i in 1 2; do
    for j in 1 4 16 32; do
        if [ $i != 2 ] || [ $j != 1 ]
        then
            echo "openMPI by columns - number of nodes = $i; number of processes = $j" >> $FILE
            srun --reservation=fri --constraint=AMD  --mpi=pmix -n$j -N$i ./run_col_mpi >> $FILE
        fi
    done
done
