SOLOMON_HASH := \#
SOLOMON_DEF = $(shell mkdir -p spp && printf "$(SOLOMON_HASH)ifdef _OPENMP\n-D_OPENMP=_OPENMP\n$(SOLOMON_HASH)endif\n$(SOLOMON_HASH)ifdef _OPENACC\n-D_OPENACC=_OPENACC\n$(SOLOMON_HASH)endif\n" > spp/_solomon.F && $(SOLOMON_FC) $(SOLOMON_FLAGS) -E spp/_solomon.F 2>/dev/null | grep -- "-D_" && rm -f spp/_solomon.F)

SOLOMON_DEF += $(filter -D% -I%,$(SOLOMON_FC) $(SOLOMON_FLAGS))

spp/%.f90: %.f90
	mkdir -p spp
	@case "$(SOLOMON_DEF)" in (*-D_OPENACC*|*-D_OPENMP*) ;; (*) echo 'solomon: note: neither _OPENACC nor _OPENMP was detected from SOLOMON_FC and SOLOMON_FLAGS; all directives expand to serial code (add an offload flag such as -acc, -mp=gpu, or -fopenmp to SOLOMON_FLAGS if offloading was intended)' >&2 ;; esac
	cpp -P $< -DSOLOMON_FORTRAN $(SOLOMON_DEF) > spp/$*.i.f90
	sed 's/^#pragma /!$$/g' spp/$*.i.f90 > spp/$*.f90
	@rm -f spp/$*.i.f90
