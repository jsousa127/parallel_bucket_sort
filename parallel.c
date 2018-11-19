#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>

typedef struct bucket {
    int start;
    int i;
    int size;
} Bucket;

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

double bucket_sort(int* array, int n, int nt, int max) {
    int size_thread[nt];
    int start_thread[nt];
    int nb = nt * nt;
    int *new = malloc(n * sizeof(int));
    // Definir numero de threads;
    omp_set_num_threads(nt);    
    // Criação dos buckets
    Bucket *buckets = (struct bucket *) calloc(nb, sizeof(struct bucket));
    int t1 = omp_get_wtime();
    
    #pragma omp parallel 
    {
        int i, b, tb, lb, size=0; 
        int id = omp_get_thread_num();
        nt = omp_get_thread_num();
        // Calcular o tamanho de cada bucket
        #pragma omp for private(i,tb,b)
            for(i = 0; i < n; i++) {
               tb = (array[i] * nt) / max;
               b = tb + id * nt;
               if(!(buckets[b].size)) buckets[b].size = 1;
               else buckets[b].size++;
        }
        // Calcular o tamanho do conjunto de buckets correspondentes a esta thread
        for(i = id; i < nb; i = i + nt) {
            if(buckets[i].size) size += buckets[i].size;
        }
        size_thread[id] = size;
        // Esperar que todas as threads calculem o seu tamanho
        
        #pragma omp barrier
        // O master inicializa o primeiro bucket de cada thread
        #pragma omp master
        {
            start_thread[0] = 0;
            buckets[0].start = 0;
            buckets[0].i = 0;
            for(i = 1; i < nt; i++) {
                start_thread[i] = start_thread[i-1] + size_thread[i-1]; 
                buckets[i].start = buckets[i-1].start + buckets[i-1].size; 
                buckets[i].i =  buckets[i].start;
            }
        }
        // Esperar que o master inicialize os buckets
        #pragma omp barrier
        // Cada thread inicializa o resto dos seus buckets
        for (i = id + nt; i < nb; i = i + nt) {
            int lb = i - nb;
            buckets[i].start = buckets[lb].start + buckets[lb].size;
            buckets[i].i = buckets[i].start;	
	    }
        #pragma omp barrier
        // Colocar os elementos do array no bucket correspondente
        #pragma omp for private(i,b,tb)
        for(i = 0; i < n; i++) {
            tb = array[i] * nt / max;
            b = tb + id * nt;
            new[buckets[b].i] = array[i];
            buckets[b].i++;
        }
        # pragma omp for private(i)
        for (i = 0; i < nt; i++) { 
                qsort(&new[start_thread[i]], size_thread[i] , sizeof(int), cmpfunc); 
        }
    }
    double ret = omp_get_wtime() - t1;
    int aux;
    for(aux = 0; aux < n; aux++) {
         array[aux] = new[aux];
    }
    
    return ret;
}

int main(int argc, char const *argv[])
{
    int *x = malloc(30000*sizeof(int)); 
    int i;
    int ord = 1;
    for(i=0; i < 30000000; i++) {
        x[i] = (int) random() % 500;
    }
    double t = bucket_sort(x,30000000,4,501);
    for (i=0; i < 29999999; i++) {
        if (x[i] > x[i+1]) ord = 0;
    }
    if (ord == 1) printf("O array foi ordenado em %f\n",t);
    free(x);    
}
