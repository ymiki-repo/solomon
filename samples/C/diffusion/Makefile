# the directory solomon/ of Solomon (padding/Makefile sets it before including this file); ?= takes the environment
# variable SOLOMON_DIR as well (a copy of this sample outside samples/C/ needs it, e.g. make SOLOMON_DIR=/path/to/solomon)
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
# GPU_ARCH	:= 120 # RTX PRO 4000 Blackwell
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

# OPT_LEVEL: 0 (no optimization flag), 1 (-O1), 2 (-O2), 3 (-O3), or 4 (-Ofast with nvc, -O3 -ffast-math otherwise)
OPT_LEVEL	:= 4
override OPT_LEVEL	:= $(strip $(OPT_LEVEL))
ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)
ifneq ($(filter-out 0 1 2 3 4,$(OPT_LEVEL))$(words $(OPT_LEVEL)),1)
$(error OPT_LEVEL takes 0, 1, 2, 3, or 4 (given: OPT_LEVEL=$(OPT_LEVEL)))
endif
endif

USE_MANAGED	:= 0
USE_UNIFIED	:= 0

APPLY_FIRST_TOUCH	:= 0

BENCHMARK	:= 0
MODEL_ID	:= 0

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

# memory mode of nvc: -gpu=mem:managed / mem:unified (-gpu=managed / unified for nvc that does not know mem:)
ifeq ($(NVHPC), 1)
ifneq ($(filter 1,$(USE_MANAGED) $(USE_UNIFIED)),)
	GPU_MEM	:= $(shell nvc -gpu=mem:managed -E -x c /dev/null > /dev/null 2>&1 && echo mem:)
endif
endif
ifeq ($(USE_MANAGED), 1)
	ARGS_NVC	+= -gpu=cc$(GPU_ARCH),$(GPU_MEM)managed
else
	ifeq ($(USE_UNIFIED), 1)
		ARGS_NVC	+= -gpu=cc$(GPU_ARCH),$(GPU_MEM)unified
		ifeq ($(APPLY_FIRST_TOUCH), 1)
			PARAM	+= -DAPPLY_FIRST_TOUCH
		endif
	else
		ARGS_NVC	+= -gpu=cc$(GPU_ARCH)
	endif
endif

ifeq ($(BENCHMARK), 1)
	PARAM	+= -DBENCHMARK_MODE
endif

INC	:= -I$(SOLOMON_DIR)
# lockf and fileno of main.c: on the command line as well, since the device pass of amdclang's OpenMP offloading does
# not take the #define at the top of main.c
PARAM	+= -D_XOPEN_SOURCE=700

CFLAGS	:= -Wall
ifeq ($(OPT_LEVEL), 1)
	CFLAGS	+= -O1
	PARAM	+= -DOPT_LEVEL="\"O1\""
else
	ifeq ($(OPT_LEVEL), 2)
		CFLAGS	+= -O2
		PARAM	+= -DOPT_LEVEL="\"O2\""
	else
		ifeq ($(OPT_LEVEL), 3)
			CFLAGS	+= -O3
			PARAM	+= -DOPT_LEVEL="\"O3\""
		else
			ifeq ($(OPT_LEVEL), 4)
				ifeq ($(NVHPC), 1)
					CFLAGS	+= -Ofast
					PARAM	+= -DOPT_LEVEL="\"Ofast\""
				else
					CFLAGS	+= -O3 -ffast-math
					PARAM	+= -DOPT_LEVEL="\"O3+ffast-math\""
				endif
			else
				PARAM	+= -DOPT_LEVEL="\"default\""
			endif
		endif
	endif
endif
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
LDLIBS	:= -lm
# the compiler name in the CSV file name (no directory, no option)
PARAM	+= -DCOMPILER="\"$(notdir $(firstword $(CC)))\"" -DMODEL_ID="($(MODEL_ID))"

SRCS	:= main.c diffusion.c misc.c
OBJS	:= $(SRCS:%.c=%.o)
# header dependencies written by the compiler (-MM -MP)
DEPENDENCIES	:= $(OBJS:.o=.d)
TARGET	:= run
# build settings of the last build: the objects and the executables are rebuilt when the settings (make variables) change
STAMP	:= settings.txt
SETTINGS	:= $(CC) $(CFLAGS) $(PARAM) $(INC) $(TARGET_ARCH) $(LINK_ARGS) $(LDFLAGS) $(LDLIBS)
MAKEFILE_DEPS	:= $(MAKEFILE_LIST)


.PHONY: all clean FORCE
all : $(TARGET)

$(TARGET) : $(OBJS) $(STAMP)
	$(CC) $(CFLAGS) $(TARGET_ARCH) $(OBJS) -o $@ $(LINK_ARGS) $(LDFLAGS) $(LDLIBS)

%.o : %.c $(STAMP) $(MAKEFILE_DEPS)
	$(CC) $(CFLAGS) $(PARAM) $(INC) $(TARGET_ARCH) -c $<
	$(CC) $(CFLAGS) $(PARAM) $(INC) $(TARGET_ARCH) -MM -MP -MT $@ -MF $(@:.o=.d) $<

$(STAMP) : FORCE
	@printf '%s\n' '$(subst ','\'',$(SETTINGS))' | cmp -s - $@ || printf '%s\n' '$(subst ','\'',$(SETTINGS))' > $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(DEPENDENCIES)
	$(RM) $(STAMP)
	$(RM) *~

-include $(DEPENDENCIES)
