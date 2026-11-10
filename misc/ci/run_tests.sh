#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# GPU-free tests of the Solomon tree, run by .github/workflows/ci.yml (and by hand: no GPU, no network).
#
#   run_tests.sh [--only <group>[,<group>...]] [--no-skip] [--workdir <dir>]
#
#   --only <groups>   run only these groups: host, lint, migrate (comma separated; default: all)
#   --no-skip         count a SKIP (a missing tool, or a sample skipped inside a stage) as a FAIL; for CI,
#                     where every tool is installed and a silent SKIP would hide a test that did not run
#   --workdir <dir>   where build copies and logs go; must not exist yet (default: a new directory under
#                     ${RUNNER_TEMP:-${TMPDIR:-/tmp}})
#
# Groups (one PASS / FAIL / SKIP line per stage, then a summary):
#   host        the samples are built with the host compilers (gcc/g++/gfortran, and clang/clang++ when present)
#               in two modes: serial (neither OpenACC nor OpenMP) and host OpenMP (-fopenmp). Each build runs on
#               a copy of the tree. The preprocessed sources are also checked: serial must expand to no omp/acc
#               directive, host OpenMP to at least one omp directive and no acc / target one (the host fallback).
#               No sample is run: the builds and expansions are checked.
#   lint        every *.sh of the tree passes bash -n and shellcheck -S warning
#   migrate     smoke test of misc/migrate/solomon_migrate_v1_to_v2.sh in its default dry-run mode: the samples
#               (v2.0.0 code) need no change, a v1.x snippet would be converted, and no file is written
#
# A stage is SKIPped (with the reason) only when a tool it needs is missing; anything else that goes wrong
# is a FAIL. Exit status: 0 when nothing FAILed, 1 when a stage FAILed, 2 on a usage error.
# Environment: BOOST_INC, when set, is the boost include directory for samples/C/nbody (its Makefile reads
# the same variable). Requires bash 4.4, GNU make and GNU coreutils.

if [ -z "${BASH_VERSION-}" ]; then
	echo "run_tests.sh: run it with bash" >&2
	exit 2
fi
if [ "${BASH_VERSINFO[0]:-0}" -lt 4 ] || { [ "${BASH_VERSINFO[0]}" -eq 4 ] && [ "${BASH_VERSINFO[1]:-0}" -lt 4 ]; }; then
	echo "run_tests.sh: bash 4.4 or newer is required (this is ${BASH_VERSION})" >&2
	exit 2
fi
set -u
set -o pipefail

# ---- settings (one place) ----------------------------------------------------------------------------
# host compiler sets: label:C compiler:C++ compiler:Fortran compiler (empty = no Fortran with this set)
HOST_SETS=("gcc:gcc:g++:gfortran" "clang:clang:clang++:")
HOST_MODES=(serial openmp)
HOST_C_SAMPLES=(C/diffusion C/diffusion/padding C/nbody C/declare)
HOST_F_SAMPLES=(F/diffusion F/declare)
# macros the sample sources expect from their Makefiles (PARAM); NTHREADS is used in a clause of nbody
HOST_PARAM='-DCOMPILER="\"host\"" -DMODEL_ID="(0)" -DOPT_LEVEL="\"O2\"" -DNTHREADS="(128)"'
HOST_PP_DEFS=('-DCOMPILER="host"' '-DMODEL_ID=(0)' '-DOPT_LEVEL="O2"' '-DNTHREADS=(128)')
HOST_OPT="-O2"
# the header samples/C/nbody needs besides Solomon
NBODY_BOOST_PROBE="boost/math/constants/constants.hpp"
ALL_GROUPS=(host lint migrate)

# the usage (to stdout for -h / --help, to stderr on a usage error)
usage() {
	sed -n '6,12p' "$(readlink -f "$0")" | sed 's/^# \{0,1\}//'
}

