#!/usr/bin/env bash
# solomon_migrate_v1_to_v2.sh: convert Solomon v1.x macro spellings to the v2.0.0 SOLOMON_-prefixed ones
#
## usage: solomon_migrate_v1_to_v2.sh [--apply] [--all] [--exclude <pattern>]... [--] <file|directory> ...
##
## Convert Solomon v1.x macro spellings to the v2.0.0 ones (e.g., OFFLOAD(...) -> SOLOMON_OFFLOAD(...),
## AS_INDEPENDENT -> SOLOMON_CLAUSE_INDEPENDENT; the full list is in MIGRATION.md).
##
##   (default)            dry-run: print a unified diff of the changes to stdout; no file is written
##   --apply              rewrite the files in place, keeping the original as <file>.v1.bak
##                        (<file>.v1.bak.1, .2, ... when that name is taken)
##   --all                also convert the sources that do not #include solomon.hpp
##   --exclude <pattern>  skip the files and directories matching the shell pattern; the pattern is matched
##                        against the path and each of its trailing parts (src/gen/a.c, gen/a.c, a.c), so
##                        it does not depend on how the directory was given (e.g., --exclude third_party
##                        --exclude 'src/gen' --exclude '*_gen.c'); a leading ./ in the pattern is ignored
##                        and a trailing / makes it match directories only; can be repeated, and
##                        --exclude=<pattern> also works
##   -h, --help           show this help
##
## What is converted:
##   * sources: the files with these extensions, searched for recursively in directories:
##     @SOURCE_EXTS@
##     by default only the sources that #include solomon.hpp (<solomon.hpp>, "solomon.hpp", or a path
##     ending in solomon.hpp such as <solomon/solomon.hpp>); the others are skipped with a note (see --all)
##   * build files (e.g., Makefile, *.mk, *.inc): only when given as arguments
##   * the replacement is word-based and idempotent: running it again, or on v2.0.0 code, changes nothing;
##     names in comments and string literals are replaced as well, so please review the diff
##
## What is left unchanged:
##   * the configuration macros OFFLOAD_BY_* and -DOFFLOAD_BY_* (they keep working in v2.0.0; in sources,
##     defined(OFFLOAD_BY_*) tells the backend actually in use, so renaming it would change the meaning)
##   * raw directive lines: #pragma with its continuation lines, Fortran !$omp / !$acc in any letter case,
##     and c$omp / *$omp / c$acc / *$acc in column 1
##   * in build files, the lines with make / shell variables or assignments: $( ${ $NAME := ::= ?= += !=,
##     NAME = at the start of a line, export / unexport / override / undefine / ifdef / ifndef / unset
##     (review such lines by hand)
##   * Solomon itself: a directory holding Solomon's solomon.hpp is skipped, with a note
##   * symbolic links to directories and to sources (give the real path instead) and the paths matched by
##     --exclude, with a note
##
## Notes and a summary (how many files were examined, changed and skipped) go to stderr.
## AS_BLOCK / NUM_BLOCKS are rewritten as well, but their OpenACC output changed in v2.0.0 (gang / num_gangs
## instead of worker / num_workers); a note is printed for each file that uses them (see MIGRATION.md).
##
## Exit status (as diff):
##   0  dry-run: nothing to change; --apply: done (or nothing to change)
##   1  dry-run: some files would change
##   2  trouble (bad arguments, paths that cannot be read, a file that could not be converted, no GNU sed);
##      with --apply, nothing is written when the trouble is found before the conversion starts
#
# requirements: bash (the script re-runs itself with bash when started as `sh <script>`) and GNU sed
# (word boundaries \b and \| in regular expressions; often installed as gsed on macOS / BSD); sed is
# checked at start-up and the script stops when no suitable sed is found

# re-run with bash when started by another shell
if [ -z "${BASH_VERSION:-}" ]; then
	exec bash "$0" "$@"
fi

set -u

