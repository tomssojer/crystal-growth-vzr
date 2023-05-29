#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "/usr/include/openmpi-x86_64/mpi.h"
#include "../constants.h"
#include "modelMPI.h"

int main(int argc, char *argv[])
{
    // // ------------- Začetek inicializacije ------------- //

    Cell *cells = malloc(NUM_CELLS * sizeof *cells);

    // mpi init
    int id;              // process id
    int num_p;           // total number of processes
    int source;          // sender id
    int destination;     // receiver id
    int tag = 0;         // message tag
    int buffer[1];       // message buffer
    MPI_Status status;   // message status
    MPI_Request request; // MPI request
    int flag;            // request status flag

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // Čas
    double start_time = MPI_Wtime();

    if (id == 0)
    {
        init_grid(cells);
        init_state(cells);
        // draw_board(cells);
    }

    // ------------- Konec inicializacije ------------- //

    int cells_per_process = NUM_CELLS / num_p; // ROWS*COLUMNS / procesors
    int rows_per_process = ROWS / num_p;
    int start_process = id * cells_per_process;
    int end_process = (id + 1) * cells_per_process;

    MPI_Datatype cell_type, cell_type_resized, row_type;

    int lengths[3] = {1, 1, 6}; // dolzine podatkov
    MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_INT};

    MPI_Aint displacements[3] = {offsetof(Cell, type), offsetof(Cell, state), offsetof(Cell, neighbors)};
    MPI_Aint lb, extent;
    MPI_Type_create_struct(3, lengths, displacements, types, &cell_type);
    MPI_Type_get_extent(cell_type, &lb, &extent);
    MPI_Type_create_resized(cell_type, lb, extent, &cell_type_resized);
    MPI_Type_commit(&cell_type_resized);
    ///////////////////////////

    MPI_Type_contiguous(COLUMNS, cell_type_resized, &row_type);
    MPI_Type_commit(&row_type);

    Cell *cell_buffer = malloc(cells_per_process * sizeof(Cell));
    Cell *top_process = malloc(COLUMNS * sizeof(Cell));
    Cell *bottom_process = malloc(COLUMNS * sizeof(Cell));

    // Scatter work
    MPI_Scatter(cells, rows_per_process, row_type, cell_buffer, rows_per_process, row_type, 0, MPI_COMM_WORLD);

    // --------- driver code ----------//
    float average = 0;
    double *stateTemp = (double *)malloc(cells_per_process * sizeof(double));

    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {

        MPI_Sendrecv(&cell_buffer[cells_per_process - COLUMNS], 1, row_type, (id + 1) % num_p, 0,
                     top_process, 1, row_type, (id + num_p - 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        MPI_Sendrecv(&cell_buffer[0], 1, row_type, (id + num_p - 1) % num_p, 0,
                     bottom_process, 1, row_type, (id + 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        // Send data to others and receive
        for (int j = 0; j < cells_per_process; j++) // posodobi vsa stanja - difuzija, konvekcija
        {
            // We deal with one cell at the time, do not deal with edge type
            if (cell_buffer[j].type != 3)
            {
                // if (id == 1)
                // Calculate average state of neighbors, needs current cell's neighbours and pointer to all cells
                average = average_state(cell_buffer, top_process, bottom_process, start_process, end_process, j);

                stateTemp[j] = change_state(cell_buffer[j].type, cell_buffer[j].state, average);
            }
        }

        for (int j = 0; j < cells_per_process; j++) // sedaj posodobi tipe celic
        {
            cell_buffer[j].state = stateTemp[j];
            if (cell_buffer[j].state >= 1)
            {
                cell_buffer[j].type = 0; // turns into ice cell
            }
        }
        // Spet poslji zadnjo in prvo vrstico posodobljen buffer, da se izvede update celic
        MPI_Sendrecv(&cell_buffer[cells_per_process - COLUMNS], 1, row_type, (id + 1) % num_p, 0,
                     top_process, 1, row_type, (id + num_p - 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        MPI_Sendrecv(&cell_buffer[0], 1, row_type, (id + num_p - 1) % num_p, 0,
                     bottom_process, 1, row_type, (id + 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        // // Če je ena od sosed celice tipa edge, prekini simulacijo
        for (int j = 0; j < cells_per_process; j++)
        {
            if (cell_buffer[j].type == 2)
                set_type_boundary_MPI(cell_buffer, top_process, bottom_process, start_process, end_process, j);
        }

        // int termination_flag = 0;
        // for (int j = 0; j < cells_per_process; j++)
        // {
        //     for (int k = 0; k < NUM_NEIGHBORS; k++)
        //     {
        //         if (cell_buffer[j].type == 1)
        //         {
        //             int sosed = cell_buffer[j].neighbors[k];
        //             if (sosed < end_process && sosed >= 0 && sosed >= start_process)
        //             {
        //                 sosed -= start_process;
        //                 if (cell_buffer[sosed].type == 3)
        //                 {
        //                     MPI_Test(&request, &flag, &status);
        //                     i = STEPS;
        //                     j = cells_per_process;
        //                     // MPI_Abort(MPI_COMM_WORLD, 0);
        //                     // ali
        //                     for (int x = 0; x < num_p; x++)
        //                     {
        //                         if (x != id)
        //                         {
        //                             termination_flag = 1;
        //                             printf("breaking %d \n", x);
        //                             MPI_Send(&termination_flag, 1, MPI_INT, x, 0, MPI_COMM_WORLD);
        //                         }
        //                     }
        //                     // break;
        //                 }
        //             }
        //         }
        //     }
        // }
        // // MPI_Finalize();
        // if (termination_flag)
        // {
        //     i = STEPS;
        //     MPI_Cancel(&request);
        //     MPI_Wait(&request, &status);
        //     // break;
        // }
        // if (id == 0 && i % STEPS_TO_DRAW == 0)
        // {
        //     MPI_Gather(cell_buffer, rows_per_process, row_type, cells, rows_per_process, row_type, 0, MPI_COMM_WORLD);
        //     draw_board(cells);
        // }
    }

    MPI_Gather(cell_buffer, rows_per_process, row_type, cells, rows_per_process, row_type, 0, MPI_COMM_WORLD);
    // --------- driver code ----------//

    double end_time = MPI_Wtime();

    if (id == 0)
    {
        // draw_board(cells);
        printf("Time elapsed: %.3lf seconds\n", end_time - start_time);
        //  Free allocated memory
    }

    free(cells);
    free(top_process);
    free(bottom_process);
    free(cell_buffer);
    free(stateTemp);

    MPI_Type_free(&cell_type);
    MPI_Type_free(&cell_type_resized);
    MPI_Type_free(&row_type);
    MPI_Finalize();

    return 0;
}