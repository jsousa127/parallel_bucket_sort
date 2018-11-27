#!/bin/sh
#
#PBS -N teste
#PBS -l walltime=60:00
#PBS -l nodes=1:r641:ppn=32
#PBS -q mei

module load gcc/5.3.0
gcc -O3 -std=gnu99 -fopenmp -lm parallel.c -o bucket_par
gcc -O3 -std=gnu99 -fopenmp -lm sequential.c -o bucket_seq

for size in 3000 30000 300000 3000000 30000000
do
    for n_threads in 1 2 4 8 16 32
    do
        ./bucket_seq $size $n_threads >> seq.csv
        export OMP_NUM_THREADS=$n_threads
        ./bucket_par $size >> par.csv
    done
    echo "\n" >> par.csv
    echo "\n" >> seq.csv 
done