PROG=${0##*/}

# extensions of the sources, searched for in directories (case-sensitive)
SOURCE_EXTS="c h cc cpp cxx hh hpp hxx C CC CPP f for f77 f90 f95 f03 f08 fpp F FOR F77 F90 F95 F03 F08 FPP"

# a line that includes Solomon (grep -E)
INCLUDE_SOLOMON='^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]([^>"]*/)?solomon\.hpp[>"]'

# seconds to wait for another run that is converting the same file
LOCK_WAIT=30

# v1.x -> v2.0.0 macro mapping (whole words); OFFLOAD_OUTER_LOOP, END_OFFLOAD_OUTER_LOOP, PARALLELIZE_INNER_LOOP,
# END_OFFLOAD, DECLARE_END_OFFLOADED, ALLOCATE_ON_DEVICE and DEALLOCATE_ON_DEVICE were not in v1.1.0; they existed
# only in the pre-release version of v2.0.0 (the fortran branch) and are kept for the codes written against it
MAP='
OFFLOAD_OUTER_LOOP SOLOMON_OFFLOAD_OUTER_LOOP
END_OFFLOAD_OUTER_LOOP SOLOMON_END_OFFLOAD_OUTER_LOOP
PARALLELIZE_INNER_LOOP SOLOMON_PARALLELIZE_INNER_LOOP
END_OFFLOAD SOLOMON_END_OFFLOAD
OFFLOAD SOLOMON_OFFLOAD
IF_NOT_OFFLOADED SOLOMON_IF_NOT_OFFLOADED
DECLARE_OFFLOADED_END SOLOMON_DECLARE_OFFLOADED_END
DECLARE_END_OFFLOADED SOLOMON_DECLARE_OFFLOADED_END
DECLARE_OFFLOADED SOLOMON_DECLARE_OFFLOADED
SYNCHRONIZE SOLOMON_SYNCHRONIZE
WAIT_QUEUE SOLOMON_WAIT_QUEUE
ATOMIC_UPDATE SOLOMON_ATOMIC_UPDATE
ATOMIC_READ SOLOMON_ATOMIC_READ
ATOMIC_WRITE SOLOMON_ATOMIC_WRITE
ATOMIC_CAPTURE SOLOMON_ATOMIC_CAPTURE
ATOMIC SOLOMON_ATOMIC
DATA_ACCESS_BY_HOST SOLOMON_DATA_ACCESS_BY_HOST
DATA_ACCESS_BY_DEVICE SOLOMON_DATA_ACCESS_BY_DEVICE
USE_DEVICE_DATA_FROM_HOST SOLOMON_USE_DEVICE_DATA_FROM_HOST
MALLOC_ON_DEVICE SOLOMON_MALLOC_ON_DEVICE
ALLOCATE_ON_DEVICE SOLOMON_ALLOCATE_ON_DEVICE
FREE_FROM_DEVICE SOLOMON_FREE_FROM_DEVICE
DEALLOCATE_ON_DEVICE SOLOMON_DEALLOCATE_ON_DEVICE
MEMCPY_D2H SOLOMON_MEMCPY_D2H
MEMCPY_H2D SOLOMON_MEMCPY_H2D
DECLARE_DATA_ON_DEVICE SOLOMON_DECLARE_DATA_ON_DEVICE
AS_INDEPENDENT SOLOMON_CLAUSE_INDEPENDENT
AS_SEQUENTIAL SOLOMON_CLAUSE_SEQUENTIAL
NUM_THREADS SOLOMON_CLAUSE_NUM_THREADS
NUM_BLOCKS SOLOMON_CLAUSE_NUM_BLOCKS
NUM_GRIDS SOLOMON_CLAUSE_NUM_GRIDS
AS_THREAD SOLOMON_CLAUSE_THREAD
AS_BLOCK SOLOMON_CLAUSE_BLOCK
AS_GRID SOLOMON_CLAUSE_GRID
COLLAPSE SOLOMON_CLAUSE_COLLAPSE
AS_ASYNC SOLOMON_CLAUSE_ASYNC
ASYNC_QUEUE SOLOMON_CLAUSE_ASYNC_QUEUE
REDUCTION SOLOMON_CLAUSE_REDUCTION
ENABLE_IF SOLOMON_CLAUSE_IF
AS_PRIVATE SOLOMON_CLAUSE_PRIVATE
AS_FIRSTPRIVATE SOLOMON_CLAUSE_FIRSTPRIVATE
AS_DEVICE_PTR SOLOMON_CLAUSE_DEVICE_PTR
COPY_BEFORE_AND_AFTER_EXEC SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC
COPY_H2D_BEFORE_EXEC SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC
COPY_D2H_AFTER_EXEC SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC
APPEND_ARGS SOLOMON_APPEND_ARGS
'

# lines printed unchanged: #pragma (and its continuation lines: a line ending in \, with or without the CR of a
# CRLF line end), Fortran !$omp / !$acc, and c$ / *$ in column 1
DIRECTIVE_RULES='
/^[[:space:]]*#[[:space:]]*pragma\b/{
:solomon_pragma
/\\\r\{0,1\}$/{
n
b solomon_pragma
}
b
}
/^[[:space:]]*![$]\([oO][mM][pP]\|[aA][cC][cC]\)/b
/^[cC*][$]\([oO][mM][pP]\|[aA][cC][cC]\)/b
'

