# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# Verification run of this sample: build and run every model (MODEL_ID) once and plot the errors (sourced by sh/<system>/verify.sh).
# Exits with 0 only when every build and run succeeds and every run conserves the energy:
# 0 < |E(t=10)/E(0) - 1| <= TOLERANCE (log/collapse_error.dat) and E_pot(t=0) != 0 (dat/collapse_energy.dat).

# shellcheck source=sh/common/setup.sh
. sh/common/setup.sh || exit 1

DUMP=verify
TOLERANCE=1.0e-2
guard_leftovers bin log dat fig "${DUMP}"
mkdir -p "${DUMP}"
PLOT=1
if ! command -v gnuplot > /dev/null 2>&1; then
	guard_warn "gnuplot not found: the figures (fig/) are not drawn"
	PLOT=0
fi

for MODEL_ID in "${MODEL_ID_LIST[@]}"; do
	set_model
	set_threads

	make dir || guard_die "make dir failed in $PWD (see the message of make above)"
	make clean || guard_die "make clean failed in $PWD (see the message of make above)"
	BUILT=1
	if ! make all NVHPC="$USE_NVHPC" AMDCLANG="$USE_AMDCLANG" ICPX="$USE_ICPX" ACPP="$USE_ACPP" USE_OPENACC="$USE_OPENACC" USE_ACC_PARALLEL="$USE_ACC_PARALLEL" USE_OMP_DISTRIBUTE="$USE_OMP_DISTRIBUTE" USE_FAST_MATH="$USE_FAST_MATH" MODEL_ID="${MODEL_ID}" NUM_THREADS="$THREADS" GPU_ARCH="${ARCH}" BENCHMARK=0; then
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
			echo "${COMMAND[*]}"
			if "${COMMAND[@]}"; then
				ERROR=$(awk 'NF >= 2 { v = $2 } END { print v }' log/collapse_error.dat 2> /dev/null)
				EPOT0=$(awk '!/^#/ && NF >= 4 { print $4; exit }' dat/collapse_energy.dat 2> /dev/null)
				if ! awk -v x="$ERROR" -v p="$EPOT0" -v tol="$TOLERANCE" 'BEGIN {
					num = "^[-+]?([0-9]+\\.?[0-9]*|\\.[0-9]+)([eE][-+]?[0-9]+)?$"
					if (x !~ num || p !~ num) exit 1
					a = (x < 0) ? -x : x
					exit !(a > 0 && a <= tol + 0 && p + 0 != 0)
				}'; then
					guard_fail "result: model ${MODEL_ID}, ${TAG}: |E(t=10)/E(0) - 1| = ${ERROR:-(missing)}, E_pot(t=0) = ${EPOT0:-(missing)} (expected 0 < |x| <= ${TOLERANCE} and E_pot(t=0) != 0)"
				fi
				# the figures of a successful run; a failed plot is reported, but it is not a failure of the run
				if [ "$PLOT" = 1 ]; then
					for GP in plt/error.gp plt/virial.gp plt/map.gp; do
						gnuplot "$GP" || guard_warn "plot: model ${MODEL_ID}, ${TAG}: gnuplot ${GP} failed"
					done
				fi
			else
				guard_fail "run (exit $?): ${COMMAND[*]}"
			fi
			mkdir -p "${DUMP}/model${MODEL_ID}/${TAG}"
			mv --backup=numbered log dat fig "${DUMP}/model${MODEL_ID}/${TAG}/" ||
				guard_fail "move: log/, dat/, and fig/ of model ${MODEL_ID}, ${TAG} into ${DUMP}/model${MODEL_ID}/${TAG}/"
		elif [ "$BUILT" = 1 ]; then
			guard_fail "build: model ${MODEL_ID}: make made no ${BIN}"
		fi
	done
	mkdir -p "${DUMP}/model${MODEL_ID}"
	mv --backup=numbered bin "${DUMP}/model${MODEL_ID}/" || guard_fail "move: bin/ of model ${MODEL_ID} into ${DUMP}/model${MODEL_ID}/"
done

HOST=$(guard_host)
DEST=${HOST}_${TARGET}_${ARCH}
mkdir -p "${DEST}"
mv --backup=numbered "${DUMP}" "${DEST}/" || guard_fail "move: ${DUMP}/ into ${DEST}/ (the results stay in ${PWD}/${DUMP}/)"
guard_finish
