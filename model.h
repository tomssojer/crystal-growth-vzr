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

void set_type_boundary(Cell *cells, int *neighbors)
{

    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {

        // Preveri, da je valid sosed
        if (neighbors[i] >= 0)
        {
            // Dodeli tip boundary le, če ni frozen ali edge
            if (cells[neighbors[i]].type != 0 && cells[neighbors[i]].type != 3)
                cells[neighbors[i]].type = 1;
        }
    }
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

    // Konvekcija
    // if (type == 0 || type == 1)
    //     state += GAMMA;

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

void init_grid(Cell *cells)
{
    //  sosede velikosti 6 sosed -2(x,y) ;
    //  index 0 ZGORAJ LEVO index 1 ZGORAJ DESNO | Y-1, X-1 X+1
    //  index 0        LEVO index 1        DESNO | Y  , X-2 X+2
    //  index 0 SPODAJ LEVO index 5 SPODAJ DESNO | Y+1, X-1 X+1

    int **sosede = (int **)malloc(NUM_NEIGHBORS * sizeof(int *));    // indeks 0 <- y-1,x-1 ; 1 <- y-1, x+1
    int *mapped_sosede = (int *)malloc(NUM_NEIGHBORS * sizeof(int)); // indeks 0 <- y-1,x-1 ; 1 <- y-1, x+1

    for (int i = 0; i < NUM_NEIGHBORS; i++)
        sosede[i] = (int *)malloc(2 * sizeof(int));

    int index = 0;
    int vrstice = ROWS;
    int stolpci = COLUMNS * 3 - 2;

    for (int i = 0; i < vrstice; i++)
    {
        // Definicija ničelnih elementov v heksagonalni strukturi
        int null_elements = COLUMNS - i - 1;

        for (int j = null_elements; j < stolpci - i; j += 2)
        {
            // cells[index].x = i;
            // cells[index].y = j;
            //   Nastavimo dummy vrednosti v array
            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
                sosede[k][0] = -2;
                sosede[k][1] = -2;
                mapped_sosede[k] = -99;
            }

            // Inicializiramo tipe celic
            if (i == 0 || i == vrstice - 1 || j == null_elements || j + 1 == stolpci - i)
            {
                cells[index].type = 3;
            }
            else
            {
                cells[index].type = 2;
            }

            // Indeks zgornje LEVE celice -> ((trenutna celica - 1) - (št null elementov v zgornji)) / normaliziraj z 2
            int map_top = ((j - 1) - (null_elements + 1)) / 2;

            // Indeks DESNE celice -> (razlika med trenutno in null elementi) / 2 + shiftaj v desno
            int map_current = (j - null_elements) / 2 + 1;

            // Indeks spodnje LEVE celice -> ((trenutna celica - 1) - (št null elementov v spodnji)) / normaliziraj z 2
            int map_bottom = ((j - 1) - (null_elements - 1)) / 2;

            /////////////////
            // Zgornje sosede
            /////////////////

            if (i - 1 >= 0)
            {
                // Zgornja leva
                if (j - 1 >= 0 && j != COLUMNS - i - 1)
                {
                    sosede[0][0] = j - 1;
                    sosede[0][1] = i - 1;
                    mapped_sosede[0] = (i - 1) * COLUMNS + map_top;
                }
                else
                {
                    sosede[0][0] = -1;
                    sosede[0][1] = -1;
                }

                // Zgornja desna
                if (j + 1 < stolpci)
                {
                    sosede[1][0] = j + 1;
                    sosede[1][1] = i - 1;
                    mapped_sosede[1] = (i - 1) * COLUMNS + map_top + 1;
                }
                else
                {
                    sosede[1][0] = -1;
                    sosede[1][1] = -1;
                }
            }
            // Če ni zgornjih sosed
            else
            {
                sosede[0][0] = -1;
                sosede[0][1] = -1;
                sosede[1][0] = -1;
                sosede[1][1] = -1;
            }

            ////////////////////////
            // Sosede v isti vrstici
            ////////////////////////

            // Leva
            if (j - 2 >= 0 && j != COLUMNS - i - 1)
            {
                sosede[2][0] = j - 2;
                sosede[2][1] = i;
                mapped_sosede[2] = i * COLUMNS + map_current - 2;
            }
            else
            {
                sosede[2][0] = -1;
                sosede[2][1] = -1;
            }

            // Desna
            if (j + 2 < stolpci - i && j != stolpci - i)
            {
                sosede[3][0] = j + 2;
                sosede[3][1] = i;
                mapped_sosede[3] = i * COLUMNS + map_current;
            }
            else
            {
                sosede[3][0] = -1;
                sosede[3][1] = -1;
            }

            /////////////////
            // Spodnje sosede
            /////////////////

            if (i + 1 < vrstice && j != COLUMNS - i - 1)
            {
                // Spodnja leva
                if (j - 1 >= 0)
                {
                    sosede[4][0] = j - 1;
                    sosede[4][1] = i + 1;
                    mapped_sosede[4] = (i + 1) * COLUMNS + map_bottom;
                }
                else
                {
                    sosede[4][0] = -1;
                    sosede[4][1] = -1;
                }

                // Spodnja desna
                if (j + 1 < stolpci && j != stolpci - i)
                {
                    sosede[5][0] = j + 1;
                    sosede[5][1] = i + 1;
                    mapped_sosede[5] = (i + 1) * COLUMNS + map_bottom + 1;
                }
                else
                {
                    sosede[5][0] = -1;
                    sosede[5][1] = -1;
                }
            }
            // Če ni spodnjih sosed
            else
            {
                sosede[4][0] = -1;
                sosede[4][1] = -1;
                sosede[5][0] = -1;
                sosede[5][1] = -1;
            }

            // inicializacija pomnilnika
            cells[index].neighbors = (int *)malloc(NUM_NEIGHBORS * sizeof(int));

            //  Vpis v strukturo
            memcpy(cells[index].neighbors, mapped_sosede, sizeof(int) * NUM_NEIGHBORS);
            cells[index].id = index;
            index++;
        }
    }

    // Free memory
    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {
        free(sosede[i]);
    }
    free(sosede);
    free(mapped_sosede);

    // nastavitev ledene celice in sosed
    int position = ROWS * ROWS / 2 + COLUMNS / 2;
    cells[position].type = 0;
    set_type_boundary(cells, cells[position].neighbors);
}

