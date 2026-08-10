.PHONY: all
all: life_cpu life_cu

life_cpu: life.cc
	$(CXX) life.cc $(shell pkg-config sdl2 --cflags --libs) -o life_cpu

life_cu: life.cu
	gouda life.cu --transpile --backend opencl
	$(CXX) tmp.cc $(shell pkg-config OpenCL sdl2 --cflags --libs) -o life_cu

