# shellcheck shell=bash
# shellcheck disable=SC2034 # settings are read by sh/common/*.sh (permanent)
# Settings for a local machine (sourced by sh/local/*.sh). To add a system, copy this directory and change the values.

# compiler: activate exactly one
USE_NVHPC=0
USE_AMDCLANG=0
USE_ICPX=1
USE_ACPP=0 # AdaptiveCpp: not supported yet (1 stops the job script)

# GPU vendor: activate exactly one
NVIDIA_GPU=0
AMD_GPU=0
INTEL_GPU=1

# GPU selection and architecture
GPU_ID=0
NVIDIA_ARCH=90 # 80, 90, 120
AMD_GPU_ID=2
AMD_ARCH=gfx90a # gfx908
INTEL_ARCH=pvc

# environment modules
MODULE_USE=() # directories added by `module use` after `module purge`
CUDA_MODULE=cuda
NVHPC_MODULE=nvhpc
NVHPC_PURGE=1 # 1: `module purge` before loading NVHPC_MODULE
ROCM_MODULE=rocm
INTEL_MODULE=intel
BOOST_MODULE=boost

# 1: set CUDA_VISIBLE_DEVICES / ROCR_VISIBLE_DEVICES to GPU_ID (a value set by the job scheduler is kept instead) and bind the runs to its NUMA node; 0: skip
DETECT_NUMA=1
# NUMA node of an Intel GPU (not detected; empty: the first node of `numactl --show`)
INTEL_NUMA_NODE=0

# upper bound of the time of one run of survey.sh (a duration of timeout(1), e.g. 40s or 2m): a longer run counts as a
# failure; a run at N = 2M takes about 40 s at 3 TFlop/s, so raise it for a slower GPU
RUN_TIMEOUT=40s
