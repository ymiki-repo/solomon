#!/bin/sh
# solomon_migrate_v1_to_v2.sh: convert Solomon v1.x macro spellings to the v2.0.0 SOLOMON_-prefixed ones
#
# usage:
#   solomon_migrate_v1_to_v2.sh [--apply] <file|directory> ...
#
#   default   : dry-run; print a unified diff of the changes without modifying any file
#   --apply   : rewrite the files in place (a backup is kept as <file>.v1.bak)
#
# notes:
#   * directories are searched recursively for C/C++/Fortran sources
#     (.c .h .cc .cpp .cxx .hpp .f .for .f90 .f95 .f03 .f08 and their upper-case variants);
#     pass build scripts (e.g., Makefile) explicitly as file arguments if desired
#   * replacements are word-boundary based and idempotent (running twice is harmless);
#     occurrences inside comments and string literals are also replaced, so please review the diff
#   * configuration macros (-DOFFLOAD_BY_*) are converted to -DSOLOMON_OFFLOAD_BY_* as well,
#     although the unprefixed spellings keep working unconditionally
#   * to verify a migration, compare the preprocessed sources before and after
#     (C/C++: compiler -E output; Fortran: the spp output under spp/); they must be identical

APPLY=0
FILES=""

usage() {
    echo "usage: $0 [--apply] <file|directory> ..." >&2
    exit 2
}

