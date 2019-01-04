#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>
#include <math.h>
#include <unistd.h>

typedef struct bucket {
    int* array;
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

void generateArray(int* x, int size) {
    int i;
    for(i=0; i < size; i++)
        x[i] = (int) random() % 10000;
    clear_cache();
}

int* calculateDispls(int* sizes, int np) {
    int i;
    int* displs= (int *) malloc(sizeof(int)*np);
    displs[0] = 0;
    for(i = 1; i < np+1; i++)
        displs[i] = displs[i-1] + sizes[i-1];
    return displs;    
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

Bucket bucket_sort(int* array, int n, int nb, int max) {
    int i, b;
    // Criação dos buckets
    Bucket buckets = calloc(nb, sizeof(struct bucket));

    // Contagem de inteiros em cada bucket
    for(i = 0; i < n; i++) {
        b = (array[i] * nb) / max;
        if(b>=nb) 
            b = nb - 1;
        if(!(buckets[b].size)) 
            buckets[b].size = 1;
        else buckets[b].size++;
    }

    for(i = 0; i < nb; i++) 
        buckets[i].array = malloc(buckets[i].size * sizeof(int));
    

    // Colocar os inteiros no array final de acordo com o bucket correspondente
    for(i = 0; i < n; i++) {
        b = array[i] * nb / max;
        if(b>=nb) 
            b = nb - 1;
        buckets[b].array[buckets[b].i] = array[i];
        buckets[b].i++;
    }

    return &buckets[0];
}


int main(int argc, char const *argv[])
{   int i, j, rank, np, div, current, flg = 0, size = atoi(argv[1]);
    int *x, *buc,*buf, *sizes;
    double start, end, time = 0;
    Bucket bs;
    MPI_Status status;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    
    div = size/(float)np;
    buf = malloc(size*sizeof(int));
    buc = malloc(size*sizeof(int));
    double t[20];
    for(j=0;j<20;j++) {
        if(rank == 0) {
            x = malloc(size*sizeof(int)); 
            generateArray(x,size);
            start = MPI_Wtime();
        }

        MPI_Scatter(x,div,MPI_INT,buf,div,MPI_INT,0,MPI_COMM_WORLD);
        bs = bucket_sort(buf,div,np,10000);

        MPI_Request *requests = (MPI_Request *) malloc(sizeof(MPI_Request) * np);
        for(i = 0; i < np; i++)
            /* Make sure I'm not sending to myself */
            if(i != rank)
                MPI_Isend(bs[i].array,bs[i].size,MPI_INT,i,bs[i].size,MPI_COMM_WORLD,&requests[i]);
            else
                for(current = 0; current < bs[rank].size; current++)
                    buc[current] = bs[rank].array[current];

        for(i = 0; i < np-1; i++) {
            MPI_Recv(&buc[current],div,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
            current += status.MPI_TAG;
        }

        qsort(buc,current,sizeof(int),cmpfunc);
        
        sizes = (int *) malloc(sizeof(int)*np);
        MPI_Gather(&current,1,MPI_INT,sizes,1,MPI_INT,0,MPI_COMM_WORLD);
        int * displs; 
        if(rank == 0) 
            displs = calculateDispls(sizes,np);
        
        MPI_Gatherv(buc,current,MPI_INT,x,sizes,displs,MPI_INT,0,MPI_COMM_WORLD);
        if(rank == 0){
            end = MPI_Wtime();
            t[j] = end - start;
            if(kBest(t,j+1,3,5.0)) {
                time = t[0];
            }
        }
        
    }
    MPI_Finalize();
    if (rank==0 && time!=0) printf("%f;",time);
    else if (rank==0) printf("%f",t[0]);
    return 0;
}



