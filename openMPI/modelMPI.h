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

// Poglej sosede, če je kateri tipa 1, posodobi trenutno celico
void set_type_boundary_MPI(Cell *cell_buffer, Cell *top_process, Cell *bottom_process, int start_process, int end_process, int j, int id)
{

    for (int k = 0; k < NUM_NEIGHBORS; k++)
    {
        int sosed = cell_buffer[j].neighbors[k];

        // Preveri, da je valid sosed
        if (sosed >= 0)
        {
            // preveri kje lezi sosed v top_process, cell_buffer ali bottom>_process
            if (sosed < end_process && sosed >= start_process)
            {
                // Dodeli tip boundary le, če ni frozen ali edge
                if (cell_buffer[sosed - start_process].type == 0)
                {
                    cell_buffer[j].type = 1;
                    break;
                }
            }

            else if (sosed >= end_process)
            {
                if (bottom_process[sosed - end_process].type == 0)
                {
                    cell_buffer[j].type = 1;
                    break;
                }
            }

            else if (sosed < start_process)
            {
                if (top_process[sosed - start_process + COLUMNS].type == 0)
                {
                    cell_buffer[j].type = 1;
                    break;
                }
            }
        }
    }
}
// Average state for diffusion
double average_state(Cell *cell_buffer, Cell *top_process, Cell *bottom_process, int start_process, int end_process, int j)
{
    double average = 0;
    for (int k = 0; k < NUM_NEIGHBORS; k++)
    {
        int sosed = cell_buffer[j].neighbors[k];
        // check if neighbour exists
        if (sosed >= 0)
        {
            // Indeksi sosedov znotraj bufferja
            if (sosed < end_process && sosed >= start_process)
            {
                if (cell_buffer[sosed - start_process].type > 1)
                {
                    average += cell_buffer[sosed - start_process].state;
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
                if (top_process[sosed - start_process + COLUMNS].type > 1)
                {
                    //   printf("Top process index: %d\t", start_process - sosed - 1);
                    average += top_process[sosed - start_process + COLUMNS].state;
                }
            }
        }
    }

    average /= 6;

    return average;
}
