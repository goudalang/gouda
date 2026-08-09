#include <cuda_runtime.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

__global__ void fill_arr(int* d_matrix, int size) {
    // Calculate the global thread ID
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= size) return;
    d_matrix[idx] = idx;
}

int main() {
    const int SIZE = 10;
    const size_t BYTES = SIZE * sizeof(int);

    // Host (CPU) pointer
    int h_matrix[SIZE];

    // Device (GPU) pointer
    int* d_matrix = nullptr;

    cudaMalloc(&d_matrix, BYTES);

    int threadsPerBlock = 4;
    int blocksPerGrid = (SIZE + threadsPerBlock - 1) / threadsPerBlock;

    fill_arr<<<blocksPerGrid, threadsPerBlock>>>(d_matrix, SIZE);
    cudaDeviceSynchronize();

    cudaMemcpy(h_matrix, d_matrix, BYTES, cudaMemcpyDeviceToHost);
    cudaFree(d_matrix);

    printf("Got: ");
    for (int i = 0; i < SIZE; i++) {
        printf("%d ", h_matrix[i]);
    }
    printf("\n");

    return 0;
}
