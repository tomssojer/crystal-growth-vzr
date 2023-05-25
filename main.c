#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "constants.h"
#include "model.h"

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

void serial(Cell *cells)
{
    float average = 0;
    for (int i = 0; i < STEPS; i++)
    {
        for (int j = 0; j < NUM_CELLS; j++)
        {
            // We deal with one cell at the time, do not deal with edge type
            if (cells[j].type != 3)
            {
                // Calculate average state of neighbors, needs current cell's neighbours and pointer to all cells
                average = average_state(cells[j].neighbors, cells);
                cells[j].state = change_state(cells[j].type, cells[j].state, average);
            }
        }

        for (int j = 0; j < NUM_CELLS; j++)
            set_type_boundary(cells, cells[j].neighbors);
    }

    // Sprintaj vse elemente v strukturi
    printStructs(cells);
}

int main(int argc, int *argv[])
{
    // // ------------- Začetek inicializacije ------------- //
    printHexagon(ROWS);

    // Definicija arraya s structi
    Cell *cells = malloc(NUM_CELLS * sizeof *cells);

    // Dodaj sosede in indekse v struct
    init_grid(cells);

    // Določi začetno vrednost glede na tip celice
    init_state(cells);

    // ------------- Konec inicializacije ------------- //

    double start_time, end_time;
    double time_difference;
    start_time = time(NULL);

    serial(cells);

    end_time = time(NULL);
    time_difference = difftime(end_time, start_time);
    printf("Time elapsed: %.3f seconds\n", time_difference);

    // Free allocated memory
    for (int i = 0; i < NUM_CELLS; i++)
    {
        free(cells[i].neighbors);
    }
    free(cells);

    return 0;
}