# lines printed unchanged in build files: make / shell variables and assignments
VARIABLE_RULE='/[$][({A-Za-z_]\|[:?+!]=\|\b\(export\|unexport\|override\|undefine\|ifdef\|ifndef\|unset\)\b\|^[[:space:]]*[A-Za-z_][A-Za-z0-9_]*[[:space:]]*=/b'

# print the help: the lines starting with ## at the top of this file
usage() {
	local line
	while IFS= read -r line; do
		case $line in
			'##'*)
				line=${line#'##'}
				line=${line# }
				printf '%s\n' "${line//@SOURCE_EXTS@/$SOURCE_EXTS}"
				;;
		esac
	done < "$0"
}

note() {
	printf '%s: %s\n' "$PROG" "$*" >&2
}

NERR=0
error() {
	printf '%s: error: %s\n' "$PROG" "$*" >&2
	NERR=$((NERR + 1))
}

# ---- arguments

APPLY=0
ALL=0
EXCLUDES=()
ARGS=()
while [ $# -gt 0 ]; do
	case $1 in
		--apply) APPLY=1 ;;
		--all) ALL=1 ;;
		--exclude)
			[ $# -ge 2 ] || {
				error "--exclude needs a pattern"
				usage >&2
				exit 2
			}
			EXCLUDES+=("$2")
			shift
			;;
		--exclude=*) EXCLUDES+=("${1#--exclude=}") ;;
		-h | --help)
			usage
			exit 0
			;;
		--)
			shift
			ARGS+=("$@")
			break
			;;
		-*)
			error "unknown option: $1"
			usage >&2
			exit 2
			;;
		*) ARGS+=("$1") ;;
	esac
	shift