# ---- options -------------------------------------------------------------------------------------------------
ONLY=""
WORKDIR=""
NO_SKIP=0
while [ $# -gt 0 ]; do
	case "$1" in
		--only)
			[ $# -ge 2 ] || {
				usage >&2
				exit 2
			}
			ONLY="$2"
			shift 2
			;;
		--no-skip)
			NO_SKIP=1
			shift
			;;
		--workdir)
			[ $# -ge 2 ] || {
				usage >&2
				exit 2
			}
			WORKDIR="$2"
			shift 2
			;;
		-h | --help)
			usage
			exit 0
			;;
		*)
			echo "run_tests.sh: unknown argument: $1" >&2
			usage >&2
			exit 2
			;;
	esac
done

GROUPS_TO_RUN=("${ALL_GROUPS[@]}")
if [ -n "${ONLY}" ]; then
	IFS=',' read -r -a GROUPS_TO_RUN <<< "${ONLY}"
	for g in "${GROUPS_TO_RUN[@]}"; do
		case " ${ALL_GROUPS[*]} " in
			*" ${g} "*) ;;
			*)
				echo "run_tests.sh: unknown group: ${g} (known: ${ALL_GROUPS[*]})" >&2
				exit 2
				;;
		esac
	done
fi
wants() {
	case " ${GROUPS_TO_RUN[*]} " in
		*" $1 "*) return 0 ;;
		*) return 1 ;;
	esac
}

# the tree is two levels above this script (misc/ci/run_tests.sh)
TREE="$(cd "$(dirname "$(readlink -f "$0")")/../.." && pwd)" || exit 2
[ -f "${TREE}/solomon/solomon.hpp" ] || {
	echo "run_tests.sh: ${TREE} does not hold solomon/solomon.hpp" >&2
	exit 2
}
if [ -n "${WORKDIR}" ]; then
	[ -e "${WORKDIR}" ] && {
		echo "run_tests.sh: --workdir ${WORKDIR} already exists" >&2
		exit 2
	}
	# resolved before it is created, so that a refused one is not left in the tree
	W="$(readlink -m -- "${WORKDIR}")" || exit 2
else
	W="$(mktemp -d "${RUNNER_TEMP:-${TMPDIR:-/tmp}}/solomon_tests.XXXXXX")" || exit 2
	W="$(cd -- "${W}" && pwd -P)" || exit 2
fi
case "${W}/" in
	"${TREE}/"*)
		echo "run_tests.sh: the work directory ${W} must be outside the tree ${TREE} (the tree is copied)" >&2
		[ -n "${WORKDIR}" ] || rmdir -- "${W}"
		exit 2
		;;
esac
mkdir -p -- "${W}/logs" || exit 2
echo "run_tests.sh: tree ${TREE}, work directory ${W}"

# ---- result bookkeeping ------------------------------------------------------------------------------------------
N_PASS=0
N_FAIL=0
N_SKIP=0
# $1 = PASS|FAIL|SKIP, $2 = stage, $3 = detail
record() {
	if [ "$1" = SKIP ] && [ "${NO_SKIP}" -eq 1 ]; then
		set -- FAIL "$2" "$3 (a SKIP is a FAIL with --no-skip)"
	fi
	printf '%-4s  %s: %s\n' "$1" "$2" "$3"
	case "$1" in
		PASS) N_PASS=$((N_PASS + 1)) ;;
		FAIL) N_FAIL=$((N_FAIL + 1)) ;;
		SKIP) N_SKIP=$((N_SKIP + 1)) ;;
	esac
}
# indented detail line under the stage being run
note() {
	printf '      %s\n' "$*"
}
# show the end of a log after a failure, so the cause is visible without opening the file
show_tail() {
	[ -f "$1" ] || return 0
	tail -n 20 -- "$1" | sed 's/^/      | /'
}

