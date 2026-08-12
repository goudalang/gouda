# Matrix Multiplication (matmul)

This demo performs a massive matrix multiplication. This problem is used in many domains,
from engineering and science to machine learning.

The implementation here is a "naive" implementation, without tuning to the specifics of
a given GPU. This gives us a quick and approximate way to test for the relative
speeds of various backends, without over-specializing to special features
allowed by some devices/backends but not others.

We multiply two 4000x4000 matricies to yield another 4000x4000 matrix.

In the future, we plan to also include these per-platform optimizations, to demonstrate
how much further performance is available for a given backend. If you'd like to provide
an optimized matmul for ex, NVIDIA or Apple Silicon, please send a pull request.

## Results

Server benchmarks:
aws g4dn.xlarge (4cpu, 16GiB memory, Nvidia T4 GPU)

| Compiler | Device | Running time (sec) |
| -- | -- | -- |
| g++ | xeon cpu | 7m56.2s = 476 sec|
| nvcc | Tesla T4 (CUDA) | 2.372 sec |
| gouda(winner) | Tesla T4 (Gouda/OpenCL) | 2.348 sec |

Laptop benchmarks:
(X1 Carbon, i7 8th gen)

| Compiler | Device | Running time (sec) |
| -- | -- | -- |
| g++ | i7 cpu | 16m30.498s = 990 sec |
| gouda(winner) | i7 cpu (Gouda/OpenCL) | 7.7 sec |

## Hardware

### Server
AWS g4dn.xlarge (T4 Nvidia GPU, 4vcpu).

```bash
$ uname -a
Linux ip-172-31-75-99.ec2.internal 6.18.39-79.141.amzn2023.x86_64 #2 SMP PREEMPT_DYNAMIC Sun Jul 26 23:56:49 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux

$ head /proc/cpuinfo
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 85
model name      : Intel(R) Xeon(R) Platinum 8259CL CPU @ 2.50GHz
stepping        : 7
microcode       : 0x5003901
cpu MHz         : 3098.491
cache size      : 36608 KB
physical id     : 0

$ nvidia-smi
Wed Aug 12 18:45:39 2026
+-----------------------------------------------------------------------------------------+
| NVIDIA-SMI 610.57.04              KMD Version: 610.57.04     CUDA UMD Version: 13.3     |
+-----------------------------------------+------------------------+----------------------+
| GPU  Name                 Persistence-M | Bus-Id          Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |
|                                         |                        |               MIG M. |
|=========================================+========================+======================|
|   0  Tesla T4                       Off |   00000000:00:1E.0 Off |                    0 |
| N/A   38C    P0             27W /   70W |       0MiB /  15360MiB |      0%      Default |
|                                         |                        |                  N/A |
+-----------------------------------------+------------------------+----------------------+

+-----------------------------------------------------------------------------------------+
| Processes:                                                                              |
|  GPU   GI   CI              PID   Type   Process name                        GPU Memory |
|        ID   ID                                                               Usage      |
|=========================================================================================|
|  No running processes found                                                             |
+-----------------------------------------------------------------------------------------+
```

### Laptop
X1 Carbon Laptop, 8th gen.

```bash
$ uname -a
Linux 7.0.0-28-generic #28~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC Wed Jul  1 15:50:57 UTC 2 x86_64 x86_64 x86_64 GNU/Linux

$ head /proc/cpuinfo
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 142
model name      : Intel(R) Core(TM) i7-8665U CPU @ 1.90GHz
stepping        : 12
microcode       : 0x100
cpu MHz         : 799.994
cache size      : 8192 KB

$ clinfo
Number of platforms                               1
  Platform Name                                   Intel(R) OpenCL Graphics
  Platform Vendor                                 Intel(R) Corporation
  Platform Version                                OpenCL 3.0
  Platform Profile                                FULL_PROFILE
...
Slices (Intel)                                  1
Sub-slices per slice (Intel)                    3
EUs per sub-slice (Intel)                       8
Threads per EU (Intel)                          7
```

## Raw data

```bash

### Server (awslinux):

$ nvcc matmul.cu -o matmul_cuda
$ time ./matmul_cuda
done

real    0m2.372s
user    0m0.397s
sys     0m1.883s

$ gouda matmul.cu --backend opencl -o matmul_gouda
$ time ./matmul_gouda
done

real    0m2.348s
user    0m0.429s
sys     0m1.843s

$ g++ matmul.cc -O3 -o matmul_cpu
$ time ./matmul_cpu
done

real    7m56.216s
user    7m56.035s
sys     0m0.130s

==========

### Laptop
$ time ./matmul_cpu
done

real    16m30.498s
user    16m29.784s
sys     0m0.082s

$ time ./matmul_cu
done

real    0m7.729s
user    0m0.059s
sys     0m0.143s

```
