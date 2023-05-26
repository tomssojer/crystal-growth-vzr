#!/bin/bash

set -e

if [ $# -eq 0 ]; then
gcc -o main main.c
echo "No argument provided"

elif  [ $1 == "cuda" ]; then
nvcc -o main cuda/mainCuda.cu
echo "Argument is $1"
fi

./main
