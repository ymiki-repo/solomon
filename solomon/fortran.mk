# Solomon rule for Fortran sources: spp/<name>.f90 is made from <name>.f90 (also in subdirectories: spp/sub/x.f90
# from sub/x.f90). Set SOLOMON_FC and SOLOMON_FLAGS (with the OpenACC / OpenMP flag and the -D / -I flags) before
# the rule is used; both are read as shell words (quoted values and the forms -I <dir> / -D <macro> are accepted).
# The options of the preprocessor in them reach cpp as well, in the order they are written: -I, -D, -U, -include,
# -imacros, -isystem, -idirafter, -iquote (each also joined to its value), -Xpreprocessor <option>, -Wp,<options>,
# --include=<file> / --include <file>.
# Work files go to a private directory next to the output (spp/.solomon.<pid>.<n>/) and are removed also when a
# stage fails; the output is written only when every stage succeeded.
# Nothing about the compiler or the flags is decided by their spelling: the compiler itself is asked, with
# SOLOMON_FLAGS as given. _OPENACC / _OPENMP (and nvfortran's __NVCOMPILER as __SOLOMON_NVCOMPILER__, read by
# solomon/omp/clause_tag.hpp), and the macros of cpp that are not reserved names (linux, unix: undefined for cpp
# unless the compiler defines them too) come from a probe file the compiler preprocesses (-E); the source
# form comes from a two-line free-form program the compiler compiles (-c): when that fails and a program written
# from column 7 compiles, the compiler reads the source as fixed form and the rule stops (fixed form is not
# supported); when both fail, the rule stops with the compiler's message. The probes run in the current directory
# with their input and output in the work directory, so that the flags keep their meaning (relative paths, @files)
# and the by-products named after the output (-MD, -MMD, ...) stay in the work directory; a by-product named after
# the probe input (_solomon*) that a compiler writes in the current directory is removed afterwards; a file the
# flags name explicitly (-MF x.d) is written as asked.
# Quoted includes of the source are searched in the directory of the source first, then in the -I directories, as
# when the source itself is preprocessed; in an included file, a quoted include is searched in the directory of
# that file, then in the directory of the source, then in the -I directories (a direct preprocessing does not
# search the directory of the source there). The automatic profiler tags name the source as given to make (#line).
# Stops with a message when SOLOMON_FC is empty, when the compiler, cpp, awk, or sed is not found, when the
# compiler does not preprocess or compile the probes with SOLOMON_FLAGS (also when they give fixed form), when the
# source is not a regular file, when the output path is a directory (when the rule runs: make does not run it while
# that directory is newer than the source), when SHELL does not split words as a POSIX sh, and for a path with a
# backslash or a double quote (#line cannot carry it); make cannot handle file names with spaces (use spp.sh for
# them).
# Notes on stderr: neither _OPENACC nor _OPENMP detected, or a requested backend (SOLOMON_OFFLOAD_BY_OPENACC /
# SOLOMON_OFFLOAD_BY_OPENMP_TARGET) disabled because its macro was not detected.

# GCC's cpp only (other preprocessors reject the option): -ftrack-macro-expansion=0 keeps __LINE__ (the line
# numbers of the automatic profiler tags) right in long files; evaluated once when this file is read
SOLOMON_CPP_TRACK := $(shell cpp -ftrack-macro-expansion=0 -P /dev/null >/dev/null 2>&1 && echo -ftrack-macro-expansion=0)

# the macros of cpp that are not reserved names (linux, unix); evaluated once when this file is read
# (without awk the recipe stops with its own message: the shell's "awk: not found" is not shown first)
SOLOMON_CPP_NAMES := $(shell cpp -dM -E -x c /dev/null 2>/dev/null | awk 'substr($$1, 2) == "define" && $$2 ~ /^[A-Za-z][A-Za-z0-9_]*$$/ { print $$2 }' 2>/dev/null)

# $(1) quoted for the shell
SOLOMON_SQ = '$(subst ','\'',$(1))'

