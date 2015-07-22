#include "forest.h"

#ifdef USING_MPI
const MPI_Comm comm = MPI_COMM_WORLD;
int myrank, size;
#endif

int main(int argc, char** argv) {

    int i, y, seed;
    args* myArgs;
    forest* f;
    cell** swapGrid;
    struct random_data* rand_state;
    char rand_buf[256];
    memset(rand_buf, 'q', (size_t)256);

    int startX = 0, startY = 0;
    int endX = 0, endY = 0;

    int* rands;

#ifdef USING_OMP
    int threads = omp_get_max_threads(); 
#else
    int threads = 1;
#endif

#ifdef USING_MPI
    //MPI_Request request[3];
    //MPI_Status* status = (MPI_Status*) malloc(sizeof(MPI_Status) * 3);
    MPI_Status status[3];

    memset(&status, 0, sizeof(MPI_Status) * 3);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(comm, &myrank);
    MPI_Comm_size(comm, &size);

    if (size != 4) {
        printf("Unfortunately this program has been hardcoded to run on 4 nodes.\nExiting...\n");
        MPI_Finalize();
        exit(1);
    }

#endif


    myArgs = parse_args(argc, argv);
    f = alloc_forest(myArgs);

    // set up which parts of the grid we're going to iterate over
#ifdef USING_MPI

   int r0startX = 0;
   int r0startY = 0;
   //int r0endX = f->dimX/2;
   int r0endY = f->dimY/2;

   //int r1startX = (f->dimX/2) + 1;
   int r1startX = (f->dimX/2);
   int r1startY = 0;
   //int r1endX = f->dimX;
   int r1endY = f->dimY/2;

   int r2startX = 0;
   //int r2startY = (f->dimY/2) + 1;
   int r2startY = (f->dimY/2);
   //int r2endX = f->dimX/2;
   int r2endY = f->dimY;

   //int r3startX = (f->dimX/2) + 1;
   int r3startX = (f->dimX/2);
   //int r3startY = (f->dimY/2) + 1;
   int r3startY = (f->dimY/2);
   //int r3endX = f->dimX;
   int r3endY = f->dimY;

    switch(myrank) {
        case 0:
           startX = 0;
           startY = 0;
           endX = f->dimX/2;
           endY = f->dimY/2;
           break;

        case 1:
           //startX = (f->dimX/2) + 1;
           startX = (f->dimX/2);
           startY = 0;
           endX = f->dimX;
           endY = f->dimY/2;
           break;

        case 2:
           startX = 0;
           //startY = (f->dimY/2) + 1;
           startY = (f->dimY/2);
           endX = f->dimX/2;
           endY = f->dimY;
           break;

        case 3:
           //startX = (f->dimX/2) + 1;
           startX = (f->dimX/2);
           //startY = (f->dimY/2) + 1;
           startY = (f->dimY/2);
           endX = f->dimX;
           endY = f->dimY;
           break;
    }

#else

    startX = 0;
    startY = 0;
    endX = f->dimX;
    endY = f->dimY;

#endif

    // ncurses requires some setup
    if (myArgs->output == NCURSES) {
       init_ncurses(endX, endY);
    }

    if (myArgs->logging) {
        myArgs->log = fopen("forest.log","w");
    }
    
    rand_state = (struct random_data*) malloc(sizeof(struct random_data) * threads);
    
    memset(rand_state, 0, sizeof(struct random_data) * threads);

    if (rand_state == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }

    for (int q = 0; q < threads; q++) {

        if (myArgs->output == VERIFY) {
            seed = 5;                   //deterministic
        } else {
#ifdef USING_MPI
            seed = time(NULL) ^ (q + myrank);
#else
            seed = time(NULL) ^ q;      //non-deterministic
#endif
        }

        initstate_r(seed, rand_buf, 256, &rand_state[q]);
        srandom_r(seed, &rand_state[q]);
    }
#ifdef USING_MPI
    //fprintf(stderr,"rank %d is working on x %d to %d and y %d to %d\n",myrank, startX, endX, startY, endY);
#endif


    rands = (int*) malloc(sizeof(int)*4);
    memset(rands, 0, sizeof(int)*4);
    // main loop
    for (i = 0; i < f->simLength; i++) {
        //fprintf(stdout,"rank %d got to timestep %d\n",myrank,f->time);
        //fflush();
        
            #pragma omp parallel for private(rands)
            for (y = startY; y < endY; y++) {
                for (int x = startX; x < endX; x+=4) {
                    
#ifdef USING_OMP
                    random_r(&rand_state[omp_get_thread_num()], &rands[0]);
                    random_r(&rand_state[omp_get_thread_num()], &rands[1]);
                    random_r(&rand_state[omp_get_thread_num()], &rands[2]);
                    random_r(&rand_state[omp_get_thread_num()], &rands[3]);
#else
                    random_r(&rand_state[0], &rands[0]);
                    random_r(&rand_state[0], &rands[1]);
                    random_r(&rand_state[0], &rands[2]);
                    random_r(&rand_state[0], &rands[3]);
#endif
                    cell_auto(f, x, y, f->oldGrid[y][x].status, &rands[0]);
                    cell_auto(f, x+1, y, f->oldGrid[y][x+1].status, &rands[1]);
                    cell_auto(f, x+2, y, f->oldGrid[y][x+2].status, &rands[2]);
                    cell_auto(f, x+3, y, f->oldGrid[y][x+3].status, &rands[3]);
                    }
                }

        if (myArgs->logging) {
            fprintf(myArgs->log, "%d %d %d\n", i, f->treeCount, f->burnCount);
        }

#ifdef USING_MPI

        int z = 0;

         int sendOne = 0, sendTwo = 0, sendThree = 0;
        //int err = 0;
        //size_t xferSize = sizeof(cell) * (f->dimX/2) * (f->dimY/2);
        int xferSize = (int)sizeof(cell) * (f->dimX/2) * (f->dimY/2);
        size_t rowSize = sizeof(cell) * (f->dimX/2);
        //fprintf(stderr,"dimx = %d, dimy = %d, xfersize = %d, rowsize = %d\n",f->dimX, f->dimY, (int)xferSize, (int)rowSize);
        //fprintf(stderr, "size of a cell is %d bytes\n",(int)sizeof(cell));
       
        //fprintf(stderr, "there should be %d cells in the sendGrid.\n",((f->dimX/2) * (f->dimY/2)));
        //fprintf(stderr, "but we are copying in %d times %d cells.\n", (endY - startY), f->dimX/2);

        // copy worked-on part of grid to sendGrid for sending
        for (int e = startY; e < endY; e++) {
            memcpy(f->sendGrid[z], &f->newGrid[e][startX], rowSize);
            z++;
        }
        
        switch(myrank) {

            case 0:
               

                sendOne = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 1, 0, *f->recvGrid1, xferSize, MPI_BYTE, 1, 0, comm, &status[0]);
                sendTwo = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 2, 0, *f->recvGrid2, xferSize, MPI_BYTE, 2, 0, comm, &status[1]);
                sendThree = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 3, 0, *f->recvGrid3, xferSize, MPI_BYTE, 3, 0, comm, &status[2]);
                
               
                //checksum_grid(f->recvGrid1, f->dimX/2, f->dimY/2);
                // copy from recvgrids
                z = 0;
                for (int e = r1startY; e < r1endY; e++) {
                    memcpy(&f->newGrid[e][r1startX], f->recvGrid1[z], rowSize);
                    z++;
                }
                
                z = 0;
                for (int e = r2startY; e < r2endY; e++) {
                    memcpy(&f->newGrid[e][r2startX], f->recvGrid2[z], rowSize);
                    z++;
                }

                z = 0;
                for (int e = r3startY; e < r3endY; e++) {
                    memcpy(&f->newGrid[e][r3startX], f->recvGrid3[z], rowSize);
                    z++;
                }
                
                break;


            case 1:
                

                        sendOne = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 0, 0, *f->recvGrid1, xferSize, MPI_BYTE, 0, 0, comm, &status[0]);
                        sendTwo = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 3, 0, *f->recvGrid2, xferSize, MPI_BYTE, 3, 0, comm, &status[1]);
                        sendThree = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 2, 0, *f->recvGrid3, xferSize, MPI_BYTE, 2, 0, comm, &status[2]);
                
        
                //checksum_grid(f->sendGrid, f->dimX/2, f->dimY/2);
                // copy from recvgrids
                z = 0;
                for (int e = r0startY; e < r0endY; e++) {
                    memcpy(&f->newGrid[e][r0startX], f->recvGrid1[z], rowSize);
                    z++;
                }

                z = 0;
                for (int e = r3startY; e < r3endY; e++) {
                    memcpy(&f->newGrid[e][r3startX], f->recvGrid2[z], rowSize);
                    z++;
                }

                z = 0;
                for (int e = r2startY; e < r2endY; e++) {
                    memcpy(&f->newGrid[e][r2startX], f->recvGrid3[z], rowSize);
                    z++;
                }

                break;

            case 2:
                
                        sendOne = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 3, 0, *f->recvGrid1, xferSize, MPI_BYTE, 3, 0, comm, &status[0]);
                        sendTwo = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 0, 0, *f->recvGrid2, xferSize, MPI_BYTE, 0, 0, comm, &status[1]);
                        sendThree = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 1, 0, *f->recvGrid3, xferSize, MPI_BYTE, 1, 0, comm, &status[2]);
               
                // copy from recvgrids
                
                z = 0;
                for (int e = r3startY; e < r3endY; e++) {
                    memcpy(&f->newGrid[e][r3startX], f->recvGrid1[z], rowSize);
                    z++;
                }

                z = 0;
                for (int e = r0startY; e < r0endY; e++) {
                    memcpy(&f->newGrid[e][r0startX], f->recvGrid2[z], rowSize);
                    z++;
                }
               
                z = 0;
                for (int e = r1startY; e < r1endY; e++) {
                    memcpy(&f->newGrid[e][r1startX], f->recvGrid3[z], rowSize);
                    z++;
                }

                break;

            case 3:
               

                        sendOne = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 2, 0, *f->recvGrid1, xferSize, MPI_BYTE, 2, 0, comm, &status[0]);
                        sendTwo = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 1, 0, *f->recvGrid2, xferSize, MPI_BYTE, 1, 0, comm, &status[1]);
                        sendThree = MPI_Sendrecv(*f->sendGrid, xferSize, MPI_BYTE, 0, 0, *f->recvGrid3, xferSize, MPI_BYTE, 0, 0, comm, &status[2]);

                // copy from recvgrids
                
                z = 0;
                for (int e = r2startY; e < r2endY; e++) {
                    memcpy(&f->newGrid[e][r2startX], f->recvGrid1[z], rowSize);
                    z++;
                }

                z = 0;
                for (int e = r1startY; e < r1endY; e++) {
                    memcpy(&f->newGrid[e][r1startX], f->recvGrid2[z], rowSize);
                    z++;
                }
                
                z = 0;
                for (int e = r0startY; e < r0endY; e++) {
                    memcpy(&f->newGrid[e][r0startX], f->recvGrid3[z], rowSize);
                    z++;
                }

                break;

            default:
                fprintf(stderr,"your switch case is broke\n");
                break;
        }
        if (sendOne || sendTwo || sendThree) {
            fprintf(stderr,"i'm %d, %d %d %d\n",myrank, sendOne, sendTwo, sendThree);
        }
        // sanity check
        /*
        for (int zz = 0; zz < f->dimY; zz++) {
            f->newGrid[25][zz].status = BURN;
            f->newGrid[26][zz].status = BURN;
            f->newGrid[27][zz].status = BURN;
            f->newGrid[28][zz].status = BURN;
        }
        */
        // only one rank needs to output 
        if (myrank == 0) {
            myArgs->out(f);
        }
 
