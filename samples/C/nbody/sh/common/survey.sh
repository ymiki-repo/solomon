# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# Performance survey of this sample: every model (MODEL_ID) at N = NUM for every power-of-two THREADS (sourced by sh/<system>/survey.sh).
# Exits with 0 only when every build and run succeeds (a run cut by RUN_TIMEOUT counts as failed; the results are not checked).

# shellcheck source=sh/common/setup.sh
. sh/common/setup.sh || exit 1

NUM=2097152
# upper bound of the time of one run (RUN_TIMEOUT of sh/<system>/config.sh; 40s when not set): a longer run fails (exit 124)
RUN_TIMEOUT=${RUN_TIMEOUT:-40s}
if ! [[ $RUN_TIMEOUT =~ ^[0-9]+(\.[0-9]+)?[smhd]?$ && $RUN_TIMEOUT =~ [1-9] ]]; then
	guard_die "RUN_TIMEOUT='${RUN_TIMEOUT}' in ${CONFIG:-sh/<system>/config.sh}: set a positive duration of timeout (0 would mean no limit), e.g. 40s or 2m"
fi
NUM_ITERATE=3

DUMP=survey
guard_leftovers bin log dat fig "${DUMP}"

# range of THREADS for the GPU vendor
if [ "$VENDER" = nvidia ]; then
	MIN_THREADS=32
	MAX_THREADS=1024
fi
if [ "$VENDER" = amd ]; then
	MIN_THREADS=64
	MAX_THREADS=1024
fi
if [ "$VENDER" = intel ]; then
	MIN_THREADS=16
	MAX_THREADS=1024
fi

guard_command timeout
make dir || guard_die "make dir failed in $PWD (see the message of make above)"

THREADS_MIN=${MIN_THREADS}
THREADS_MAX=${MAX_THREADS}
for MODEL_ID in "${MODEL_ID_LIST[@]}"; do
	set_model

	for ((THREADS = THREADS_MIN; THREADS <= THREADS_MAX; THREADS *= 2)); do
		make clean || guard_die "make clean failed in $PWD (see the message of make above)"
		BUILT=1
		if ! make all NVHPC="$USE_NVHPC" AMDCLANG="$USE_AMDCLANG" ICPX="$USE_ICPX" ACPP="$USE_ACPP" USE_OPENACC="$USE_OPENACC" USE_ACC_PARALLEL="$USE_ACC_PARALLEL" USE_OMP_DISTRIBUTE="$USE_OMP_DISTRIBUTE" USE_FAST_MATH="$USE_FAST_MATH" MODEL_ID="${MODEL_ID}" NUM_THREADS="$THREADS" GPU_ARCH="${ARCH}" BENCHMARK=1 SET_NMIN="${NUM}" SET_NMAX="${NUM}"; then
			guard_fail "build: model ${MODEL_ID}, THREADS ${THREADS}"
			BUILT=0
		fi
		APPEND=${COMPILER}_${ARCH}_model${MODEL_ID}_thrd${THREADS}
		for BIN in bin/nbody_pragma bin/acc_pragma bin/omp_pragma; do
			EXEC=${BIN}_${APPEND}
			if [ "$BUILT" = 1 ] && mv "${BIN}" "$EXEC"; then
				COMMAND=(timeout "${RUN_TIMEOUT}" "${NUMACTL[@]}" "$EXEC")
				for ((COUNTER = 0; COUNTER < NUM_ITERATE; COUNTER += 1)); do
					echo "${COMMAND[*]}"
					"${COMMAND[@]}"
					RC=$?
					if [ "$RC" -eq 124 ]; then
						guard_fail "run (exit ${RC}, timed out after RUN_TIMEOUT=${RUN_TIMEOUT}): ${COMMAND[*]}"
					elif [ "$RC" -ne 0 ]; then
						guard_fail "run (exit ${RC}): ${COMMAND[*]}"
					fi
				done
			elif [ "$BUILT" = 1 ]; then
				guard_fail "build: model ${MODEL_ID}, THREADS ${THREADS}: make made no ${BIN}"
			fi
		done
	done
done

mkdir -p "${DUMP}"
mv --backup=numbered bin log "${DUMP}/" || guard_fail "move: bin/ and log/ into ${DUMP}/"
HOST=$(guard_host)
DEST=${HOST}_${TARGET}_${ARCH}
mkdir -p "${DEST}"
mv --backup=numbered "${DUMP}" "${DEST}/" || guard_fail "move: ${DUMP}/ into ${DEST}/ (the results stay in ${PWD}/${DUMP}/)"
guard_finish