done
if [ ${#ARGS[@]} -eq 0 ]; then
	usage >&2
	exit 2
fi

# ---- sed scripts and the check of sed

# $1: src (a file with one of SOURCE_EXTS) or other (a build file: keeps the lines with variables)
build_script() {
	local old new
	printf '%s\n' "$DIRECTIVE_RULES"
	if [ "$1" = other ]; then
		printf '%s\n' "$VARIABLE_RULE"
	fi
	while read -r old new; do
		[ -n "$old" ] || continue
		printf 's/\\b%s\\b/%s/g\n' "$old" "$new"
	done <<< "$MAP"
}
SCRIPT_SRC=$(build_script src)
SCRIPT_OTHER=$(build_script other)

# prints the first of sed / gsed that is GNU sed and gives the expected output for the scripts above
# shellcheck disable=SC2016 # the probe texts hold make / shell variables literally on purpose (permanent)
find_sed() {
	local cand out_src out_other probe_crlf probe_in probe_src probe_other
	# the last two lines have CRLF line ends (a continuation line of #pragma must be kept there as well)
	probe_crlf=$'#pragma omp atomic \\\r\nATOMIC\r'
	probe_in='OFFLOAD(AS_INDEPENDENT, NUM_THREADS(4)) END_OFFLOAD_X
#pragma omp atomic \
ATOMIC
!$OMP ATOMIC
x := $(NUM_THREADS) ${AS_BLOCK} $COLLAPSE
NUM_THREADS ?= 4'$'\n'"$probe_crlf"
	probe_src='SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_NUM_THREADS(4)) END_OFFLOAD_X
#pragma omp atomic \
ATOMIC
!$OMP ATOMIC
x := $(SOLOMON_CLAUSE_NUM_THREADS) ${SOLOMON_CLAUSE_BLOCK} $SOLOMON_CLAUSE_COLLAPSE
SOLOMON_CLAUSE_NUM_THREADS ?= 4'$'\n'"$probe_crlf"
	# in the files that are not sources, only the first line changes
	probe_other="${probe_src%%$'\n'*}"$'\n'"${probe_in#*$'\n'}"

	for cand in sed gsed; do
		command -v "$cand" > /dev/null 2>&1 || continue
		"$cand" --version 2> /dev/null | grep -a -q -e '^[^ ]* (GNU sed) ' -e '^GNU sed version ' || continue
		out_src=$(printf '%s\n' "$probe_in" | LC_ALL=C "$cand" -e "$SCRIPT_SRC" 2> /dev/null)
		out_other=$(printf '%s\n' "$probe_in" | LC_ALL=C "$cand" -e "$SCRIPT_OTHER" 2> /dev/null)
		if [ "$out_src" = "$probe_src" ] && [ "$out_other" = "$probe_other" ]; then
			printf '%s\n' "$cand"
			return 0
		fi
	done
	return 1
}

for cmd in grep cmp diff find mktemp; do
	if ! command -v "$cmd" > /dev/null 2>&1; then
		error "$cmd is required but not found in PATH"
		exit 2
	fi
done

if ! SED=$(find_sed); then
	error "GNU sed is required (word boundaries \\b and \\| in regular expressions), but neither sed nor gsed is a GNU sed that gives the expected output; install GNU sed (as gsed on macOS / BSD)"
	exit 2
fi

# ---- collecting the files

# compare a file with its conversion; returns 0 when unchanged, 1 when changed, 2 on trouble
compare() {
	local st
	[ -r "$1" ] || return 2
	# cmp stops reading at the first difference; the rest of the output of sed is drained, so that sed never
	# gets a broken pipe (where SIGPIPE is ignored, that is an error of sed, not a silent end)
	# shellcheck disable=SC2094 # both sides only read the file (permanent)
	LC_ALL=C "$SED" -e "$2" < "$1" | (
		cmp -s "$1" -
		st=$?
		cat > /dev/null
		exit "$st"
	)
	st=("${PIPESTATUS[@]}")
	[ "${st[0]}" -eq 0 ] || return 2
	return "${st[1]}"
}

script_of() {
	if [ "$1" = other ]; then
		printf '%s' "$SCRIPT_OTHER"
	else
		printf '%s' "$SCRIPT_SRC"
	fi
}

FILES=()
KINDS=()
SKIP_PREFIXES=()

FIND_EXPR=('(' -type d -o -type l -o '(' -type f '(')
for e in $SOURCE_EXTS; do
	[ "${FIND_EXPR[${#FIND_EXPR[@]} - 1]}" = '(' ] || FIND_EXPR+=(-o)
	FIND_EXPR+=(-name "*.$e")
done
FIND_EXPR+=(')' ')' ')')

is_source_name() {
	local name=$1
	case $name in
		*.*) ;;
		*) return 1 ;;
	esac
	case " $SOURCE_EXTS " in
		*" ${name##*.} "*) return 0 ;;
	esac
	return 1
}

