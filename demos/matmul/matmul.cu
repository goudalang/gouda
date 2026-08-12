#include <cuda_runtime.h>
#include <stdio.h>
#include <cassert>

__global__ void matmul(const float* A, const float* B, float* C, int M, int N,
                                             int K) {
    int x = blockIdx.x*blockDim.x + threadIdx.x;
    int y = blockIdx.y*blockDim.y + threadIdx.y;
    float sum = 0.0;
    int AW = N;
    int AH = M;
    int BW = K;
    int BH = N;
    if (x >= K || y >= M) return;
    for (int i = 0; i < N; i++) {
        int ay = y;
        int ax = i;
        int by = i;
        int bx = x;
        sum += A[AW * ay + ax] * B[BW * by + bx];
    }
    C[y * K + x] = sum;
}

int div_ceil(int num, int denom) {
    return (num + denom - 1) / denom;
}

int main() {
    int M = 4000;
    int N = 4000;
    int K = 4000;
    const int SIZE_A = sizeof(float)*M*N;
    const int SIZE_B = sizeof(float)*N*K;
    const int SIZE_C = sizeof(float)*M*K;

    float* a = (float*)malloc(SIZE_A);
    float* b = (float*)malloc(SIZE_B);
    float* c = (float*)malloc(SIZE_C);

    float *device_a, *device_b, *device_c;
    cudaMalloc(&device_a, SIZE_A);
    cudaMalloc(&device_b, SIZE_B);
    cudaMalloc(&device_c, SIZE_C);

    cudaMemcpy(device_a, a, SIZE_A, cudaMemcpyHostToDevice);
    cudaMemcpy(device_b, b, SIZE_B, cudaMemcpyHostToDevice);

    dim3 threads(16, 16, 1);
    dim3 tiles(
        div_ceil(M, threads.x),
        div_ceil(K, threads.y),
        1
    );

    matmul<<<tiles, threads>>>(device_a, device_b, device_c, M, N, K);
    cudaDeviceSynchronize();

    cudaMemcpy(c, device_c, SIZE_C, cudaMemcpyDeviceToHost);


    puts("done");
    return 0;
}

