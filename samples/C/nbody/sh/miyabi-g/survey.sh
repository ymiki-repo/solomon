#!/usr/bin/env bash
#PBS -q short-g
#PBS -l select=1
#PBS -l walltime=07:00:00
#PBS -W group_list=gj14
#PBS -N survey

if [ -z "${BASH_VERSION:-}" ]; then
	echo "run this job script with bash: bash $0" >&2
	exit 1
fi

SAMPLE=C/nbody
CONFIG=sh/miyabi-g/config.sh
if [ $# -gt 0 ]; then
	echo "this job script takes no arguments (its settings are in $CONFIG): bash $0" >&2
	exit 1
fi
# a job scheduler may start the job script in another directory: move to the directory where it was submitted,
# unless the settings are here (an interactive allocation keeps the directory where the job script is run)
if [ ! -f "$CONFIG" ]; then
	cd "${PBS_O_WORKDIR:-.}" || exit 1
fi
if [ ! -f "$CONFIG" ]; then
	echo "$CONFIG not found in $PWD: run (or submit) this job script in the directory samples/$SAMPLE" >&2
	exit 1
fi
# shellcheck source-path=SCRIPTDIR/../.. source=sh/miyabi-g/config.sh
. "$CONFIG" || exit 1
# shellcheck source-path=SCRIPTDIR/../.. source=sh/common/survey.sh
. sh/common/survey.sh
