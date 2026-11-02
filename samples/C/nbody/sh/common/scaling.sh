# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# Performance run of this sample: every model (MODEL_ID) for N from NUM_MIN to NUM_MAX, NUM_ITERATE times (sourced by sh/<system>/scaling.sh).
# Exits with 0 only when every build and run succeeds (the results are not checked).

# shellcheck source=sh/common/setup.sh
. sh/common/setup.sh || exit 1

NUM_MIN=128
NUM_MAX=4194304
NUM_ITERATE=10

DUMP=scaling
guard_leftovers bin log dat fig "${DUMP}"
mkdir -p "${DUMP}"

for MODEL_ID in "${MODEL_ID_LIST[@]}"; do
	set_model
	set_threads

	make dir || guard_die "make dir failed in $PWD (see the message of make above)"
	make clean || guard_die "make clean failed in $PWD (see the message of make above)"
	BUILT=1
	if ! make all NVHPC="$USE_NVHPC" AMDCLANG="$USE_AMDCLANG" ICPX="$USE_ICPX" ACPP="$USE_ACPP" USE_OPENACC="$USE_OPENACC" USE_ACC_PARALLEL="$USE_ACC_PARALLEL" USE_OMP_DISTRIBUTE="$USE_OMP_DISTRIBUTE" USE_FAST_MATH="$USE_FAST_MATH" MODEL_ID="${MODEL_ID}" NUM_THREADS="$THREADS" GPU_ARCH="${ARCH}" BENCHMARK=1 SET_NMIN="${NUM_MIN}" SET_NMAX="${NUM_MAX}"; then
		guard_fail "build: model ${MODEL_ID}"
		BUILT=0
	fi
	APPEND=${COMPILER}_${ARCH}_model${MODEL_ID}_thrd${THREADS}
	for TAG in nbody acc omp; do
		BIN=bin/${TAG}_pragma
		EXEC=${BIN}_${APPEND}
		if [ "$BUILT" = 1 ] && mv "${BIN}" "$EXEC"; then
			mkdir -p log dat fig
			COMMAND=("${NUMACTL[@]}" "$EXEC")
			for ((COUNTER = 0; COUNTER < NUM_ITERATE; COUNTER += 1)); do
				echo "${COMMAND[*]}"
				"${COMMAND[@]}" || guard_fail "run (exit $?): ${COMMAND[*]}"
			done

			mkdir -p "${DUMP}/model${MODEL_ID}/${TAG}"
			mv --backup=numbered log "${DUMP}/model${MODEL_ID}/${TAG}"
		elif [ "$BUILT" = 1 ]; then
			guard_fail "build: model ${MODEL_ID}: make made no ${BIN}"
		fi
	done
	mkdir -p "${DUMP}/model${MODEL_ID}"
	mv --backup=numbered bin "${DUMP}/model${MODEL_ID}"
done

HOST=$(guard_host)
DEST=${HOST}_${TARGET}_${ARCH}
mkdir -p "${DEST}"
mv --backup=numbered "${DUMP}" "${DEST}"
guard_finish
