#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

typedef struct bucket {
    int start;
    int i;
    int size;
} *Bucket;

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int* bucket_sort(int* array, int n, int nb, int max) {
    int i, b;
    int *new = calloc(n, sizeof(int));
    // Criação dos buckets
    Bucket buckets = calloc(nb, sizeof(struct bucket));
    int t1 = omp_get_wtime();
    // Contagem de inteiros em cada bucket
    for(i = 0; i < n; i++) {
        b = (array[i] * nb) / max;
        if(!(buckets[b].size)) buckets[b].size = 1;
        else buckets[b].size++;
    }
    // Atribuição do inicio de cada bucket no array que vai ser ordenado
    buckets[0].start = 0;
    buckets[0].i = 0;
    for(i = 1; i < nb; i++) {
        buckets[i].start = buckets[i-1].start + buckets[i-1].size; 
        buckets[i].i =  buckets[i].start;
    }
    // Colocar os inteiros no array final de acordo com o bucket correspondente
    for(i = 0; i < n; i++) {
        b = array[i] * nb / max;
        new[buckets[b].i] = array[i];
        buckets[b].i++;
    } 
    //Ordenação dos buckets
    for(i = 0; i < nb; i++) 
         qsort(&new[buckets[i].start], buckets[i].size , sizeof(int), cmpfunc);
    printf("O array demorou %f s a ser ordenado", omp_get_wtime() - t1);
    return new;
}


int main(int argc, char const *argv[])
{   int *x = malloc(30000000*sizeof(int)); 
    int i;
    for(i=0; i < 30000000; i++) {
        x[i] = (int) random() % 500;
    }
    x = bucket_sort(x,30000000,16,501);
}
