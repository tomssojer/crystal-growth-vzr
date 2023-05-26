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

// compile
// nvcc mainCUDA.cu -O2  -o mainCUDA

// 1. Začni z eno frozen celico, okoli nje so boundary
// 2. Za vse celice, ki so boundary in unreceptive poteka difuzija
// 3. Za vse celice, ki so frozen in boundary poteka konvekcija
// 3.a Upoštevaj, da le sosede, ki so edge ali unreceptive sharajo vodo
// 4. Preveri, če ima celica state >= 1 -> nastavi na frozen, njene sosede na boundary
// 5. Preveri, če je boundary celica soseda z edge celico, prekini simulacijo
__global__ void testGPU()
{
    printf("Hello world from the GPU!\n");
}

__global__ void get_states(Cell *d_cells,Cell *h_cells,double *stateTemp, int size)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;

    if (x < size && d_cells[x].type!=3) // ne presežem limite slike
    {

        double state = d_cells[x].state;
        double average = 0.0;
        int* neighbors = d_cells[x].neighbors;
        for (int i = 0; i < 6; i++)
        {   
            
            int sosed= neighbors[i];
            //printf("sosed: %d \t ||",sosed);
            if (sosed >= 0)
            {
                // Če je type sosednje celice unreceptive ali edge, potem pridobi del od nje
                if (d_cells[sosed].type > 1)
                {
                    average += d_cells[sosed].state;
                    //printf("x %d je: %d => %f \n",x,sosed,d_cells[sosed].state);
                }
            }
        }

        average = average/ 6;

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
     
        
        stateTemp[x] =  state;  // cells[x].state + double((ALPHA/2)) + GAMMA; //state;
    }
}


void parallel_cuda(Cell *d_cells,Cell *cells)
{
    //printf("x: delaaaa %d\t",NUM_CELLS);
    double *stateT= (double*)calloc(NUM_CELLS, sizeof(double));
       
    double *d_stateTemp;
    checkCudaErrors(cudaMalloc((void **)&d_stateTemp, NUM_CELLS * sizeof(double)));
    //cudaMemset(d_stateTemp, 0, NUM_CELLS * sizeof(double));
    //init d_stateTemp on GPU
    
    int blockSize = 128;
    int numBlocks = (NUM_CELLS + blockSize - 1) / blockSize;
    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {
        //cudaMemcpy(d_cells, cells, NUM_CELLS * sizeof(Cell), cudaMemcpyHostToDevice);
        //cudaDeviceSynchronize();
        // update states of board
        get_states<<<numBlocks, blockSize>>>(d_cells,cells, d_stateTemp, NUM_CELLS);
        cudaDeviceSynchronize(); 
        checkCudaErrors(cudaMemcpy(stateT, d_stateTemp, NUM_CELLS * sizeof(double), cudaMemcpyDeviceToHost));

        //Cell *cells2 = (Cell *)malloc(NUM_CELLS * sizeof(*cells2));
        checkCudaErrors(cudaMemcpy(cells, d_cells, NUM_CELLS * sizeof(Cell), cudaMemcpyDeviceToHost));
        cudaDeviceSynchronize();
        //printf("tu\n");
        // for (int k = 0; k < NUM_CELLS; k++)
        // {
        //     //stateT[k]=0;
        //   printf("average: %lf\n",stateT[k]);
        // }
        // for(int i=0; i< NUM_CELLS;i++ ) {
        //     for(int j=0;j<6;j++) {
        //         printf("%d\n",cells2[i].neighbors[j]);
        //     }
        // }
        //free(cells2);
            
        // }
        for (int j = 0; j < NUM_CELLS; j++) // sedaj posodobi tipe celic
        {
            cells[j].state = stateT[j];
            if (cells[j].state >= 1)
            {
                cells[j].type = 0; // turns into ice cell
                set_type_boundary(cells, cells[j].neighbors);
            }
        }

        // Če je ena od sosed celice tipa edge, prekini simulacijo
        for (int j = 0; j < NUM_CELLS; j++)
        {
            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
                if (cells[j].type == 1 && cells[j].neighbors[k] == 3)
                {
                    printf("break %d\n", i);
                    i = STEPS;
                    j = NUM_CELLS;
                    break;
                }
            }
        }

        // printf("Step: %d ----------------------------------------------------------\n", i);
        //draw_board(cells);
        checkCudaErrors(cudaMemcpy(d_cells, cells, NUM_CELLS * sizeof(Cell), cudaMemcpyHostToDevice));
        
    }
    getLastCudaError("printGPU() execution failed\n");
    free(stateT);
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
void run_CUDA(Cell *cells)
{
    // Allocate memory on GPU
    Cell *d_cells;
    checkCudaErrors(cudaMalloc((void **)&d_cells, NUM_CELLS * sizeof(Cell)));
    checkCudaErrors(cudaMemcpy(d_cells, cells, NUM_CELLS * sizeof(Cell), cudaMemcpyHostToDevice));

    cudaDeviceSynchronize();

 
     getLastCudaError("printGPU() execution failed\n");
    // for(int i=0;i<NUM_CELLS;i++ ) {
    //     checkCudaErrors(cudaMemcpy(d_cells[i].neighbors, cells[i].neighbors, 6 * sizeof(int), cudaMemcpyHostToDevice));
    // }
    //printf("se dela");
    // Run kernel
    parallel_cuda(d_cells,cells);
    printf(" se dela 2\n");
    // Copy data from GPU to CPU
       //cudaDeviceSynchronize();
   //Cell *cells2 = (Cell *)malloc(2*NUM_CELLS * sizeof(*cells));

    //checkCudaErrors(cudaMemcpy(cells, d_cells, NUM_CELLS * sizeof(Cell), cudaMemcpyDeviceToHost));

    // Free memory on GPU
    cudaFree(d_cells);
  
}

int main(int argc, char *argv[])
{
    // // ------------- Začetek inicializacije ------------- //
    // printHexagon(ROWS); //

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

    draw_board(cells);

    //check_CUDA();
    printf("Starting simulation...\n");
    run_CUDA(cells);

    // serial(cells);
    draw_board(cells);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    printf("Elapsed time: %0.3f milliseconds \n", milliseconds);

    // Free allocated memory
    // for (int i = 0; i < NUM_CELLS; i++)
    // {
    //     free(cells[i].neighbors);
    // }
    free(cells);

    return 0;
}