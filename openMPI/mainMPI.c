#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
// #include <mpi.h>
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

        // Določi začetno vrednost glede na tip celice
        init_state(cells);
        // Dodaj sosede in indekse v struct
        // draw_board(cells);
    }

    // ------------- Konec inicializacije ------------- //

    int cells_per_process = NUM_CELLS / num_p; // ROWS*COLUMNS / procesors
    int rows_per_process = ROWS / num_p;
    int start_process = id * cells_per_process;
    int end_process = (id + 1) * cells_per_process;

    // Cell cell;
    // MPI_Aint base_address;
    // MPI_Get_address(&cell, &base_address);
    // MPI_Get_address(&cell.type, &displacements[0]);
    // MPI_Get_address(&cell.state, &displacements[1]);
    // MPI_Get_address(&cell.neighbors[0], &displacements[2]);

    // // offsets
    // displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    // displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    // displacements[2] = MPI_Aint_diff(displacements[2], base_address);

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

    // if (id == 0)
    // {
    //     for (int i = 0; i < cells_per_process; i++)
    //     {
    //         if (cell_buffer[i].type == 1)
    //             printf("Cell buffer: %d, %lf, %d\n", cell_buffer[i].type, cell_buffer[i].state, cell_buffer[i].neighbors[0]);
    //     }
    // }

    // --------- driver code ----------//
    float average = 0;
    double *stateTemp = (double *)malloc(cells_per_process * sizeof(double));

    // double *state_temp_array = NULL;

    // if (id == 0)
    // {
    //     // Process 0 allocates memory to store gathered data
    //     state_temp_array = (double *)malloc(NUM_CELLS * sizeof(double));
    // }

    for (int i = 0; i < STEPS; i++) // iteracije, oz stanja po casu
    {

        MPI_Sendrecv(&cell_buffer[cells_per_process - COLUMNS], 1, row_type, (id + 1) % num_p, 0,
                     top_process, 1, row_type, (id + num_p - 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        MPI_Sendrecv(&cell_buffer[0], 1, row_type, (id + num_p - 1) % num_p, 0,
                     bottom_process, 1, row_type, (id + 1) % num_p, 0,
                     MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

        // printf("id: %d send tag: %d recieve tag: %d \n", id, (id + 1) % num_p, (id + num_p - 1) % num_p);
        //  if (id == 1)
        //  {
        //      for (int j = start_process; j < end_process; j++)
        //      {
        //          for (int k = 0; k < NUM_NEIGHBORS; k++)
        //          {
        //              int sosed = cell_buffer[j - start_process].neighbors[k];
        //              printf("Sosed: %d, %d, %d, %d, %lf\n", j, k, sosed, cell_buffer[j - start_process].type, cell_buffer[j].state);
        //          }
        //      }

        //     //     for (int j = 0; j < cells_per_process; j++)
        //     //     {
        //     //         for (int k = 0; k < 6; k++)
        //     //             printf("Sosed: %d, %d, %lf, %d\n", j, cell_buffer[j].type, cell_buffer[j].state, cell_buffer[j].neighbors[k]);
        //     //     }
        // }

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
        } // mpi gather avrage values v enem celem arryju TO-DO

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
                set_type_boundary_MPI(cell_buffer, top_process, bottom_process, start_process, end_process, j, id);
        }

        // MPI_Gather(cell_buffer, rows_per_process, row_type, cells, rows_per_process, row_type, 0, MPI_COMM_WORLD);
        // if (id == 0)
        //     draw_board(cells);

        for (int j = 0; j < cells_per_process; j++)
        {
            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
                if (cell_buffer[j].type == 1)
                {
                    int sosed = cell_buffer[j].neighbors[k] - start_process;
                    if (sosed < cells_per_process && sosed >= 0 && sosed >= start_process)
                    {
                        if (cell_buffer[sosed].type == 3)
                        {

                            printf("break %d\n", i);
                            i = STEPS;
                            j = cells_per_process;
                            break;
                        }
                    }
                }
            }
        }

        // MPI_Gather(cell_buffer, rows_per_process, row_type, cells, rows_per_process, row_type, 0, MPI_COMM_WORLD);
        // for (int j = 0; j < NUM_CELLS; j++) // sedaj posodobi tipe celic
        // {
        //     cells[j].state = stateTemp[j];
        //     if (cells[j].state >= 1)
        //     {
        //         cells[j].type = 0; // turns into ice cell
        //         set_type_boundary(cells, cells[j].neighbors);
        //     }
        // }

        // for (int j = start_process; j < end_process; j++) // sedaj posodobi tipe celic
        // {
        //     int offset = j - start_process;
        //     cell_buffer[offset].state = cells[j].state;
        //     cell_buffer[offset].type = cells[j].type;
        //     for (int k = 0; k < 6; k++)
        //     {
        //         cell_buffer[offset].neighbors[k] = cells[j].neighbors[k];
        //     }
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
    // free(state_temp_array);

    MPI_Type_free(&cell_type);
    MPI_Type_free(&cell_type_resized);
    MPI_Type_free(&row_type);
    MPI_Finalize();

    return 0;
}