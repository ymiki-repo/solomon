# shellcheck shell=bash
# Checks shared by the recipes of this sample (sourced by the recipes in sh/common/); run from the sample directory.
# Defines: guard_die, guard_warn, guard_sample, guard_flag, guard_value, guard_integer, guard_command, guard_module, guard_host,
# guard_lock, guard_leftovers, guard_fail (record a failure of the run), and guard_finish (report the failures and exit).

GUARD_SAMPLE=F/diffusion # this sample; the job scripts in sh/<system>/ set SAMPLE to the sample they belong to
GUARD_FAILURES=()

# guard_die <message>: print the message to stderr and exit with 1
guard_die() {
	echo "ERROR: $*" >&2
	exit 1
}

# guard_warn <message>: print the message to stderr
guard_warn() {
	echo "WARNING: $*" >&2
}

# guard_sample: stop unless the job script (SAMPLE) belongs to the sample of this directory
guard_sample() {
	if [ "${SAMPLE:-}" != "$GUARD_SAMPLE" ]; then
		guard_die "$PWD holds samples/${GUARD_SAMPLE}, but the job script is for samples/${SAMPLE:-(unknown)}: run (or submit) it in the directory of its sample"
	fi
}

# guard_flag <name>: stop unless the setting <name> is 0 or 1
guard_flag() {
	case "${!1-}" in
		0 | 1) ;;
		*) guard_die "$1='${!1-}' in ${CONFIG:-sh/<system>/config.sh}: set 0 or 1" ;;
	esac
}

# guard_value <name>: stop if the setting <name> is not set or empty
guard_value() {
	if [ -z "${!1-}" ]; then
		guard_die "$1 is not set or empty: define it in ${CONFIG:-sh/<system>/config.sh}"
	fi
}

# guard_integer <name>: stop unless the setting <name> is a non-negative integer
guard_integer() {
	if ! [[ ${!1-} =~ ^[0-9]+$ ]]; then
		guard_die "$1='${!1-}' in ${CONFIG:-sh/<system>/config.sh}: set a non-negative integer"
	fi
}

# guard_command <command>...: stop unless each command is found (in PATH, or as a shell function)
guard_command() {
	local cmd
	for cmd in "$@"; do
		command -v "$cmd" > /dev/null 2>&1 || guard_die "$cmd not found: the job scripts of this sample need it (install it or add it to PATH)"
	done
}

# guard_module <name> <command>: load the module named by the setting <name>, then run <command> --version;
# stop if either fails
guard_module() {
	module load "${!1}" || guard_die "module load ${!1} failed: check $1 in ${CONFIG:-sh/<system>/config.sh}"
	"$2" --version || guard_die "$2 --version failed after module load ${!1}"
}

# guard_host: print the short name of this host (the first label of hostname -s, or of uname -n where that fails)
guard_host() {
	local host
	host=$(hostname -s 2> /dev/null) || host=
	if [ -z "$host" ]; then
		host=$(uname -n)
	fi
	echo "${host%%.*}"
}

# guard_lock: hold the lock of this sample directory (sh/.lock) until the job script exits;
# stop if another job script holds it, and go on with a warning where flock cannot be used
guard_lock() {
	if ! command -v flock > /dev/null 2>&1; then
		guard_warn "flock not found: running without the lock (do not run two job scripts in this directory at the same time)"
		return 0
	fi
	if ! { exec {GUARD_LOCK_FD}>> sh/.lock; } 2> /dev/null; then
		guard_warn "cannot open sh/.lock: running without the lock (do not run two job scripts in this directory at the same time)"
		return 0
	fi
	flock -n -E 75 "$GUARD_LOCK_FD"
	case $? in
		0) ;;
		75) guard_die "another job script is running in $PWD (it holds sh/.lock): wait until it ends, or run in a copy of the sample directory" ;;
		*) guard_warn "flock failed on sh/.lock: running without the lock (do not run two job scripts in this directory at the same time)" ;;
	esac
}

# guard_leftovers <path>...: stop if a path exists (a directory: if it holds a file); these are products of an
# earlier run that this run would mix into its results, and they are not removed here
guard_leftovers() {
	local path left=()
	local -A seen=()
	for path in "$@"; do
		if [ -z "$path" ] || [ -n "${seen[$path]-}" ]; then
			continue
		fi
		seen[$path]=1
		if [ -d "$path" ]; then
			if [ -n "$(find "$path" -type f)" ]; then
				left+=("${path%/}/")
			fi
		elif [ -e "$path" ] || [ -L "$path" ]; then
			left+=("$path")
		fi
	done
	if [ ${#left[@]} -gt 0 ]; then
		echo "ERROR: products of an earlier run are left in $PWD (they are not removed); move them out of the sample directory and run again:" >&2
		printf '  %q\n' "${left[@]}" >&2
		exit 1
	fi
}

# guard_fail <message>: record a failure of this run (reported by guard_finish)
guard_fail() {
	GUARD_FAILURES+=("$*")
	echo "FAILED: $*" >&2
}

# guard_finish: exit with 0 if no failure was recorded; otherwise list the failures on stderr and exit with 1
guard_finish() {
	if [ ${#GUARD_FAILURES[@]} -gt 0 ]; then
		echo "ERROR: ${#GUARD_FAILURES[@]} failure(s) in this run:" >&2
		printf '  %s\n' "${GUARD_FAILURES[@]}" >&2
		exit 1
	fi
	echo "finished without failure"
	exit 0
}
