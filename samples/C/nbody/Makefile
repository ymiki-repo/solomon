SRCDIR	:= src
# the directory solomon/ of Solomon; ?= takes the environment variable SOLOMON_DIR as well
# (a copy of this sample outside samples/C/ needs it, e.g. make SOLOMON_DIR=/path/to/solomon)
SOLOMON_DIR	?= ../../../solomon
# SOLOMON_DIR is checked unless the only goal is clean
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
SPECIAL_CHARS	:= ' " \ ( ) ; & | < > : [ ] * ? `
ifneq ($(word 2,$(SOLOMON_DIR))$(strip $(foreach c,$(SPECIAL_CHARS),$(findstring $(c),$(SOLOMON_DIR)))),)
$(error SOLOMON_DIR='$(SOLOMON_DIR)' holds a space, a quote, or one of \ ( ) ; & | < > : [ ] * ? and the backquote, which the compile commands and the rules of make cannot take: move Solomon to a path without them)
endif
ifeq ($(wildcard $(SOLOMON_DIR)/solomon.hpp),)
$(error $(SOLOMON_DIR)/solomon.hpp is not found: set SOLOMON_DIR to the directory solomon/ of Solomon, e.g. make SOLOMON_DIR=/path/to/solomon)
endif
endif

# NUM_THREADS	:= 1024
NUM_THREADS	:= 512
# NUM_THREADS	:= 256
# NUM_THREADS	:= 64
PARAM	:= -DNTHREADS="($(NUM_THREADS))"

USE_OPENACC	:= 0
ifeq ($(USE_OPENACC), 1)
	USE_ACC_PARALLEL	:= 1
else
	USE_OMP_DISTRIBUTE	:= 0
endif
USE_FAST_MATH	:= 1
MODEL_ID	:= 0

DEBUG_MODE	:= 0
# compiler: set one of AMDCLANG, ICPX to 1 (NVHPC is 1 unless another one is selected);
# with all of them 0, $(CXX) builds a host binary without offloading. ACPP (AdaptiveCpp) is not supported yet
AMDCLANG	:= 0
ICPX	:= 0
ACPP	:= 0
ifeq ($(filter 1,$(AMDCLANG) $(ICPX) $(ACPP)),)
	NVHPC	:= 1
else
	NVHPC	:= 0
endif
# the settings are checked unless the only goal is clean
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
ifneq ($(filter-out 0 1,$(NVHPC) $(AMDCLANG) $(ICPX) $(ACPP)),)
$(error NVHPC, AMDCLANG, ICPX, and ACPP take 0 or 1 (given: NVHPC=$(NVHPC) AMDCLANG=$(AMDCLANG) ICPX=$(ICPX) ACPP=$(ACPP)))
endif
ifneq ($(word 2,$(filter 1,$(NVHPC) $(AMDCLANG) $(ICPX) $(ACPP))),)
$(error select at most one compiler (given: NVHPC=$(NVHPC) AMDCLANG=$(AMDCLANG) ICPX=$(ICPX) ACPP=$(ACPP)); e.g. make AMDCLANG=1 GPU_ARCH=gfx90a)
endif
ifeq ($(ACPP), 1)
$(error ACPP=1: AdaptiveCpp is not supported by this sample yet; select NVHPC, AMDCLANG, or ICPX)
endif
ifeq ($(USE_OPENACC), 1)
ifneq ($(filter 1,$(AMDCLANG) $(ICPX)),)
$(error USE_OPENACC=1 needs NVHPC=1 (amdclang++ and icpx do not compile OpenACC))
endif
endif
ifeq ($(filter 1,$(NVHPC) $(AMDCLANG) $(ICPX)),)
$(info NVHPC=AMDCLANG=ICPX=0: host build with $(CXX), without offloading to GPU)
endif
endif

BENCHMARK	:= 0
SET_NMIN	:= 0
SET_NMAX	:= 0
SET_NCRIT	:= 0

