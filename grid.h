#include <stdio.h>
#include "constants.h"

typedef struct Cell
{
    // Type of cell (0 - frozen, 1 - boundary, 2 - unreceptive, 3 - edge)
    int type;

    // Amount of water
    double state;

    // Neighbors of the cell [6] - [i1, i2, ...]
    int neighbors[NUM_NEIGHBORS];
} Cell;

void set_type_boundary_serial(Cell *cells, int j)
{

    for (int i = 0; i < NUM_NEIGHBORS; i++)
    {
        int sosed = cells[j].neighbors[i];
        if (sosed >= 0)
        {
            if (cells[sosed].type == 0)
            {
                cells[j].type = 1;
                break;
            }
        }
    }
}

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

void init_grid(Cell *cells)
{
    //  sosede velikosti 6 sosed -2(x,y) ;
    //  index 0 ZGORAJ LEVO index 1 ZGORAJ DESNO | Y-1, X-1 X+1
    //  index 0        LEVO index 1        DESNO | Y  , X-2 X+2
    //  index 0 SPODAJ LEVO index 5 SPODAJ DESNO | Y+1, X-1 X+1

    int *mapped_sosede = (int *)malloc(NUM_NEIGHBORS * sizeof(int)); // indeks 0 <- y-1,x-1 ; 1 <- y-1, x+1

    int index = 0;
    int vrstice = ROWS;
    int stolpci = COLUMNS * 3 - 2;

    for (int i = 0; i < vrstice; i++)
    {
        // Definicija ničelnih elementov v heksagonalni strukturi
        int null_elements = COLUMNS - i - 1;

        for (int j = null_elements; j < stolpci - i; j += 2)
        {
            //   Nastavimo dummy vrednosti v array
            for (int k = 0; k < NUM_NEIGHBORS; k++)
            {
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

                    mapped_sosede[0] = (i - 1) * COLUMNS + map_top;
                }
                // Zgornja desna
                if (j + 1 < stolpci)
                {
                    mapped_sosede[1] = (i - 1) * COLUMNS + map_top + 1;
                }
            }

            ////////////////////////
            // Sosede v isti vrstici
            ////////////////////////

            // Leva
            if (j - 2 >= 0 && j != COLUMNS - i - 1)
            {
                mapped_sosede[2] = i * COLUMNS + map_current - 2;
            }

            // Desna
            if (j + 2 < stolpci - i && j != stolpci - i)
            {
                mapped_sosede[3] = i * COLUMNS + map_current;
            }

            /////////////////
            // Spodnje sosede
            /////////////////

            if (i + 1 < vrstice)
            {
                // Spodnja leva
                if (j - 1 >= 0)
                {
                    mapped_sosede[4] = (i + 1) * COLUMNS + map_bottom;
                }

                // Spodnja desna
                if (j + 1 < stolpci && j != stolpci - i)
                {
                    mapped_sosede[5] = (i + 1) * COLUMNS + map_bottom + 1;
                }
            }

            //  Vpis v strukturo
            memcpy(cells[index].neighbors, mapped_sosede, sizeof(int) * NUM_NEIGHBORS);
            index++;
        }
    }

    free(mapped_sosede);

    // nastavitev ledene celice in sosed
    int position = ROWS * ROWS / 2 + COLUMNS / 2;
    cells[position].type = 0;
    set_type_boundary(cells, cells[position].neighbors);
}

// function for visualization of board
void draw_board(Cell *cells)
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

                j++;
                index++;
            }
            else
                printf(" ");
        }
        printf("\n");
    }
}

void write_to_file(Cell *cells, FILE *file)
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
                int type = cells[index].type;
                fprintf(file, "%d ", i);
                fprintf(file, "%d ", j);
                fprintf(file, "%d ", type);

                j++;
                index++;
            }
        }
    }
    fprintf(file, "\n");
}