#else

        myArgs->out(f);       // it's a function pointer!

#endif

        f->time++;
        swapGrid = f->oldGrid;
        f->oldGrid = f->newGrid;
        f->newGrid = swapGrid;

    }

    if (myArgs->output == NCURSES) {
        endwin();
    }
    if (myArgs->logging) {
        fclose(myArgs->log);
    }

#ifdef USING_MPI
    text_output(f->newGrid, f->dimX, f->dimY);
    //checksum_grid(f->newGrid, f->dimX, f->dimY);
    MPI_Finalize();
#endif

    return 0;

}

void text_output(cell** grid, int x, int y) {

    char* out = (char*) malloc(sizeof(char) * x * y + (y + 1));
    int c = 0;
    int arr;

    for (int s = 0; s < y; s++) {
        for (int t = 0; t < x; t++) {
            switch(grid[s][t].status) {
                case EMPTY:
                    out[c++] = ' ';
                    break;

                case TREE:
                    out[c++] = 'T';
                    break;

                case BURN:
                    out[c++] = 'B';
                    break;

                default:
                    fprintf(stderr,"well that's not good - %d %d\n",s,t);
                    break;
            }
        }
        out[c++] = '\n';
    }
    out[c++] = '\0';
#ifdef USING_MPI
    arr = myrank;
#else
    arr = 0;
#endif

    fprintf(stderr,"hi i'm %d and this is what i have for you\n%s\n",arr,out);
    free(out);
}


