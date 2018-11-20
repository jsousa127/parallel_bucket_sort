#!/bin/sh
#
#PBS -N teste
#PBS -l walltime=30:00
#PBS -l nodes=2:r641:ppn=32
#PBS -q mei
module load gcc/5.3.0

for size in 3000 30000 300000 #3000000 30000000
do
    for n_threads in 1 2 4 8 # 16 32 64
    do
        ./bucket_seq $size $n_threads >> sequential.csv
        export OMP_NUM_THREADS=$n_threads
        ./bucket_par $size >> parallel.csv
    done
done