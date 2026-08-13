<div align="center">
  <img src="./logo_large.png" alt="gouda : cross platform cuda compatible compiler" width="450" height="450" />
</div>

<img src="https://img.shields.io/github/downloads/goudalang/gouda/total?label=Downloads" alt="number of downloads" />
<img src="https://img.shields.io/github/actions/workflow/status/goudalang/gouda/ci.yml" alt="build status" />



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

We also have an installer script which should detect your platform and install the latest release:

```bash
# Installer from https://github.org/goudalang/gouda_installer/
curl -fsSl https://gouda-lang.org/install.sh -O && bash ./install.sh
```

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

## AI Disclaimer
ChatGPT was used for the logo, all other code was hand written.
