#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
// #include "/usr/include/openmpi-x86_64/mpi.h"
#include <mpi.h>
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
    int columns_per_process = COLUMNS / num_p;
    // Spodnji definiciji sta pri rowMPI drugačni
    int start_process = id * columns_per_process;
    int end_process = (id + 1) * columns_per_process;

    MPI_Datatype cell_type, cell_type_resized, column_type, column_type_resized, row_type, row_type_resized;

    // Definicija cell structov
    int lengths[3] = {1, 1, 6}; // dolzine podatkov
    MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_INT};
    MPI_Aint displacements[3] = {offsetof(Cell, type), offsetof(Cell, state), offsetof(Cell, neighbors)};
    MPI_Aint lb, extent;

    MPI_Type_create_struct(3, lengths, displacements, types, &cell_type);
    MPI_Type_get_extent(cell_type, &lb, &extent);
    MPI_Type_create_resized(cell_type, lb, extent, &cell_type_resized);
    MPI_Type_commit(&cell_type_resized);
    ///////////////////////////

    // Definicija column typa
    MPI_Type_vector(ROWS, 1, columns_per_process, cell_type_resized, &column_type);
    MPI_Type_create_resized(column_type, 0, sizeof(Cell), &column_type_resized);
    MPI_Type_commit(&column_type_resized);

    MPI_Type_vector(COLUMNS, 1, sizeof(Cell), cell_type_resized, &row_type); // posljemo le en stolpec
    MPI_Type_create_resized(row_type, 0, sizeof(Cell), &row_type_resized);
    MPI_Type_commit(&row_type_resized);

    Cell *cell_buffer = malloc(cells_per_process * sizeof(Cell));
    Cell *left_process = malloc(ROWS * sizeof(Cell));
    Cell *right_process = malloc(ROWS * sizeof(Cell));
    // scatter initial matrix
    // MPI_Scatter(boardptr, mycols, column_resized,
    // 			*myboard, mycols, column_perCore_resized,
    // 			0, MPI_COMM_WORLD);
    // Scatter work

    MPI_Scatter(cells, columns_per_process, column_type_resized, cell_buffer,
                columns_per_process, column_type_resized, 0, MPI_COMM_WORLD);
    if (id == 0)
    {
        for (int i = 0; i < cells_per_process; i++)
            for (int j = 0; j < NUM_NEIGHBORS; j++)
                printf("Id: %d, cell neighbor: %d\n", i, cell_buffer[i].neighbors[j]);
    }
    // --------- driver code ----------//

    float average = 0;
    double *stateTemp = (double *)malloc(cells_per_process * sizeof(double));

    for (int i = 0; i < STEPS; i++)
    {

        MPI_Sendrecv(&cell_buffer[cells_per_process - ROWS], 1, column_type_resized, (id + 1) % num_p, 0,
                     left_process, 1, column_type_resized, (id + num_p - 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        MPI_Sendrecv(&cell_buffer[0], 1, column_type_resized, (id + num_p - 1) % num_p, 0,
                     right_process, 1, column_type_resized, (id + 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        // Iteracija skozi cell_buffer
        for (int j = 0; j < cells_per_process; j++)
        {
            if (cell_buffer[j].type != 3)
            {
                average = average_state_column(cell_buffer, left_process, right_process, start_process, end_process, j);
                stateTemp[j] = change_state(cell_buffer[j].type, cell_buffer[j].state, average);
            }
        }

        // Posodobi celice v frozen
        for (int j = 0; j < cells_per_process; j++)
        {
            cell_buffer[j].state = stateTemp[j];
            if (cell_buffer[j].state >= 1)
            {
                cell_buffer[j].type = 0;
            }
        }

        // Spet poslji zadnjo in prvo vrstico posodobljen buffer, da se izvede update celic
        MPI_Sendrecv(&cell_buffer[cells_per_process - ROWS], 1, column_type_resized, (id + 1) % num_p, 0,
                     left_process, 1, column_type_resized, (id + num_p - 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        MPI_Sendrecv(&cell_buffer[0], 1, column_type_resized, (id + num_p - 1) % num_p, 0,
                     right_process, 1, column_type_resized, (id + 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        // Posodobi celice v boundary
        for (int j = 0; j < cells_per_process; j++)
        {
            if (cell_buffer[j].type == 2)
                set_type_boundary_column_MPI(cell_buffer, left_process, right_process, start_process, end_process, j);
        }

        // if (id == 0 && i % STEPS_TO_DRAW == 0)
        // {
        //     MPI_Gather(cell_buffer, rows_per_process, row_type, cells, rows_per_process, row_type, 0, MPI_COMM_WORLD);
        //     draw_board(cells);
        // }
    }

    MPI_Gather(cell_buffer, columns_per_process, column_type_resized, cells, columns_per_process, column_type_resized, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (id == 0)
    {
        draw_board(cells);
        printf("Time elapsed: %.3lf seconds\n", end_time - start_time);
    }

    //  Free allocated memory
    free(cells);
    free(left_process);
    free(right_process);
    free(cell_buffer);
    free(stateTemp);

    MPI_Type_free(&cell_type);
    MPI_Type_free(&cell_type_resized);
    MPI_Type_free(&column_type);
    MPI_Type_free(&column_type_resized);
    MPI_Finalize();

    return 0;
}