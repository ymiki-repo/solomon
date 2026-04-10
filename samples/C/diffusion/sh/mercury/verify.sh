#!/bin/bash
#SBATCH -J verify
#SBATCH -p share-batch
#SBATCH --gres=gpu:1
#SBATCH --time=02:00:00

USE_NVHPC=1
USE_AMDCLANG=0
USE_ICPX=0
USE_ACPP=0
if [ $(($USE_NVHPC + $USE_AMDCLANG + $USE_ICPX + $USE_ACPP)) != 1 ]; then
	echo "Only one compiler can be activated: USE_NVHPC, USE_AMDCLANG, USE_ICPX, and USE_ACPP"
	exit 1
fi

TIMEOUT=120s
GPU_ID=0

# OpenMP loop/distribute (2)
MIN_MODEL_ID=0
MAX_MODEL_ID=1
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
# module use $HOME/opt/modules/compiler
hostname

# recipe for NVIDIA GPU
module load cuda/12.3
nvcc --version
VENDER=nvidia
ARCH=90

# recipe for NVIDIA HPC SDK
if [ $USE_NVHPC == 1 ]; then
	COMPILER=nvhpc
	module load nvidia
	nvc++ --version
	# MODEL_ID_LIST+=(`seq $(($MAX_MODEL_ID + 1)) 3`) # (OpenMP loop/distribute (2) + OpenACC kernels/parallel (2)) = 4 models
	MODEL_ID_LIST+=(`seq $(($MAX_MODEL_ID + 1)) 7`) # (OpenMP loop/distribute (2) + OpenACC kernels/parallel (2)) * (data/managed (2)) = 8 models
	# MODEL_ID_LIST+=(`seq $(($MAX_MODEL_ID + 1)) 15`) # (OpenMP loop/distribute (2) + OpenACC kernels/parallel (2)) * (data/managed/unified/unified+first touch (4)) = 16 models
fi

# # NUMA configuration
# export CUDA_VISIBLE_DEVICES=$GPU_ID
# BUS_ID=`nvidia-smi --format=csv,noheader --query-gpu=gpu_bus_id -i $GPU_ID | awk -F ":" '{print "0000:" $2 ":" $3}' | tr '[:upper:]' '[:lower:]'`
# NUMA_NODE=`cat /sys/bus/pci/devices/$BUS_ID/numa_node`
# if [ "${NUMA_NODE}" == "" ]; then
# 	AVAILABLE_NUMA_NODE=`LANG=C numactl --show | sed -n 's/^nodebind: *//p'`
# 	NUMA_NODE=${AVAILABLE_NUMA_NODE[0]}
# fi

cd $SLURM_SUBMIT_DIR
TARGET=${COMPILER}_${VENDER}
module list

DUMP=verify
mkdir -p "${DUMP}"

for MODEL_ID in ${MODEL_ID_LIST[@]}
do
	USE_OPENACC=0
	USE_ACC_PARALLEL=0
	USE_OMP_DISTRIBUTE=0
	USE_MANAGED=0
	USE_UNIFIED=0
	APPLY_FIRST_TOUCH=0

	if [ $(( $MODEL_ID % 4 )) -eq 1 ]; then
		USE_OMP_DISTRIBUTE=1
	fi
	if [ $(( $MODEL_ID % 4 )) -eq 2 ]; then
		USE_OPENACC=1
	fi
	if [ $(( $MODEL_ID % 4 )) -eq 3 ]; then
		USE_OPENACC=1
		USE_ACC_PARALLEL=1
	fi
	if [ $MODEL_ID -ge 4 ]; then
		USE_MANAGED=1
	fi
	if [ $MODEL_ID -ge 8 ]; then
		USE_MANAGED=0
		USE_UNIFIED=1
	fi
	if [ $MODEL_ID -ge 12 ]; then
		APPLY_FIRST_TOUCH=1
	fi

	mkdir -p ${DUMP}/model${MODEL_ID}
	BIN=./run
	APPEND=${COMPILER}_${ARCH}_model${MODEL_ID}
	BINARY=${BIN}_${APPEND}
	for OPT_LEVEL in 0 1 2 3 4
	do
		make all NVHPC=$USE_NVHPC AMDCLANG=$USE_AMDCLANG ICPX=$USE_ICPX ACPP=$USE_ACPP USE_OPENACC=$USE_OPENACC USE_ACC_PARALLEL=$USE_ACC_PARALLEL USE_OMP_DISTRIBUTE=$USE_OMP_DISTRIBUTE USE_MANAGED=$USE_MANAGED USE_UNIFIED=$USE_UNIFIED APPLY_FIRST_TOUCH=${APPLY_FIRST_TOUCH} MODEL_ID=${MODEL_ID} OPT_LEVEL=${OPT_LEVEL} GPU_ARCH=${ARCH} BENCHMARK=0
		EXEC=${BINARY}_opt${OPT_LEVEL}
		mv ${BIN} $EXEC
		if [ -e $EXEC ]; then
			for NUM in 32 64 128 256 512
			do
				COMMAND="timeout ${TIMEOUT} numactl --localalloc $EXEC $NUM"
				echo ${COMMAND}
				eval ${COMMAND}
			done
		fi
		make clean
	done
	mv --backup=numbered ${BINARY}_* *.csv ${DUMP}/model${MODEL_ID}/
done

HOST=`hostname --short`
DEST=${HOST}_${TARGET}_${ARCH}
mkdir -p "${DEST}"
mv --backup=numbered ${DUMP} ${DEST}

exit 0
