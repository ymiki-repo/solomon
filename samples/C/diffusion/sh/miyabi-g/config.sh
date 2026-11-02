# shellcheck shell=bash
# shellcheck disable=SC2034 # settings are read by sh/common/*.sh (permanent)
# Settings for Miyabi-G (sourced by sh/miyabi-g/*.sh). To add a system, copy this directory and change the values.

# compiler: activate exactly one
USE_NVHPC=1
USE_AMDCLANG=0
USE_ICPX=0
USE_ACPP=0 # AdaptiveCpp: not supported yet (1 stops the job script)

# GPU vendor: activate exactly one
NVIDIA_GPU=1
AMD_GPU=0
INTEL_GPU=0

# GPU selection and architecture
GPU_ID=0
NVIDIA_ARCH=90 # 80, 90, 120
AMD_GPU_ID=2
AMD_ARCH=gfx90a # gfx908
INTEL_ARCH=pvc

# environment modules
MODULE_USE=() # directories added by `module use` after `module purge`
CUDA_MODULE=cuda
NVHPC_MODULE=nvidia
NVHPC_PURGE=1 # 1: `module purge` before loading NVHPC_MODULE
ROCM_MODULE=rocm
INTEL_MODULE=intel

# 1: set CUDA_VISIBLE_DEVICES / ROCR_VISIBLE_DEVICES to GPU_ID (a value set by the job scheduler is kept instead) and detect its NUMA node (not used by the recipes of this sample); 0: skip
DETECT_NUMA=0

# upper bound of the time of one run (a duration of timeout(1), e.g. 600s or 10m): a longer run counts as a failure;
# the runs at N=512 on whirlpool (RTX PRO 4000, models 0 to 3) take up to about 70 s, so raise it for a slower GPU
RUN_TIMEOUT=600s

# models built with NVIDIA HPC SDK: MODEL_ID up to
#  3: (OpenMP loop/distribute (2) + OpenACC kernels/parallel (2)) = 4 models
#  7: (OpenMP loop/distribute (2) + OpenACC kernels/parallel (2)) * (data/managed (2)) = 8 models
# 15: (OpenMP loop/distribute (2) + OpenACC kernels/parallel (2)) * (data/managed/unified/unified+first touch (4)) = 16 models
NVHPC_MAX_MODEL_ID=15
