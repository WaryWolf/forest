#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <ncurses.h>
#include <crypt.h>
#include <errno.h>

#ifdef USING_OMP
#include <omp.h>
#endif

#ifdef USING_MPI
#include <mpi.h>

#define HORIZONTAL 25
#define VERTICAL 50
#define DIAGONAL 75

#endif

#define EMPTY 0
#define TREE 1
#define BURN 2


#define NCURSES 5
#define PNG 6
#define NULLOUT 7
#define VERIFY 8

// chances for certain events to happen, based on probability out of 10000.
// BURNCHANCE is probability in 1000000 (one million).

#define TREECHANCE 500  // chance to be a tree on forest initialisation

#define GROWCHANCE 100  // chance for empty cell to grow a tree each timestep

#define BURNCHANCE 1   // chance for a new fire to spawn on a tree each timestep

#define SPREADCHANCE 3000   // chance for fire to spread to a neighbouring tree

#define BURNLENGTH 3    // timesteps taken for tree to burn into empty cell


/* png stuff */

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t; 

typedef struct {
    pixel_t* pixels;
    size_t width;
    size_t height;
} bitmap_t;



/* structs */

typedef struct {        // struct for each cell in the forest

    int status;         // 0 = empty, 1 = tree, 2 = burning
    int burnTime;       // time left until cell finishes burning
    int age;            // tree age, begins at 0
} cell;

typedef struct {        // struct for an entire forest

    int dimX;           // dimensions of forest
    int dimY;
    cell** newGrid;      // 2d array (malloced) of cells
    cell** oldGrid;      // alternating pair of grids
    int treeCount;      // number of trees in the forest
    int burnCount;      // number of burning cells in the forest
    int time;           // current timestep
    int simLength;      // maximum timestep
    bitmap_t* png;      // pixel array for output
#ifdef USING_MPI
    cell** sendGrid;    // extra grids used for sending
    cell** recvGrid1;   // and receiving from other ranks
    cell** recvGrid2;
    cell** recvGrid3;
#endif

} forest;


typedef struct {        // commandline args struct

    int dimX;            // dimensions of forest
    int dimY;
    int rules;          // which ruleset to use TODO
    int output;         // output method (png or terminal)
    void (*out)(forest*);   // function pointer for output
    int simLength;      // length of simulation in timesteps
    int logging;        // true if logging has been enabled
    FILE* log;          // log file pointer
} args;

/* function prototypes */

args* parse_args(int argc, char** argv);

void print_usage();
forest* alloc_forest(args* myArgs);

void cell_auto(forest* f, int i, int j, int mode, int* rand);

char* direction(int x, int y);

cell** alloc_grid(int x, int y);

cell** alloc_2d_grid(int x, int y);

void checksum_grid(cell** grid, int x, int y);

void text_output(cell** grid, int x, int y);

void out_null(forest* f);

void out_verify(forest* f);

//cell* cell_at(int x, int y, int width, cell** grid);

void out_png(forest* f);
//int save_png_to_file(bitmap_t* bitmap, const char *path);
//pixel_t* pixel_at(bitmap_t* bitmap, int x, int y);


/* ncurses stuff */
void init_ncurses(int x, int y);
void out_ncurses(forest* f);