void out_null(forest* f) {

    return;
}

void out_verify(forest* f) {

    int x, y;
    int treeCheck = 0;
    int burnCheck = 0;
    char* strGrid;
    char salt[] = "$1$cosc3500";
    char* res;

    if (f->time == f->simLength) {
        fprintf(stderr,"Calculating grid integrity...\n");
        strGrid = (char*)malloc((sizeof(char) * f->dimX * f->dimY) + 1);
        if (strGrid == NULL) {
            fprintf(stderr,"malloc failed!\n");
            exit(17);
        }
        strGrid[f->dimX * f->dimY] = '\0';

        for (y = 0; y < f->dimY; y++) {
            for (x = 0; x < f->dimX; x++) {
                switch(f->newGrid[y][x].status) {
                    
                    case EMPTY:
                        strGrid[(y * f->dimY) + x] = ' ';
                        break;
                    
                    case TREE:
                        strGrid[(y * f->dimY) + x] = 'T';
                        treeCheck++;
                        break;

                    case BURN:
                        strGrid[(y * f->dimY) + x] = 'B';
                        burnCheck++;
                        break;

                    default:
                        fprintf(stderr, "bad tree status at %d %d\n",x,y);
                        break;
                }
            }
        }

        if (treeCheck == f->treeCount && burnCheck == f->burnCount) {
            fprintf(stderr, "treeCount and burnCount okay\n");
        } else {
            fprintf(stderr, "error: treeCount = %d, treeCheck = %d\n", f->treeCount, treeCheck);
            fprintf(stderr, "error: burnCount = %d, burnCheck = %d\n", f->burnCount, burnCheck);
        }
            

        res = crypt(strGrid, salt);
        fprintf(stderr,"Grid checksum is: %s\n",res);
        
    
    }
    return;
}

