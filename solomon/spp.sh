#!/bin/sh

# Solomon preprocessor for a single source file; writes the preprocessed source to stdout.
#   spp.sh -compiler=<compiler> [compiler flags] [-I<dir>] [-D<macro>] <source>
# The source is the last argument. -compiler=, -I, -D (also as -I <dir>, -D <macro>) are read anywhere; every other
# argument is passed to the compiler as given, when the compiler is asked about the source. The options of the
# preprocessor in these spellings reach the system cpp as well, in the order they are written (those among the words
# of -compiler= first): -I, -D, -U, -include, -imacros, -isystem, -idirafter, -iquote (each also joined to its value),
# -Xpreprocessor <option>, -Wp,<options>, --include=<file> / --include <file>.
# Nothing about the compiler or the flags is decided by their spelling: the compiler itself is asked.
#   * _OPENACC / _OPENMP, the macros of the system cpp that are not reserved names (linux, unix), and for a C-family
#     source __cplusplus, __STDC_VERSION__, __STRICT_ANSI__: the compiler preprocesses (-E) a probe file that has
#     the suffix of the source, so that the language of a .c file follows the compiler (g++, mpicxx, ...) and -std=
#     (in any spelling, also inside -compiler=) is the compiler's own. For C and C++, the system cpp is given the
#     -std= of the standard whose value the compiler answers (the next older one cpp accepts when it does not know
#     that one); then every probed macro whose value still differs from the one of the system cpp is undefined and
#     given the compiler's value. A probe that the compiler does not preprocess (a suffix it does not read, as g++
#     with .CC; an unknown flag) stops spp.sh with the first line of its message.
#   * the source form of Fortran: the compiler compiles (-c) a two-line free-form program with the suffix of the
#     source; when that fails and a program written from column 7 compiles, the compiler reads the source as fixed
#     form and spp.sh stops (fixed form is not supported); when both fail, spp.sh stops with the compiler's message.
# The probes run in the current directory with their input and their output (-o) in the work directory, so that
# the flags keep their meaning (relative paths, @files) and the by-products named after the output (-MD, -MMD, ...)
# stay in the work directory; a by-product named after the probe input (_solomon*) that a compiler writes in the
# current directory is removed afterwards; a file the flags name explicitly (-MF x.d) is written as asked.
# Work files go to a private directory spp/.solomon.<pid>.<n>/ under the current directory (spp/ is created when
# missing; nothing is written next to the source), and are removed on exit, on failure, and on interruption.
# Quoted includes (#include "...") of the source are searched in the directory of the source first, then in the
# -I directories, as when the source itself is preprocessed; in an included file, a quoted include is searched in
# the directory of that file, then in the directory of the source, then in the -I directories (a direct
# preprocessing does not search the directory of the source there).
# Notes on stderr when no backend is detected or a requested backend is disabled (stdout is unchanged).
# Fortran: the automatic profiler tags name the source as given on the command line (#line); a source path
# with a backslash or a double quote is refused, since #line cannot carry it unchanged.

COMPILER=""
DEFS=""
FLAGS=""
PPA=""
PPC=""
SRC=""
LASTOPT=""

# print an error message and stop
die() {
	printf '%s\n' "$0: error: $1" >&2
	exit 1
}

# quote one argument for eval (single quotes, embedded single quotes escaped)
quote() {
	printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"
}

# the message of a compiler in the file $1: its first line that is not empty, followed by the next line that is not
# indented when the first one only names a place (GCC's "<command-line>:1:0:"; empty when there is no message)
first_message() {
	awk 'm == "" { if (NF) { m = $0; if (m !~ /:[ \t]*$/) exit } next } NF && $0 !~ /^[ \t]/ { m = m " " $0; exit } END { print m }' "$1"
}

# the tools that spp.sh runs besides the compiler
for t in cpp awk sed; do
	command -v "$t" > /dev/null 2>&1 || die "$t is not found in PATH (spp.sh needs cpp, the C preprocessor of GCC or LLVM, awk, and sed)"
done