// function for visualization of board
void draw_board(Cell *cells, FILE *file)
{
    int stolpci = 3 * COLUMNS - 2;
    int index = 0;

    for (int i = 0; i < ROWS; i++)
    {
        int null_elements = COLUMNS - i - 1;

        for (int j = 0; j < stolpci; j++)
        {
            if (j >= null_elements && j < stolpci - i)
            {
                // Type of cell (0 - frozen, 1 - boundary, 2 - unreceptive, 3 - edge)
                int type = cells[index].type;
                if (type == 0)
                    printf("F ");
                else if (type == 1)
                    printf("B ");
                else if (type == 2)
                    printf("  ");
                else if (type == 3 && j < stolpci - i - 1)
                    printf("E ");
                else if (type == 3)
                    printf("E");

                fprintf(file, "%d ", i);
                fprintf(file, "%d ", j);
                fprintf(file, "%d ", type);

                j++;
                index++;
            }
            else
                printf(" ");
        }
        printf("\n");
    }
    fprintf(file, "\n");
}

// void write_to_file(Cellice *cells, int **array_to_file)
// {
//     FILE *file = fopen("output_serial", "w");
//     if (file == NULL)
//     {
//         printf("Could not open file.")
//             exit(-1);
//     }

//     for (int i = 0; i < SIZE; i++)
//     {
//         array_to_file[i][0] = cells[i].state
//     }
// }

// void printHexagon(int size)
// { // indeksi sosed so [y-1][x-1][x+1] in [y][x-1][x-2] in [y+1][x-1][x-2]
//     int i, j;
//     for (i = 0; i < ROWS; i++)
//     {
//         for (j = 0; j < COLUMNS - i - 1; j++)
//             printf(".");

//         for (j = 0; j < size; j++)
//         {
//             if (j == size - 1)
//                 printf("*");
//             else
//                 printf("*.");
//         }

//         for (j = size - i; j < size; j++)
//             printf(".");

//         printf("\n");
//     }
// }