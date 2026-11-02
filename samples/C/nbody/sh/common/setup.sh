# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# shellcheck disable=SC2034 # the variables set here are read by the sourcing recipe (permanent)
# Common environment setup for the job scripts of this sample (sourced by sh/common/*.sh).
# Requires the settings of sh/<system>/config.sh (CONFIG names it) and SAMPLE; run from the sample directory.
# Checks the settings, takes the lock of the sample directory (guard_lock), and loads the modules.
# Sets: COMPILER, VENDER, ARCH, GPU_ID, NUMA_NODE, NUMACTL (array), TARGET, and MODEL_ID_LIST;
# defines set_model (USE_* of MODEL_ID) and set_threads (THREADS of MODEL_ID for the compiler).

# shellcheck source=sh/common/guard.sh
. sh/common/guard.sh || exit 1
guard_sample

for _var in USE_NVHPC USE_AMDCLANG USE_ICPX USE_ACPP NVIDIA_GPU AMD_GPU INTEL_GPU NVHPC_PURGE DETECT_NUMA; do
	guard_flag "$_var"
done
for _var in GPU_ID AMD_GPU_ID; do
	guard_integer "$_var"
done
for _var in NVIDIA_ARCH AMD_ARCH INTEL_ARCH CUDA_MODULE NVHPC_MODULE ROCM_MODULE INTEL_MODULE BOOST_MODULE; do
	guard_value "$_var"
done
unset _var
if [ -z "${INTEL_NUMA_NODE+set}" ]; then
	guard_die "INTEL_NUMA_NODE is not set: define it in ${CONFIG:-sh/<system>/config.sh} (empty: the first node of numactl --show)"
fi
if [ -n "$INTEL_NUMA_NODE" ]; then
	guard_integer INTEL_NUMA_NODE
fi

if [ "$USE_ACPP" = 1 ]; then
	guard_die "USE_ACPP=1 in ${CONFIG:-sh/<system>/config.sh}: AdaptiveCpp is not supported by this sample yet (no recipe and no Makefile rule); set USE_ACPP=0 and activate another compiler"
fi
if [ $((USE_NVHPC + USE_AMDCLANG + USE_ICPX + USE_ACPP)) != 1 ]; then
	guard_die "Only one compiler can be activated: USE_NVHPC, USE_AMDCLANG, USE_ICPX, and USE_ACPP"
fi
if [ $((NVIDIA_GPU + AMD_GPU + INTEL_GPU)) != 1 ]; then
	guard_die "Only one vendor can be activated: NVIDIA, AMD, and Intel"
fi
if [ $((USE_NVHPC * NVIDIA_GPU + USE_AMDCLANG * AMD_GPU + USE_ICPX * INTEL_GPU)) != 1 ]; then
	guard_die "The compiler does not match the vendor in ${CONFIG:-sh/<system>/config.sh}: USE_NVHPC=1 needs NVIDIA_GPU=1, USE_AMDCLANG=1 needs AMD_GPU=1, and USE_ICPX=1 needs INTEL_GPU=1"
fi

guard_lock

# OpenMP target: loop/distribute, w/ or w/o dedicated options
MIN_MODEL_ID=0
MAX_MODEL_ID=3
mapfile -t MODEL_ID_LIST < <(seq "$MIN_MODEL_ID" "$MAX_MODEL_ID")

# clear modules
for _modules_init in \
	/usr/share/Modules/init/bash \
	/usr/share/modules/init/bash \
	/etc/profile.d/modules.sh \
	/usr/local/Modules/init/bash; do
	if [ -f "$_modules_init" ]; then
		# shellcheck source=/dev/null # the module init script is found at run time
		. "$_modules_init"
		break
	fi
done
unset _modules_init
if ! command -v module > /dev/null 2>&1; then
	guard_die "the module command (Environment Modules) is not found: the job scripts of this sample load the compilers with it, and no init script was found in /usr/share/Modules/init/bash, /usr/share/modules/init/bash, /etc/profile.d/modules.sh, or /usr/local/Modules/init/bash"
fi
module purge
if ! declare -p MODULE_USE > /dev/null 2>&1; then
	guard_die "MODULE_USE is not set: define it in ${CONFIG:-sh/<system>/config.sh} (MODULE_USE=() for none)"