# the options of the preprocessor (GCC's spellings) that the system cpp takes as well: sets PPN to the number of
# words of the option at $1 (0: not one of them) and PPW to them, quoted; a -D also goes to DEFS (read by the notes)
pp_option() {
	PPN=1
	PPW=" $(quote "$1")"
	case "$1" in
		-I | -D | -U | -include | -imacros | -isystem | -idirafter | -iquote | -Xpreprocessor | --include)
			[ $# -ge 2 ] || die "missing argument to $1"
			PPN=2
			PPW="$PPW $(quote "$2")"
			;;
		-I?* | -D?* | -U?* | -include?* | -imacros?* | -isystem?* | -idirafter?* | -iquote?* | -Wp,* | --include=*) ;;
		*) PPN=0 ;;
	esac
	case "$1" in
		-D) DEFS="$DEFS -D$2" ;;
		-D?*) DEFS="$DEFS $1" ;;
	esac
}

# everything but the last argument; the options of the preprocessor are given to cpp in the order they are written
# (-I and -D to cpp only, the others to the compiler as well)
while [ $# -gt 1 ]; do
	case "$1" in
		-compiler=*)
			COMPILER=${1#-compiler=}
			shift
			continue
			;;
	esac
	pp_option "$@"
	if [ "$PPN" -eq 0 ]; then
		# a compiler flag, or the value of one (e.g. -MF deps.d): kept as given, quoted
		FLAGS="$FLAGS $(quote "$1")"
		shift
		continue
	fi
	[ "$PPN" -lt 2 ] || [ $# -ge 3 ] || die "missing argument to $1 (the last argument $2 is read as the source: the source is the last argument)"
	PPA="$PPA$PPW"
	case "$1" in
		-I* | -D*) ;;
		*) FLAGS="$FLAGS$PPW" ;;
	esac
	shift "$PPN"
done

# the last argument is the source
case "${1:-}" in
	-compiler=*)
		COMPILER=${1#-compiler=}
		;;
	"") ;;
	-*)
		LASTOPT=$1
		;;
	*)
		SRC=$1
		;;
esac

# the compiler is split into words (-compiler="gfortran -fopenmp" is accepted), never globbed
set -f

[ -n "$COMPILER" ] || die "compiler not specified"

# the options of the preprocessor among the words of -compiler= after the compiler go to cpp as well, before those
# of the arguments (as fortran.mk takes them from SOLOMON_FC before SOLOMON_FLAGS)
# shellcheck disable=SC2086 # COMPILER is a word list on purpose
set -- $COMPILER
[ $# -gt 0 ] || die "compiler not specified (-compiler= has no word)"
command -v "$1" > /dev/null 2>&1 || die "the compiler is not found: $1"
shift
while [ $# -gt 0 ]; do
	pp_option "$@"
	if [ "$PPN" -eq 0 ]; then
		shift
		continue
	fi
	PPC="$PPC$PPW"
	shift "$PPN"
done
PPARGS="$PPC$PPA"

[ -n "$SRC" ] || [ -z "$LASTOPT" ] || die "source file not specified (the source is the last argument): the last argument $LASTOPT is read as a flag (give the source last; a source whose name starts with - as ./<name>)"
[ -n "$SRC" ] || die "source file not specified (the source is the last argument)"

[ -r "$SRC" ] || die "failed to read source file: $SRC"

[ -f "$SRC" ] || die "the source is not a regular file: $SRC"

# the family of the source by its suffix (which path spp.sh takes); the compiler decides the rest
SUF=".${SRC##*.}"
case "$SUF" in
	.c | .C | .cc | .cpp | .cxx | .CC | .CPP | .c++ | .cp)
		LANGUAGE=c
		PROBE=_solomon$SUF
		;;
	.[fF]*)
		LANGUAGE=fortran
		# the macro probe is preprocessed by every Fortran compiler as .F; the form probes take the suffix of the source
		PROBE=_solomon.F
		;;
	*)
		die "unknown source file suffix: $SRC"
		;;
esac

if [ "$LANGUAGE" = fortran ]; then
	case "$SRC" in
		*\\* | *\"*)
			die "the source path contains a backslash or a double quote, which #line cannot carry: $SRC (rename the file or the directory)"
			;;
	esac
