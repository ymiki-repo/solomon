#!/usr/bin/env bash
#SBATCH -J iterate
#SBATCH -t 24:00:00
#SBATCH -p regular
#SBATCH --gres=gpu:1

if [ -z "${BASH_VERSION:-}" ]; then
	echo "run this job script with bash: bash $0" >&2
	exit 1
fi

SAMPLE=C/diffusion/padding
CONFIG=sh/whirlpool/config.sh
if [ $# -gt 0 ]; then
	echo "this job script takes no arguments (its settings are in $CONFIG): bash $0" >&2
	exit 1
fi
# a job scheduler may start the job script in another directory: move to the directory where it was submitted,
# unless the settings are here (an interactive allocation keeps the directory where the job script is run)
if [ ! -f "$CONFIG" ]; then
	cd "${SLURM_SUBMIT_DIR:-.}" || exit 1
fi
if [ ! -f "$CONFIG" ]; then
	echo "$CONFIG not found in $PWD: run (or submit) this job script in the directory samples/$SAMPLE" >&2
	exit 1
fi
# shellcheck source-path=SCRIPTDIR/../.. source=sh/whirlpool/config.sh
. "$CONFIG" || exit 1
# shellcheck source-path=SCRIPTDIR/../.. source=sh/common/iterate.sh
. sh/common/iterate.sh
