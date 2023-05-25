#include <stdio.h>
#include "constants.h"

typedef struct Cell
{
    // Cell id
    int id;

    // Type of cell (0 - frozen, 1 - boundary, 2 - unreceptive, 3 - edge)
    int type;

    // Amount of water
    double state;

    // Neighbors of the cell [6] - [i1, i2, ...]
    int *neighbors;
} Cell;

void set_type(Cell *cells)
{
    for (int i = 0; i < ROWS * COLUMNS; i++)
        cells[i].type = 1;
}

void init_state(Cell *cells)
{
    for (int i = 0; i < ROWS * COLUMNS; i++)
    {
        if (cells[i].type != 0)
            cells[i].state = BETA;
        else
            cells[i].state = 1;
    }
}

double change_state(int type, double state, double average) // pohitritev aplha pre defined
{
    state = state + ALPHA / 2 * (average - state);

    if (type == 1)
        state += GAMMA;

    return state;
}

double get_state(struct Cell cells) // pohitritev aplha pre defined
{
    return cells.state;
}

/*double get_state(int pos_x, int pos_y)
{
    double state=0;

    // Find cell with the specific position
    // double state = cells[position].state;

    return state;
}*/

double average_state(int *neighbors, struct Cell *cells) // dobi cel seznam
{
    double average = 0;
    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {
        int x = neighbors[i];
        int y = neighbors[i];
        int pos = x * COLUMNS + y;
        average += cells[pos].state;
        // average += get_state(cells, neighbors[i][0], neighbors[i][1]);
    }

    average /= NUM_NEIGHBORS;

    return average;
}
