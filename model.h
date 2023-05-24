#include <stdio.h>
#include "constants.h"

typedef struct Cell
{
    // Hexagon indexes in array of structs
    int i;
    int j;

    // Type of cell (0 - frozen, 1 - boundary, 2 - unreceptive, 3 - edge)
    int type;

    // Amount of water
    double state;
    int ice;

    // Neighbours of the cell [6][2] - [[x1][y1], ...]
    int **neighbors;
}Cell;

double initialize_state(int type)
{
    double state = 0;

    if (type != 0)
        state = BETA;
    else
        state = 1;

    return state;
}


double change_state(int type, double state, double average) //pohitritev aplha pre defined
{
    state = state + ALPHA / 2 * (average - state);

    if (type == 1)
        state += GAMMA;

    return state;
}

double get_state(struct Cell cells) //pohitritev aplha pre defined
{
    return cells.state;;
}

/*double get_state(int pos_x, int pos_y)
{
    double state=0;

    // Find cell with the specific position
    // double state = cells[position].state;

    return state;
}*/

double average_state(int **neighbors, struct Cell *cells) //dobi cel seznam
{
    double average = 0;
    for (int i = 0; i < NUM_NEIGHBORS;  i++) {
        int x=neighbors[i][0];
        int y=neighbors[i][1];
        int pos= x*COLUMNS+y;
        average += cells[pos].state;
        //average += get_state(cells, neighbors[i][0], neighbors[i][1]);
    }

    average /= NUM_NEIGHBORS;

    return average;
}

void change_type(int ice, int type)
{
    if (ice == 1)
        type = 0;
}
