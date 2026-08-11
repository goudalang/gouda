.PHONY: all
all: mandelbrot_cpu mandelbrot_cu

OPENCL_FLAGS=
ifeq (Darwin,$(findstring Darwin,$(shell uname)))
    override OPENCL_FLAGS = -framework OpenCL
else
    override OPENCL_FLAGS = $(shell pkg-config --cflags --libs OpenCL)
endif

mandelbrot_cpu: mandelbrot.cc
	$(CXX) mandelbrot.cc -std=c++17 $(shell pkg-config sdl2 --cflags --libs) -o mandelbrot_cpu

mandelbrot_cu: mandelbrot.cu
	gouda mandelbrot.cu --transpile --backend opencl
	$(CXX) tmp.cc $(OPENCL_FLAGS) $(shell pkg-config sdl2 --cflags --libs) -o mandelbrot_cu
