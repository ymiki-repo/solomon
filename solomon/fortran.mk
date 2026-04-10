SOLOMON_DEF = $(shell mkdir -p spp && printf "\
\#ifdef _OPENMP\n\
	-D_OPENMP=_OPENMP\n\
\#endif\n\
\#ifdef _OPENACC\n\
	-D_OPENACC=_OPENACC\n\
\#endif\n" > spp/_solomon.F && \
$(SOLOMON_FC) $(SOLOMON_FLAGS) -E spp/_solomon.F 2>/dev/null | grep -- "-D_" && \
rm -f spp/_solomon.F)

SOLOMON_DEF += $(filter -D% -I%,$(SOLOMON_FC) $(SOLOMON_FLAGS))

spp/%.f90: %.f90
	mkdir -p spp
	cpp -P $< -DSOLOMON_FORTRAN $(SOLOMON_DEF) > spp/$*.i.f90
	sed 's/^#pragma /!$$/g' spp/$*.i.f90 > spp/$*.f90
	@rm -f spp/$*.i.f90
