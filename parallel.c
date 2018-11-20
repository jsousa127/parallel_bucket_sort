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

int cache[30000000];

int cmpfunc (const void * a, const void * b) {
   return (*(int*)a < *(int*)b) ? -1 : (*(int*)a > *(int*)b);
}

void clear_cache() {
    for(unsigned i = 0; i < 30000000; i++) {
        cache[i] = i;
    }
}

double bucket_sort(int* array, int n) {
    int nt;
    #pragma omp parallel
    {
        #pragma omp single
        nt = omp_get_num_threads(); 
    } 
    int size_thread[nt];
    int start_thread[nt];
    int nb = nt * nt;
    int *new = malloc(n * sizeof(int));
    int r = 10000 / nt;
    // Definir numero de threads;
    omp_set_num_threads(nt);    
    // Criação dos buckets
    Bucket *buckets = (struct bucket *) calloc(nb, sizeof(struct bucket));
    
    memset(size_thread, 0, sizeof(int)*nt);
    memset(start_thread, 0, sizeof(int)*nt);

    int t1 = omp_get_wtime();
    #pragma omp parallel 
    {
        int i, j, b, tb, lb; 
        int id = omp_get_thread_num();
        nt = omp_get_num_threads();
        // Calcular o tamanho de cada bucket
        #pragma omp for private(i,b)
            for(i = 0; i < n; i++) {
               tb = array[i] / r;

            if (tb > nt-1) tb = nt - 1;
               b = tb + id * nt;
               buckets[b].size++;
        }
        // Calcular o tamanho do conjunto de buckets correspondentes a esta thread
        int size = 0;
        for(j = id; j < nb; j = j + nt) {
            size += buckets[j].size;
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
            for(j = 1; j < nt; j++) {
                start_thread[j] = start_thread[j-1] + size_thread[j-1]; 
                buckets[j].start = buckets[j-1].start + size_thread[j-1]; 
                buckets[j].i =  buckets[j].start;
            }
        }
        // Esperar que o master inicialize os buckets
        #pragma omp barrier
        // Cada thread inicializa o resto dos seus buckets
        for (j = id + nt; j < nb; j = j + nt) {
            int lb = j - nt;
            buckets[j].start = buckets[lb].start + buckets[lb].size;
            buckets[j].i = buckets[j].start;	
	    }
        #pragma omp barrier
        // Colocar os elementos do array no bucket correspondente
        #pragma omp for private(i,b)
        for(i = 0; i < n; i++) {
            tb = array[i] / r;
            if (tb > nt-1) tb = nt - 1;
            b = tb + id * nt;
            new[buckets[b].i] = array[i];
            buckets[b].i++;
        }
        # pragma omp for private(i)
        for (i = 0; i < nt; i++) { 
                qsort(new+start_thread[i], size_thread[i] , sizeof(int), cmpfunc); 
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
    int i, j, out, size = atoi(argv[1]);
    int *x = malloc(size*sizeof(int)); 
    int ord = 1;
    double t[5];
    for(j=0; j < 5; j++){
        clear_cache();
        for(i=0; i < size; i++) {
            x[i] = (int) random() % 10000;
        }
        t[j] = bucket_sort(x,size);
        for (i=0; i < size-1; i++) {
            if (x[i] > x[i+1]) 
                ord = 0;
        }
    }
    qsort(&t[0], 5, sizeof(&t[0]), cmpfunc);
    if (ord == 1) printf("%f ",t[2]);
    else printf("%f ",-1.0); 
    return 1;   
}