# target GPU (GPU_ARCH on the command line): NVIDIA defaults to 90; AMD and Intel GPUs have no default
# GPU_ARCH	:= pvc # Intel PVC
# GPU_ARCH	:= 80 # NVIDIA A100
# GPU_ARCH	:= 90 # NVIDIA H100
# GPU_ARCH	:= 120 # RTX PRO 4000 Blackwell
# GPU_ARCH	:= gfx908 # CDNA 1 GPU (MI100)
# GPU_ARCH	:= gfx90a # CDNA 2 GPUs (MI210, MI250, MI250X)
ifneq ($(origin GPU_ARCH), command line)
ifeq ($(NVHPC), 1)
	GPU_ARCH	:= 90 # NVIDIA H100
else
	GPU_ARCH	:=
endif
endif
override GPU_ARCH	:= $(strip $(GPU_ARCH))
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
ifneq ($(filter 1,$(NVHPC) $(AMDCLANG) $(ICPX)),)
ifeq ($(GPU_ARCH),)
$(error GPU_ARCH is not set: give the target GPU, e.g. make AMDCLANG=1 GPU_ARCH=gfx90a or make ICPX=1 GPU_ARCH=pvc)
endif
endif
endif

# option(s)
STD	:= --std=c++17
FLAG	:= -Wall -Wextra -Wunused -Wcast-qual -Wcast-align
ARGS	:= -O3 -ffast-math
ifeq ($(NVHPC), 1)
	ARGS	:= -Ofast
endif

ifeq ($(USE_OPENACC), 1)
	PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENACC
	ifeq ($(USE_ACC_PARALLEL), 1)
		PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL
	endif
	ARGS_NVC	:= -acc=gpu -Minfo=accel,opt
else
	PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET
	ifeq ($(USE_OMP_DISTRIBUTE), 1)
		PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
	endif
	ARGS_NVC	:= -mp=gpu -Minfo=accel,opt,mp
	ARGS_AMD	:= -fopenmp --offload-arch=$(GPU_ARCH)
	ARGS_ICPX	:= -fiopenmp -fopenmp-targets=spir64_gen
	# -Xs "-device pvc" is for Ahead-of-Time compilation (used at the link)
	LINK_ICPX	:= -Xs "-device $(GPU_ARCH)"
endif
ifeq ($(USE_FAST_MATH), 1)
# adding fastmath is required to achieve a reasonable performance (-Mfprelaxed=rsqrt only is not enough)
# in CUDA, --use_fast_math implies --ftz=true --prec-div=false --prec-sqrt=false --fmad=true
# therefore, amdclang++/icpx should activate corresponding optimization with --ftz=true
	ARGS_NVC	+= -Mfprelaxed=rsqrt -gpu=cc$(GPU_ARCH),fastmath
	ARGS_AMD	+= -fgpu-flush-denormals-to-zero
	ARGS_ICPX	+= -ftz
else
	ARGS_NVC	+= -gpu=cc$(GPU_ARCH)
	ARGS_ICPX	+= -no-ftz
endif

ifeq ($(NVHPC), 1)
	CXX	:= nvc++
	ARGS	+= $(ARGS_NVC)
# nvc++ warns [independent_loop_type] on every compute construct with the independent clause, about the device version
# of the enclosing function that it infers (not called on the device in this sample); suppress it when nvc++ knows the name
	ifeq ($(USE_OPENACC), 1)
		ARGS	+= $(shell $(CXX) --diag_suppress independent_loop_type -E -x c++ /dev/null > /dev/null 2>&1 && echo --diag_suppress independent_loop_type)
	endif
else
	ifeq ($(AMDCLANG), 1)
		CXX	:= amdclang++
		ARGS	+= $(ARGS_AMD)
	else
		ifeq ($(ICPX), 1)
			CXX	:= icpx
			ARGS	+= $(ARGS_ICPX)
			LINK_ARGS	:= $(LINK_ICPX)
		endif
	endif
endif
# the compiler name in the CSV file name of BENCHMARK=1 (no directory, no option)
PARAM	+= -DCOMPILER="\"$(notdir $(firstword $(CXX)))\"" -DMODEL_ID="($(MODEL_ID))"

ifeq ($(BENCHMARK), 1)
PARAM	+= -DBENCHMARK_MODE
ifneq ($(SET_NMIN), 0)
PARAM	+= -DNMIN="($(SET_NMIN))"
endif
ifneq ($(SET_NMAX), 0)
PARAM	+= -DNMAX="($(SET_NMAX))"
endif
ifneq ($(SET_NCRIT), 0)
PARAM	+= -DNMAX_CRIT="($(SET_NCRIT))"
endif
endif

