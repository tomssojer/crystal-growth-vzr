#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "constants.h"
#include "model.h"


void printHexagon(int size) { // indeksi sosed so [y-1][x-1][x+1] in [y][x-1][x-2] in [y+1][x-1][x-2]
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLUMNS - i - 1; j++) {
            printf(".");
            //printf("%d",j);
        }
        for (j = 0; j < size; j++) {

            if(j==size-1) {
                printf("*");
            } else {
                printf("*.");
            }
        }
        for (j = size - i ; j < size  ; j++) {
            printf("%d",j);
        }
        printf("\n");
    }
}
void printTab(int **tab) {
    for(int i=0;i<6;i++) {
        printf("%d_(%2d,%2d) ",i,tab[i][0],tab[i][1]);
    }
}
void neighbours(char **plosca) {
    //printf("dela");
    int** sosede = (int**)malloc(6 * sizeof(int*));// indeks 0 <- y-1,x-1 ; 1 <- y-1, x+1
    for (int i= 0; i < 6; i++) {
        sosede[i] = (int*)malloc(2 * sizeof(int));
        sosede[i][0]=-2;
        sosede[i][1]=-2;
    }
    printf("dela\n");
    int vrstice=ROWS;
    int stolpci=COLUMNS*3-2;
    for(int i=0;i<vrstice;i++) {
        for (int j = 0; j <   (stolpci) ; j++) {
            if(plosca[i][j]=='*') {

            if(i-1 >=0) { // zgornje sosede
                if(j-1>=0) { //zgornje LEVA
                    sosede[0][0] = i-1;
                    sosede[0][1] = j-1;
                } else if(j-1<0) { //ni leve sosede
                    sosede[0][0] = -1;
                    sosede[0][1] = -1;
                }
                if(j+1<stolpci) { // zgoraj desna soseda
                    sosede[1][0] = i-1;
                    sosede[1][1] = j+1;
                } else if(j+1>=stolpci) { // zgoraj ni desne sosede
                    sosede[1][0] = -1;
                    sosede[1][1] = -1;
                }
            } else if(i-1<0) { // zgornjih sosed NI
                sosede[0][0] = -1;
                sosede[0][1] = -1;
                sosede[1][0] = -1;
                sosede[1][1] = -1;
            }

            if(j-1>=0) { // ISTA VRSTICA; LEVA
                sosede[2][0] = i;
                sosede[2][1] = j-2;
            } else if(j-1<0) { //ni leve sosede
                sosede[2][0] = -1;
                sosede[2][1] = -1;
            }
            if(j+1<stolpci) { // DESNA soseda
                sosede[3][0] = i;
                sosede[3][1] = j+2;
            } else if(j+1>=stolpci) { //  ni desne sosede
                sosede[3][0] = -1;
                sosede[3][1] = -1;
            }

            if(i+1 <vrstice) { // spodnje  sosede
                if(j-1>=0) { //leva
                    sosede[4][0] = i+1;
                    sosede[4][1] = j-1;
                } else if(j-1<0) { //ni leve sosede
                    sosede[4][0] = -1;
                    sosede[4][1] = -1;
                }
                if(j+1<stolpci) {
                    sosede[5][0] = i+1;
                    sosede[5][1] = j+1;
                } else if(j+1>=stolpci) { //ni desne sosede
                    sosede[5][0] = -1;
                    sosede[5][1] = -1;
                }
            } else if(i+1>=vrstice) { // ni spodnjih sosed
                sosede[4][0] = -1;
                sosede[4][0] = -1;
                sosede[5][1] = -1;
                sosede[5][1] = -1;
            }
            printTab(sosede);
            printf("\n");
            }
        }


    }

    for (int i= 0; i < 6; i++) {
        free(sosede[i]);
    }
    free(sosede);
}
char** calculate() {
    char** matrix = (char**)malloc(ROWS * sizeof(char*));
    for(int i=0;i<ROWS;i++) {
        matrix[i] = (char*)malloc((3*COLUMNS-2) * sizeof(char));
        for(int j=0;j<3*COLUMNS-2;j++) {
            if(j >COLUMNS-i-2 && j<(COLUMNS)*3-i-2) {
                    matrix[i][j] = '*';
                    //sprintf(&matrix[i][j], "%d", j);
                    matrix[i][j+1] = '.';
                    j++;


            } else {
                matrix[i][j] = '.';
            }

        }
    }
    for(int i=0;i<ROWS;i++) {
        for(int j=0;j<3*COLUMNS-2;j++) {
            printf("%c",matrix[i][j]);
        }
        printf("\n");
    }
    return(matrix);
}

int main() {

    char** matrix=calculate();
    for (int i  = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
    neighbours(matrix);
    free(matrix);
    return 0;
}

int main() {

    int N = 5;
    float* X;
    float* Y;
    int array_size = (3 * COLUMNS - 2)* ROWS;

    // Definicija arraya s structi
    struct Cell cell;
    struct Cell* cells = malloc(array_size * sizeof *cells);
    // struct Cell cell_array[array_size];

    make_hex_grid(N, &X, &Y);

    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("(%f, %f) ", X[i * N + j], Y[i * N + j]);
            initialize(cell);
        }
        printf("\n");
    }

    // Free allocated memory
    free(X);
    free(Y);

    return 0;
}