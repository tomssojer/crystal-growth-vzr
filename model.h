#include <stdio.h>
#include "constants.h"

typedef struct Cell
{
    // Cell id
    int id;

    int x;
    int y;

    // Type of cell (0 - frozen, 1 - boundary, 2 - unreceptive, 3 - edge)
    int type;

    // Amount of water
    double state;

    // Neighbors of the cell [6] - [i1, i2, ...]
    int *neighbors;
} Cell;

void set_type_boundary(Cell *cells, int *neighbors)
{
    for (int i = 0; i < NUM_NEIGHBORS; i++)

        // Preveri, da je valid sosed
        if (cells[neighbors[i]].type >= 0)

            // Dodeli tip boundary le, če ni frozen ali edge
            if (cells[neighbors[i]].type != 0 || cells[neighbors[i]].type != 3)
                cells[neighbors[i]].type = 1;
}

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
    // Difuzija s sosedi
    state = state + ALPHA / 2 * (average - state);

    // Konvekcija
    if (type == 0 || type == 1)
        state += GAMMA;

    return state;
}

double average_state(int *neighbors, Cell *cells) // dobi cel seznam
{
    double average = 0;
    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {
        if (neighbors[i] >= 0)
        {
            average += cells[neighbors[i]].state;
        }
    }

    average /= NUM_NEIGHBORS;

    return average;
}