fi
for _module_dir in "${MODULE_USE[@]}"; do
	if [ ! -d "$_module_dir" ]; then
		guard_die "MODULE_USE in ${CONFIG:-sh/<system>/config.sh} holds ${_module_dir}, which is not a directory"
	fi
	module use "$_module_dir"
done
unset _module_dir
hostname

# recipe for NVIDIA GPU
if [ "$NVIDIA_GPU" = 1 ]; then
	guard_module CUDA_MODULE nvcc
	VENDER=nvidia
	ARCH=$NVIDIA_ARCH
fi

# recipe for AMD GPU
if [ "$AMD_GPU" = 1 ]; then
	guard_module ROCM_MODULE hipcc
	VENDER=amd
	GPU_ID=$AMD_GPU_ID
	ARCH=$AMD_ARCH
fi

# recipe for Intel GPU
if [ "$INTEL_GPU" = 1 ]; then
	VENDER=intel
	ARCH=$INTEL_ARCH
fi

# recipe for NVIDIA HPC SDK
if [ "$USE_NVHPC" = 1 ]; then
	COMPILER=nvhpc
	if [ "$NVHPC_PURGE" = 1 ]; then
		module purge
	fi
	guard_module NVHPC_MODULE nvc++
	mapfile -t -O "${#MODEL_ID_LIST[@]}" MODEL_ID_LIST < <(seq $((MAX_MODEL_ID + 1)) 7) # OpenACC: kernels/parallel, w/ or w/o dedicated options
fi

# recipe for ROCm
if [ "$USE_AMDCLANG" = 1 ]; then
	COMPILER=amdclang
	guard_module ROCM_MODULE amdclang++
fi

# recipe for Intel oneAPI
if [ "$USE_ICPX" = 1 ]; then
	COMPILER=icpx
	guard_module INTEL_MODULE icpx
fi

module load "$BOOST_MODULE" || guard_die "module load $BOOST_MODULE failed: check BOOST_MODULE in ${CONFIG:-sh/<system>/config.sh}"

guard_command numactl

# GPU selection and NUMA node of the GPU (numa_node of the GPU in sysfs, INTEL_NUMA_NODE for an Intel GPU, or the
# first node of `numactl --show`); the runs are bound to it only when it is a node number
NUMA_NODE=
if [ "$DETECT_NUMA" = 1 ]; then
	if [ "$VENDER" = amd ]; then
		# a job scheduler that assigns the GPU sets ROCR_VISIBLE_DEVICES: keep it, and use its (first) GPU
		if [ -n "${ROCR_VISIBLE_DEVICES:-}" ]; then
			GPU_ID=${ROCR_VISIBLE_DEVICES%%,*}
			echo "ROCR_VISIBLE_DEVICES=${ROCR_VISIBLE_DEVICES} is kept (GPU_ID of the config is not used)"
		else
			export ROCR_VISIBLE_DEVICES=$GPU_ID
		fi
		NUMA_NODE=$(LANG=C rocm-smi -d "$GPU_ID" --showtoponuma | sed -n 's/^GPU\['"$GPU_ID"'\]\t*//p' | sed -n 's/: (Topology) Numa Node: *//p')
	fi
	if [ "$VENDER" = nvidia ]; then
		# a job scheduler that assigns the GPU sets CUDA_VISIBLE_DEVICES: keep it, and use its (first) GPU
		if [ -n "${CUDA_VISIBLE_DEVICES:-}" ]; then
			GPU_ID=${CUDA_VISIBLE_DEVICES%%,*}
			echo "CUDA_VISIBLE_DEVICES=${CUDA_VISIBLE_DEVICES} is kept (GPU_ID of the config is not used)"
		else
			export CUDA_VISIBLE_DEVICES=$GPU_ID
		fi
		# nvidia-smi prints the PCI domain in 8 hex digits, sysfs in 4 or more (00000000:21:00.0 is 0000:21:00.0)
		BUS_ID=$(nvidia-smi --format=csv,noheader --query-gpu=gpu_bus_id -i "$GPU_ID" | tr '[:upper:]' '[:lower:]')
		if [[ $BUS_ID =~ ^([0-9a-f]+):([0-9a-f]+:[0-9a-f]+\.[0-9a-f]+)$ ]]; then
			BUS_ID=$(printf '%04x:%s' "$((16#${BASH_REMATCH[1]}))" "${BASH_REMATCH[2]}")
			NUMA_NODE=$(cat "/sys/bus/pci/devices/$BUS_ID/numa_node")
		fi
	fi
	if [ "$VENDER" = intel ]; then
		NUMA_NODE=$INTEL_NUMA_NODE
	fi
	# not detected: the first node of numactl --show (-1, a GPU without a NUMA node, is left unbound)
	if [ -z "$NUMA_NODE" ]; then
		NUMA_NODE=$(LANG=C numactl --show | sed -n 's/^nodebind: *//p' | awk '{ print $1 }')
		if [ "$VENDER" != intel ]; then
			guard_warn "NUMA node of GPU ${GPU_ID} not detected: binding the runs to node ${NUMA_NODE:-(none)}, the first node of numactl --show"
		fi
	fi
	if ! [[ $NUMA_NODE =~ ^[0-9]+$ ]]; then
		guard_warn "NUMA node of the GPU not found (got '${NUMA_NODE}'): the runs are not bound to a NUMA node"
		NUMA_NODE=
	fi
