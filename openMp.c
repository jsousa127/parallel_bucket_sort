#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <string.h>
#include <unistd.h>


typedef struct bucket {
    int start;
    int i;
    int size;
} Bucket;

int cache[30000000];

int cmpfunc (const void * a, const void * b) {
   return (*(int*)a < *(int*)b) ? -1 : (*(int*)a > *(int*)b);
}

static int cmpdouble (const void * a, const void * b)
{
  if (*(double*)a > *(double*)b) return 1;
  else if (*(double*)a < *(double*)b) return -1;
  else return 0;  
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
    int r = 10000 / (nt);
    // Definir numero de threads;
    omp_set_num_threads(nt);    
    // Criação dos buckets
    Bucket *buckets = (struct bucket *) calloc(nb, sizeof(struct bucket));
    
    memset(size_thread, 0, sizeof(int)*nt);
    memset(start_thread, 0, sizeof(int)*nt);

    double t1 = omp_get_wtime();
    #pragma omp parallel 
    {
        int i, j, b, tb, lb; 
        int id = omp_get_thread_num();
        nt = omp_get_num_threads();
        // Calcular o tamanho de cada bucket
        #pragma omp for private(i,b)
            for(i = 0; i < n; i++) {
               tb = array[i] / r;

            if (tb > nt-1) tb = (nt) - 1;
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
            if (tb > (nt)-1) tb = (nt) - 1;
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

int kBest(double* array,int size, int k, double tol) {
    int i;
    double sigma, top;
    if(size < 3) return 0;
    qsort(array, size, sizeof(double), cmpdouble);
    sigma = (double)(array[0] * 0.05);
    top = (double)(array[0] + sigma);
    for(i=1; i<k; i++) {
        if(array[i] > top) return 0;
    }
    return 1;
}

int main(int argc, char const *argv[])
{
    int i, j, out, size = atoi(argv[1]);
    int *x = malloc(size*sizeof(int)); 
    int ord = 1;
    srand(getpid());
    for(i=0; i < size; i++) {
        x[i] = (int) random() % 10000;
    }
    clear_cache();
    double t[20];
    for(j=0; j < 20; j++){
        

        t[j] = bucket_sort(x,size);
        if(kBest(t,j+1,3,5.0)) {
            printf("%f;",t[0]);
            return 0;
        }
    }
    printf("%f;",t[0]); 
    return 1;   
}