INC	:= -I$(SOLOMON_DIR)
INC	+= -isystem=$(BOOST_INC)
# LIB	+= -L$(BOOST_LIB) #-lboost_filesystem
LDFLAGS	:=
LDLIBS	:=

# debug mode
ifeq ($(DEBUG_MODE), 1)
	ARGS	+= -g
	# ARGS	+= -fsanitize=address -fsanitize=leak -fsanitize=undefined
endif

# directories
BINDIR	:= bin
OBJDIR	:= $(BINDIR)/obj
RUNDIR	:= dat fig log
VPATH	:= $(SRCDIR)

# source files
SRC := nbody.cpp
SRC += gravity.cpp
SRC_ACC := nbody_acc.cpp
SRC_ACC += gravity_acc.cpp
SRC_OMP := nbody_omp.cpp
SRC_OMP += gravity_omp.cpp
SRC_LIB := allocate.cpp
SRC_LIB += gen_ic.cpp
SRC_LIB += filename.cpp

# objects
OBJ	:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC) $(SRC_LIB)))
OBJ_ACC	:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC_ACC) $(SRC_LIB)))
OBJ_OMP	:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC_OMP) $(SRC_LIB)))
# header dependencies written by the compiler (-MM -MP)
DEP	:= $(sort $(OBJ:.o=.d) $(OBJ_ACC:.o=.d) $(OBJ_OMP:.o=.d))
# build settings of the last build: the objects and the executables are rebuilt when the settings (make variables) change
STAMP	:= $(OBJDIR)/settings.txt
SETTINGS	:= $(CXX) $(STD) $(FLAG) $(ARGS) $(PARAM) $(INC) $(TARGET_ARCH) $(LINK_ARGS) $(LIB) $(LDFLAGS) $(LDLIBS)

# executable
EXE	:= $(BINDIR)/nbody_pragma
ACC	:= $(BINDIR)/acc_pragma
OMP	:= $(BINDIR)/omp_pragma
all:	$(EXE) $(ACC) $(OMP)
default:	$(EXE)
acc:	$(ACC)
omp:	$(OMP)
.PHONY:	all default acc omp dir clean FORCE

# rules
$(EXE):	$(OBJ) $(STAMP) | $(RUNDIR)
	$(CXX) $(STD) $(FLAG) $(ARGS) $(TARGET_ARCH) -o $@ $(OBJ) $(LINK_ARGS) $(LIB) $(LDFLAGS) $(LDLIBS)
$(ACC):	$(OBJ_ACC) $(STAMP) | $(RUNDIR)
	$(CXX) $(STD) $(FLAG) $(ARGS) $(TARGET_ARCH) -o $@ $(OBJ_ACC) $(LINK_ARGS) $(LIB) $(LDFLAGS) $(LDLIBS)
$(OMP):	$(OBJ_OMP) $(STAMP) | $(RUNDIR)
	$(CXX) $(STD) $(FLAG) $(ARGS) $(TARGET_ARCH) -o $@ $(OBJ_OMP) $(LINK_ARGS) $(LIB) $(LDFLAGS) $(LDLIBS)

$(OBJDIR)/%.o:	%.cpp $(STAMP) Makefile | $(OBJDIR)
	$(CXX) $(STD) $(FLAG) $(ARGS) $(PARAM) $(INC) $(TARGET_ARCH) -o $@ -c $<
	$(CXX) $(STD) $(ARGS) $(PARAM) $(INC) $(TARGET_ARCH) -MM -MP -MT $@ -MF $(@:.o=.d) $<

$(STAMP):	FORCE | $(OBJDIR)
	@printf '%s\n' '$(subst ','\'',$(SETTINGS))' | cmp -s - $@ || printf '%s\n' '$(subst ','\'',$(SETTINGS))' > $@

# make directories (the build makes them as well)
dir:	$(OBJDIR) $(RUNDIR)
$(OBJDIR) $(RUNDIR):
	mkdir -p $@

# clean up
clean:
	rm -rf $(EXE) $(ACC) $(OMP) $(OBJDIR)/*.o $(OBJDIR)/*.d $(STAMP)

# the dependency files are not read when the only goal is clean (a broken one would stop make clean)
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
-include $(DEP)
endif
