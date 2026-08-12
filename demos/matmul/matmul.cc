#include <stdio.h>
#include <vector>
#include <cassert>
using std::vector;

__inline__ int idx2D(int width, int x, int y) {
    return width * y + x;
}

void matmul_naive_cpu(const vector<float>& a, const vector<float>& b, size_t M, size_t N, size_t K, vector<float> *out) {
    // Naive baseline matrix multiplication.
    assert(a.size() == M*N);
    assert(b.size() == N*K);
    assert(out->size() == M*K);
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < K; x++) {
            float sum = 0.0;
            for (int n = 0; n < N; n++) {
                sum += a[idx2D(N, n, y)] * b[idx2D(K, x, n)];
            }
            (*out)[idx2D(K, x, y)] = sum;
        }
    }
}

int main() {
    int M = 4000;
    int N = 4000;
    int K = 4000;
    vector<float> a(M*N);
    vector<float> b(N*K);
    vector<float> c(M*K);

    matmul_naive_cpu(a, b, M, N, K, &c);
    puts("done");
    return 0;
}

