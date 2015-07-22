#include "forest.h"

void init_ncurses(int x, int y) {


    //system("resize -s 50 50");
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_RED);
    init_pair(3, COLOR_GREEN, COLOR_GREEN);
    wresize(stdscr, x, y);
}

void out_ncurses(forest* f) {


    int x, y, s, t;
    cell* c;
    s = f->dimX;
    t = f->dimY;
    erase();
    
    for (x = 0; x < s; x++) {
        for (y = 0; y < t; y++) {
            c = &f->newGrid[x][y];
            switch(c->status) {
                case EMPTY:
                    attron(COLOR_PAIR(1));
                    mvprintw(x,y,"X");
                    attroff(COLOR_PAIR(1));
                    break;

                case BURN:
                    attron(COLOR_PAIR(2));
                    mvprintw(x,y,"X");
                    attroff(COLOR_PAIR(2));
                    break;

                case TREE:
                    attron(COLOR_PAIR(3));
                    mvprintw(x,y,"X");
                    attroff(COLOR_PAIR(3));
                    break;

                default:
                    mvprintw(x,y,"*");
                    break;
            }
        }
        printw("\n");
    }

    refresh();
    usleep(100000);
    //getch();
    //endwin();

    //return 0;


}
