# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# shellcheck disable=SC2034 # the variables set here are read by sh/common/sweep.sh (permanent)
# Verification run of this sample: every model, optimization level, and problem size once (sourced by sh/<system>/verify.sh).
# Exits with 0 only when every build and run succeeds and every run prints an error below TOLERANCE.

DUMP=verify
BENCHMARK=0
NUM_ITERATE=1
SIZES=(32 37 64 128 256 512) # 37 takes an odd number of time steps with every compiler tried
# a result one time step old gives 4.4e-4 at N=37, and the largest correct error seen is 9.4e-5 (N=512)
TOLERANCE=2.0e-4
# shellcheck source=sh/common/sweep.sh
. sh/common/sweep.sh
