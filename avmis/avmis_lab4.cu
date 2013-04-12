/* 
 * Compile with
 * nvcc -arch=sm_11 bitonic_sort.cu
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Every thread gets exactly one value in the unsorted array. */
#define THREADS 512 // 2^9
#define BLOCKS 128 // 2^15
#define NUM_VALS THREADS*BLOCKS

double getElapsedTime(clock_t start, clock_t stop)
{
    return ((double) (stop - start)) / CLOCKS_PER_SEC;
    //printf("Elapsed time: %.3fs\n", elapsed);
}

void fillArray(float *array, int length)
{
    srand(time(NULL));
    int i;
    for (i = 0; i < length; i++) {
        array[i] = (float)rand()/(float)RAND_MAX;
    }
}

__global__ void bitonic_sort_step(float *dev_values, int j, int k)
{
    unsigned int i, ixj; /* Sorting partners: i and ixj */
    i = threadIdx.x + blockDim.x * blockIdx.x;
    ixj = i^j;

    /* The threads with the lowest ids sort the array. */
    if ((ixj)>i) {
        if ((i&k)==0) {
            /* Sort ascending */
            if (dev_values[i]>dev_values[ixj]) {
                /* exchange(i,ixj); */
                float temp = dev_values[i];
                dev_values[i] = dev_values[ixj];
                dev_values[ixj] = temp;
            }
        }
        if ((i&k)!=0) {
            /* Sort descending */
            if (dev_values[i]<dev_values[ixj]) {
                /* exchange(i,ixj); */
                float temp = dev_values[i];
                dev_values[i] = dev_values[ixj];
                dev_values[ixj] = temp;
            }
        }
    }
}

/**
 * Inplace bitonic sort using CUDA.
 */
void gpuBitonicSort(float *values)
{
    float *devValues;
    size_t size = NUM_VALS * sizeof(float);

    cudaMalloc((void**) &devValues, size);
    cudaMemcpy(devValues, values, size, cudaMemcpyHostToDevice);

    dim3 blocks(BLOCKS,1);    /* Number of blocks   */
    dim3 threads(THREADS,1);  /* Number of threads  */

    int j, k;
    /* Major step */
    for (k = 2; k <= NUM_VALS; k <<= 1) {
        /* Minor step */
        for (j=k>>1; j>0; j=j>>1) {
            bitonic_sort_step<<<blocks, threads>>>(devValues, j, k);
        }
    }

    cudaDeviceSynchronize();

    cudaMemcpy(values, devValues, size, cudaMemcpyDeviceToHost);
    cudaFree(devValues);
}

const int ASCENDING = 1;
const int DESCENDING = 0;

void compare(float *a, int i, int j, int dir)
{
  if (dir == (a[i] > a[j])) {
        float h=a[i];
        a[i]=a[j];
        a[j]=h;
    }
}

void cpuBitonicMerge(float* values, int lo, int count, int direction)
{
    if (count > 1) {
        int k = count/2;
      
        for (int i=lo; i<lo+k; i++)
           compare(values, i, i+k, direction);
        cpuBitonicMerge(values, lo, k, direction);
        cpuBitonicMerge(values, lo+k, k, direction);
    }
}

void cpuBitonicSort(float *values, int lo, int count, int direction)
{
    if (count > 1) {
        int k = count/2;
        cpuBitonicSort(values, lo, k, ASCENDING);
        cpuBitonicSort(values, lo + k, k, DESCENDING);
        cpuBitonicMerge(values, lo, count, direction);
    }
}

int floatcomp(const void* elem1, const void* elem2)
{
    if(*(const float*)elem1 < *(const float*)elem2)
        return -1;
    return *(const float*)elem1 > *(const float*)elem2;
}

int main(void)
{
    clock_t start, stop;

    float *array    = (float*) malloc( NUM_VALS * sizeof(float));
    float *cpuArray = (float*) malloc( NUM_VALS * sizeof(float));
    float *gpuArray = (float*) malloc( NUM_VALS * sizeof(float));
    
    fillArray(array, NUM_VALS);
    for(int i = 0; i < NUM_VALS; i++) {
        cpuArray[i] = array[i];
        gpuArray[i] = array[i];
    }

    start = clock();
    gpuBitonicSort(gpuArray);
    stop = clock();

    printf("\nElapsed time on gpu: %.3fs\n", getElapsedTime(start, stop));

    start = clock();
    cpuBitonicSort(cpuArray, 0, NUM_VALS, ASCENDING);
    stop = clock();

    printf("\nElapsed time on cpu: %.3fs\n", getElapsedTime(start, stop));    
   
    //std::qsort(array, NUM_VALS, sizeof(float), floatcomp);
    for(int i = 0; i < NUM_VALS; i++) {
        if(gpuArray[i] != cpuArray[i]){
            printf("Error [%d] %f != %f\n", i, gpuArray[i], cpuArray[i]);
        }
    } 
  
    free(array);
    free(cpuArray);
    free(gpuArray);
}
