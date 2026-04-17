SOLOMON_DIR := ../../../solomon

USE_OPENACC	:= 0
ifeq ($(USE_OPENACC), 1)
	USE_ACC_PARALLEL	:= 1
else
	USE_OMP_DISTRIBUTE	:= 0
endif

GPU_ARCH	:= 90 # NVIDIA H100
# GPU_ARCH	:= gfx90a # CDNA 2 GPUs (MI210, MI250, MI250X)
# GPU_ARCH	:= pvc # Intel PVC
GPU_ARCH	:= $(strip $(GPU_ARCH))

NVHPC	:= 1
AMDCLANG	:= 0
ICPX	:= 0

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
	ARGS_NVC	:= -acc=gpu -Minfo=accel,opt
else
	PARAM	+= -DOFFLOAD_BY_OPENMP_TARGET
	ifeq ($(USE_OMP_DISTRIBUTE), 1)
		PARAM	+= -DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
	endif
	ARGS_NVC	:= -mp=gpu -Minfo=accel,opt,mp
	ARGS_AMD	:= -fopenmp --offload-arch=$(GPU_ARCH)
	ARGS_ICPX	:= -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device $(GPU_ARCH)"
endif

ifeq ($(USE_MANAGED), 1)
	ARGS_NVC	+= -gpu=cc$(GPU_ARCH),managed
else
	ifeq ($(USE_UNIFIED), 1)
		ARGS_NVC	+= -gpu=cc$(GPU_ARCH),unified
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

GCC  = gcc
RM  = rm -f
MAKEDEPEND = makedepend
GFLAGS    = -std=c99 -Wall -Wextra -Wunused -Wcast-qual -Wcast-align -O3 -ffast-math

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
	CC   := nvc -std=c99
	CXX  := nvc++
	CFLAGS	+= $(ARGS_NVC)
else
	ifeq ($(AMDCLANG), 1)
		CC	:= amdclang -std=c99
		CXX	:= amdclang++
		CFLAGS	+= $(ARGS_AMD)
	else
		ifeq ($(ICPX), 1)
			CC	:= icx -std=c99
			CXX	:= icpx
			CFLAGS	+= $(ARGS_ICPX)
		endif
	endif
endif
CXXFLAGS  = $(CFLAGS)
LDFLAGS   =
PARAM	+= -DCOMPILER="\"$(CXX)\"" -DMODEL_ID="($(MODEL_ID))"
ifeq ($(AMDCLANG), 1)
	LDFLAGS	+= -lm
endif

SRCS    = main.c diffusion.c misc.c
TARGET = run
DISTTARGET = $(TARGET)_1.0.0

OBJS += $(filter %.o,$(SRCS:%.c=%.o))
OBJS += $(filter %.o,$(SRCS:%.cc=%.o))
OBJS += $(filter %.o,$(SRCS:%.cpp=%.o))


DEPENDENCIES = $(subst .o,.d,$(OBJS))


.PHONY: all
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(TARGET_ARCH) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CC) $(CFLAGS) $(PARAM) $(INC) $(TARGET_ARCH)-c $<

%.o : %.cc
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CXX) $(CXXFLAGS) $(PARAM) $(INC) $(TARGET_ARCH) -c $<

%.o : %.cpp
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CXX) $(CXXFLAGS) $(PARAM) $(INC) $(TARGET_ARCH) -c $<

.PHONY: dist
dist :
	mkdir -p $(DISTTARGET)
	@for h in `makedepend -Y -f- -- $(CXXFLAGS) -- $(SRCS) | grep -e ":" | sed -e "s/.*: //" | tr " " "\n" | sort | uniq` ; \
	do \
		cp -p $$h $(DISTTARGET); \
	done
	cp -p $(SRCS) $(DISTTARGET)
	cp -p Makefile $(DISTTARGET)
	tar -zcvf $(DISTTARGET).tar.gz $(DISTTARGET)
	rm -rf $(DISTTARGET)


.PHONY: clean
clean :
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(DEPENDENCIES)
	$(RM) *~



ifneq "$(MAKECMDGOALS)" "clean"
  -include $(DEPENDENCIES)
endif

# $(call make-depend,source-file,object-file,depend-file)
define make-depend
  @$(GCC) -MM            \
          -MF $3         \
          -MP            \
          -MT $2         \
          $(GFLAGS)      \
          $(TARGET_ARCH) \
          $1
endef