fi

TARGET=${COMPILER}_${VENDER}
module list
if [ -n "$NUMA_NODE" ]; then
	NUMACTL=(numactl "--cpunodebind=$NUMA_NODE" --localalloc)
else
	NUMACTL=(numactl --localalloc)
fi

# USE_* of MODEL_ID
set_model() {
	USE_OPENACC=0
	USE_ACC_PARALLEL=0
	USE_OMP_DISTRIBUTE=0
	USE_FAST_MATH=0
	if [ $((MODEL_ID % 2)) -eq 1 ]; then
		USE_FAST_MATH=1
	fi
	if [ "$MODEL_ID" -ge 4 ]; then
		USE_OPENACC=1
	fi
	if [ "$MODEL_ID" -eq 2 ] || [ "$MODEL_ID" -eq 3 ]; then
		USE_OMP_DISTRIBUTE=1
	fi
	if [ "$MODEL_ID" -eq 6 ] || [ "$MODEL_ID" -eq 7 ]; then
		USE_ACC_PARALLEL=1
	fi
}

# THREADS of MODEL_ID for the compiler
set_threads() {
	if [ "$USE_NVHPC" = 1 ]; then
		if [ "$MODEL_ID" -eq 0 ]; then
			THREADS=128
		fi
		if [ "$MODEL_ID" -eq 1 ]; then
			THREADS=128
		fi
		if [ "$MODEL_ID" -eq 2 ]; then
			THREADS=128
		fi
		if [ "$MODEL_ID" -eq 3 ]; then
			THREADS=256
		fi
		if [ "$MODEL_ID" -eq 4 ]; then
			THREADS=64
		fi
		if [ "$MODEL_ID" -eq 5 ]; then
			THREADS=64
		fi
		if [ "$MODEL_ID" -eq 6 ]; then
			THREADS=128
		fi
		if [ "$MODEL_ID" -eq 7 ]; then
			THREADS=128
		fi
	fi

	if [ "$USE_AMDCLANG" = 1 ]; then
		if [ "$MODEL_ID" -eq 0 ]; then
			THREADS=256
		fi
		if [ "$MODEL_ID" -eq 1 ]; then
			THREADS=256
		fi
		if [ "$MODEL_ID" -eq 2 ]; then
			THREADS=64
		fi
		if [ "$MODEL_ID" -eq 3 ]; then
			THREADS=64
		fi
	fi

	if [ "$USE_ICPX" = 1 ]; then
		if [ "$MODEL_ID" -eq 0 ]; then
			THREADS=256
		fi
		if [ "$MODEL_ID" -eq 1 ]; then
			THREADS=32
		fi
		if [ "$MODEL_ID" -eq 2 ]; then
			THREADS=512
		fi
		if [ "$MODEL_ID" -eq 3 ]; then
			THREADS=32
		fi
	fi
}
