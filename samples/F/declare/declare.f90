#include <solomon.hpp>

! usage of the declare directives: procedures called in offloaded regions and variables resident on the device
! The sum of n ones is computed on the device, then squared by a procedure called inside an offloaded region through a
! device-resident variable; the program prints the result and stops with code 1 unless it equals n * n.
! When offloading, it also checks that the host copy of the device-resident variable kept the value set on the host
! before the region: the device copy is a separate variable and is not copied back. Define SHARED_HOST_DEVICE_MEMORY
! to skip this check where the host and the device share the variable (e.g., unified memory or host fallback).

module resident_mod
  implicit none
  integer, parameter :: dp = selected_real_kind(15)
  ! a module variable that also lives on the device (OpenACC: declare create(resident), OpenMP target: declare target
  ! (resident))
  real(dp) :: resident(4)
  SOLOMON_DECLARE_ON_DEVICE(resident)
end module resident_mod

! in-procedure form: the directive in the specification part makes this procedure callable on the device (OpenACC:
! routine seq, OpenMP target: declare target); SOLOMON_DECLARE_OFFLOADED_END is not needed in Fortran
subroutine square(a)
  use resident_mod, only: dp
  implicit none
  real(dp), intent(inout) :: a
  SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_SEQUENTIAL)
  a = a * a
end subroutine square

program main
  use resident_mod
  implicit none
  interface
    subroutine square(a)
      import :: dp
      real(dp), intent(inout) :: a
    end subroutine square
  end interface
  ! named form: tell the caller that square is callable on the device (OpenACC: routine (square) seq, OpenMP target:
  ! declare target (square))
  SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(square), SOLOMON_CLAUSE_SEQUENTIAL)
  integer, parameter :: n = 1000
  real(dp), parameter :: sentinel = -1.0_dp
  integer :: i
  real(dp) :: x(n), s, expected

  x = 1.0_dp
  s = 0.0_dp
  SOLOMON_MALLOC_ON_DEVICE(x)
  SOLOMON_MEMCPY_H2D(x)

  ! sum on the device
  SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_REDUCTION(+:s))
  do i = 1, n
    s = s + x(i)
  end do
  SOLOMON_END_OFFLOAD

  ! set the host copy only: the device copy of resident is not updated by this
  resident = sentinel

  ! call the declared procedure on the device through the device-resident variable
  SOLOMON_OFFLOAD_SERIAL(SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(s))
  resident(1) = s
  call square(resident(1))
  s = resident(1)
  SOLOMON_END_OFFLOAD_SERIAL
  SOLOMON_SYNCHRONIZE()

  SOLOMON_FREE_FROM_DEVICE(x)

  expected = real(n, dp) * real(n, dp)
  print '(f9.1)', s
  if (s /= expected) then
    write(*, '("FAILED: ", f9.1, " (expected ", f9.1, ")")') s, expected
    stop 1
  end if
  ! checked only when solomon.hpp selected an offloading backend (the macros below stay defined only for the backend in use,
  ! whether -DSOLOMON_OFFLOAD_BY_* or -DOFFLOAD_BY_* was given)
#if (defined(OFFLOAD_BY_OPENACC) || defined(OFFLOAD_BY_OPENMP_TARGET)) && !defined(SHARED_HOST_DEVICE_MEMORY)
  if (resident(1) /= sentinel) then
    write(*, '("FAILED: the host copy of resident changed to ", f9.1, " (expected ", f9.1, ")")') resident(1), sentinel
    write(*, '("        the device copy is not separate: SOLOMON_DECLARE_ON_DEVICE(resident) is missing, or the")')
    write(*, '("        program ran on the host (host fallback) or with unified memory; define")')
    write(*, '("        SHARED_HOST_DEVICE_MEMORY for these")')
    stop 1
  end if
#endif
end program main
