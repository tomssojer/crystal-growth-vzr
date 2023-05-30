#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "../constants.h"
#include "modelCUDA.h"
#include "helper_cuda.h"
#define THREADS_PER_BLOCK 128

__device__ bool stopProcessing = false;

__global__ void testGPU()
{
    // printf("Hello world from the GPU!\n");
}

__global__ void stop_sim(Cell *d_cells)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;

    if (x < NUM_CELLS) // ne presežem limite slike
    {
        // Če je ena od sosed celice tipa edge, prekini simulacijo
        if (d_cells[x].type == 1)
        {

            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
                int sosed = d_cells[x].neighbors[k];
                if (sosed >= 0)
                {
                    if (d_cells[sosed].type == 3)
                    {
                        // return something  to driver function so it stops
                        stopProcessing = true;
                    }
                }
            }
        }
    }
}
__global__ void cell_type(Cell *d_cells, double *stateTemp)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;

    if (x < NUM_CELLS) // ne presežem limite slike
    {
        d_cells[x].state = stateTemp[x];
        if (d_cells[x].state >= 1)
        {
            d_cells[x].type = 0; // turns into ice cell
            for (int i = 0; i < NUM_NEIGHBORS; i++)
            {
                int sosed = d_cells[x].neighbors[i];
                // Preveri, da je valid sosed
                if (sosed >= 0)
                {
                    // Dodeli tip boundary le, če ni frozen ali edge
                    if (d_cells[sosed].type != 0 && d_cells[sosed].type != 3)
                    {
                        d_cells[sosed].type = 1;
                    }
                }
            }
        }
    }
}
__global__ void get_states(Cell *d_cells, double *stateTemp, int size)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;

    if (x < size && d_cells[x].type != 3) // ne presežem limite slike
    {

        double state = d_cells[x].state;
        double average = 0.0;
        int *neighbors = d_cells[x].neighbors;
        for (int i = 0; i < NUM_NEIGHBORS; i++)
        {

            int sosed = neighbors[i];
            // printf("sosed: %d \t ||",sosed);
            if (sosed >= 0)
            {
                // Če je type sosednje celice unreceptive ali edge, potem pridobi del od nje
                if (d_cells[sosed].type > 1)
                {
                    average += d_cells[sosed].state;
                    // printf("x %d je: %d => %f \n",x,sosed,d_cells[sosed].state);
                }
            }
        }

        average = average / NUM_NEIGHBORS;

        int type = d_cells[x].type;
        if (type < 2)
        {
            state = state + (ALPHA / 2) * average + GAMMA;
        }
        //  unreceptive, edge
        else
        {
            state = state + ALPHA / 2 * (average - state);
        }

        stateTemp[x] = state; // cells[x].state + double((ALPHA/2)) + GAMMA; //state;
    }
}
__global__ void get_states_cache(Cell *d_cells, double *stateTemp, int size)
{
     __shared__ Cell cache[7*THREADS_PER_BLOCK];
    //__shared__ Cell cache[THREADS_PER_BLOCK];
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    if (x < size)  {
        int threadId = threadIdx.x;
       
        cache[threadId] = d_cells[x];
        __syncthreads();

        if (cache[threadId].type != 3) // ne presežem limite slike
        {
            double state = cache[threadId].state;
            double average = 0.0;
            
            for (int i = 0; i < NUM_NEIGHBORS; i++)
            {
                int sosed = cache[threadId].neighbors[i];

                // printf("sosed: %d \t ||",sosed);
                if (sosed >= 0)
                {
                    cache[(i+1)*THREADS_PER_BLOCK + threadId] = d_cells[sosed];
                    // Če je type sosednje celice unreceptive ali edge, potem pridobi del od nje
                    if (cache[(i+1)*THREADS_PER_BLOCK+ threadId].type > 1)
                    {
                        average += cache[(i+1)*THREADS_PER_BLOCK + threadId].state;
                            //printf("avg %d \t",avrage);
                        // printf("x %d je: %d => %f \n",x,sosed,d_cells[sosed].state);
                    }
                }
            }
            average = average / NUM_NEIGHBORS;

            int type =   cache[threadId].type;
            if (type < 2)
            {
                state = state + (ALPHA / 2) * average + GAMMA;
            }
            //  unreceptive, edge
            else
            {
                state = state + ALPHA / 2 * (average - state);
            }

            stateTemp[x] = state; // cells[x].state + double((ALPHA/2)) + GAMMA; //state;
        }
     }
}

