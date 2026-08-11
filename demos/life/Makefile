.PHONY: all
all: life_cpu life_cu

OPENCL_FLAGS=
ifeq (Darwin,$(findstring Darwin,$(shell uname)))
    override OPENCL_FLAGS = -framework OpenCL
else
    override OPENCL_FLAGS = $(shell pkg-config --cflags --libs OpenCL)
endif

life_cpu: life.cc
	$(CXX) life.cc $(shell pkg-config sdl2 --cflags --libs) -o life_cpu

life_cu: life.cu
	gouda life.cu --transpile --backend opencl
	$(CXX) tmp.cc $(OPENCL_FLAGS) $(shell pkg-config sdl2 --cflags --libs) -o life_cu