fi

# directory of the source, searched first for quoted includes
case "$SRC" in
	*/*)
		SRCDIR=${SRC%/*}
		[ -n "$SRCDIR" ] || SRCDIR=/
		;;
	*)
		SRCDIR=.
		;;
esac

# private work directory under ./spp (one per process)
if ! mkdir -p spp 2> /dev/null || [ ! -d spp ]; then
	die "failed to create the directory spp in the current directory"
fi
WORK=""
n=0
while [ "$n" -lt 100 ]; do
	if mkdir "spp/.solomon.$$.$n" 2> /dev/null; then
		WORK="spp/.solomon.$$.$n"
		break
	fi
	n=$((n + 1))
done
[ -n "$WORK" ] || die "failed to create a work directory under spp/ (spp/.solomon.$$.*)"
trap 'rm -rf "$WORK"' EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

# GCC's cpp only (other preprocessors reject the option): -ftrack-macro-expansion=0 keeps __LINE__ (the line
# numbers of the automatic profiler tags) right in long files
TRACK=""
if cpp -ftrack-macro-expansion=0 -P /dev/null > /dev/null 2>&1; then
	TRACK=-ftrack-macro-expansion=0
fi

# awk program (run with LC_ALL=C): a "#pragma solomon_fprof" line (an automatic profiler tag) whose statement is
# longer than 132 bytes is split into lines of at most 132 bytes with free-form continuation (& at the end, & at
# the start of the next line), never inside a UTF-8 character
# shellcheck disable=SC2016 # the $ are awk's, not the shell's (permanent)
FOLD='{
	p = "#pragma solomon_fprof "
	if (substr($0, 1, 22) != p || length($0) <= 154) { print; next }
	s = substr($0, 23); c = ""
	while (length(c) + length(s) > 132) {
		n = 131 - length(c)
		while (n > 1 && substr(s, n + 1, 1) >= "\200" && substr(s, n + 1, 1) < "\300") n--
		print p c substr(s, 1, n) "&"
		s = substr(s, n + 1); c = "&"
	}
	print p c s
}'

# the macros of the language and the standard that the system cpp takes from the compiler for a C-family source
CMACROS="__cplusplus __STDC_VERSION__ __STRICT_ANSI__"
# the macros of the system cpp that are not reserved names (linux, unix): taken from the compiler as well
UNRES=$(cpp -dM -E -x c /dev/null 2> /dev/null | awk '$1 == "#define" && $2 ~ /^[A-Za-z][A-Za-z0-9_]*$/ { print $2 }' | tr '\n' ' ')
NAMES=$UNRES
[ "$LANGUAGE" != c ] || NAMES="$CMACROS $UNRES"
# the standards that the -std= of the system cpp is chosen from: <value>/<names> (- for any later value; a name
# starting with = is used without the prefix c / gnu / c++ / gnu++)
CSTDS="0/89,90 199409/=iso9899:199409 199901/99,9x 201112/11,1x 201710/17,18 202311/23,2x -/2y"
CXXSTDS="199711/98,03 201103/11,0x 201402/14,1y 201703/17,1z 202002/20,2a 202302/23,2b -/26,2c"

# the probe: a line that only a preprocessor turns into -D__SOLOMON_PROBE__=1 (the proof that the compiler read the
# file), and one -D<name>=<value> line per macro that solomon.hpp branches on
{
	printf '#define SOLOMON_PROBE_VALUE 1\n-D__SOLOMON_PROBE__=SOLOMON_PROBE_VALUE\n'
	printf '#ifdef _OPENMP\n-D_OPENMP=_OPENMP\n#endif\n#ifdef _OPENACC\n-D_OPENACC=_OPENACC\n#endif\n'
	for m in $NAMES; do
		printf '#ifdef %s\n-D%s=%s\n#endif\n' "$m" "$m" "$m"
	done
	if [ "$LANGUAGE" != c ]; then
		# Fortran: nvfortran's __NVCOMPILER as __SOLOMON_NVCOMPILER__ (the system cpp does not define the compiler's
		# own macros; solomon/omp/clause_tag.hpp reads it, D-087 (5))
		printf '#ifdef __NVCOMPILER\n-D__SOLOMON_NVCOMPILER__=__NVCOMPILER\n#endif\n'
	fi
} > "$WORK/$PROBE" 2> /dev/null || die "failed to create a temporary file: $WORK/$PROBE"

# the compiler and the flags, as words, as given
eval "set -- $FLAGS"
# shellcheck disable=SC2086 # COMPILER is a word list on purpose
set -- $COMPILER "$@"

# a by-product that a compiler names after the probe input and writes in the current directory (ifx -MD) is
# removed when it is newer than this stamp
: > "$WORK/_stamp"
sweep() {
	find . -path './*/*' -prune -o -type f -name '_solomon*' -newer "$WORK/_stamp" -exec rm -f {} + 2> /dev/null
}

