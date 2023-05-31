// number of rows
#define ROWS 2500

// number of columns
#define COLUMNS ROWS

// total number of cells
#define NUM_CELLS ROWS *COLUMNS

// number of neighbors
#define NUM_NEIGHBORS 6

// diffusion coefficient
#define ALPHA 1.2

// background vapour level
#define BETA 0.5

// vapour addition
#define GAMMA 0.002

// število korakov simulacije
#define STEPS 1500

// število iteracij za izpis
#define STEPS_TO_DRAW 10

// GPU paralel
// 3412.7 ms  -> 1000*1000 polje, A = 1.2, B=0.68, G = 0.0048, STEPS=900
// 57439.346 ms CPU SERIAL