# ---- host: serial and host-OpenMP builds -----------------------------------------------------------------------
# count lines matching an extended regex over files (0 when none); $1 = -i or "", $2 = regex, rest = files
count_lines() {
	local icase="$1" re="$2"
	shift 2
	[ $# -gt 0 ] || {
		echo 0
		return
	}
	# shellcheck disable=SC2086 # icase is empty or one flag
	cat -- "$@" | grep -ac ${icase} -E "${re}" || true
}

# $1 = mode, $2 = omp, $3 = acc, $4 = target counts; prints why the expansion is wrong and fails
pragma_expectation() {
	case "$1" in
		serial)
			[ "$2" -eq 0 ] && [ "$3" -eq 0 ] || {
				echo "serial expanded to ${2} omp / ${3} acc directives (expected none)"
				return 1
			}
			;;
		openmp)
			[ "$2" -gt 0 ] && [ "$3" -eq 0 ] && [ "$4" -eq 0 ] || {
				echo "host OpenMP expanded to ${2} omp (${4} with target) / ${3} acc directives (expected >0 omp, no target, no acc)"
				return 1
			}
			;;
	esac
}

# true when a compiler builds and links a minimal OpenMP program with -fopenmp (omp.h / omp_lib and the
# runtime library are separate packages for some compilers, e.g. libomp-dev for a distribution's clang)
# $1 = c|c++|f90 (source language), $2 = compiler, $3 = label of the output file
can_openmp() {
	local lang="$1" comp="$2" d="${W}/probe" src
	mkdir -p "${d}" || return 1
	case "${lang}" in
		f90)
			src="${d}/omp_probe.f90"
			printf 'program omp_probe\n  use omp_lib\n  print *, omp_get_max_threads()\nend program omp_probe\n' > "${src}"
			;;
		*)
			src="${d}/omp_probe.c"
			printf '#include <omp.h>\nint main(void) { return omp_get_max_threads() > 0 ? 0 : 1; }\n' > "${src}"
			;;
	esac
	if [ "${lang}" = c++ ]; then
		"${comp}" -fopenmp -x c++ "${src}" -o "${d}/$3" > "${d}/$3.log" 2>&1
	else
		"${comp}" -fopenmp "${src}" -o "${d}/$3" > "${d}/$3.log" 2>&1
	fi
}

# true when the C++ compiler finds the boost header that samples/C/nbody needs; $1 = C++ compiler
have_boost() {
	local inc=()
	[ -n "${BOOST_INC-}" ] && inc=(-isystem "${BOOST_INC}")
	printf '#include <%s>\n' "${NBODY_BOOST_PROBE}" | "$1" -x c++ "${inc[@]}" -E - > /dev/null 2>&1
}

