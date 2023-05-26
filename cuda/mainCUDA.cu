#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include "constants.h"
#include "modelCUDA.h"

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

__global__ void get_states(Cell *cells, double *stateTemp, int size)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;

    if (x < size && cells[x].type != 3) // ne presežem limite slike
    {
        // average_state(cells[j].neighbors, cells);
    }
}

void printTab(int **tab, int j, int mappIdx)
{
    for (int i = 0; i < 6; i++)
    {
        printf("%d->%d |(%2d,%2d) ", j, mappIdx, tab[i][0], tab[i][1]); // x y
    }
}

void printmapped(int **tab, int j, int x, int *mapp)
{
    for (int i = 0; i < 6; i++)
    {
        printf("[%d,%d]  (%2d,%2d)->%d |", j, x, tab[i][0], tab[i][1], mapp[i]); // x y
    }
}

void printStructs(Cell *cells)
{
    for (int i = 0; i < ROWS * COLUMNS; i++)
    {
        printf("id:%d,\ttype: %d,\tstate: %lf,\tneighbors: ", cells[i].id, cells[i].type, cells[i].state);

        for (int j = 0; j < NUM_NEIGHBORS; j++)
            printf("%d, ", cells[i].neighbors[j]);

        printf("\n");
    }
}

void parallel_cuda(Cell *cells)
{
    float average = 0;
    double *g_stateTemp;
    cudaMalloc((void **)&g_stateTemp, NUM_CELLS * sizeof(double));
    // init g_stateTemp on GPU

    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {
        for (int j = 0; j < NUM_CELLS; j++) // posodobi vsa stanja - difuzija, konvekcija
        {
            // We deal with one cell at the time, do not deal with edge type
            if (cells[j].type != 3)
            {
                // Calculate average state of neighbors, needs current cell's neighbours and pointer to all cells

                // average = average_state(cells[j].neighbors, cells);

                // stateTemp[j] = change_state(cells[j].type, cells[j].state, average);
                //  cells[j].state = change_state(cells[j].type, cells[j].state, average);
            }
        }

        for (int j = 0; j < NUM_CELLS; j++) // sedaj posodobi tipe celic
        {
            cells[j].state = stateTemp[j];
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

        // for (int k = 0; k < NUM_CELLS; k++)
        // {
        //     if (cells[k].type == 0 || cells[k].type == 1)
        //         printf("id: %d,\ttype: %d,\tstate: %lf\n", k, cells[k].type, cells[k].state);
        // }
        // printf("\n");

        if (i % 20 == 0)
            draw_board(cells);
    }
    free(stateTemp);
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
    cudaMalloc((void **)&d_cells, NUM_CELLS * sizeof(Cell));

    // Copy data from CPU to GPU
    cudaMemcpy(d_cells, cells, NUM_CELLS * sizeof(Cell), cudaMemcpyHostToDevice);

    // Run kernel
    parallel_cuda(d_cells);

    // Copy data from GPU to CPU
    cudaMemcpy(cells, d_cells, NUM_CELLS * sizeof(Cell), cudaMemcpyDeviceToHost);

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

    clock_t start_time, end_time;
    start_time = clock();
    // draw_board(cells);
    check_CUDA();

    // serial(cells);
    // draw_board(cells);

    end_time = clock();
    printf("Time elapsed: %.3lf seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    // Free allocated memory
    for (int i = 0; i < NUM_CELLS; i++)
    {
        free(cells[i].neighbors);
    }
    free(cells);

    return 0;
}