SRCDIR	:= src
SOLOMON_DIR := ../../../solomon

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
NVHPC	:= 1
AMDCLANG	:= 0
ICPX	:= 0
ACPP	:= 0

BENCHMARK	:= 0
SET_NMIN	:= 0
SET_NMAX	:= 0
SET_NCRIT	:= 0

# GPU_ARCH	:= pvc # Intel PVC
# GPU_ARCH	:= 80 # NVIDIA A100
GPU_ARCH	:= 90 # NVIDIA H100
# GPU_ARCH	:= 120 # RTX PRO 4000 Blackwell
# GPU_ARCH	:= gfx908 # CDNA 1 GPU (MI100)
# GPU_ARCH	:= gfx90a # CDNA 2 GPUs (MI210, MI250, MI250X)
GPU_ARCH	:= $(strip $(GPU_ARCH))

# option(s)
STD	:= --std=c++17
FLAG	:= -Wall -Wextra -Wunused -Wcast-qual -Wcast-align
ARGS	:= -O3 -ffast-math
ifeq ($(NVHPC), 1)
	ARGS	:= -Ofast
endif

ifeq ($(USE_OPENACC), 1)
	PARAM	+= -DOFFLOAD_BY_OPENACC
	ifeq ($(USE_ACC_PARALLEL), 1)
		PARAM	+= -DOFFLOAD_BY_OPENACC_PARALLEL
	endif
	ARGS_NVC	:= -acc=gpu -Minfo=accel,opt
else
	PARAM	+= -DOFFLOAD_BY_OPENMP_TARGET
	ifeq ($(USE_OMP_DISTRIBUTE), 1)
		PARAM	+= -DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
	endif
	ARGS_NVC	:= -mp=gpu -Minfo=accel,opt,mp
	ARGS_AMD	:= -fopenmp --offload-arch=$(GPU_ARCH)
	ARGS_ICPX	:= -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device $(GPU_ARCH)" # -Xs "-device pvc" is for Ahead-of-Time compilation
endif
ifeq ($(USE_FAST_MATH), 1)
# adding fastmath is required to achieve a reasonal performance (-Mfprelaxed=rsqrt only is not enough)
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
else
	ifeq ($(AMDCLANG), 1)
		CXX	:= amdclang++
		ARGS	+= $(ARGS_AMD)
	else
		ifeq ($(ICPX), 1)
			CXX	:= icpx
			ARGS	+= $(ARGS_ICPX)
		endif
	endif
endif
PARAM	+= -DCOMPILER="\"$(CXX)\"" -DMODEL_ID="($(MODEL_ID))"

ifeq ($(BENCHMARK), 1)
PARAM	+= -DBENCHMARK_MODE
ifneq ($(SET_NMIN), 0)
PARAM	+= -DNMIN="($(SET_NMIN))"
endif
ifneq ($(SET_NMAX), 0)
PARAM	+= -DNMAX="($(SET_NMAX))"
endif
ifneq ($(SET_NCRIT), 0)
PARAM	+= -DNCRIT="($(SET_NCRIT))"
endif
endif

INC	:= -I$(SOLOMON_DIR)
INC	+= -isystem=$(BOOST_INC)
# LIB	+= -L$(BOOST_LIB) #-lboost_filesystem

# debug mode
ifeq ($(DEBUG_MODE), 1)
	ARGS	+= -g
	# ARGS	+= -fsanitize=address -fsanitize=leak -fsanitize=undefined
endif

# directoties
BINDIR	:= bin
OBJDIR	:= $(BINDIR)/obj
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

# objects
OBJ	:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC) $(SRC_LIB)))
OBJ_ACC	:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC_ACC) $(SRC_LIB)))
OBJ_OMP	:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC_OMP) $(SRC_LIB)))

# executable
EXE	:= $(BINDIR)/nbody_pragma
ACC	:= $(BINDIR)/acc_pragma
OMP	:= $(BINDIR)/omp_pragma
all:	$(EXE) $(ACC) $(OMP)
default:	$(EXE)
acc:	$(ACC)
omp:	$(OMP)

# rules
$(EXE):	$(OBJ)
	$(CXX) $(STD) $(FLAG) $(ARGS) -o $@ $(OBJ) $(LIB)
$(ACC):	$(OBJ_ACC)
	$(CXX) $(STD) $(FLAG) $(ARGS) -o $@ $(OBJ_ACC) $(LIB)
$(OMP):	$(OBJ_OMP)
	$(CXX) $(STD) $(FLAG) $(ARGS) -o $@ $(OBJ_OMP) $(LIB)

$(OBJDIR)/%.o:	%.cpp
	$(CXX) $(STD) $(FLAG) $(ARGS) $(PARAM) $(INC) -o $@ -c $<

# make directoires
dir:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p dat fig log

# clean up
clean:
	rm -rf $(EXE) $(ACC) $(OMP) $(OBJDIR)/*.o

# dependency
$(OBJDIR)/allocate.o:	Makefile	$(SRCDIR)/allocate.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/common.hpp
$(OBJDIR)/gen_ic.o:	Makefile	$(SRCDIR)/gen_ic.hpp	$(SRCDIR)/type.hpp
$(OBJDIR)/gravity.o:	Makefile	$(SRCDIR)/gravity.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/common.hpp	$(SRCDIR)/benchmark.hpp
$(OBJDIR)/gravity_acc.o:	Makefile	$(SRCDIR)/gravity.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/common.hpp	$(SRCDIR)/benchmark.hpp
$(OBJDIR)/gravity_omp.o:	Makefile	$(SRCDIR)/gravity.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/common.hpp	$(SRCDIR)/benchmark.hpp
$(OBJDIR)/nbody.o:	Makefile	$(SRCDIR)/common.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/benchmark.hpp
$(OBJDIR)/nbody_acc.o:	Makefile	$(SRCDIR)/common.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/benchmark.hpp
$(OBJDIR)/nbody_omp.o:	Makefile	$(SRCDIR)/common.hpp	$(SRCDIR)/type.hpp	$(SRCDIR)/benchmark.hpp