void checksum_grid(cell** grid, int x, int y) {

    int i, j;
    int e = 0, t = 0, b = 0;
    char salt[] = "$1$cosc3500";
    char* res;
    char* strGrid = (char*)malloc((sizeof(char) * x * y) + 1);

    if (strGrid == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }

    strGrid[x * y] = '\0';
    int c = 0;
    for (i = 0; i < y; i++) {
        for (j = 0; j < x; j++) {
            switch(grid[i][j].status) {
                    
                case EMPTY:
                    strGrid[c++] = ' ';
                    e++;
                    break;
                    
                case TREE:
                    strGrid[c++] = 'T';
                    t++;
                    break;

                case BURN:
                    strGrid[c++] = 'B';
                    b++;
                    break;

                default:
                    strGrid[c++] = '.';
                    fprintf(stderr, "bad tree status at %d %d, status is %d\n",x,y,grid[i][j].status);
                    break;
            }
        }
    }
    strGrid[c++] = '\0'; 
    res = crypt(strGrid, salt);
#ifdef USING_MPI
    fprintf(stderr,"rank %d, e=%d, t=%d, b=%d, %s\n", myrank, e, t, b, res);
    //fprintf(stderr,"strgrid is %d chars long and c is %d \n",(int)strlen(strGrid),c);
#endif
    free(strGrid);
}
 

/* perform cellular automata rules on each cell in the forest */
void cell_auto(forest* f, int x, int y, int mode, int* rand) {

    int dx,dy;
    int rx,ry;
    cell* c = &f->oldGrid[y][x];
    cell* n = &f->newGrid[y][x];
    
    switch(mode) {
        case EMPTY:
            if ( (*rand % 10000) < GROWCHANCE) {
                n->status = TREE;
                n->age = 0;
                #pragma omp atomic
                f->treeCount++;
            } else {
                n->status = EMPTY;
            }
            return;

        case TREE:

            // chance of catching on fire from neighbouring trees
            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {
                    
                    if (dx == 0 && dy == 0) { // don't examine yourself
                        continue;
                    }
                    rx = x + dx;
                    ry = y + dy;
                    if (rx >= 0 && rx < f->dimX && 
                        ry >= 0 && ry < f->dimY) { // bounds checking

                        if (f->oldGrid[ry][rx].status == BURN &&
                            (*rand % 720) < c->age) {
                            n->status = BURN;
                            n->age = 0;
                            n->burnTime = BURNLENGTH;
                            #pragma omp atomic
                            f->burnCount++;
                            f->treeCount--;
                            return; // nothing else to do
                        }
                    }
                }
            }
            // chance of bursting into flames spontaneously
            if ( (*rand % 150000) < BURNCHANCE) {
                n->status = BURN;
                n->burnTime = BURNLENGTH;
                #pragma omp atomic
                f->burnCount++;
                f->treeCount--;
                return;
            }
            
            
            // if we get here, it's a tree and is still a tree...?
            n->status = TREE;
            
            if (c->age < 240) {
                n->age = c->age + 4;
            }
            return;

        case BURN:
            if (c->burnTime == 0) {
                n->status = EMPTY;
                n->age = 0;
                #pragma omp atomic
                f->burnCount--;
            } else {
                n->status = BURN;
                n->burnTime = c->burnTime - 1;
            }
            return;

        default:
            fprintf(stderr,"unitialised cell at %d,%d, cell has status %d\n", x, y, mode);
            return;
    }
}

