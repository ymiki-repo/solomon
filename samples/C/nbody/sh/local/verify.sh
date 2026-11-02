#!/usr/bin/env bash
# Run from the sample directory: bash sh/local/verify.sh

if [ -z "${BASH_VERSION:-}" ]; then
	echo "run this job script with bash: bash $0" >&2
	exit 1
fi

SAMPLE=C/nbody
CONFIG=sh/local/config.sh
if [ $# -gt 0 ]; then
	echo "this job script takes no arguments (its settings are in $CONFIG): bash $0" >&2
	exit 1
fi
if [ ! -f "$CONFIG" ]; then
	echo "$CONFIG not found in $PWD: run (or submit) this job script in the directory samples/$SAMPLE" >&2
	exit 1
fi
# shellcheck source-path=SCRIPTDIR/../.. source=sh/local/config.sh
. "$CONFIG" || exit 1
# shellcheck source-path=SCRIPTDIR/../.. source=sh/common/verify.sh
. sh/common/verify.sh
