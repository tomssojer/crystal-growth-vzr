#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "../constants.h"
#include "modelMP.h"
#include <omp.h>

// gcc -fopenmp -o mainMP mainMP.c --openmp -lm  
// 1. Začni z eno frozen celico, okoli nje so boundary
// 2. Za vse celice, ki so boundary in unreceptive poteka difuzija
// 3. Za vse celice, ki so frozen in boundary poteka konvekcija
// 3.a Upoštevaj, da le sosede, ki so edge ali unreceptive sharajo vodo
// 4. Preveri, če ima celica state >= 1 -> nastavi na frozen, njene sosede na boundary
// 5. Preveri, če je boundary celica soseda z edge celico, prekini simulacijo

int terminateSimulation = 0;

void paralel(Cell *cells)
{
     // uporabil sem gnezdenje da se vsi štirje loop izvajajo, samo nisem prepričan če nebi bilo bolje brez ali pa samo 2
                                          //  barrier condition sem dodal zato da se niti počakajo preden gremo naprej 
                                        // znotraj parralizacije sem vključil tudi edge pogoj preverjanje.
    float average = 0;
    double *stateTemp = (double *)malloc(NUM_CELLS * sizeof(double));

    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {
        #pragma omp parallel for
        for (int j = 0; j < NUM_CELLS; j++) // posodobi vsa stanja - difuzija, konvekcija
        {
            // We deal with one cell at the time, do not deal with edge type
            if (cells[j].type != 3)
            {
                // Calculate average state of neighbors, needs current cell's neighbours and pointer to all cells
                average = average_state(cells[j].neighbors, cells);

                stateTemp[j] = change_state(cells[j].type, cells[j].state, average);
                // cells[j].state = change_state(cells[j].type, cells[j].state, average);
            }
        }
        #pragma omp parallel for
        for (int j = 0; j < NUM_CELLS; j++) // sedaj posodobi tipe celic
        {
            cells[j].state = stateTemp[j];
            if (cells[j].state >= 1)
            {
                cells[j].type = 0; // turns into ice cell
                set_type_boundary(cells, cells[j].neighbors);
            }
        }

         #pragma omp parallel for
         for (int j = 0; j < NUM_CELLS; j++)
        {
            if (!terminateSimulation && cells[j].type == 1) {
                for (int k = 0; k < NUM_NEIGHBORS; k++)
                {
                    if(cells[j].neighbors[k] == 3)
                    {
                       
                        #pragma omp critical
                        {
                            terminateSimulation = 1;
                        }
                        break;
                        }
                }
            }
        }
         if (terminateSimulation)
        {
            printf("break %d\n", i);
            break;
        }
        //printf("Step: %d ----------------------------------------------------------\n", i);

        // for (int k = 0; k < NUM_CELLS; k++)
        // {
        //     if (cells[k].type == 0 || cells[k].type == 1)
        //         printf("id: %d,\ttype: %d,\tstate: %lf\n", k, cells[k].type, cells[k].state);
        // }
        // printf("\n");

        //draw_board(cells);
    
    free(stateTemp);
    }
}

int main(int argc, int *argv[])
{
    // // ------------- Začetek inicializacije ------------- //
    // printHexagon(ROWS); //

    // Definicija arraya s structi
    Cell *cells = malloc(NUM_CELLS * sizeof *cells);

    // Dodaj sosede in indekse v struct
    init_grid(cells);

    // Določi začetno vrednost glede na tip celice
    init_state(cells);

    // ------------- Konec inicializacije ------------- //

    clock_t start_time, end_time;
    start_time = clock();

    draw_board(cells);
    paralel(cells);
    draw_board(cells);

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