# the compiler preprocesses the probe: some compilers write -E to -o, others to stdout; both are taken (the lines may
# be indented: flang 22 and later write a .F in fixed form, from column 7)
"$@" -E "$WORK/$PROBE" -o "$WORK/_solomon.i" > "$WORK/_solomon.out" 2> "$WORK/_solomon.err"
cat "$WORK/_solomon.i" >> "$WORK/_solomon.out" 2> /dev/null
sweep
if ! grep -q -- "^[[:space:]]*-D__SOLOMON_PROBE__=1[[:space:]]*$" "$WORK/_solomon.out"; then
	msg=$(first_message "$WORK/_solomon.err")
	[ -n "$msg" ] || msg="(no message from the compiler; a flag that keeps -E from writing the preprocessed text, as -MD / -MMD of nvc and nvc++, has this effect)"
	die "the compiler did not preprocess the probe file $PROBE with the given flags ($COMPILER$FLAGS): $msg"
fi
# only plain -D<name>=<value> lines are taken (one line of words, since it goes to eval)
MACRO=$(grep -- "^[[:space:]]*-D[A-Za-z0-9_]*=[A-Za-z0-9_]*[[:space:]]*$" "$WORK/_solomon.out" | sed "s/^[[:space:]]*//" | grep -v -- "^-D__SOLOMON_PROBE__=" | sort -u | tr '\n\r' '  ')

# Fortran: the compiler tells the source form it reads with these flags
if [ "$LANGUAGE" = fortran ]; then
	# (a compiler that does not read the suffix at all returns 0 without an object: "linker input file unused")
	printf 'program solomon_probe\nend program solomon_probe\n' > "$WORK/_solomon_free$SUF"
	"$@" -c "$WORK/_solomon_free$SUF" -o "$WORK/_solomon_free.o" > "$WORK/_solomon_free.err" 2>&1
	if [ ! -s "$WORK/_solomon_free.o" ]; then
		printf '      program solomon_probe\n      end program solomon_probe\n' > "$WORK/_solomon_both$SUF"
		"$@" -c "$WORK/_solomon_both$SUF" -o "$WORK/_solomon_both.o" > /dev/null 2>&1
		if [ -s "$WORK/_solomon_both.o" ]; then
			sweep
			die "fixed-form Fortran is not supported: the compiler reads $SRC as fixed form with the given flags ($COMPILER$FLAGS); use free form (a free-form suffix such as .f90, or the compiler's free-form flag)"
		fi
		sweep
		msg=$(first_message "$WORK/_solomon_free.err")
		[ -n "$msg" ] || msg="(no message from the compiler; a flag that keeps -c from writing an object, as -fsyntax-only, has this effect)"
		die "the compiler did not compile a free-form Fortran source with the suffix of $SRC and the given flags ($COMPILER$FLAGS): $msg"
	fi
	sweep
fi

# notes on stderr (stdout and the exit status do not change): no backend detected, or a requested backend
# disabled; a -D_OPENACC / -D_OPENMP given on the command line counts as detected
ACC_FOUND=""
OMP_FOUND=""
case "$MACRO $DEFS" in
	*-D_OPENACC*) ACC_FOUND=1 ;;
