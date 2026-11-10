# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# Common recipe of this sample: build and run every model (MODEL_ID) at every optimization level and problem size.
# Sourced by sh/common/verify.sh and sh/common/iterate.sh, which set DUMP (output directory name),
# BENCHMARK (passed to make), NUM_ITERATE (runs per binary and size), SIZES (problem sizes), and
# TOLERANCE (upper bound of the error each run prints; empty: not checked).
# Exits with 1 when a build or a run fails (or, with TOLERANCE, a run does not print an error below it).

# shellcheck source=sh/common/setup.sh
. sh/common/setup.sh || exit 1
guard_leftovers ./run_* ./*.csv "${DUMP}"

# upper bound of the time of one run (RUN_TIMEOUT of sh/<system>/config.sh; 600s when not set): a longer run fails (exit 124)
RUN_TIMEOUT=${RUN_TIMEOUT:-600s}
if ! [[ $RUN_TIMEOUT =~ ^[0-9]+(\.[0-9]+)?[smhd]?$ && $RUN_TIMEOUT =~ [1-9] ]]; then
	guard_die "RUN_TIMEOUT='${RUN_TIMEOUT}' in ${CONFIG:-sh/<system>/config.sh}: set a positive duration of timeout (0 would mean no limit), e.g. 600s or 10m"
fi
guard_command timeout numactl

# timed_out <exit code>: ", timed out after RUN_TIMEOUT" for the exit code of timeout (124); nothing otherwise
timed_out() {
	if [ "$1" -eq 124 ]; then
		echo ", timed out after RUN_TIMEOUT=${RUN_TIMEOUT}"
	fi
}

mkdir -p "${DUMP}"

make clean || guard_die "make clean failed in $PWD (see the message of make above)"
for MODEL_ID in "${MODEL_ID_LIST[@]}"; do
	USE_OPENACC=0
	USE_ACC_PARALLEL=0
	USE_OMP_DISTRIBUTE=0
	USE_MANAGED=0
	USE_UNIFIED=0
	APPLY_FIRST_TOUCH=0

	if [ $((MODEL_ID % 4)) -eq 1 ]; then
		USE_OMP_DISTRIBUTE=1
	fi
	if [ $((MODEL_ID % 4)) -eq 2 ]; then
		USE_OPENACC=1
	fi
	if [ $((MODEL_ID % 4)) -eq 3 ]; then
		USE_OPENACC=1
		USE_ACC_PARALLEL=1
	fi
	if [ "$MODEL_ID" -ge 4 ]; then
		USE_MANAGED=1
	fi
	if [ "$MODEL_ID" -ge 8 ]; then
		USE_MANAGED=0
		USE_UNIFIED=1
	fi
	if [ "$MODEL_ID" -ge 12 ]; then
		APPLY_FIRST_TOUCH=1
	fi

	mkdir -p "${DUMP}/model${MODEL_ID}"
	BIN=./run
	APPEND=${COMPILER}_${ARCH}_model${MODEL_ID}
	BINARY=${BIN}_${APPEND}
	for OPT_LEVEL in 0 1 2 3 4; do
		EXEC=${BINARY}_opt${OPT_LEVEL}
		if ! make all NVHPC="$USE_NVHPC" AMDCLANG="$USE_AMDCLANG" ICPX="$USE_ICPX" ACPP="$USE_ACPP" USE_OPENACC="$USE_OPENACC" USE_ACC_PARALLEL="$USE_ACC_PARALLEL" USE_OMP_DISTRIBUTE="$USE_OMP_DISTRIBUTE" USE_MANAGED="$USE_MANAGED" USE_UNIFIED="$USE_UNIFIED" APPLY_FIRST_TOUCH="${APPLY_FIRST_TOUCH}" MODEL_ID="${MODEL_ID}" OPT_LEVEL="${OPT_LEVEL}" GPU_ARCH="${ARCH}" BENCHMARK="${BENCHMARK}"; then
			guard_fail "build: model ${MODEL_ID}, opt ${OPT_LEVEL}"
		elif ! mv "${BIN}" "$EXEC"; then
			guard_fail "build: model ${MODEL_ID}, opt ${OPT_LEVEL}: make made no ${BIN}"
		else
			for NUM in "${SIZES[@]}"; do
				for ((COUNTER = 0; COUNTER < NUM_ITERATE; COUNTER += 1)); do
					COMMAND=(timeout "${RUN_TIMEOUT}" numactl --localalloc "$EXEC" "$NUM")
					echo "${COMMAND[*]}"
					if [ -z "${TOLERANCE}" ]; then
						"${COMMAND[@]}"
						RC=$?
						if [ "$RC" -ne 0 ]; then
							guard_fail "run (exit ${RC}$(timed_out "$RC")): ${COMMAND[*]}"
						fi
						continue
					fi
					OUTPUT=$("${COMMAND[@]}")
					RC=$?
					if [ -n "$OUTPUT" ]; then
						printf '%s\n' "$OUTPUT"
					fi
					if [ "$RC" -ne 0 ]; then
						guard_fail "run (exit ${RC}$(timed_out "$RC")): ${COMMAND[*]}"
						continue
					fi
					ERROR=$(printf '%s\n' "$OUTPUT" | awk '/^Error\[/ { sub(/^[^=]*= */, ""); v = $1 } END { print v }')
					if ! awk -v x="$ERROR" -v tol="$TOLERANCE" 'BEGIN { exit !(x ~ /^[-+]?([0-9]+\.?[0-9]*|\.[0-9]+)([eE][-+]?[0-9]+)?$/ && x + 0 < tol + 0) }'; then
						guard_fail "result: ${COMMAND[*]}: Error = ${ERROR:-(no Error line)} (expected below ${TOLERANCE})"
					fi
				done
			done
		fi
		make clean || guard_die "make clean failed in $PWD (see the message of make above)"
	done
	shopt -s nullglob
	OUTPUTS=("${BINARY}"_* ./*.csv)
	shopt -u nullglob
	if [ ${#OUTPUTS[@]} -gt 0 ]; then
		mv --backup=numbered -- "${OUTPUTS[@]}" "${DUMP}/model${MODEL_ID}/" || guard_fail "move: the outputs of model ${MODEL_ID} into ${DUMP}/model${MODEL_ID}/"
	fi
done

HOST=$(guard_host)
DEST=${HOST}_${TARGET}_${ARCH}
mkdir -p "${DEST}"
mv --backup=numbered "${DUMP}" "${DEST}/" || guard_fail "move: ${DUMP}/ into ${DEST}/ (the results stay in ${PWD}/${DUMP}/)"
guard_finish
