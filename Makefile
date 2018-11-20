all: par seq

par:
    gcc -O3 -std=c99 -fopenmp -lm parallel.c -o bucket_par

seq:
    gcc -O3 -std=c99 -fopenmp -lm sequential.c -o bucket_seq

clean:
    rm bucket_par bucket_seq