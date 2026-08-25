#!/bin/sh

COMPILER=""
INCS=""
DEFS=""
FLAGS=""
SRC=""

while [ $# -gt 0 ]; do
    case "$1" in
	-compiler=*)
	    COMPILER=${1#-compiler=}
	    shift
	    ;;
	-I*)
	    if [ "$1" = "-I" ]; then
		INCS="$INCS -I$2"
		shift 2
	    else
		INCS="$INCS $1"
		shift
	    fi
	    ;;
	-D*)
	    if [ "$1" = "-D" ]; then
		DEFS="$DEFS -D$2"
		shift 2
	    else
		DEFS="$DEFS $1"
		shift
	    fi
	    ;;
	-*)
	    # compiler flags (e.g., -acc, -mp=gpu, -fopenmp, --offload-arch=gfx90a)
	    # passed to the compiler when probing _OPENACC and _OPENMP;
	    # each flag is kept quoted so that arguments containing spaces survive
	    FLAGS="$FLAGS '$1'"
	    shift
	    ;;
	*)
	    if [ -n "$SRC" ]; then
		echo "$0: error: duplicate source files: $1" >&2
		exit 1
	    fi
	    SRC=$1
	    shift
	    ;;
    esac
done

if [ -z "$COMPILER" ]; then
    echo "$0: error: compiler not specified" >&2
    exit 1
fi

if ! $COMPILER --version > /dev/null 2>&1; then
    echo "$0: error: failed to run compiler: $COMPILER" >&2
    exit 1
fi

if [ -z "$SRC" ]; then
    echo "$0: error: source file not specified" >&2
    exit 1
fi

if [ ! -r "$SRC" ]; then
    echo "$0: error: failed to read source file: $SRC" >&2
    exit 1
fi

#echo "COMPILER = $COMPILER"
#echo "INCS = $INCS"
#echo "DEFS = $DEFS"
#echo "SRC = $SRC"

CONFTEST=_solomon.F

cat <<'EOF' | tee $CONFTEST > /dev/null 2>&1
#ifdef __cplusplus
-D__cplusplus=__cplusplus
#endif
#ifdef _OPENMP
-D_OPENMP=_OPENMP
#endif
#ifdef _OPENACC
-D_OPENACC=_OPENACC
#endif
EOF
if [ $? -ne 0 ]; then
    echo "$0: error: failed to create a temporal file: $CONFTEST" >&2
    exit 1
fi

MACRO=$(eval "$COMPILER $FLAGS -E $CONFTEST" | grep -- "^-D")

rm -f $CONFTEST

case "$SRC" in
    *.c|*.C)
	cpp -P $MACRO $INCS $DEFS $SRC
	;;
    *.cc|*.cpp|*.cxx|*.CC|*.CPP)
	cpp -P $MACRO $INCS $DEFS $SRC
	;;
    *.f|*.for|*.f90|*.f95|*.f03|*.f08|*.F|*.F90|*.F95|*.F03|*.F08)
	# protect Fortran string concatenation (//) from being eaten as a C++ comment by cpp;
	# __SOLOMON_FC_CONCAT__ is a reserved token restored after preprocessing
	sed 's,//,__SOLOMON_FC_CONCAT__,g' $SRC > _$SRC.spp
	cpp -P $MACRO -DSOLOMON_FORTRAN $INCS $DEFS _$SRC.spp -o _$SRC.i
	rc=$?
	if [ "$rc" -eq 0 ]; then
	    sed -e 's/^#pragma solomon_fprof //' -e 's/^#pragma /!$/g' -e 's,__SOLOMON_FC_CONCAT__,//,g' _$SRC.i
	fi
	rm -f _$SRC.spp _$SRC.i
	exit $rc
	;;
    *)
	echo "$0: error: unknown source file suffix: $SRC" >&2
	exit 1
	;;
esac
