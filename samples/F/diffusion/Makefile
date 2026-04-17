SOLOMON_DIR = ../../../solomon

USE_OPENACC	:= 0
ifeq ($(USE_OPENACC), 1)
	USE_ACC_PARALLEL	:= 1
else
	USE_OMP_DISTRIBUTE	:= 0
endif

# GPU_ARCH	:= 90 # NVIDIA H100
# GPU_ARCH	:= gfx90a # CDNA 2 GPUs (MI210, MI250, MI250X)
GPU_ARCH	:= pvc # Intel PVC

NVHPC	:= 0
AMDFLANG	:= 0
IFX	:= 1

# -O1, -O2, -O3, -Ofast, or default (w/o optimization flag)
OPT_LEVEL	:= 4

USE_MANAGED	:= 0
USE_UNIFIED	:= 0

APPLY_FIRST_TOUCH	:= 0

BENCHMARK	:= 0
MODEL_ID	:= 0

ifeq ($(USE_OPENACC), 1)
	PARAM	+= -DOFFLOAD_BY_OPENACC
	ifeq ($(USE_ACC_PARALLEL), 1)
		PARAM	+= -DOFFLOAD_BY_OPENACC_PARALLEL
	endif
	ARGS_NV	:= -acc=gpu -Minfo=accel,opt
else
	PARAM	+= -DOFFLOAD_BY_OPENMP_TARGET
	ifeq ($(USE_OMP_DISTRIBUTE), 1)
		PARAM	+= -DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
	endif
	ARGS_NV	:= -mp=gpu -Minfo=accel,opt,mp
	ARGS_AMD	:= -fopenmp --offload-arch=$(GPU_ARCH)
	ARGS_INTEL	:= -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device $(GPU_ARCH)"
endif

ifeq ($(USE_MANAGED), 1)
	ARGS_NV	+= -gpu=cc$(GPU_ARCH),managed
else
	ifeq ($(USE_UNIFIED), 1)
		ARGS_NV	+= -gpu=cc$(GPU_ARCH),unified
		ifeq ($(APPLY_FIRST_TOUCH), 1)
			PARAM	+= -DAPPLY_FIRST_TOUCH
		endif
	else
		ARGS_NV	+= -gpu=cc$(GPU_ARCH)
	endif
endif

ifeq ($(BENCHMARK), 1)
	PARAM	+= -DBENCHMARK_MODE
endif

INC	:= -I$(SOLOMON_DIR)

GCC  = gcc
RM  = rm -f
MAKEDEPEND = makedepend
GFLAGS    = -std=c99 -Wall -Wextra -Wunused -Wcast-qual -Wcast-align -Ofast

FFLAGS	:= -Wall
ifeq ($(OPT_LEVEL), 1)
	FFLAGS	+= -O1
	PARAM	+= -DOPT_LEVEL="\"O1\""
else
	ifeq ($(OPT_LEVEL), 2)
		FFLAGS	+= -O2
		PARAM	+= -DOPT_LEVEL="\"O2\""
	else
		ifeq ($(OPT_LEVEL), 3)
			FFLAGS	+= -O3
			PARAM	+= -DOPT_LEVEL="\"O3\""
		else
			ifeq ($(OPT_LEVEL), 4)
				FFLAGS	+= -Ofast
				PARAM	+= -DOPT_LEVEL="\"Ofast\""
			else
				PARAM	+= -DOPT_LEVEL="\"default\""
			endif
		endif
	endif
endif
ifeq ($(NVHPC), 1)
	FC   := nvfortran
	FFLAGS	+= $(ARGS_NV)
	CC   := nvc -std=c99
	CXX	:= nvc++
else
	ifeq ($(AMDFLANG), 1)
		FC	:= amdflang
		FFLAGS	+= $(ARGS_AMD)
		CC	:= amdclang -std=c99
		CXX	:= amdclang++
	else
		ifeq ($(IFX), 1)
			FC	:= ifx
			FFLAGS	+= $(ARGS_INTEL)
			CC	:= icx -std=c99
			CXX	:= icpx
		endif
	endif
endif
LDFLAGS   =
PARAM	+= -DCOMPILER="\"$(FC)\"" -DMODEL_ID="($(MODEL_ID))"
ifeq ($(AMDFLANG), 1)
	LDFLAGS	+= -lm
endif


SRCS    = main.f90 diffusion.f90 misc.f90
TARGET = run
DISTTARGET = $(TARGET)_1.0.0

OBJS += $(filter %.o,$(SRCS:%.f90=%.o))
# OBJS += $(filter %.o,$(SRCS:%.cc=%.o))
# OBJS += $(filter %.o,$(SRCS:%.cpp=%.o))

SOLOMON_FC    = $(FC)
SOLOMON_FLAGS = $(FFLAGS) $(PARAM) $(INC)

DEPENDENCIES = $(subst .o,.d,$(OBJS))


.PHONY: all
all : $(TARGET)

.SUFFIXES:
.SUFFIXES: .f90 .cpp .cc .c .o
.SECONDARY:

$(TARGET): $(OBJS)
	$(FC) $(FFLAGS) $(OBJS) $(LIBS) -o $@

%.o: %.c
	$(CC)  -c $(FFLAGS) $(SOLMON_ARGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(FFLAGS) $(SOLMON_ARGS) $< -o $@

%.o: spp/%.f90
	$(FC) -c $(FFLAGS) $< -o $@

.PHONY: dist
dist :
	mkdir -p $(DISTTARGET)
	@for h in `makedepend -Y -f- -- $(FFLAGS) -- $(SRCS) | grep -e ":" | sed -e "s/.*: //" | tr " " "\n" | sort | uniq` ; \
	do \
		cp -p $$h $(DISTTARGET); \
	done
	cp -p $(SRCS) $(DISTTARGET)
	cp -p Makefile $(DISTTARGET)
	tar -acvf $(DISTTARGET).tar.gz $(DISTTARGET)
	rm -rf $(DISTTARGET)


.PHONY: clean
clean :
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -f $(DEPENDENCIES)
	rm -f *.mod
	rm -f *.modmic
	rm -f spp/*
	rm -f *~

main.o: diffusion.o misc.o
diffusion.o: misc.o
misc.o:

include $(SOLOMON_DIR)/fortran.mk