# awk program: the message of a compiler, its first line that is not empty, followed by the next line that is not
# indented when the first one only names a place (GCC's "<command-line>:1:0:"; the same program as in spp.sh)
SOLOMON_MSG = m == "" { if (NF) { m = $$0; if (m !~ /:[ \t]*$$/) exit } next } NF && $$0 !~ /^[ \t]/ { m = m " " $$0; exit } END { print m }

# awk program (run with LC_ALL=C): a "#pragma solomon_fprof" line (an automatic profiler tag) whose statement is
# longer than 132 bytes is split into lines of at most 132 bytes with free-form continuation, never inside a
# UTF-8 character (the same program as in spp.sh)
SOLOMON_FOLD = { \
	p = "\#pragma solomon_fprof "; \
	if (substr($$0, 1, 22) != p || length($$0) <= 154) { print; next } \
	s = substr($$0, 23); c = ""; \
	while (length(c) + length(s) > 132) { \
		n = 131 - length(c); \
		while (n > 1 && substr(s, n + 1, 1) >= "\200" && substr(s, n + 1, 1) < "\300") n--; \
		print p c substr(s, 1, n) "&"; \
		s = substr(s, n + 1); c = "&" \
	} \
	print p c s \
}

# The recipe needs a POSIX sh as SHELL (make's default /bin/sh; bash and dash as well; zsh does not split an unquoted
# variable into words, and the recipe stops there); it also works in a shell run with -e (.POSIX:, .SHELLFLAGS := -ec).
# cpp searches the directory of its input first for quoted includes: the input sits alone at the bottom of a chain
# of private directories one deeper than the source directory (so that no ../ path from there leaves the chain),
# and the source directory follows as -iquote. -x c makes a cpp that is clang -E read the input as C (it takes an
# unknown suffix as a linker input and writes nothing).
spp/%.f90: %.f90
	@set -f; \
	die() { printf 'solomon: error: %s\n' "$$1" >&2; exit 1; }; \
	t='a b'; set -- $$t; [ $$# -eq 2 ] || die 'fortran.mk needs a POSIX sh as SHELL (the default /bin/sh, bash, or dash): this shell does not split an unquoted variable into words'; \
	for t in cpp awk sed; do command -v $$t > /dev/null 2>&1 || die "$$t is not found in PATH (fortran.mk needs cpp, the C preprocessor of GCC or LLVM, awk, and sed)"; done; \
	msg() { awk '$(SOLOMON_MSG)' "$$1"; }; \
	src=$(call SOLOMON_SQ,$<); out=$(call SOLOMON_SQ,$@); \
	fc=$(call SOLOMON_SQ,$(SOLOMON_FC)); fl=$(call SOLOMON_SQ,$(SOLOMON_FLAGS)); \
	case "$$src" in *\\* | *\"*) die "the source path contains a backslash or a double quote, which #line cannot carry: $$src (rename the file or the directory)" ;; esac; \
	case "$$src$$out" in *[[:space:]]*) die "fortran.mk cannot handle a file name with spaces: $$src (rename it, or preprocess it with spp.sh)" ;; esac; \
	[ -f "$$src" ] || die "the source is not a regular file: $$src"; \
	[ ! -d "$$out" ] || die "the output path is a directory: $$out (remove it)"; \
	(eval "set -- $$fc $$fl") 2> /dev/null || die "SOLOMON_FC and SOLOMON_FLAGS cannot be read as shell words (unbalanced quotes?): $$fc $$fl"; \
	eval "set -- $$fc"; \
	[ $$# -gt 0 ] || die 'SOLOMON_FC is empty (set it to the Fortran compiler, e.g. SOLOMON_FC = $$(FC))'; \
	command -v "$$1" > /dev/null 2>&1 || die "the compiler in SOLOMON_FC is not found: $$1"; \
	eval "set -- $$fc $$fl"; shift; p=; acc=; omp=; acc_req=; omp_req=; \
	for a in "$$@"; do case "$$a" in -I | -D | -U | -include | -imacros | -isystem | -idirafter | -iquote | -Xpreprocessor | --include) p=$$a ;; *) p= ;; esac; done; \
	[ -z "$$p" ] || die "$$p at the end of SOLOMON_FC / SOLOMON_FLAGS has no argument"; \
	printf 'solomon: %s -> %s\n' "$$src" "$$out"; \
	mkdir -p $(call SOLOMON_SQ,$(dir $@)) || exit 1; \
	w=; n=0; \
	while [ $$n -lt 100 ]; do \
		if mkdir $(call SOLOMON_SQ,$(dir $@)).solomon.$$$$.$$n 2> /dev/null; then w=$(call SOLOMON_SQ,$(dir $@)).solomon.$$$$.$$n; break; fi; \
		n=$$((n + 1)); \
	done; \
	[ -n "$$w" ] || die "failed to create a work directory next to $$out"; \
	trap 'rm -rf "$$w"' EXIT; trap 'exit 129' HUP; trap 'exit 130' INT; trap 'exit 143' TERM; \
	sweep() { find . -path './*/*' -prune -o -type f -name '_solomon*' -newer "$$w/_stamp" -exec rm -f {} + 2> /dev/null || :; }; \
	printf '#define SOLOMON_PROBE_VALUE 1\n-D__SOLOMON_PROBE__=SOLOMON_PROBE_VALUE\n#ifdef _OPENMP\n-D_OPENMP=_OPENMP\n#endif\n#ifdef _OPENACC\n-D_OPENACC=_OPENACC\n#endif\n#ifdef __NVCOMPILER\n-D__SOLOMON_NVCOMPILER__=__NVCOMPILER\n#endif\n' > "$$w/_solomon.F" || exit 1; \
	for m in $(SOLOMON_CPP_NAMES); do printf '#ifdef %s\n-D%s=%s\n#endif\n' "$$m" "$$m" "$$m"; done >> "$$w/_solomon.F" || exit 1; \
	: > "$$w/_stamp"; \
	eval "set -- $$fc $$fl"; \
	"$$@" -E "$$w/_solomon.F" -o "$$w/_solomon.i" > "$$w/_solomon.out" 2> "$$w/_solomon.err" || :; \
	cat "$$w/_solomon.i" >> "$$w/_solomon.out" 2> /dev/null || :; \
	sweep; \
	if ! grep -q -- "^[[:space:]]*-D__SOLOMON_PROBE__=1[[:space:]]*$$" "$$w/_solomon.out"; then \
		m=$$(msg "$$w/_solomon.err"); \
		[ -n "$$m" ] || m='(no message from the compiler; a flag that keeps -E from writing the preprocessed text has this effect)'; \
		die "the compiler in SOLOMON_FC did not preprocess the probe file with SOLOMON_FLAGS ($$fc $$fl): $$m"; \
	fi; \
	det=$$(grep -- "^[[:space:]]*-D[A-Za-z0-9_]*=[A-Za-z0-9_]*[[:space:]]*$$" "$$w/_solomon.out" | sed "s/^[[:space:]]*//" | grep -v -- "^-D__SOLOMON_PROBE__=" | sort -u | tr '\n\r' '  ' || :); \
	for m in $(SOLOMON_CPP_NAMES); do case " $$det " in *" -D$$m="*) ;; *) det="-U$$m $$det" ;; esac; done; \
	printf 'program solomon_probe\nend program solomon_probe\n' > "$$w/_solomon_free.f90" || exit 1; \
	"$$@" -c "$$w/_solomon_free.f90" -o "$$w/_solomon_free.o" > "$$w/_solomon_free.err" 2>&1 || :; \
	if [ ! -s "$$w/_solomon_free.o" ]; then \
		printf '      program solomon_probe\n      end program solomon_probe\n' > "$$w/_solomon_both.f90" || exit 1; \
		"$$@" -c "$$w/_solomon_both.f90" -o "$$w/_solomon_both.o" > /dev/null 2>&1 || :; \
		if [ -s "$$w/_solomon_both.o" ]; then sweep; die "fixed-form Fortran is not supported: the compiler reads $$src as fixed form with SOLOMON_FC and SOLOMON_FLAGS ($$fc $$fl); use free form"; fi; \
		sweep; m=$$(msg "$$w/_solomon_free.err"); \
		[ -n "$$m" ] || m='(no message from the compiler; a flag that keeps -c from writing an object, as -fsyntax-only, has this effect)'; \
		die "the compiler did not compile a free-form Fortran source with SOLOMON_FC and SOLOMON_FLAGS ($$fc $$fl): $$m"; \
	fi; \
	sweep; \
	shift; \
	n=$$#; \
	for a in "$$@"; do \
		if [ -n "$$p" ]; then \
			case "$$p" in -I | -D | -U) a=$$p$$a ;; *) set -- "$$@" "$$p" ;; esac; p=; \
		else \
			case "$$a" in -I | -D | -U | -include | -imacros | -isystem | -idirafter | -iquote | -Xpreprocessor | --include) p=$$a; continue ;; -I?* | -D?* | -U?* | -include?* | -imacros?* | -isystem?* | -idirafter?* | -iquote?* | -Wp,* | --include=*) ;; *) continue ;; esac; \
		fi; \
		case "$$a" in \
			-D_OPENACC*) acc=1 ;; -D_OPENMP*) omp=1 ;; \
			-DSOLOMON_OFFLOAD_BY_OPENACC* | -DOFFLOAD_BY_OPENACC*) acc_req=1 ;; \
			-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET* | -DOFFLOAD_BY_OPENMP_TARGET*) omp_req=1 ;; \
		esac; \
		set -- "$$@" "$$a"; \
	done; \
	shift $$n; \
	case " $$det" in *-D_OPENACC*) acc=1 ;; esac; \
	case " $$det" in *-D_OPENMP*) omp=1 ;; esac; \
	[ -n "$$acc$$omp" ] || printf '%s\n' 'solomon: note: neither _OPENACC nor _OPENMP was detected from SOLOMON_FC and SOLOMON_FLAGS; all directives expand to serial code (add an offload flag such as -acc, -mp=gpu, or -fopenmp to SOLOMON_FLAGS if offloading was intended)' >&2; \
	[ -z "$$acc_req" ] || [ -n "$$acc" ] || printf '%s\n' 'solomon: note: OpenACC offloading was requested (SOLOMON_OFFLOAD_BY_OPENACC) but _OPENACC was not detected from SOLOMON_FC and SOLOMON_FLAGS; the OpenACC backend is disabled and the directives expand to host OpenMP or serial code (add an OpenACC flag such as -acc or -fopenacc)' >&2; \
	[ -z "$$omp_req" ] || [ -n "$$omp" ] || printf '%s\n' 'solomon: note: OpenMP target offloading was requested (SOLOMON_OFFLOAD_BY_OPENMP_TARGET) but _OPENMP was not detected from SOLOMON_FC and SOLOMON_FLAGS; the OpenMP target backend is disabled and the directives expand to serial code (add an OpenMP flag such as -mp=gpu, -fopenmp, or -fiopenmp)' >&2; \
	sd=$(call SOLOMON_SQ,$(dir $<)); \
	q=$$(CDPATH='' cd -- "$$sd" > /dev/null && pwd -P) || exit 1; \
	in=$$w/.s; \
	while [ -n "$$q" ] && [ "$$q" != / ]; do q=$${q%/*}; in=$$in/.s; done; \
	mkdir -p "$$in" || exit 1; \
	{ printf '#line 1 "%s"\n' "$$src"; sed 's,//,__SOLOMON_FC_CONCAT__,g' "$$src"; } > "$$in/.solomon.spp" || exit 1; \
	cpp -P $(SOLOMON_CPP_TRACK) -x c "$$in/.solomon.spp" -DSOLOMON_FORTRAN $$det -iquote "$$sd" "$$@" -o "$$w/_i.spp" || exit 1; \
	[ -f "$$w/_i.spp" ] || die 'cpp wrote no output (cpp must be the C preprocessor of GCC or LLVM)'; \
	LC_ALL=C awk '$(SOLOMON_FOLD)' "$$w/_i.spp" > "$$w/_fold.spp" || exit 1; \
	sed -e 's/^#pragma solomon_fprof //' -e 's/^#pragma /!$$/g' -e 's,__SOLOMON_FC_CONCAT__,//,g' "$$w/_fold.spp" > "$$w/_out.spp" || exit 1; \
	mv -f "$$w/_out.spp" "$$out"
