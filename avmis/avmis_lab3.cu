
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "windows.h"
#include <limits>
#include <cmath>
#include <sys\timeb.h>

#include <stdio.h>

#define BLOCK_SIZE 16
#define N 512

cudaError_t mulWithCuda(float* c, const float* a, const float* b, size_t size);

__global__ void mulKernel(float* c, const float* a, const float* b)
{
    int j = (blockIdx.y * blockDim.y) + threadIdx.y;
	int i = (blockIdx.x * blockDim.x) + threadIdx.x;
	
	float sum = 0;	

	for(int k = 0; k < N; ++k){
        float a_elem = a[i * N + k];
        float b_elem = b[k * N + j];
        sum += a_elem * b_elem;
    }

	c[i * N + j] = sum;
}

int main()
{
    float* a = new float[N * N];
    float* b = new float[N * N];
    float* c_cuda = new float[N * N];
    float* c_omp  = new float[N * N];
    float* c_cpu  = new float[N * N];

    LARGE_INTEGER frequency;        // ticks per second
    LARGE_INTEGER t1, t2;           // ticks
    double elapsedTime;

	for(int i = 0; i < N; ++i) {
		for(int j = 0; j < N; ++j) {
			a[i * N + j] = 1.0f;
			b[i * N + j] = 1.0f;
			c_cuda[i * N + j] = 0.0f;
            c_omp[i * N + j]  = 0.0f;
            c_cpu[i * N + j]  = 0.0f;
		}		
	}

    QueryPerformanceFrequency(&frequency);   

    // Multiply on CPU
    printf("Multiplication on CPU...\n");

    // start timer
    QueryPerformanceCounter(&t1);
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {            
            for(int k = 0; k < N; k++) {
               c_cpu[i * N + j] += a[i * N + k] * b[k * N + j];                 
            }                   
        }
    }
    // stop timer
    QueryPerformanceCounter(&t2);

     // compute and print the elapsed time in millisec
    elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
    printf("done in %f ms\n\n", elapsedTime); 

    // Multiply on CPU with OpenMP
    printf("Multiplication with OMP...\n");

    // start timer
    QueryPerformanceCounter(&t1);
    #pragma omp parallel for num_threads(2)
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            for(int k = 0; k < N; k++) {
                c_omp[i + N * j] += a[i + N * k] * b[k + N * j]; 
            }            
        }
    }
    // stop timer
    QueryPerformanceCounter(&t2);

     // compute and print the elapsed time in millisec
    elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
    printf("done in %f ms\n\n", elapsedTime); 

    //Multiply on GPU
    printf("Multiplication with CUDA...\n");

    QueryPerformanceCounter(&t1);
    // Add vectors in parallel.
    cudaError_t cudaStatus = mulWithCuda(c_cuda, a, b, N*N);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "mulWithCuda failed!");
        return 1;
    }
    // stop timer
    QueryPerformanceCounter(&t2);

     // compute and print the elapsed time in millisec
    elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
    printf("done in %f ms\n\n", elapsedTime); 

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
		system("pause");
        return 1;
    }

    int errors = 0;

    for(int i = 0; i < N*N; i++) {
        if(c_cuda[i] != c_cpu[i]) {
            //printf("Error in %d element\n", i);
            errors ++;
        }
    }   

    if(errors == 0) {
        printf("No errors.\n");
    }
    else {
        printf("%d errors\n", errors);
    }  
	
	system("pause");
    return 0;
}

// Helper function for using CUDA to multiply vectors in parallel.
cudaError_t mulWithCuda(float* c, const float* a, const float* b, size_t size)
{
    float *dev_a = 0;
    float *dev_b = 0;
    float *dev_c = 0;
    cudaError_t cudaStatus;	

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		system("pause");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc(&dev_c, size * sizeof(float));
    if (cudaStatus != cudaSuccess) {
        cudaError_t error = cudaGetLastError();
        fprintf(stderr,"ERROR: cudaMalloc: %s\n", cudaGetErrorString(error));

		system("pause");
        goto Error;
    }

    cudaStatus = cudaMalloc(&dev_a, size * sizeof(float));
    if (cudaStatus != cudaSuccess) {
        cudaError_t error = cudaGetLastError();
        fprintf(stderr,"ERROR: cudaMalloc: %s\n", cudaGetErrorString(error));

		system("pause");
        goto Error;
    }

    cudaStatus = cudaMalloc(&dev_b, size * sizeof(float));
    if (cudaStatus != cudaSuccess) {
        cudaError_t error = cudaGetLastError();
        fprintf(stderr,"ERROR: cudaMalloc: %s\n", cudaGetErrorString(error));

		system("pause");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(float), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
		system("pause");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(float), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
		system("pause");
        goto Error;
    }
	
	
    dim3 threadsPerBlock(BLOCK_SIZE, BLOCK_SIZE); // block size -- threads per block - X*X
    dim3 numBlocks(N/threadsPerBlock.x, 
                   N/threadsPerBlock.y); //grid size -- number of blocks


    // Launch a kernel on the GPU with one thread for each element.
    mulKernel<<<numBlocks, threadsPerBlock>>>(dev_c, dev_a, dev_b);

    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		system("pause");
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(float), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
		system("pause");
        goto Error;
    }    

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);	
    
    return cudaStatus;
}