for arg in "$@"; do
    case "$arg" in
        --apply) APPLY=1 ;;
        -h|--help) usage ;;
        -*) echo "$0: error: unknown option: $arg" >&2; usage ;;
        *)
            if [ -d "$arg" ]; then
                found=$(find "$arg" -type f \( \
                    -name '*.c' -o -name '*.h' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.hpp' \
                    -o -name '*.C' -o -name '*.CC' -o -name '*.CPP' \
                    -o -name '*.f' -o -name '*.for' -o -name '*.f90' -o -name '*.f95' -o -name '*.f03' -o -name '*.f08' \
                    -o -name '*.F' -o -name '*.F90' -o -name '*.F95' -o -name '*.F03' -o -name '*.F08' \) )
                FILES="$FILES $found"
            elif [ -f "$arg" ]; then
                FILES="$FILES $arg"
            else
                echo "$0: error: no such file or directory: $arg" >&2
                exit 1
            fi
            ;;
    esac
done

[ -n "$FILES" ] || usage

# v1.x -> v2.0.0 macro mapping (word-boundary; \b requires GNU sed)
SED_SCRIPT='
s/\bOFFLOAD_OUTER_LOOP\b/SOLOMON_OFFLOAD_OUTER_LOOP/g
s/\bEND_OFFLOAD_OUTER_LOOP\b/SOLOMON_END_OFFLOAD_OUTER_LOOP/g
s/\bPARALLELIZE_INNER_LOOP\b/SOLOMON_PARALLELIZE_INNER_LOOP/g
s/\bEND_OFFLOAD\b/SOLOMON_END_OFFLOAD/g
s/\bOFFLOAD\b/SOLOMON_OFFLOAD/g
s/\bIF_NOT_OFFLOADED\b/SOLOMON_IF_NOT_OFFLOADED/g
s/\bDECLARE_OFFLOADED_END\b/SOLOMON_DECLARE_OFFLOADED_END/g
s/\bDECLARE_END_OFFLOADED\b/SOLOMON_DECLARE_OFFLOADED_END/g
s/\bDECLARE_OFFLOADED\b/SOLOMON_DECLARE_OFFLOADED/g
s/\bSYNCHRONIZE\b/SOLOMON_SYNCHRONIZE/g
s/\bWAIT_QUEUE\b/SOLOMON_WAIT_QUEUE/g
s/\bATOMIC_UPDATE\b/SOLOMON_ATOMIC_UPDATE/g
s/\bATOMIC_READ\b/SOLOMON_ATOMIC_READ/g
s/\bATOMIC_WRITE\b/SOLOMON_ATOMIC_WRITE/g
s/\bATOMIC_CAPTURE\b/SOLOMON_ATOMIC_CAPTURE/g
s/\bATOMIC\b/SOLOMON_ATOMIC/g
s/\bDATA_ACCESS_BY_HOST\b/SOLOMON_DATA_ACCESS_BY_HOST/g
s/\bDATA_ACCESS_BY_DEVICE\b/SOLOMON_DATA_ACCESS_BY_DEVICE/g
s/\bUSE_DEVICE_DATA_FROM_HOST\b/SOLOMON_USE_DEVICE_DATA_FROM_HOST/g
s/\bMALLOC_ON_DEVICE\b/SOLOMON_MALLOC_ON_DEVICE/g
s/\bALLOCATE_ON_DEVICE\b/SOLOMON_ALLOCATE_ON_DEVICE/g
s/\bFREE_FROM_DEVICE\b/SOLOMON_FREE_FROM_DEVICE/g
s/\bDEALLOCATE_ON_DEVICE\b/SOLOMON_DEALLOCATE_ON_DEVICE/g
s/\bMEMCPY_D2H\b/SOLOMON_MEMCPY_D2H/g
s/\bMEMCPY_H2D\b/SOLOMON_MEMCPY_H2D/g
s/\bDECLARE_DATA_ON_DEVICE\b/SOLOMON_DECLARE_DATA_ON_DEVICE/g
s/\bAS_INDEPENDENT\b/SOLOMON_CLAUSE_INDEPENDENT/g
s/\bAS_SEQUENTIAL\b/SOLOMON_CLAUSE_SEQUENTIAL/g
s/\bNUM_THREADS\b/SOLOMON_CLAUSE_NUM_THREADS/g
s/\bNUM_BLOCKS\b/SOLOMON_CLAUSE_NUM_BLOCKS/g
s/\bNUM_GRIDS\b/SOLOMON_CLAUSE_NUM_GRIDS/g
s/\bAS_THREAD\b/SOLOMON_CLAUSE_THREAD/g
s/\bAS_BLOCK\b/SOLOMON_CLAUSE_BLOCK/g
s/\bAS_GRID\b/SOLOMON_CLAUSE_GRID/g
s/\bCOLLAPSE\b/SOLOMON_CLAUSE_COLLAPSE/g
s/\bAS_ASYNC\b/SOLOMON_CLAUSE_ASYNC/g
s/\bASYNC_QUEUE\b/SOLOMON_CLAUSE_ASYNC_QUEUE/g
s/\bREDUCTION\b/SOLOMON_CLAUSE_REDUCTION/g
s/\bENABLE_IF\b/SOLOMON_CLAUSE_IF/g
s/\bAS_PRIVATE\b/SOLOMON_CLAUSE_PRIVATE/g
s/\bAS_FIRSTPRIVATE\b/SOLOMON_CLAUSE_FIRSTPRIVATE/g
s/\bAS_DEVICE_PTR\b/SOLOMON_CLAUSE_DEVICE_PTR/g
s/\bCOPY_BEFORE_AND_AFTER_EXEC\b/SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC/g
s/\bCOPY_H2D_BEFORE_EXEC\b/SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC/g
s/\bCOPY_D2H_AFTER_EXEC\b/SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC/g
s/\bAPPEND_ARGS\b/SOLOMON_APPEND_ARGS/g
s/\bOFFLOAD_BY_OPENACC_PARALLEL\b/SOLOMON_OFFLOAD_BY_OPENACC_PARALLEL/g
s/\bOFFLOAD_BY_OPENACC\b/SOLOMON_OFFLOAD_BY_OPENACC/g
s/\bOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE\b/SOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE/g
s/\bOFFLOAD_BY_OPENMP_TARGET\b/SOLOMON_OFFLOAD_BY_OPENMP_TARGET/g
s/-DOFFLOAD_BY_/-DSOLOMON_OFFLOAD_BY_/g
'

status=0
for f in $FILES; do
    tmp="${f}.v2.tmp"
    sed "$SED_SCRIPT" "$f" > "$tmp" || { echo "$0: error: sed failed on $f" >&2; rm -f "$tmp"; exit 1; }
    if cmp -s "$f" "$tmp"; then
        rm -f "$tmp"
        continue
    fi
    if [ "$APPLY" -eq 1 ]; then
        cp "$f" "${f}.v1.bak" && mv "$tmp" "$f" && echo "converted: $f (backup: ${f}.v1.bak)"
    else
        diff -u "$f" "$tmp"
        rm -f "$tmp"
        status=1
    fi
done

if [ "$APPLY" -eq 0 ] && [ "$status" -eq 1 ]; then
    echo ""
    echo "dry-run only; re-run with --apply to rewrite the files (backups are kept as *.v1.bak)"
fi
exit 0
