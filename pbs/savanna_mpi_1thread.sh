#!/bin/bash
#PBS -N mpi-test
#PBS -q workq
#PBS -A uq-COSC3500
#PBS -lwalltime=00:10:00
#PBS -lnodes=4:ppn=1
#PBS -lmem=500mb

. /usr/share/modules/init/bash

module load intel-mpi intel-cc-13 valgrind
#module load mpich2

cd $PBS_O_WORKDIR
export TIMEFORMAT="%E sec"

export OMP_NUM_THREADS=1
export DIM=2560
export DIM2=1440
export LENGTH=1000

echo -n "forest with $DIM x $DIM2 grid for $LENGTH timesteps on $OMP_NUM_THREADS threads using 4 MPI nodes took: " >&2
#time ./forest $DIM $DIM null 500
time mpirun -envall -np 4 forest-mpi $DIM $DIM2 null $LENGTH
