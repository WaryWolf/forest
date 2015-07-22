all: forest

CFlags=-Wall -pedantic -std=gnu99 -lpng -lncurses -lcrypt

forest: forest.c forest.h png.c ncurses.c
	gcc -O2 forest.c png.c ncurses.c -o forest $(CFlags)
    
omp: forest.c forest.h png.c ncurses.c
	gcc -O2 forest.c png.c ncurses.c -o forest-omp $(CFlags) -fopenmp -DUSING_OMP=1

mpi-moss: forest.c forest.h png.c ncurses.c
	mpicc -O2 forest.c png.c ncurses.c -o forest-mpi $(CFlags) -fopenmp -DUSING_OMP=1 -DUSING_MPI=1

mpi-savanna: forest.c forest.h png.c ncurses.c
	 mpiicc -O2 forest.c png.c ncurses.c -o forest-mpi $(CFlags) -fopenmp -DUSING_OMP=1 -DUSING_MPI=1

movie:
	ffmpeg -i out/img%05d.png -vcodec libx264 -threads 4 -crf 23 output-`date +"%Y-%m-%d_%H-%M"`.mp4 || \
	avconv -i out/img%05d.png -vcodec libx264 -threads 4 -crf 23 output-`date +"%Y-%m-%d_%H-%M"`.mp4

lossmovie:
	ffmpeg -i out/img%05d.png -vcodec libx264 -threads 4 -crf 0 output-`date +"%Y-%m-%d_%H-%M"`.mp4 || \
	avconv -i out/img%05d.png -vcodec libx264 -threads 4 -crf 0 output-`date +"%Y-%m-%d_%H-%M"`.mp4

clean:
	rm -rf forest forest-mpi forest-omp forest.log report*.aux report*.log report*.toc report*.pdf output*.mp4 out/*

report-1: tex/report-1.tex
	pdflatex tex/report-1.tex && pdflatex tex/report-1.tex 

report-2: tex/report-2.tex
	pdflatex tex/report-2.tex && pdflatex tex/report-2.tex 

report-3: tex/report-3.tex
	pdflatex tex/report-3.tex && pdflatex tex/report-3.tex 

modload-savanna:
	 module load intel-mpi intel-cc-13 pbs ffmpeg valgrind subversion 
