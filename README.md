# Gouda Compiler

A drop-in replacement for nvcc. Take your existing CUDA Cpp programs, and
recompile them to target your other hardware. CPU / OpenCL and more.

Read more about the [gouda compiler on the web](https://gouda-lang.org/)

## Installation

Our initial releases are in binary form only. Visit the [releases](https://github.com/goudalang/gouda/releases) to find the compiler for your host system.

## Usage:

```bash
$ gouda --backend opencl matmul.cu -o matmul_cl
$ ./matmul_cl
# examine the generated source code, useful for integrating into larger projects.
$ cat tmp.cc
```

## Backends:
- opencl
- cpu1 (single-threaded, useful for unit testing or development)