/* allocate and populate the forest struct, including initial trees */
forest* alloc_forest(args* myArgs) {

    bitmap_t* png;
    int x = myArgs->dimX;
    int y = myArgs->dimY;

    forest* f = (forest*)malloc(sizeof(forest));
    if (f == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }
    
    f->newGrid = alloc_grid(x, y);
    f->oldGrid = alloc_grid(x, y);

#ifdef USING_MPI
    f->sendGrid = alloc_2d_grid(x/2, y/2);
    f->recvGrid1 = alloc_2d_grid(x/2, y/2);
    f->recvGrid2 = alloc_2d_grid(x/2, y/2);
    f->recvGrid3 = alloc_2d_grid(x/2, y/2);
#endif

    if (myArgs->output == PNG) {
        // alloc pixel array
        png = (bitmap_t*)malloc(sizeof(bitmap_t));
        if (png == NULL) {
            fprintf(stderr,"malloc failed!\n");
            exit(17);
        }
        png->pixels = calloc(sizeof(pixel_t), x * y);
        png->width = x;
        png->height = y;
        f->png = png;
    }

    f->dimX = x;
    f->dimY = y;
    f->treeCount = 0;
    f->burnCount = 0;
    f->time = 1;
    f->simLength = myArgs->simLength;

    return f;
}

cell** alloc_grid(int x, int y) {

    int i;

    cell** grid = (cell**)malloc(sizeof(cell*) * y);
    if (grid == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }

    // i'm so sorry valgrind
    memset(grid, 0, sizeof(cell*) * y);

    for (i = 0; i < y; i++) {
        grid[i] = (cell*)malloc(sizeof(cell) * x);
        if (grid[i] == NULL) {
            fprintf(stderr,"malloc failed!\n");
            exit(17);
        }
        
        memset(grid[i], 0, sizeof(cell) * x);

        for (int j = 0; j < x; j++) {
            grid[i][j].status = EMPTY;
        }
    }

    return grid;
}


// returns a 2d array that is allocated as one contiguous
// block of memory. cool trick!
cell** alloc_2d_grid(int x, int y) {


    cell* data = (cell*)malloc(sizeof(cell) * x * y);
    if (data == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }

    // according to valgrind i am a horrible person
    memset(data, 0, sizeof(cell) * x * y);

    cell** array = (cell**)malloc(sizeof(cell*) * y);
    if (array == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }

    for (int i = 0; i < y; i++) {
        array[i] = &(data[x * i]);
    }

    return array;
}

/* parse argv for valid parameters and return as a struct */
args* parse_args(int argc, char** argv) {
    
    args* myArgs = (args*)malloc(sizeof(args));
    if (myArgs == NULL) {
        fprintf(stderr,"malloc failed!\n");
        exit(17);
    }
    myArgs->logging = 0;
    switch(argc) {
        case 6:
            if (!strcmp(argv[5],"log")) {
                myArgs->logging = 1;
            } else {
                printf("Usage: forest dimensionX dimensionY output simlength {log}\n");
            }
        case 5:
            myArgs->dimX = atoi(argv[1]);
            myArgs->dimY = atoi(argv[2]);
            if (myArgs->dimX % 8 != 0 || myArgs->dimY % 8 != 0) {
                printf("Error: please use mod8 grid dimensions\n");
                printf("Usage: forest dimensionX dimensionY output simlength {log}\n");
                exit(1);
            }
            myArgs->simLength = atoi(argv[4]);

            if (!strcmp(argv[3],"ncurses")) {
                myArgs->output = NCURSES;
                myArgs->out = out_ncurses;
            } else if (!strcmp(argv[3],"png")) {
                myArgs->output = PNG;
                myArgs->out = out_png;
            } else if (!strcmp(argv[3],"null")) {
                myArgs->output = NULLOUT;
                myArgs->out = out_null;
            } else if (!strcmp(argv[3],"verify")) {
                myArgs->output = VERIFY;
                myArgs->out = out_verify;
            } else {
                printf("Error: output should be one of 'png' 'ncurses' 'null' 'verify' 'log'\n");
                printf("Usage: forest dimensionX dimensionY output simlength {log]\n");
                exit(1);
            }

            return myArgs;
        default:
            printf("Usage: forest dimensionX dimensionY output simlength\n");
            exit(1);
    }

    return myArgs;
    // shouldn't reach here but whatever
    exit(1);
}