esac
case "$MACRO $DEFS" in
	*-D_OPENMP*) OMP_FOUND=1 ;;
esac
if [ -z "$ACC_FOUND$OMP_FOUND" ]; then
	printf '%s\n' "solomon: note: neither _OPENACC nor _OPENMP was detected from the compiler and flags; all directives expand to serial code (add an offload flag such as -acc, -mp=gpu, or -fopenmp if offloading was intended)" >&2
fi
case "$DEFS" in
	*-DSOLOMON_OFFLOAD_BY_OPENACC* | *-DOFFLOAD_BY_OPENACC*)
		if [ -z "$ACC_FOUND" ]; then
			printf '%s\n' "solomon: note: OpenACC offloading was requested (SOLOMON_OFFLOAD_BY_OPENACC) but _OPENACC was not detected; the OpenACC backend is disabled and the directives expand to host OpenMP or serial code (add an OpenACC flag such as -acc or -fopenacc)" >&2
		fi
		;;
esac
case "$DEFS" in
	*-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET* | *-DOFFLOAD_BY_OPENMP_TARGET*)
		if [ -z "$OMP_FOUND" ]; then
			printf '%s\n' "solomon: note: OpenMP target offloading was requested (SOLOMON_OFFLOAD_BY_OPENMP_TARGET) but _OPENMP was not detected; the OpenMP target backend is disabled and the directives expand to serial code (add an OpenMP flag such as -mp=gpu, -fopenmp, or -fiopenmp)" >&2
		fi
		;;
esac

# the language and the standard of the system cpp follow the compiler's answer (C and C++)
XLANG="-x c"
STD=""
# $1 = the standards (CSTDS or CXXSTDS), $2 = the compiler's value (without L; 0 when undefined), $3 = the prefix
# of the names; sets STD to the standard whose value is the first not below $2 (the newest when $2 is beyond) or,
# when cpp does not accept its names, to the next older one cpp accepts (none: STD is empty)
cpp_std() {
	tried=""
	for e in $1; do
		tried="$e $tried"
		b=${e%%/*}
		[ "$b" = - ] || [ "$2" -gt "$b" ] || break
	done
	STDDOWN=1
	for e in $tried; do
		for a in $(printf '%s\n' "${e#*/}" | tr ',' ' '); do
			case "$a" in
				=*) n=${a#=} ;;
				*) n=$3$a ;;
			esac
			# shellcheck disable=SC2086 # XLANG is a word list on purpose
			if cpp -P $XLANG "-std=$n" /dev/null > /dev/null 2>&1; then
				STD="-std=$n"
				[ "$e" != "${tried%% *}" ] || STDDOWN=""
				return
			fi
		done
	done
}
# $1 = name; prints its value among the -D<name>=<value> words of $2
value_of() {
	# shellcheck disable=SC2086 # $2 is a word list on purpose
	printf '%s\n' $2 | sed -n "s/^-D$1=//p"
}
if [ "$LANGUAGE" = c ]; then
	pre=gnu
	[ -z "$(value_of __STRICT_ANSI__ "$MACRO")" ] || pre=c
	vname=__cplusplus
	v=$(value_of __cplusplus "$MACRO")
	stds=$CSTDS
	if [ -n "$v" ]; then
		XLANG="-x c++"
		stds=$CXXSTDS
		pre=$pre++
	else
		vname=__STDC_VERSION__
		v=$(value_of __STDC_VERSION__ "$MACRO")
	fi
	answer=${v:-undefined}
	v=${v%L}
	case "$v" in "" | *[!0-9]*) v=0 ;; esac
	cpp_std "$stds" "$v" "$pre"
	if [ -n "$STDDOWN" ]; then
		printf '%s\n' "solomon: note: the system cpp does not know the standard of the compiler ($vname $answer); it preprocesses with ${STD:-its default standard} and the compiler's values of __cplusplus, __STDC_VERSION__ and __STRICT_ANSI__, so the other macros of the standard (such as the __cpp_* feature macros) are those of ${STD:-its default standard}" >&2
	fi
