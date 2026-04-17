#!/bin/sh

COMPILER=""
INCS=""
DEFS=""
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
	    echo "$0: error: unknown option: $1" >&2
	    exit 1
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

MACRO=$($COMPILER -E $CONFTEST | grep -- "^-D")

rm -f $CONFTEST

case "$SRC" in
    *.c|*.C)
	cpp -P $MACRO $INCS $DEFS $SRC
	;;
    *.cc|*.cpp|*.cxx|*.CC|*.CPP)
	cpp -P $MACRO $INCS $DEFS $SRC
	;;
    *.f|*.for|*.f90|*.f95|*.f03|*.f08|*.F|*.F90|*.F95|*.F03|*.F08)
	cpp -P $MACRO -DSOLOMON_FORTRAN $INCS $DEFS $SRC -o _$SRC.i
	rc=$?
	if [ "$rc" -eq 0 ]; then
	    sed 's/^#pragma /!$/g'  _$SRC.i
	fi
	rm -f _$SRC.i
	exit $rc
	;;
    *)
	echo "$0: error: unknown source file suffix: $SRC" >&2
	exit 1
	;;
esac
