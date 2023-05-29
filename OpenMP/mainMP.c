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

void open_MPI(Cell *cells, FILE *file,int num_threds)
{
    double *stateTemp = (double *)malloc(NUM_CELLS * sizeof(double));
    
    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {
        #pragma omp parallel for schedule(dynamic,NUM_CELLS/num_threds)   // atomicne ven
        for (int j = 0; j < NUM_CELLS; j++) // posodobi vsa stanja - difuzija, konvekcija
        {
            // We deal with one cell at the time, do not deal with edge type
            if (cells[j].type != 3)
            {
                // Calculate average state of neighbors, needs current cell's neighbours and pointer to all cells
                  float average  = average_state(cells[j].neighbors, cells);

                stateTemp[j] = change_state(cells[j].type, cells[j].state, average);
                // cells[j].state = change_state(cells[j].type, cells[j].state, average);
            }
        }

        #pragma omp parallel for schedule(dynamic,NUM_CELLS/num_threds) 
        for (int j = 0; j < NUM_CELLS; j++) // sedaj posodobi tipe celic
        {
            cells[j].state = stateTemp[j];
            if (cells[j].state >= 1)
            {
                cells[j].type = 0; // turns into ice cell
                set_type_boundary(cells, cells[j].neighbors);
                // for (int k = 0; k < NUM_NEIGHBORS; k++)
                // {
                //     if (cells[j].type == 1 && cells[j].neighbors[k] == 3)
                //     {
                //         printf("break %d\n", i);
                //         i = STEPS;
                //     }
                // }
            }
        }
        int exitFlag = 0;
        // Če je ena od sosed celice tipa edge, prekini simulacijo
        #pragma omp parallel for schedule(dynamic,NUM_CELLS/num_threds) shared(exitFlag) 
        for (int j = 0; j < NUM_CELLS; j++)
        {
            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
                int sosed = cells[j].neighbors[k];
                if (sosed >= 0)
                {
                    if (cells[j].type == 1 && cells[sosed].type == 3)
                    {
                        //printf("break %d\n", i);
                        j=NUM_CELLS;
                        k=NUM_NEIGHBORS;
                        #pragma omp atomic write
                            exitFlag = 1;
                   
                    }
                }
            }
        }
        
        if(exitFlag)
            break;
        // if (i % STEPS_TO_DRAW == 0)
        // {
        //     // printf("Step number: %d\n", i);
        //     draw_board(cells);
        //     // write_to_file(cells, file);
        //     // printf("Step number: %d\n", i);
        //     draw_board(cells);
        //     // write_to_file(cells, file);
        // }
    }

    free(stateTemp);
}

int main(int argc, int *argv[])
{
    // // ------------- Začetek inicializacije ------------- //
    // printHexagon(ROWS); //
    int num_threds=1;
        #pragma omp parallel
        {
    #pragma omp single
            num_threds=omp_get_num_threads();
            printf("num_threads = %d\n",num_threds);
        }
    // Definicija arraya s structi
    Cell *cells = malloc(NUM_CELLS * sizeof(*cells));
    Cell *cells = malloc(NUM_CELLS * sizeof(*cells));

    // Dodaj sosede in indekse v struct
    init_grid(cells);

    // Določi začetno vrednost glede na tip celice
    init_state(cells);

    // Ime datoteke - odvisno od št vrstic, alfe, bete, game
    char *file_name = "serial_array.txt";
    FILE *file = fopen(file_name, "w");

    // if (file == NULL)
    // {
    //     printf("Could not open file.");
    //     exit(-1);
    // }
    // Ime datoteke - odvisno od št vrstic, alfe, bete, game
    char *file_name = "serial_array.txt";
    FILE *file = fopen(file_name, "w");

    // if (file == NULL)
    // {
    //     printf("Could not open file.");
    //     exit(-1);
    // }
    // ------------- Konec inicializacije ------------- //

   double dt = omp_get_wtime();

    open_MPI(cells, file,num_threds);
    // write_to_file(cells, file);

    dt = omp_get_wtime() - dt;

    printf("Time elapsed: %.3lf seconds\n", (double)(dt));
    //draw_board(cells);

    fclose(file);

    // Free allocated memory
    free(cells);

    return 0;
}
