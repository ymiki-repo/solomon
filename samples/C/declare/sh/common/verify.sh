# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# Verification run of this sample: build and run it in the four models (OpenACC kernels / parallel, OpenMP target
# loop / distribute) with the NVIDIA HPC SDK (sourced by sh/<system>/verify.sh; run from the sample directory).
# Requires NVIDIA_ARCH and NVHPC_MODULE of sh/<system>/config.sh (CONFIG names it) and SAMPLE.
# Exits with 0 only when every model prints the expected value and exits with 0.

# shellcheck source=sh/common/guard.sh
. sh/common/guard.sh || exit 1
guard_sample
guard_value NVIDIA_ARCH
guard_value NVHPC_MODULE
guard_lock

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
	guard_die "the module command (Environment Modules) is not found: this job script loads the compiler with it, and no init script was found in /usr/share/Modules/init/bash, /usr/share/modules/init/bash, /etc/profile.d/modules.sh, or /usr/local/Modules/init/bash"
fi
module purge
module load "$NVHPC_MODULE" || guard_die "module load $NVHPC_MODULE failed: check NVHPC_MODULE in ${CONFIG:-sh/<system>/config.sh}"
guard_command make nvc nvidia-smi

hostname
nvidia-smi --query-gpu=name,driver_version,compute_cap --format=csv,noheader
nvc --version
nvfortran --version

MODELS=(
	"acc_kernels:USE_OPENACC=1 USE_ACC_PARALLEL=0"
	"acc_parallel:USE_OPENACC=1 USE_ACC_PARALLEL=1"
	"omp_loop:USE_OPENACC=0 USE_OMP_DISTRIBUTE=0"
	"omp_distribute:USE_OPENACC=0 USE_OMP_DISTRIBUTE=1"
)
for entry in "${MODELS[@]}"; do
	model="${entry%%:*}"
	read -r -a vars <<< "${entry#*:}"
	echo "=== ${model}"
	make clean > /dev/null || guard_die "make clean failed in $PWD (see the message of make above)"
	if ! make "${vars[@]}" GPU_ARCH="$NVIDIA_ARCH"; then
		echo "RESULT ${model}: FAIL (build)"
		guard_fail "build: ${model}"
		continue
	fi
	./run
	rc=$?
	if [ "$rc" -eq 0 ]; then
		echo "RESULT ${model}: PASS"
	else
		echo "RESULT ${model}: FAIL (exit code ${rc})"
		guard_fail "run (exit ${rc}): ${model}"
	fi
done
make clean > /dev/null || guard_die "make clean failed in $PWD (see the message of make above)"
guard_finish