# $1 = copy of the tree, $2 = sample (e.g. C/diffusion), $3 = cc, $4 = cxx, $5 = flag, $6 = mode, $7 = log prefix
host_c_sample() {
	local copy="$1" s="$2" cc="$3" cxx="$4" flag="$5" mode="$6" pfx="$7" dir log ppdir src n=0 msg
	local inc=() files=()
	dir="${copy}/samples/${s}"
	log="${pfx}_${s//\//_}.log"
	ppdir="${pfx}_${s//\//_}_pp"
	[ -n "${BOOST_INC-}" ] && inc=(-isystem "${BOOST_INC}")
	# the Makefile of nbody creates its object directory in a separate target
	if grep -aqE '^dir:' "${dir}/Makefile"; then
		make --no-print-directory -C "${dir}" dir > "${log}" 2>&1 || {
			note "NG  ${s}: make dir failed (log: ${log})"
			return 1
		}
	fi
	# the NVHPC defaults of the Makefiles are switched off; PARAM is replaced so that no OFFLOAD_BY_* is
	# defined (with -fopenmp, OFFLOAD_BY_OPENMP_TARGET would select target offload, not the host fallback);
	# no link option is added: a Makefile that stops linking libm must fail here
	if ! make --no-print-directory -C "${dir}" all NVHPC=0 AMDCLANG=0 ICPX=0 ACPP=0 \
		CC="${cc} -std=c99 ${flag}" CXX="${cxx} ${flag}" CFLAGS="-Wall ${HOST_OPT}" ARGS="${HOST_OPT}" \
		PARAM="${HOST_PARAM}" >> "${log}" 2>&1; then
		show_tail "${log}"
		note "NG  ${s}: build failed (log: ${log})"
		return 1
	fi
	mkdir -p "${ppdir}" || return 1
	for src in "${dir}"/*.c "${dir}"/*.cpp "${dir}"/src/*.c "${dir}"/src/*.cpp; do
		[ -f "${src}" ] || continue
		grep -aqE '^[[:space:]]*#[[:space:]]*include[[:space:]]*<solomon\.hpp>' "${src}" || continue
		case "${src}" in
			*.c) set -- "${cc}" -std=c99 ;;
			*) set -- "${cxx}" --std=c++17 "${inc[@]}" ;;
		esac
		# shellcheck disable=SC2086 # flag is empty or one flag
		if ! (cd "$(dirname "${src}")" && "$@" ${flag} -E -P "${HOST_PP_DEFS[@]}" -I"${copy}/solomon" "${src}") \
			> "${ppdir}/$(basename "${src}").i" 2>> "${log}"; then
			note "NG  ${s}: preprocessing $(basename "${src}") failed (log: ${log})"
			return 1
		fi
		files+=("${ppdir}/$(basename "${src}").i")
		n=$((n + 1))
	done
	[ "${n}" -gt 0 ] || {
		note "NG  ${s}: no source includes <solomon.hpp>"
		return 1
	}
	local re='^[[:space:]]*#[[:space:]]*pragma[[:space:]]+'
	if ! msg="$(pragma_expectation "${mode}" \
		"$(count_lines "" "${re}omp([[:space:]]|$)" "${files[@]}")" \
		"$(count_lines "" "${re}acc([[:space:]]|$)" "${files[@]}")" \
		"$(count_lines "" "${re}omp[[:space:]].*target" "${files[@]}")")"; then
		note "NG  ${s}: ${msg} (preprocessed: ${ppdir})"
		return 1
	fi
	note "ok  ${s}: built; ${n} Solomon sources expand as expected for ${mode}"
}

# $1 = copy of the tree, $2 = sample, $3 = fc, $4 = flag, $5 = mode, $6 = log prefix
host_f_sample() {
	local copy="$1" s="$2" fc="$3" flag="$4" mode="$5" pfx="$6" dir log msg
	local files=()
	dir="${copy}/samples/${s}"
	log="${pfx}_${s//\//_}.log"
	if ! make --no-print-directory -C "${dir}" all NVHPC=0 AMDFLANG=0 IFX=0 FC="${fc}" FFLAGS="-Wall ${HOST_OPT} ${flag}" \
		PARAM="${HOST_PARAM}" > "${log}" 2>&1; then
		show_tail "${log}"
		note "NG  ${s}: build failed (log: ${log})"
		return 1
	fi
	# the outputs only: spp/**/*.f90 outside the work directories of fortran.mk (spp/**/.solomon.*/)
	if [ -d "${dir}/spp" ]; then
		mapfile -d '' -t files < <(find "${dir}/spp" -name '.*' -prune -o -type f -name '*.f90' -print0 | sort -z)
	fi
	[ "${#files[@]}" -gt 0 ] || {
		note "NG  ${s}: fortran.mk produced no spp/*.f90"
		return 1
	}
	if ! msg="$(pragma_expectation "${mode}" \
		"$(count_lines -i '^[[:space:]]*!\$omp([[:space:]]|$)' "${files[@]}")" \
		"$(count_lines -i '^[[:space:]]*!\$acc([[:space:]]|$)' "${files[@]}")" \
		"$(count_lines -i '^[[:space:]]*!\$omp[[:space:]].*target' "${files[@]}")")"; then
		note "NG  ${s}: ${msg} (spp output: ${dir}/spp)"
		return 1
	fi
	note "ok  ${s}: built; ${#files[@]} spp outputs expand as expected for ${mode}"
}

