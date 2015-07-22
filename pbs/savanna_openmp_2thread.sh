#!/bin/bash
#PBS -N mpi-test
#PBS -q workq
#PBS -A uq-COSC3500
#PBS -lwalltime=00:10:00
#PBS -lnodes=1:ppn=8
#PBS -lmem=1gb

. /usr/share/modules/init/bash

#module load intel-mpi intel-cc-13
#module load mpich2

cd $PBS_O_WORKDIR
export TIMEFORMAT="%E sec"

export OMP_NUM_THREADS=2
export DIM=2560
export DIM2=1440
export LENGTH=1000

echo -n "forest with $DIM x $DIM2 grid for $LENGTH timesteps on $OMP_NUM_THREADS threads WITHOUT MPI took: " >&2
#time ./forest $DIM $DIM null 500
time forest-omp $DIM $DIM2 null $LENGTH
