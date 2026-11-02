# the directory solomon/ of Solomon; ?= takes the environment variable SOLOMON_DIR as well
# (a copy of this sample outside samples/F/ needs it, e.g. make SOLOMON_DIR=/path/to/solomon)
SOLOMON_DIR	?= ../../../solomon
SPECIAL_CHARS	:= ' " \ ( ) ; & | < > `
ifneq ($(word 2,$(SOLOMON_DIR))$(strip $(foreach c,$(SPECIAL_CHARS),$(findstring $(c),$(SOLOMON_DIR)))),)
$(error SOLOMON_DIR='$(SOLOMON_DIR)' holds a space, a quote, or one of \ ( ) ; & | < > and the backquote, which the compile commands cannot take: move Solomon to a path without them)
endif
ifeq ($(wildcard $(SOLOMON_DIR)/fortran.mk),)
$(error $(SOLOMON_DIR)/fortran.mk is not found: set SOLOMON_DIR to the directory solomon/ of Solomon, e.g. make SOLOMON_DIR=/path/to/solomon)
endif

USE_OPENACC	:= 0
ifeq ($(USE_OPENACC), 1)
	USE_ACC_PARALLEL	:= 1
else
	USE_OMP_DISTRIBUTE	:= 0
endif

# compiler: set one of AMDFLANG, IFX to 1 (NVHPC is 1 unless another one is selected);
# with all of them 0, $(FC) (gfortran unless given) builds a host binary without offloading
AMDFLANG	:= 0
IFX	:= 0
ifeq ($(filter 1,$(AMDFLANG) $(IFX)),)
	NVHPC	:= 1
else
	NVHPC	:= 0
endif

# target GPU (GPU_ARCH on the command line): NVIDIA defaults to 90; AMD and Intel GPUs have no default
# GPU_ARCH	:= 90 # NVIDIA H100
# GPU_ARCH	:= gfx90a # CDNA 2 GPUs (MI210, MI250, MI250X)
# GPU_ARCH	:= pvc # Intel PVC
ifneq ($(origin GPU_ARCH), command line)
ifeq ($(NVHPC), 1)
	GPU_ARCH	:= 90 # NVIDIA H100
else
	GPU_ARCH	:=
endif
endif
override GPU_ARCH	:= $(strip $(GPU_ARCH))

ifeq ($(origin FC), default)
	FC	:= gfortran
endif

# the settings are checked unless the only goal is clean
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
ifneq ($(filter-out 0 1,$(NVHPC) $(AMDFLANG) $(IFX)),)
$(error NVHPC, AMDFLANG, and IFX take 0 or 1 (given: NVHPC=$(NVHPC) AMDFLANG=$(AMDFLANG) IFX=$(IFX)))
endif
ifneq ($(word 2,$(filter 1,$(NVHPC) $(AMDFLANG) $(IFX))),)
$(error select at most one compiler (given: NVHPC=$(NVHPC) AMDFLANG=$(AMDFLANG) IFX=$(IFX)); e.g. make AMDFLANG=1 GPU_ARCH=gfx90a)
endif
ifeq ($(USE_OPENACC), 1)
ifneq ($(filter 1,$(AMDFLANG) $(IFX)),)
$(error USE_OPENACC=1 needs NVHPC=1 (amdflang and ifx do not compile OpenACC))
endif
endif
ifneq ($(filter 1,$(NVHPC) $(AMDFLANG) $(IFX)),)
ifeq ($(GPU_ARCH),)
$(error GPU_ARCH is not set: give the target GPU, e.g. make AMDFLANG=1 GPU_ARCH=gfx90a or make IFX=1 GPU_ARCH=pvc)
endif
else
$(info NVHPC=AMDFLANG=IFX=0: host build with $(FC), without offloading to GPU)
endif
endif

ifeq ($(USE_OPENACC), 1)
	PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENACC
	ifeq ($(USE_ACC_PARALLEL), 1)
		PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL
	endif
	ARGS_NV	:= -acc=gpu -Minfo=accel
else
	PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET
	ifeq ($(USE_OMP_DISTRIBUTE), 1)
		PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
	endif
	ARGS_NV	:= -mp=gpu -Minfo=accel,mp
	ARGS_AMD	:= -fopenmp --offload-arch=$(GPU_ARCH)
	ARGS_INTEL	:= -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device $(GPU_ARCH)"
endif
ARGS_NV	+= -gpu=cc$(GPU_ARCH)

INC	:= -I$(SOLOMON_DIR)

FFLAGS	:= -Wall -O2
ifeq ($(IFX), 1)
	FFLAGS	:= -warn all -O2
endif
ifeq ($(NVHPC), 1)
	FC	:= nvfortran
	FFLAGS	+= $(ARGS_NV)
else
	ifeq ($(AMDFLANG), 1)
		FC	:= amdflang
		FFLAGS	+= $(ARGS_AMD)
	else
		ifeq ($(IFX), 1)
			FC	:= ifx
			FFLAGS	+= $(ARGS_INTEL)
		endif
	endif
endif

SRCS	:= declare.f90
OBJS	:= $(SRCS:%.f90=%.o)
TARGET	:= run

SOLOMON_FC	= $(FC)
SOLOMON_FLAGS	= $(FFLAGS) $(PARAM) $(INC)

# settings of the last build: the sources are preprocessed and compiled again when the settings (make variables) change
STAMP	:= settings.txt
SETTINGS	:= $(FC) $(FFLAGS) $(PARAM) $(INC)
MAKEFILE_DEPS	:= $(MAKEFILE_LIST)
# the headers of Solomon, read by the preprocessing of every source
SOLOMON_HEADERS	:= $(shell find $(SOLOMON_DIR) -name '*.hpp')


.PHONY: all clean FORCE
all : $(TARGET)

.SUFFIXES:
.SUFFIXES: .f90 .o
.SECONDARY:

$(TARGET): $(OBJS)
	$(FC) $(FFLAGS) $(OBJS) -o $@

%.o: spp/%.f90 $(STAMP) $(MAKEFILE_DEPS)
	$(FC) -c $(FFLAGS) $< -o $@

$(SRCS:%=spp/%): $(STAMP) $(MAKEFILE_DEPS) $(SOLOMON_DIR)/fortran.mk $(SOLOMON_HEADERS)

$(STAMP): FORCE
	@printf '%s\n' '$(subst ','\'',$(SETTINGS))' | cmp -s - $@ || printf '%s\n' '$(subst ','\'',$(SETTINGS))' > $@

clean :
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -f *.mod
	rm -rf spp
	rm -f $(STAMP)
	rm -f *~

include $(SOLOMON_DIR)/fortran.mk
