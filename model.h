#include <stdio.h>
#include "constants.h"
/**/
/*
type:
0 = forzen
1 = boundary
2 = unreceptive
3 = edge 
*/

typedef struct Cell
{
    int i;
    int j;
    int type;
    double state;
    int ice;
    // neighbors[0], [1]
    int *neighbors;
};

void initialize(struct Cell cell) 
{
    if (cell.type != 0)
        cell.state = BETA;
    else
        cell.state = 1;
}

double change_state(int type, double state, double average) //pohitritev aplha pre defined
{     
    state = state + ALPHA / 2 * (average - state);

    if (type == 1)
        state += GAMMA;

    return state;
}

double get_state(double **array, int pos_x, int pos_y)
{
    double state = 0;

    // Find cell with the specific position
    struct Cell cell = array[pos_x][pos_y];

    state = cell.state; 

    return state;
}

// double average_state(int neighbours, int *positions)
// {
//     double average = 0;
//     for (int i = 0; i < sizeof(positions), i += 2) {
//         average += get_state();
//     }

//     return average;
// }

void change_type(struct Cell cell) 
{
    if (cell.ice == 1)
        cell.type = 0;
}
