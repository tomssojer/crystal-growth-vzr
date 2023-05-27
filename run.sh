#!/bin/bash

set -e

if [ $# -eq 0 ]; then
gcc -o main main.c
./main > serial.txt
elif  [ $1 == "cuda" ]; then
nvcc -o main cuda/mainCuda.cu
./main > paralel_cuda.txt
fi


# gcc main.c -o main
# ./main

# nvcc cuda/mainCUDA.cu -O2 -o mainCUDA
# ./mainCUDA

FILE_NAME="serial_array.txt"

cat serial.txt 

python vizualizacija.py
