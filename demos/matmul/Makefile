.PHONY: all
all: matmul_cpu matmul_cu

matmul_cpu: matmul.cc
	$(CXX) matmul.cc -O3 -o matmul_cpu

matmul_cu: matmul.cu
	gouda matmul.cu --backend opencl -o matmul_cu
