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

void printmaped(int **tab, int j, int x, int *mapp)
{
    for (int i = 0; i < 6; i++)
    {
        printf("[%d,%d]  (%2d,%2d)->%d |", j, x, tab[i][0], tab[i][1], mapp[i]); // x y
    }
}

void neighbours(Cell *cells)
{
    // printf("dela");
    //  sosede velikosti 6 sosed -2(x,y) ;
    //  index 0 ZGORAJ LEVO index 1 ZGORAJ DESNO | Y-1, X-1 X+1
    //  index 0        LEVO index 1        DESNO | Y  , X-2 X+2
    //  index 0 SPODAJ LEVO index 5 SPODAJ DESNO | Y+1, X-1 X+1
    int **sosede = (int **)malloc(6 * sizeof(int *));    // indeks 0 <- y-1,x-1 ; 1 <- y-1, x+1
    int *mapped_sosede = (int *)malloc(6 * sizeof(int)); // indeks 0 <- y-1,x-1 ; 1 <- y-1, x+1
    for (int i = 0; i < 6; i++)
    {
        sosede[i] = (int *)malloc(2 * sizeof(int));
        mapped_sosede[i] = -99;
        sosede[i][0] = -2;
        sosede[i][1] = -2;
    }
    int index = 0;
    int vrstice = ROWS;
    int stolpci = COLUMNS * 3 - 2;
    for (int i = 0; i < vrstice; i++)
    {
        for (int j = COLUMNS - i - 1; j < stolpci - i; j += 2)
        {
            // printf("%c ",plosca[i][j]);
            int mappJ = (j - COLUMNS + i + 1) / 2 - 1; //=i*COLUMNS+(j-COLUMNS+i+1)/2;
            // if(plosca[i][j]=='*') {
            if (i - 1 >= 0)
            { // zgornje sosede
                if (j - 1 >= 0 && j != COLUMNS - i - 1)
                { // zgornje LEVA
                    sosede[0][0] = j - 1;
                    sosede[0][1] = i - 1;
                    mapped_sosede[0] = (i - 1) * COLUMNS - mappJ - 1;
                }
                else
                { // ni leve sosede
                    sosede[0][0] = -1;
                    sosede[0][1] = -1;
                }
                if (j + 1 < stolpci)
                { // zgoraj desna soseda
                    sosede[1][0] = j + 1;
                    sosede[1][1] = i - 1;
                    mapped_sosede[1] = (i - 1) * COLUMNS + mappJ + 1;
                }
                else
                { // zgoraj ni desne sosede
                    sosede[1][0] = -1;
                    sosede[1][1] = -1;
                }
            }
            else
            { // zgornjih sosed NI
                sosede[0][0] = -1;
                sosede[0][1] = -1;
                sosede[1][0] = -1;
                sosede[1][1] = -1;
            }

            if (j - 2 >= 0 && j != COLUMNS - i - 1)
            { // ISTA VRSTICA; LEVA
                sosede[2][0] = j - 2;
                ;
                sosede[2][1] = i;
                mapped_sosede[2] = i * COLUMNS + mappJ - 2;
            }
            else
            { // ni leve sosede
                sosede[2][0] = -1;
                sosede[2][1] = -1;
            }
            if (j + 2 < stolpci && j != stolpci - i)
            { // DESNA soseda
                sosede[3][0] = j + 2;
                sosede[3][1] = i;
                mapped_sosede[3] = i * COLUMNS + mappJ + 2;
            }
            else
            { //  ni desne sosede
                sosede[3][0] = -1;
                sosede[3][1] = -1;
            }

            if (i + 1 < vrstice && j != COLUMNS - i - 1)
            { // spodnje  sosede  //i+1 || j-1 in j+1
                if (j - 1 >= 0)
                { // leva
                    sosede[4][0] = j - 1;
                    sosede[4][1] = i + 1;
                    mapped_sosede[4] = (i + 1) * COLUMNS + mappJ - 1;
                }
                else
                { // ni leve sosede
                    sosede[4][0] = -1;
                    sosede[4][1] = -1;
                }
                if (j + 1 < stolpci && j != stolpci - i)
                { // desna spodnja soseda
                    sosede[5][0] = j + 1;
                    sosede[5][1] = i + 1;
                    mapped_sosede[5] = (i + 1) * COLUMNS + mappJ + 1;
                }
                else
                { // ni desne sosede
                    sosede[5][0] = -1;
                    sosede[5][1] = -1;
                }
            }
            else
            { // ni spodnjih sosed
                sosede[4][0] = -1;
                sosede[4][0] = -1;
                sosede[5][1] = -1;
                sosede[5][1] = -1;
            }
            // vpis v strukturu
            cells[index].neighbors = sosede;
            cells[index].i = j; // kontra?
            cells[index].j = i;

            // printTab(sosede,j,mappIdx);
            printmaped(sosede, j, i, mapped_sosede);
            printf("\n");
            // printf("index: %d - > %d\n",index,mappJ);
            index++;
            for (int x = 0; x < 6; x++)
            {
                sosede[x][0] = -2;
                sosede[x][1] = -2;
            }
        }
    }
    // printf("%d \n",index);
    for (int i = 0; i < 6; i++)
    {
        free(sosede[i]);
    }
    free(sosede);
}

int main()
{

    int array_size = COLUMNS * ROWS;
    printHexagon(6);
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