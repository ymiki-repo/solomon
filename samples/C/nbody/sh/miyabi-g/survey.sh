#!/usr/bin/env bash
#PBS -q short-g
#PBS -l select=1
#PBS -l walltime=07:00:00
#PBS -W group_list=gj14
#PBS -N survey

cd ${PBS_O_WORKDIR}

USE_NVHPC=1
USE_AMDCLANG=0
USE_ICPX=0
USE_ACPP=0
if [ $(($USE_NVHPC + $USE_AMDCLANG + $USE_ICPX + $USE_ACPP)) != 1 ]; then
	echo "Only one compiler can be activated: USE_NVHPC, USE_AMDCLANG, USE_ICPX, and USE_ACPP"
	exit 1
fi
NVIDIA_GPU=1
AMD_GPU=0
INTEL_GPU=0
if [ $(($NVIDIA_GPU + $AMD_GPU + $INTEL_GPU)) != 1 ]; then
	echo "Only one vendor can be activated: NVIDIA, AMD, and Intel"
	exit 1
fi

NUM=2097152
TIMEOUT=40s # 3 TFlop/s at N = 2M: ~40 s
NUM_ITERATE=3
GPU_ID=0

# OpenMP target: loop/distribute, w/ or w/o dedicated options
MIN_MODEL_ID=0
MAX_MODEL_ID=3
MODEL_ID_LIST=(`seq $MIN_MODEL_ID $MAX_MODEL_ID`)

# clear modules
for _modules_init in \
	/usr/share/Modules/init/bash \
	/usr/share/modules/init/bash \
	/etc/profile.d/modules.sh \
	/usr/local/Modules/init/bash; do
	if [ -f "$_modules_init" ]; then
		. "$_modules_init"
		break
	fi
done
unset _modules_init
module purge
module use /work/gj14/share/opt/modules/lib
hostname

# recipe for NVIDIA GPU
if [ $NVIDIA_GPU == 1 ]; then
	module load cuda
	nvcc --version
	VENDER=nvidia
	# ARCH=80
	ARCH=90
	MIN_THREADS=32
	MAX_THREADS=1024
fi

# recipe for AMD GPU
if [ $AMD_GPU == 1 ]; then
	module load rocm
	hipcc --version
	VENDER=amd
	# ARCH=gfx908
	GPU_ID=2
	ARCH=gfx90a
	MIN_THREADS=64
	MAX_THREADS=1024
fi

# recipe for Intel GPU
if [ $INTEL_GPU == 1 ]; then
	VENDER=intel
	ARCH=pvc
	MIN_THREADS=16
	MAX_THREADS=1024
fi

# recipe for NVIDIA HPC SDK
if [ $USE_NVHPC == 1 ]; then
	COMPILER=nvhpc
	module purge
	module load nvidia
	nvc++ --version
	MODEL_ID_LIST+=(`seq $(($MAX_MODEL_ID + 1)) 7`) # OpenACC: kernels/parallel, w/ or w/o dedicated options
fi

# recipe for ROCm
if [ $USE_AMDCLANG == 1 ]; then
	COMPILER=amdclang
	module load rocm
	amdclang++ --version
fi

# recipe for Intel oneAPI
if [ $USE_ICPX == 1 ]; then
	COMPILER=icpx
	module load intel
	icpx --version
fi

module load boost

# # NUMA configuration
# if [ $VENDER == amd ]; then
# 	export ROCR_VISIBLE_DEVICES=$GPU_ID
# 	NUMA_NODE=`LANG=C rocm-smi -d $GPU_ID --showtoponuma | sed -n 's/^GPU\['$GPU_ID'\]\t*//p' | sed -n 's/: (Topology) Numa Node: *//p'`
# fi
# if [ $VENDER == nvidia ]; then
# 	export CUDA_VISIBLE_DEVICES=$GPU_ID
# 	BUS_ID=`nvidia-smi --format=csv,noheader --query-gpu=gpu_bus_id -i $GPU_ID | awk -F ":" '{print "0000:" $2 ":" $3}' | tr '[:upper:]' '[:lower:]'`
# 	NUMA_NODE=`cat /sys/bus/pci/devices/$BUS_ID/numa_node`
# fi
# if [ $VENDER == intel ]; then
#         # tentative treatment for spr2
#         NUMA_NODE=0
# fi
# if [ "${NUMA_NODE}" == "" ]; then
# 	AVAILABLE_NUMA_NODE=`LANG=C numactl --show | sed -n 's/^nodebind: *//p'`
# 	NUMA_NODE=${AVAILABLE_NUMA_NODE[0]}
# fi

TARGET=${COMPILER}_${VENDER}
module list
make dir

THREADS_MIN=${MIN_THREADS}
THREADS_MAX=${MAX_THREADS}
for MODEL_ID in ${MODEL_ID_LIST[@]}
do
	USE_OPENACC=0
	USE_ACC_PARALLEL=0
	USE_OMP_DISTRIBUTE=0
	USE_FAST_MATH=0
	if [ $(( $MODEL_ID % 2 )) -eq 1 ]; then
		USE_FAST_MATH=1
	fi
	if [ $MODEL_ID -ge 4 ]; then
		USE_OPENACC=1
	fi
	if [ $MODEL_ID -eq 2 ] || [ $MODEL_ID -eq 3 ] ; then
		USE_OMP_DISTRIBUTE=1
	fi
	if [ $MODEL_ID -eq 6 ] || [ $MODEL_ID -eq 7 ] ; then
		USE_ACC_PARALLEL=1
	fi

	for (( THREADS = ${THREADS_MIN} ; THREADS <= ${THREADS_MAX} ; THREADS *= 2 ))
	do
		make clean
		make all NVHPC=$USE_NVHPC AMDCLANG=$USE_AMDCLANG ICPX=$USE_ICPX ACPP=$USE_ACPP USE_OPENACC=$USE_OPENACC USE_ACC_PARALLEL=$USE_ACC_PARALLEL USE_OMP_DISTRIBUTE=$USE_OMP_DISTRIBUTE USE_FAST_MATH=$USE_FAST_MATH MODEL_ID=${MODEL_ID} NUM_THREADS=$THREADS GPU_ARCH=${ARCH} BENCHMARK=1 SET_NMIN=${NUM} SET_NMAX=${NUM}
		APPEND=${COMPILER}_${ARCH}_model${MODEL_ID}_thrd${THREADS}
		for BIN in bin/nbody_pragma bin/acc_pragma bin/omp_pragma
		do
			EXEC=${BIN}_${APPEND}
			mv ${BIN} $EXEC
			if [ -e $EXEC ]; then
				COMMAND="timeout ${TIMEOUT} numactl --localalloc $EXEC"
				for (( COUNTER = 0 ; COUNTER < ${NUM_ITERATE} ; COUNTER += 1 ))
				do
					sleep ${TIMEOUT}
					echo ${COMMAND}
					eval ${COMMAND}
				done
			fi
		done
	done
done

DUMP=survey
mkdir -p "${DUMP}"
mv bin log ${DUMP}
HOST=`hostname --short`
DEST=${HOST}_${TARGET}_${ARCH}
mkdir -p "${DEST}"
mv --backup=numbered ${DUMP} ${DEST}
