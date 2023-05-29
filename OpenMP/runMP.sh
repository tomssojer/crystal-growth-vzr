#!/bin/bash
gcc -fopenmp -O3 -o mainMP mainMP.c --openmp -lm
# Define the number of cores to run the OpenMP program with
num_cores=(1 2 4 8)

# Loop over the number of cores
for cores in "${num_cores[@]}"
do
    echo "Running with $cores cores"
    export OMP_NUM_THREADS=$cores
    ./mainMP
    echo "Finished running with $cores cores"
    echo
done
