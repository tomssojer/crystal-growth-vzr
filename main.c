#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "constants.h"
#include "model.h"

void printHexagon(int size)
{ // indeksi sosed so [y-1][x-1][x+1] in [y][x-1][x-2] in [y+1][x-1][x-2]
    int i, j;
    for (i = 0; i < ROWS; i++)
    {
        for (j = 0; j < COLUMNS - i - 1; j++)
        {
            printf(".");
            // printf("%d",j);
        }
        for (j = 0; j < size; j++)
        {

            if (j == size - 1)
            {
                printf("*");
            }
            else
            {
                printf("*.");
            }
        }
        for (j = size - i; j < size; j++)
        {
            printf(".");
        }
        printf("\n");
    }
}

void printTab(int **tab, int j, int mappIdx)
{
    for (int i = 0; i < 6; i++)
    {
        printf("%d->%d |(%2d,%2d) ", j, mappIdx, tab[i][0], tab[i][1]); // x y
    }
}

void printmapped(int **tab, int j, int x, int *mapp)
{
    for (int i = 0; i < 6; i++)
    {
        printf("[%d,%d]  (%2d,%2d)->%d |", j, x, tab[i][0], tab[i][1], mapp[i]); // x y
    }
}

void neighbours(Cell *cells)
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
            // Nastavimo dummy vrednosti v array
            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
                sosede[k][0] = -2;
                sosede[k][1] = -2;
                mapped_sosede[k] = -99;
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

            // Vpis v strukturo
            cells[index].neighbors = mapped_sosede;
            cells[index].id = index;
            index++;

            printmapped(sosede, j, i, mapped_sosede);
            printf("\n");
        }
    }

    // Free memory
    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {
        free(sosede[i]);
    }
    free(sosede);
    free(mapped_sosede);
}

int main()
{

    int array_size = COLUMNS * ROWS;
    printHexagon(ROWS);
    // Definicija arraya s structi
    struct Cell *cells = malloc(array_size * sizeof *cells);
    neighbours(cells);

    int i, j;
    // First, initialize state on all cells
    for (i = 0; i < array_size; i++)
    {
        // We deal with one cell at the time
        cells[i].state = initialize_state(cells[i].type);
    }

    float average;
    for (i = 0; i < array_size; i++)
    {
        // We deal with one cell at the time
        if (cells[i].type == 1 || cells[i].type == 2)
        {
            // Calculate average state of neighbors, needs current cell's neighbours and ponter to all cells
            average = average_state(cells[i].neighbors, cells);
            cells[i].state = change_state(cells[i].type, cells[i].state, average);
        }
    }

    // Free allocated memory
    free(cells);

    return 0;
}

// int main() {

//     int N = 5;
//     float* X;
//     float* Y;
//     int array_size = (3 * COLUMNS - 2)* ROWS;

//     // Definicija arraya s structi
//     struct Cell cell;
//     struct Cell* cells = malloc(array_size * sizeof *cells);
//     // struct Cell cell_array[array_size];

//     make_hex_grid(N, &X, &Y);

//     int i, j;
//     for (i = 0; i < N; i++) {
//         for (j = 0; j < N; j++) {
//             printf("(%f, %f) ", X[i * N + j], Y[i * N + j]);
//             initialize(cell);
//         }
//         printf("\n");
//     }

//     // Free allocated memory
//     free(X);
//     free(Y);

//     return 0;
// }