# Gouda Compiler

A drop-in replacement for nvcc. Take your existing CUDA Cpp programs, and
recompile them to target your other hardware. CPU / OpenCL and more.

Read more about the [gouda compiler on the web](https://gouda-lang.org/)

While we'd like to eventually achieve CUDA like performance on other
devices, we're currently focused on improving support for special
CUDA C features and expanding backend support.

In its current form, it is primarily useful for experimenting with
parallel programming on non-CUDA hardware.

## Installation

Our initial releases are in binary form only. Visit the [releases](https://github.com/goudalang/gouda/releases) to find the compiler for your host system.

## Getting Started:

We have several interesting demos in our [demos](demos/) directory. These include both standard C as well
as parallel implementations which use Gouda. Most are visual/graphics demo, so you can get a feel for the speedup
possible by using Gouda.

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

Open an issue or [contact us](mailto:support@gouda-lang.org) to sponsor a new backend.
