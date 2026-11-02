# the directory solomon/ of Solomon; ?= takes the environment variable SOLOMON_DIR as well
# (a copy of this sample outside samples/C/ needs it, e.g. make SOLOMON_DIR=/path/to/solomon)
SOLOMON_DIR	?= ../../../solomon
# SOLOMON_DIR is checked unless the only goal is clean
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
SPECIAL_CHARS	:= ' " \ ( ) ; & | < > `
ifneq ($(word 2,$(SOLOMON_DIR))$(strip $(foreach c,$(SPECIAL_CHARS),$(findstring $(c),$(SOLOMON_DIR)))),)
$(error SOLOMON_DIR='$(SOLOMON_DIR)' holds a space, a quote, or one of \ ( ) ; & | < > and the backquote, which the compile commands cannot take: move Solomon to a path without them)
endif
ifeq ($(wildcard $(SOLOMON_DIR)/solomon.hpp),)
$(error $(SOLOMON_DIR)/solomon.hpp is not found: set SOLOMON_DIR to the directory solomon/ of Solomon, e.g. make SOLOMON_DIR=/path/to/solomon)
endif
endif

USE_OPENACC	:= 0
ifeq ($(USE_OPENACC), 1)
	USE_ACC_PARALLEL	:= 1
else
	USE_OMP_DISTRIBUTE	:= 0
endif

# compiler: set one of AMDCLANG, ICPX to 1 (NVHPC is 1 unless another one is selected);
# with all of them 0, $(CC) builds a host binary without offloading. ACPP (AdaptiveCpp) is not supported yet
AMDCLANG	:= 0
ICPX	:= 0
ACPP	:= 0
ifeq ($(filter 1,$(AMDCLANG) $(ICPX) $(ACPP)),)
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
$(error USE_OPENACC=1 needs NVHPC=1 (amdclang and icx do not compile OpenACC))
endif
endif
ifneq ($(filter 1,$(NVHPC) $(AMDCLANG) $(ICPX)),)
ifeq ($(GPU_ARCH),)
$(error GPU_ARCH is not set: give the target GPU, e.g. make AMDCLANG=1 GPU_ARCH=gfx90a or make ICPX=1 GPU_ARCH=pvc)
endif
else
$(info NVHPC=AMDCLANG=ICPX=0: host build with $(CC), without offloading to GPU)
endif
endif

ifeq ($(USE_OPENACC), 1)
	PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENACC
	ifeq ($(USE_ACC_PARALLEL), 1)
		PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL
	endif
	ARGS_NVC	:= -acc=gpu -Minfo=accel
else
	PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET
	ifeq ($(USE_OMP_DISTRIBUTE), 1)
		PARAM	+= -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
	endif
	ARGS_NVC	:= -mp=gpu -Minfo=accel,mp
	ARGS_AMD	:= -fopenmp --offload-arch=$(GPU_ARCH)
	ARGS_ICPX	:= -fiopenmp -fopenmp-targets=spir64_gen
	# -Xs "-device pvc" is for Ahead-of-Time compilation (used at the link)
	LINK_ICPX	:= -Xs "-device $(GPU_ARCH)"
endif
ARGS_NVC	+= -gpu=cc$(GPU_ARCH)

INC	:= -I$(SOLOMON_DIR)

CFLAGS	:= -Wall -O2
ifeq ($(NVHPC), 1)
	CC	:= nvc -std=c99
	CFLAGS	+= $(ARGS_NVC)
else
	ifeq ($(AMDCLANG), 1)
		CC	:= amdclang -std=c99
		CFLAGS	+= $(ARGS_AMD)
	else
		ifeq ($(ICPX), 1)
			CC	:= icx -std=c99
			CFLAGS	+= $(ARGS_ICPX)
			LINK_ARGS	:= $(LINK_ICPX)
		endif
	endif
endif
LDFLAGS	:=

SRCS	:= declare.c
OBJS	:= $(SRCS:%.c=%.o)
# header dependencies written by the compiler (-MM -MP)
DEPENDENCIES	:= $(OBJS:.o=.d)
TARGET	:= run
# build settings of the last build: the objects and the executables are rebuilt when the settings (make variables) change
STAMP	:= settings.txt
SETTINGS	:= $(CC) $(CFLAGS) $(PARAM) $(INC) $(LINK_ARGS) $(LDFLAGS)
MAKEFILE_DEPS	:= $(MAKEFILE_LIST)


.PHONY: all clean FORCE
all : $(TARGET)

$(TARGET) : $(OBJS) $(STAMP)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LINK_ARGS) $(LDFLAGS)

%.o : %.c $(STAMP) $(MAKEFILE_DEPS)
	$(CC) $(CFLAGS) $(PARAM) $(INC) -c $<
	$(CC) $(CFLAGS) $(PARAM) $(INC) -MM -MP -MT $@ -MF $(@:.o=.d) $<

$(STAMP) : FORCE
	@printf '%s\n' '$(subst ','\'',$(SETTINGS))' | cmp -s - $@ || printf '%s\n' '$(subst ','\'',$(SETTINGS))' > $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(DEPENDENCIES)
	$(RM) $(STAMP)
	$(RM) *~

-include $(DEPENDENCIES)
