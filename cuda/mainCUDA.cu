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

int THREADS_PER_BLOCK;
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

__global__ void cell_type_cache(Cell *d_cells, double *stateTemp)
{
    __shared__ Cell cache[16];
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int threadId = threadIdx.x;

    if (x < NUM_CELLS) // ne presežem limite slike
    {
        d_cells[x].state = stateTemp[x];
        if (d_cells[x].state >= 1)
        {
            d_cells[x].type = 0;

            // cache cells after modifying them
            cache[threadId] = d_cells[x];
            __syncthreads();

            for (int i = 0; i < NUM_NEIGHBORS; i++)
            {
                // ko delamo bralne dostope cachiraj
                int sosed = cache[threadId].neighbors[i];
                //  Preveri, da je valid sosed
                if (sosed >= 0)
                {
                    // printf("vel %d \t",blockIdx.x * blockDim.x);
                    if (sosed >= blockIdx.x * blockDim.x && sosed < (blockIdx.x + 1) * blockDim.x)
                    {
                        if (x - sosed == 1)
                        {
                            if (cache[threadId - 1].type == 2)
                                d_cells[sosed].type = 1;
                        }
                        else if (sosed - x == 1)
                        {
                            if (cache[threadId + 1].type == 2)
                                d_cells[sosed].type = 1;
                        }
                        else if (d_cells[sosed].type == 2)
                        {
                            d_cells[sosed].type = 1;
                        }
                    }
                    else if (d_cells[sosed].type == 2)
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
            if (sosed >= 0)
            {
                // Če je type sosednje celice unreceptive ali edge, potem pridobi del od nje
                if (d_cells[sosed].type > 1)
                {
                    average += d_cells[sosed].state;
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
    extern __shared__ Cell cache[];
    int x = threadIdx.x + blockIdx.x * blockDim.x;

    if (x < size)
    {
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
                Cell neighbor_cell;

                if (sosed >= 0)
                {
                    if (sosed >= blockIdx.x * blockDim.x && sosed < (blockIdx.x + 1) * blockDim.x)
                    {
                        if (x - sosed == 1)
                            neighbor_cell = cache[threadId - 1];
                        else if (sosed - x == 1)
                            neighbor_cell = cache[threadId + 1];
                        else
                            neighbor_cell = d_cells[sosed];
                    }
                    else
                        neighbor_cell = d_cells[sosed];

                    // Če je type sosednje celice unreceptive ali edge, potem pridobi del od nje
                    if (neighbor_cell.type > 1)
                        average += neighbor_cell.state;
                }
            }

            average = average / NUM_NEIGHBORS;

            int type = cache[threadId].type;
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

void parallel_cuda(Cell *d_cells, Cell *cells, int blocksize, int cache_flag)
{
    double *d_stateTemp;
    checkCudaErrors(cudaMalloc((void **)&d_stateTemp, NUM_CELLS * sizeof(double)));
    int numBlocks = (NUM_CELLS + blocksize - 1) / blocksize;

    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {
        bool stopFlagValue;
        // update states of board
        if (cache_flag)
            get_states_cache<<<numBlocks, blocksize, THREADS_PER_BLOCK>>>(d_cells, d_stateTemp, NUM_CELLS);
        else
            get_states<<<numBlocks, blocksize>>>(d_cells, d_stateTemp, NUM_CELLS);

        cudaDeviceSynchronize();
        cell_type<<<numBlocks, blocksize>>>(d_cells, d_stateTemp);
        cudaDeviceSynchronize();

        stop_sim<<<numBlocks, blocksize>>>(d_cells);
        cudaDeviceSynchronize();
        cudaMemcpyFromSymbol(&stopFlagValue, stopProcessing, sizeof(bool));

        if (stopFlagValue)
        {
            printf("Breaking %d\n", i);
            i = STEPS;
            break;
        }
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
void run_CUDA(Cell *cells, int blocksize, int cache_flag)
{
    // Allocate memory on GPU
    Cell *d_cells;
    checkCudaErrors(cudaMalloc((void **)&d_cells, NUM_CELLS * sizeof(Cell)));
    checkCudaErrors(cudaMemcpy(d_cells, cells, NUM_CELLS * sizeof(Cell), cudaMemcpyHostToDevice));
    cudaDeviceSynchronize();
    getLastCudaError("printGPU() execution failed\n");

    parallel_cuda(d_cells, cells, blocksize, cache_flag);
    // Free memory on GPU
    cudaFree(d_cells);
}
void get_cacheSize(int size)
{
    int deviceId;
    cudaGetDevice(&deviceId);

    cudaDeviceProp deviceProps;
    cudaGetDeviceProperties(&deviceProps, deviceId);

    int sharedMemorySize = deviceProps.sharedMemPerBlock;

    printf("Max shared memory size per block: %d bytes\n", sharedMemorySize / size);
}
int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("Not enough arguments!\n");
        return 1;
    }

    int blocksize = atoi(argv[1]);
    int cache_flag = atoi(argv[2]);

    THREADS_PER_BLOCK = blocksize;

    // // ------------- Začetek inicializacije ------------- //
    // Definicija arraya s structi
    Cell *cells = (Cell *)malloc(NUM_CELLS * sizeof(*cells));
    // get_cacheSize(sizeof(Cell));
    init_grid(cells);
    // Določi začetno vrednost glede na tip celice
    init_state(cells);

    // ------------- Konec inicializacije ------------- //

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    // draw_board(cells);
    run_CUDA(cells, blocksize, cache_flag);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    printf("Elapsed time: %0.3f seconds \n", milliseconds / 1000);

    // draw_board(cells);

    // Free allocated memory
    free(cells);

    return 0;
}