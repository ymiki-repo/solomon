# shellcheck shell=bash
# shellcheck source-path=SCRIPTDIR/../..
# shellcheck disable=SC2034 # the variables set here are read by sh/common/sweep.sh (permanent)
# Performance run of this sample: every model, optimization level, and problem size NUM_ITERATE times (sourced by sh/<system>/iterate.sh).
# Exits with 0 only when every build and run succeeds (the results are not checked).

DUMP=iterate
BENCHMARK=1
NUM_ITERATE=10
SIZES=(32 64 128 256 512)
TOLERANCE=
# shellcheck source=sh/common/sweep.sh
. sh/common/sweep.sh