fi
# every probed macro whose value differs from the one of the system cpp (in that language and standard) is undefined
# (-U) and given the compiler's value (-D); the other macros of the probe (_OPENACC, _OPENMP, ...) are given as they are
# shellcheck disable=SC2086 # XLANG and STD are word lists on purpose
CPPMACRO=$(cpp -P $XLANG $STD "$WORK/$PROBE" 2> /dev/null | grep -- "^-D[A-Za-z0-9_]*=[A-Za-z0-9_]*[[:space:]]*$" | tr '\n\r' '  ')
FIX=""
for m in $NAMES; do
	c=$(value_of "$m" "$MACRO")
	p=$(value_of "$m" "$CPPMACRO")
	[ "$c" != "$p" ] || continue
	[ -z "$p" ] || FIX="$FIX -U$m"
	[ -z "$c" ] || FIX="$FIX -D$m=$c"
done
OTHER=""
for w in $MACRO; do
	n=${w#-D}
	case " $NAMES " in
		*" ${n%%=*} "*) ;;
		*) OTHER="$OTHER $w" ;;
	esac
done
# a builtin macro undefined above gives no warning (GCC's cpp still warns on __STDC_VERSION__: that line is removed)
case "$FIX" in
	*-U__*) cpp -Wno-builtin-macro-redefined -P /dev/null > /dev/null 2>&1 && FIX="-Wno-builtin-macro-redefined $FIX" ;;
esac

case "$LANGUAGE" in
	c)
		# shellcheck disable=SC2086 # XLANG, STD, FIX, OTHER and PPARGS are word lists on purpose
		eval "cpp -P $TRACK $XLANG $STD $FIX $OTHER $PPARGS \"\$SRC\"" 2> "$WORK/_cpp.err"
		rc=$?
		grep -v -- '^<command-line>:.*__STDC_VERSION__' "$WORK/_cpp.err" >&2
		exit $rc
		;;
	*)
		# protect Fortran string concatenation (//) from being eaten as a C++ comment by cpp;
		# __SOLOMON_FC_CONCAT__ is a reserved token restored after preprocessing; the work files do not end in
		# .f90, so that a cpp driver (e.g. clang's) does not hand them to a Fortran compiler, and -x c makes a cpp
		# that is clang -E read them as C (it takes an unknown suffix as a linker input and writes nothing).
		# cpp searches the directory of its input first for quoted includes: the input sits alone at the bottom of
		# a chain of private directories one deeper than the source directory (so that no ../ path from there
		# leaves the chain), and the source directory follows as -iquote
		if ! p=$(CDPATH='' cd -- "$SRCDIR" > /dev/null && pwd -P); then
			die "failed to enter the directory of the source: $SRCDIR"
		fi
		IN=$WORK/.s
		while [ -n "$p" ] && [ "$p" != / ]; do
			p=${p%/*}
			IN=$IN/.s
		done
		if ! mkdir -p "$IN" || ! {
			printf '#line 1 "%s"\n' "$SRC"
			sed 's,//,__SOLOMON_FC_CONCAT__,g' "$SRC"
		} > "$IN/.solomon.spp"; then
			die "failed to write a work file under $WORK"
		fi
		# shellcheck disable=SC2086 # FIX and OTHER are word lists on purpose
		eval "cpp -P $TRACK -x c $FIX $OTHER -DSOLOMON_FORTRAN -iquote \"\$SRCDIR\" $PPARGS \"\$IN/.solomon.spp\" -o \"\$WORK/_i.spp\""
		rc=$?
		if [ "$rc" -ne 0 ]; then
			exit $rc
		fi
		[ -f "$WORK/_i.spp" ] || die "cpp wrote no output (cpp must be the C preprocessor of GCC or LLVM)"
		# free form: an automatic profiler tag longer than 132 columns (a long path) is continued with &
		LC_ALL=C awk "$FOLD" "$WORK/_i.spp" > "$WORK/_fold.spp" || exit 1
		sed -e 's/^#pragma solomon_fprof //' -e 's/^#pragma /!$/g' -e 's,__SOLOMON_FC_CONCAT__,//,g' "$WORK/_fold.spp"
		exit $?
		;;
esac
