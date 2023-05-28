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

double change_state(int type, double state, double average) // pohitritev alpha predefined
{
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
double average_state(int *neighbors, Cell *cells) // dobi cel seznam
{
    double average = 0;
    int count = 0;
    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {
        // check if neighbour exists
        if (neighbors[i] >= 0)
        {
            // Če je type sosednje celice unreceptive ali edge, potem pridobi del od nje
            if (cells[neighbors[i]].type > 1)
            {
                average += cells[neighbors[i]].state;
                count++;
            }
        }
    }
    if (count == 0)
        return average;

    average /= 6;

    return average;
}