# $1 = "label:cc:cxx:fc", $2 = mode
stage_host() {
	local label cc cxx fc mode="$2" name flag="" copy pfx ng=0 s skipped=()
	IFS=':' read -r label cc cxx fc <<< "$1"
	name="host:${label}-${mode}"
	[ "${mode}" = openmp ] && flag="-fopenmp"
	if ! command -v "${cc}" > /dev/null 2>&1 || ! command -v "${cxx}" > /dev/null 2>&1; then
		record SKIP "${name}" "${cc} or ${cxx} is not on PATH"
		return
	fi
	command -v make > /dev/null 2>&1 || {
		record SKIP "${name}" "make is not on PATH"
		return
	}
	if [ "${mode}" = openmp ]; then
		if ! can_openmp c "${cc}" "${label}_c" || ! can_openmp c++ "${cxx}" "${label}_cxx"; then
			record SKIP "${name}" "${cc} / ${cxx} cannot build an OpenMP program with -fopenmp (omp.h or the runtime is missing; log: ${W}/probe)"
			return
		fi
	fi
	copy="${W}/host/${label}-${mode}"
	pfx="${W}/logs/host_${label}-${mode}"
	mkdir -p "${copy}" && cp -a "${TREE}/solomon" "${TREE}/samples" "${copy}/" || {
		record FAIL "${name}" "cannot copy the tree to ${copy}"
		return
	}
	for s in "${HOST_C_SAMPLES[@]}"; do
		if [ "${s}" = C/nbody ] && ! have_boost "${cxx}"; then
			note "--  ${s}: skipped (<${NBODY_BOOST_PROBE}> not found; set BOOST_INC)"
			skipped+=("${s}: no boost")
			continue
		fi
		host_c_sample "${copy}" "${s}" "${cc}" "${cxx}" "${flag}" "${mode}" "${pfx}" || ng=1
	done
	for s in "${HOST_F_SAMPLES[@]}"; do
		if [ -z "${fc}" ]; then
			continue
		elif ! command -v "${fc}" > /dev/null 2>&1; then
			note "--  ${s}: skipped (${fc} is not on PATH)"
			skipped+=("${s}: no ${fc}")
			continue
		elif [ "${mode}" = openmp ] && ! can_openmp f90 "${fc}" "${label}_f90"; then
			note "--  ${s}: skipped (${fc} cannot build an OpenMP program with -fopenmp; log: ${W}/probe)"
			skipped+=("${s}: no OpenMP for ${fc}")
			continue
		fi
		host_f_sample "${copy}" "${s}" "${fc}" "${flag}" "${mode}" "${pfx}" || ng=1
	done
	local extra=""
	[ "${#skipped[@]}" -gt 0 ] && extra=" (skipped: ${skipped[*]})"
	[ "${#skipped[@]}" -gt 0 ] && [ "${NO_SKIP}" -eq 1 ] && ng=1
	if [ "${ng}" -ne 0 ]; then
		record FAIL "${name}" "see the NG lines above${extra}"
	else
		record PASS "${name}" "${cc}/${cxx}${fc:+/${fc}} ${mode} builds and expansions${extra}"
	fi
}