# true when the path or one of its trailing parts (a/b/c, b/c, c) matches an --exclude pattern; a leading ./ of the
# pattern is ignored (the parts are matched without it), and a pattern with a trailing / matches directories only
is_excluded() {
	local p=${1%/} part pat dironly
	[ ${#EXCLUDES[@]} -gt 0 ] || return 1
	for pat in "${EXCLUDES[@]}"; do
		while [ "${pat#./}" != "$pat" ]; do pat=${pat#./}; done
		dironly=0
		while [ "${pat%/}" != "$pat" ]; do
			pat=${pat%/}
			dironly=1
		done
		[ "$dironly" -eq 0 ] || [ -d "$1" ] || continue
		part=$p
		while :; do
			# shellcheck disable=SC2053 # the right-hand side is a pattern on purpose (permanent)
			[[ $part == $pat ]] && return 0
			case $part in
				*/*) part=${part#*/} ;;
				*) break ;;
			esac
		done
	done
	return 1
}

NSKIP_INCLUDE=0
# add a file to convert; a source that does not include Solomon is skipped unless --all
add_file() {
	# an unreadable file is kept, so that the conversion reports it
	if [ "$2" = src ] && [ "$ALL" -eq 0 ] && [ -r "$1" ] && ! grep -a -q -E "$INCLUDE_SOLOMON" "$1"; then
		NSKIP_INCLUDE=$((NSKIP_INCLUDE + 1))
		if ! compare "$1" "$SCRIPT_SRC"; then
			note "skipped (no #include of solomon.hpp, but it has v1.x names; --all converts it): $1"
		else
			note "skipped (no #include of solomon.hpp): $1"
		fi
		return
	fi
	FILES+=("$1")
	KINDS+=("$2")
}

# a directory holding Solomon's own solomon.hpp
is_solomon_dir() {
	[ -f "$1/solomon.hpp" ] && grep -a -q -e SOLOMON_SOLOMON_HPP "$1/solomon.hpp"
}

# prints the Solomon directory that holds the directory $1 (or $1 itself), if any
solomon_dir_above() {
	local d
	# CDPATH= keeps cd from printing the directory (a CDPATH holding . made d two lines and the loop endless)
	d=$(CDPATH='' cd -P -- "$1" > /dev/null 2>&1 && pwd -P && echo x) || return 1
	d=${d%$'\n'x}
	while :; do
		if is_solomon_dir "$d"; then
			printf '%s\n' "${d:-/}"
			return 0
		fi
		[ -n "$d" ] && [ "$d" != / ] || return 1
		d=${d%/*}
	done
}

is_skipped_prefix() {
	local pre
	[ ${#SKIP_PREFIXES[@]} -gt 0 ] || return 1
	for pre in "${SKIP_PREFIXES[@]}"; do
		[[ $1 == "$pre"/* ]] && return 0
	done
	return 1
}

collect_dir() {
	local root=$1 p st got=0
	while IFS= read -r -d '' p; do
		if [ -z "$p" ]; then
			# the record after an empty one is the exit status of find
			IFS= read -r -d '' st
			got=1
			[ "$st" = 0 ] || error "could not search all of $root (see the messages of find above)"
			break
		fi
		[ "$p" != "$root" ] || continue
		is_skipped_prefix "$p" && continue
		if [ -L "$p" ]; then
			if [ -d "$p" ] || is_source_name "${p##*/}"; then
				note "skipped (symbolic link): $p"
			fi
		elif [ -d "$p" ]; then
			if is_excluded "$p"; then
				SKIP_PREFIXES+=("$p")
				note "skipped (--exclude): $p"
			elif is_solomon_dir "$p"; then
				SKIP_PREFIXES+=("$p")
				note "skipped (Solomon headers): $p"
			fi
		elif is_excluded "$p"; then
			note "skipped (--exclude): $p"
		elif is_source_name "${p##*/}"; then
			add_file "$p" src
		fi
	done < <(
		find "$root" "${FIND_EXPR[@]}" -print0
		printf '\0%s\0' "$?"
	)
	[ "$got" = 1 ] || error "could not search $root"
}

for arg in "${ARGS[@]}"; do
	# keep find and the other commands from reading a path as an option or an operator
	case $arg in
		-* | '!' | '(' | ')') arg=./$arg ;;
	esac
	if [ -L "$arg" ]; then
		note "skipped (symbolic link; give the real path instead): $arg"
	elif [ -d "$arg" ]; then
		if sol=$(solomon_dir_above "$arg"); then
			note "skipped (Solomon headers in $sol): $arg"
		elif is_excluded "${arg%/}"; then
			note "skipped (--exclude): $arg"
		else
			collect_dir "$arg"
		fi
	elif [ -f "$arg" ]; then
		dir=.
		case $arg in
			*/*) dir=${arg%/*} ;;
		esac
		if sol=$(solomon_dir_above "${dir:-/}"); then
			note "skipped (Solomon headers in $sol): $arg"
		elif is_excluded "$arg"; then
			note "skipped (--exclude): $arg"
		else
			if is_source_name "${arg##*/}"; then
				add_file "$arg" src
			else
				add_file "$arg" other
			fi
		fi
	elif [ -e "$arg" ]; then
		error "not a regular file or a directory: $arg"
	else
		error "no such file or directory: $arg"
	fi
