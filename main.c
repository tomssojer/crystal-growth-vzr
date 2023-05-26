#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "constants.h"
#include "model.h"

void make_hex_grid(int N, float** X, float** Y) {
    // Allocate memory for X and Y arrays
    *X = (float*)malloc(N * N * sizeof(float));
    *Y = (float*)malloc(N * N * sizeof(float));

    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            (*X)[i * N + j] = (float)j;
            (*Y)[i * N + j] = (float)i;
        }
    }

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            (*Y)[i * N + j] *= sqrt(3) / 2;
        }
    }

    for (i = 0; i < N; i += 2) {
        for (j = 0; j < N; j++) {
            (*X)[i * N + j] += 0.5;
        }
    }
}

void printHexagon(int size) {
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

void calculate(int size) {
   // int i, j;
    char **board = (char**)malloc(ROWS * sizeof(char*));
    char** matrix = (char**)malloc(ROWS * sizeof(char*));
    for (int i = 0; i < ROWS; i++) {
        matrix[i] = (char*)malloc((3*COLUMNS-2) * sizeof(char));
        board[i] = (char*)malloc((3*COLUMNS-2) * sizeof(char));
        for (int j = 0; j < COLUMNS - i - 1; j++) {
            printf(".");
            board[i][j] =  (char)".";
            matrix[i][j] = 'A';
            //printf("%d",j);
        }
        for (int j = 0; j < 2*size; j+=2) {

            if(j==size-1) {
                board[i][j] =  "*";
                //printf("*");
            } else {
                board[i][j] =  "*";
                board[i][j+1] =  ".";
               // printf("*.");
            }
        }
        for (int j = size - i ; j < size  ; j++) {
            //printf("%d",j);
            board[i][j] =  ".";
        }
        //printf("\n");
    }
    board[0][0] = (char) "A";
    printf("%c",board[0][0]);
    for(int i=0;i<ROWS;i++) {
        for(int j=0;j<3*COLUMNS-2;j++) {
            printf("%c",board[i][j]);
        }
        printf("\n");
    }
    // Free the dynamically allocated memory
    for (int i  = 0; i < ROWS; i++) {
        free(board[i]);
    }
    free(board);


    matrix[0][0] = 'A';
    matrix[0][1] = 'B';
    printf("%c",matrix[0][0]);
    for (int i  = 0; i < ROWS; i++) {
        free(matrix[i]);
    }
}

int main() {
    int size=6;
    float neighbour_average = 0;

    // for (int i = 0; i < ROWS; i++)
    //     for (int j = 0; j < COLUMNS; j++)
    //     {
    //         neighbour_average = average(); 
    //         if (cell.type == 1 || cell.type == 2)
    //             cell.state = change_state(cell.type, cell.state, cell.average);

    //     }


    //printHexagon(size);
    calculate(6);
    return 0;
}
/*
int main() {
    int N = 5;
    float* X;
    float* Y;

    make_hex_grid(N, &X, &Y);

    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("(%f, %f) ", X[i * N + j], Y[i * N + j]);
        }
        printf("\n");
    }

    // Free allocated memory
    free(X);
    free(Y);

    return 0;
}*/