# ---- lint: every shell script of the tree --------------------------------------------------------------------
stage_lint() {
	local name="lint" log="${W}/logs/shellcheck.log" files=() f ng=0
	command -v shellcheck > /dev/null 2>&1 || {
		record SKIP "${name}" "shellcheck is not on PATH"
		return
	}
	mapfile -d '' -t files < <(find "${TREE}" -name .git -prune -o -type f -name '*.sh' -print0 | sort -z)
	[ "${#files[@]}" -gt 0 ] || {
		record FAIL "${name}" "no *.sh found under ${TREE}"
		return
	}
	: > "${log}"
	for f in "${files[@]}"; do
		bash -n -- "${f}" >> "${log}" 2>&1 || {
			note "NG  ${f#"${TREE}"/}: bash -n failed"
			ng=1
		}
	done
	shellcheck -S warning -- "${files[@]}" >> "${log}" 2>&1 || ng=1
	if [ "${ng}" -ne 0 ]; then
		show_tail "${log}"
		record FAIL "${name}" "$(shellcheck --version | sed -n 's/^version: //p'): findings in ${#files[@]} scripts (log: ${log})"
	else
		record PASS "${name}" "$(shellcheck --version | sed -n 's/^version: //p'): ${#files[@]} scripts, bash -n and shellcheck -S warning clean"
	fi
}

# ---- migrate: dry-run smoke test of the migration script ---------------------------------------------------------------
stage_migrate() {
	local name="migrate" script="${TREE}/misc/migrate/solomon_migrate_v1_to_v2.sh" d="${W}/migrate" rc
	local log="${W}/logs/migrate.log" before after
	[ -f "${script}" ] || {
		record FAIL "${name}" "${script#"${TREE}"/} is missing"
		return
	}
	mkdir -p "${d}" && cp -a "${TREE}/samples" "${d}/" || {
		record FAIL "${name}" "cannot copy samples/ to ${d}"
		return
	}
	# (1) the samples are v2.0.0 code: nothing to change (exit 0, no diff)
	sh "${script}" "${d}/samples" > "${d}/samples.diff" 2> "${log}"
	rc=$?
	if [ "${rc}" -ne 0 ] || [ -s "${d}/samples.diff" ]; then
		show_tail "${log}"
		record FAIL "${name}" "dry-run on samples/ exited ${rc} with $(wc -l < "${d}/samples.diff") diff lines (expected 0 and none; log: ${log})"
		return
	fi
	# (2) a v1.x snippet: the dry-run prints the conversion (exit 1) and leaves the file as it was
	printf '%s\n' '#include <solomon.hpp>' 'void f(int n, double *x) {' '  OFFLOAD(AS_INDEPENDENT, NUM_THREADS(128))' \
		'  for (int i = 0; i < n; i++) x[i] = 0.0;' '}' > "${d}/v1.c"
	before="$(cksum < "${d}/v1.c")"
	sh "${script}" "${d}/v1.c" > "${d}/v1.diff" 2>> "${log}"
	rc=$?
	after="$(cksum < "${d}/v1.c")"
	if [ "${rc}" -ne 1 ] || [ "${before}" != "${after}" ] ||
		! grep -aqxF '+  SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_NUM_THREADS(128))' "${d}/v1.diff"; then
		show_tail "${log}"
		record FAIL "${name}" "dry-run on a v1.x snippet: exit ${rc} (expected 1), file unchanged: $([ "${before}" = "${after}" ] && echo yes || echo no), diff: ${d}/v1.diff"
		return
	fi
	if [ -n "$(find "${d}" -name '*.v1.bak*' -print)" ]; then
		record FAIL "${name}" "the dry-run left *.v1.bak files under ${d}"
		return
	fi
	record PASS "${name}" "dry-run: samples/ need no change; a v1.x snippet would be converted; nothing written"
}

# ---- run ------------------------------------------------------------------------------------------------------
if wants host; then
	for set_ in "${HOST_SETS[@]}"; do
		for mode in "${HOST_MODES[@]}"; do
			stage_host "${set_}" "${mode}"
		done
	done
fi
wants lint && stage_lint
wants migrate && stage_migrate

echo "run_tests.sh: ${N_PASS} PASS / ${N_FAIL} FAIL / ${N_SKIP} SKIP (work directory: ${W})"
[ "${N_FAIL}" -eq 0 ]
