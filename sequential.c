#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

typedef struct bucket {
    int start;
    int i;
    int size;
} *Bucket;

int cache[30000000];

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
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

double bucket_sort(int* array, int n, int nb, int max) {
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

    double ret = omp_get_wtime() - t1;   
    int aux;
    for(aux = 0; aux < n; aux++) {
         array[aux] = new[aux];
    }    
    return ret;
}


int main(int argc, char const *argv[])
{   int size = atoi(argv[1]);
    int *x = malloc(size*sizeof(int)); 
    int i,j,ord=1;
    double t[20];
    for(j=0; j < 20; j++){
        clear_cache();
        
        for(i=0; i < size; i++) {
            x[i] = (int) random() % 500;
        }
        t[j] = bucket_sort(x,size,atoi(argv[2]),501);
        for (i=0; i < size-1; i++) {
            if (x[i] > x[i+1]) 
                ord = 0;
        }
    }
    qsort(t, 20, sizeof(&t[0]), cmpdouble);
    if (ord == 1) printf("%f ",(t[10]+t[11])/2.0); 
    else printf("%f ",-1.0);
    
}