void parallel_cuda(Cell *d_cells, Cell *cells, int blockSize)
{
    double *d_stateTemp;
    checkCudaErrors(cudaMalloc((void **)&d_stateTemp, NUM_CELLS * sizeof(double)));
    int numBlocks = (NUM_CELLS + blockSize - 1) / blockSize;

    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {
        bool stopFlagValue;
        // update states of board
        get_states_cache<<<numBlocks, blockSize>>>(d_cells, d_stateTemp, NUM_CELLS);
        cudaDeviceSynchronize();
        cell_type<<<numBlocks, blockSize>>>(d_cells, d_stateTemp);
        cudaDeviceSynchronize();

        stop_sim<<<numBlocks, blockSize>>>(d_cells);
        cudaDeviceSynchronize();
        cudaMemcpyFromSymbol(&stopFlagValue, stopProcessing, sizeof(bool));

        if (stopFlagValue)
        {
            printf("breking %d\n", i);
            i = STEPS;
            break;
        }

        // if (i % STEPS_TO_DRAW == 0)
        // {
        //      checkCudaErrors(cudaMemcpy(cells, d_cells, NUM_CELLS * sizeof(Cell), cudaMemcpyDeviceToHost));
        //     // printf("Step number: %d\n", i);
        //     draw_board(cells);
        //     // write_to_file(cells, file);
        // }
        // printf("Step: %d ----------------------------------------------------------\n", i);
        // draw_board(cells);
    }
    checkCudaErrors(cudaMemcpy(cells, d_cells, NUM_CELLS * sizeof(Cell), cudaMemcpyDeviceToHost));
    getLastCudaError("printGPU() execution failed\n");
    cudaFree(d_stateTemp);
}
void check_CUDA() // function to copy into GPU memory
{
    int deviceCount;
    printf("Hello world from the CPU!\n");

    cudaGetDeviceCount(&deviceCount);

    if (deviceCount == 0)
    {
        printf("No CUDA devices found\n");
    }

    testGPU<<<1, 1>>>(); // gred size block size
    cudaDeviceSynchronize();
}
void run_CUDA(Cell *cells, int blocksize)
{
    // Allocate memory on GPU
    Cell *d_cells;
    checkCudaErrors(cudaMalloc((void **)&d_cells, NUM_CELLS * sizeof(Cell)));
    checkCudaErrors(cudaMemcpy(d_cells, cells, NUM_CELLS * sizeof(Cell), cudaMemcpyHostToDevice));
    cudaDeviceSynchronize();
    getLastCudaError("printGPU() execution failed\n");

    parallel_cuda(d_cells, cells, blocksize);
    // Free memory on GPU
    cudaFree(d_cells);
}

int main(int argc, char *argv[])
{

    // if (argc < 2)
    // {
    //     printf("Not enough arguments!\n");
    //     return 1;
    // }

    // int blocksize = atoi(argv[1]);

    // // ------------- Začetek inicializacije ------------- //
    // Definicija arraya s structi
    Cell *cells = (Cell *)malloc(NUM_CELLS * sizeof(*cells));

    // Dodaj sosede in indekse v struct
    init_grid(cells);

    // Določi začetno vrednost glede na tip celice
    init_state(cells);

    // ------------- Konec inicializacije ------------- //

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    // draw_board(cells);
    run_CUDA(cells, THREADS_PER_BLOCK);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    printf("Elapsed time: %0.3f seconds \n", milliseconds / 1000);

    //draw_board(cells);

    // Free allocated memory
    free(cells);

    return 0;
}