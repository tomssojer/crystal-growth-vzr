#include <stdio.h>
#include "../constants.h"
#include "../grid.h"

void init_state(Cell *cells)
{
    for (int i = 0; i < NUM_CELLS; i++)
    {
        if (cells[i].type != 0)
            cells[i].state = BETA;
        else
            cells[i].state = 1;
    }
}

double change_state(int type, double state, double average) // pohitritev aplha pre defined
{
    //  unreceptive, edge
    // if (type != 0)
    //     state = state + ALPHA / 2 * (average - state);

    // receptive
    if (type < 2)
    {
        state = state + ALPHA / 2 * average + GAMMA;
    }
    //  unreceptive, edge
    else
    {
        state = state + ALPHA / 2 * (average - state);
    }

    return state;
}

// Average state for diffusion
double average_state(Cell *cell_buffer, Cell *top_process, Cell *bottom_process, int start_process, int end_process, int j)
{
    double average = 0;
    for (int k = 0; k < NUM_NEIGHBORS; k++)
    {
        int sosed = cell_buffer[j].neighbors[k];
        // check if neighbour exists
        printf("Sosed: %d, %d, %d, %d, %lf\n", j, k, sosed, cell_buffer[j].type, cell_buffer[j].state);
        /*if (sosed >= 0)
        {
            // Indeksi sosedov znotraj bufferja
            if (sosed < end_process && sosed >= start_process)
            {
                // printf("cell_buffer[sosed - start_process].state: %lf\n", cell_buffer[sosed - start_process].state);
                if (cell_buffer[sosed - start_process - 1].type > 1)
                {
                    average += cell_buffer[sosed - start_process - 1].state;
                    // printf("Current process index: %d\n", sosed - start_process - 1);
                }
            }
            // Indeksi sosedov, ki segajo čez buffer
            else if (sosed >= end_process)
            {
                // printf("index bot %d\n", sosed - end_process);
                if (bottom_process[sosed - end_process].type > 1)
                {
                    // printf("Bottom process index: %d\t", sosed - end_process);
                    average += bottom_process[sosed - end_process].state;
                }
            }
            // Indeksi sosedov, ki so pred bufferjem
            else if (sosed < start_process)
            {
                // printf("index TOP %d\n", start_process - sosed - 1);
                if (top_process[start_process - sosed - 1].type > 1)
                {
                    //   printf("Top process index: %d\t", start_process - sosed - 1);
                    average += top_process[start_process - sosed - 1].state;
                }
            }
        }*/
    }

    average /= 6;

    return average;
}