done
[ "$NERR" -eq 0 ] || exit 2

NFILES=${#FILES[@]}
if [ "$NFILES" -eq 0 ]; then
	if [ "$NSKIP_INCLUDE" -gt 0 ]; then
		note "no files to examine ($NSKIP_INCLUDE source(s) without #include of solomon.hpp skipped; --all converts them)"
	else
		note "no files to examine (directories are searched for: $SOURCE_EXTS)"
	fi
	exit 0
fi

# ---- converting

# print the notes on the block clauses whose OpenACC output changed in v2.0.0 (see MIGRATION.md)
# $1: the original content, $2: the name to show
block_notes() {
	grep -a -q -w -e AS_BLOCK -e NUM_BLOCKS "$1" || return 0
	note "note: $2: AS_BLOCK / NUM_BLOCKS emit gang / num_gangs(n) in OpenACC since v2.0.0 (v1.x: worker / num_workers(n)); write ACC_CLAUSE_WORKER / ACC_CLAUSE_NUM_WORKERS(n) to keep the v1.x meaning"
	if grep -a -q -w -e AS_GRID -e NUM_GRIDS "$1"; then
		note "note: $2: AS_GRID / NUM_GRIDS also appear; in the same clause list as AS_BLOCK / NUM_BLOCKS they now give the same clause twice (e.g., num_gangs(128) num_gangs(4))"
	fi
}

dir_of() {
	case $1 in
		*/*)
			local d=${1%/*}
			printf '%s' "${d:-/}"
			;;
		*) printf '.' ;;
	esac
}

CUR_TMP=""
CUR_LOCK=""
CUR_WRITING=""
CUR_BAK=""
release() {
	[ -z "$CUR_WRITING" ] || error "stopped while writing $CUR_WRITING (it may be incomplete); the original is kept in $CUR_BAK"
	[ -z "$CUR_TMP" ] || rm -f "$CUR_TMP"
	if [ -n "$CUR_LOCK" ]; then
		rm -f "$CUR_LOCK/owner" 2> /dev/null
		rmdir "$CUR_LOCK" 2> /dev/null
	fi
	CUR_TMP=""
	CUR_LOCK=""
	CUR_WRITING=""
}
trap release EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

# create a new backup name exclusively and copy the file there; prints the name
make_backup() {
	local bak=$1.v1.bak n=0
	while ! (set -o noclobber && : > "$bak") 2> /dev/null; do
		if { [ ! -e "$bak" ] && [ ! -L "$bak" ]; } || [ "$n" -ge 999 ]; then
			return 1
		fi
		n=$((n + 1))
		bak=$1.v1.bak.$n
	done
	cp -p "$1" "$bak" || return 1
	printf '%s\n' "$bak"
}

# take the lock of a file: the directory $2 made by this run and, inside it, the file owner created exclusively
# (noclobber); mkdir alone is not exclusive with every mkdir (uutils 0.10.0 reports success to several callers)
# $1: the file (for the messages), $2: the lock directory
take_lock() {
	local f=$1 lock=$2 n=0 gone=0 made owner
	while :; do
		made=0
		mkdir "$lock" 2> /dev/null && made=1
		if [ "$made" -eq 1 ] && (set -o noclobber && : > "$lock/owner") 2> /dev/null; then
			printf 'pid %s on %s since %s\n' "$$" "${HOSTNAME:-?}" "$(date 2> /dev/null)" > "$lock/owner" 2> /dev/null
			return 0
		fi
		if [ ! -d "$lock" ]; then
			# mkdir could not create it, or another run released it between our mkdir and this test
			gone=$((gone + 1))
			if [ "$gone" -ge 3 ]; then
				error "could not create the lock directory $lock"
				return 1
			fi
			continue
		fi
		gone=0
		n=$((n + 1))
		if [ "$n" -gt "$LOCK_WAIT" ]; then
			owner=""
			[ ! -r "$lock/owner" ] || IFS= read -r owner 2> /dev/null < "$lock/owner"
			error "$f is being converted by another run ($lock exists${owner:+; taken by $owner}); if no other run is active, remove it: rm -r -- $(printf '%q' "$lock")"
			return 1
		fi
		sleep 1
	done
}

# convert one file in place; returns 0 when converted, 1 when unchanged, 2 on trouble
apply_one() {
	local f=$1 script=$2 lock=$1.solomon-migrate.lock tmp bak
	take_lock "$f" "$lock" || return 2
	CUR_LOCK=$lock
	if ! tmp=$(mktemp "$(dir_of "$f")/.solomon-migrate.XXXXXX"); then
		error "could not create a temporary file next to $f"
		release
		return 2
	fi
	CUR_TMP=$tmp
	if ! LC_ALL=C "$SED" -e "$script" < "$f" > "$tmp"; then
		error "sed failed on $f"
		release
		return 2
	fi
	if cmp -s "$f" "$tmp"; then
		release
		return 1
	fi
	if ! bak=$(make_backup "$f"); then
		error "could not create a backup of $f"
		release
		return 2
	fi
	block_notes "$bak" "$f"
	CUR_BAK=$bak
	CUR_WRITING=$f
	if ! cat "$tmp" > "$f"; then
		CUR_WRITING=""
		error "failed to write $f; the original is kept in $bak"
		release
		return 2
	fi
	CUR_WRITING=""
	release
	printf 'converted: %s (backup: %s)\n' "$f" "$bak"
	return 0
}

NCHANGED=0
# $1: what happened to the changed files ("would change" or "converted")
summary() {
	local msg="$NFILES file(s) examined"
	if [ "$NCHANGED" -gt 0 ]; then
		msg="$msg, $NCHANGED $1"
	elif [ "$NERR" -eq 0 ]; then
		msg="$msg, nothing to change"
	fi
	[ "$NERR" -eq 0 ] || msg="$msg, $NERR error(s)"
	[ "$NSKIP_INCLUDE" -eq 0 ] || msg="$msg; $NSKIP_INCLUDE source(s) without #include of solomon.hpp skipped (--all converts them)"
	note "$msg"
}

if [ "$APPLY" -eq 0 ]; then
	for i in "${!FILES[@]}"; do
		f=${FILES[$i]}
		if [ ! -r "$f" ]; then
			error "cannot read $f"
			continue
		fi
		# shellcheck disable=SC2094 # both sides only read the file (permanent)
		LC_ALL=C "$SED" -e "$(script_of "${KINDS[$i]}")" < "$f" | diff -a -u -L "$f" -L "$f" "$f" -
		st=("${PIPESTATUS[@]}")
		if [ "${st[0]}" -ne 0 ]; then
			error "sed failed on $f"
		elif [ "${st[1]}" -eq 1 ]; then
			NCHANGED=$((NCHANGED + 1))
			block_notes "$f" "$f"
		elif [ "${st[1]}" -ne 0 ]; then
			error "diff failed on $f"
		fi
	done
	summary "would change"
	[ "$NERR" -eq 0 ] || exit 2
	if [ "$NCHANGED" -gt 0 ]; then
		note "dry-run only; re-run with --apply to rewrite the files (the originals are kept as <file>.v1.bak)"
		exit 1
	fi
	exit 0
fi

# --apply: check that every file to change can be written before writing any
for i in "${!FILES[@]}"; do
	f=${FILES[$i]}
	compare "$f" "$(script_of "${KINDS[$i]}")"
	case $? in
		0) ;;
		1)
			[ -w "$f" ] || error "cannot write $f"
			[ -w "$(dir_of "$f")" ] || error "cannot create the backup and the temporary file in $(dir_of "$f")"
			;;
		*) error "cannot read $f" ;;
	esac
done
if [ "$NERR" -ne 0 ]; then
	note "nothing was written"
	exit 2
fi
for i in "${!FILES[@]}"; do
	if apply_one "${FILES[$i]}" "$(script_of "${KINDS[$i]}")"; then
		NCHANGED=$((NCHANGED + 1))
	fi
done
summary converted
[ "$NERR" -eq 0 ] || exit 2
exit 0
