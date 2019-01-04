#!/bin/sh
#
#PBS -N teste
#PBS -l walltime=60:00
#PBS -l nodes=2:r641:ppn=32
#PBS -q mei

module load gcc/5.3.0
module load gnu/openmpi_eth/1.8.4
gcc -O3 -std=gnu99 -fopenmp -lm openMp.c -o openmp
gcc -O3 -std=gnu99 -fopenmp -lm sequential.c -o sequential
mpicc -o mpi ./mpi.c

for size in 3000 30000 300000 3000000 30000000
do
    for np in 1 2 4 8 16 32 64
    do
        ./sequential $size $np >> seq.csv
        export OMP_NUM_THREADS=$np
        ./openMp $size >> openmp.csv
        mpirun -np $np mpi $size >> mpi.csv
    done
    echo "\n" >> seq.csv
    echo "\n" >> openmp.csv
    echo "\n" >> mpi.csv
done