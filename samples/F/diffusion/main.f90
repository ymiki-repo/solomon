#include <solomon.hpp>

program main
  USE_SOLOMON_RUNTIME
  use iso_fortran_env, only: int64, error_unit
  use diffusion
  use misc
  implicit none

  real(dp), parameter :: byte_per_flop = real(storage_size(0.0_sp) / 8, dp) * 8.0_dp / 13.0_dp
  integer  :: nx, ny, nz
#if defined(APPLY_FIRST_TOUCH)
  integer  :: i, j, k
#endif
  real(sp) :: lx, ly, lz, dx, dy, dz, dt, kappa
  integer  :: nt, icnt
  real(dp) :: time, flop, elapsed_time, ferr
  real(sp), allocatable, target :: buf0(:,:,:), buf1(:,:,:)
  real(sp), pointer :: f(:,:,:), fn(:,:,:)
  character(len=1024) :: arg, filename
  logical :: ex
  integer :: fd, iostat

  ! the messages and the exit code are those of C/diffusion/main.c
  if( command_argument_count() /= 1 ) then
    write(error_unit,'("ERROR: ",i0," input parameters are given (",i0," is required)")') command_argument_count(), 1
    call usage()
    stop 1
  end if

  call get_command_argument(1,arg)
  ! N: digits only, 3 <= N, and N^3 <= huge(N) (the grid points are counted by the default integer);
  ! N^3 itself can overflow int64 (N >= 2^21), so N^2 is compared with huge(N) / N instead
  ! (max: Fortran may evaluate both operands of .or., and N is 0 for an invalid input)
  nx = 0
  if( len_trim(arg) >= 1 .and. len_trim(arg) <= 9 .and. verify(trim(arg), "0123456789") == 0 ) then
    read(arg,*,iostat=iostat) nx
    if( iostat /= 0 ) nx = 0
  end if
  if( nx < 3 .or. int(nx,int64)**2 > int(huge(nx),int64) / max(int(nx,int64),1_int64) ) then
    write(error_unit,'("ERROR: invalid input parameter: N = ",3a)') '"', trim(arg), '"'
    call usage()
    stop 1
  end if
  ny = nx
  nz = nx

  lx = 1.0_sp
  ly = 1.0_sp
  lz = 1.0_sp

  dx = lx/real(nx,sp)
  dy = ly/real(ny,sp)
  dz = lz/real(nz,sp)

  kappa = 0.1_sp
  dt = 0.1_sp * min(min(dx * dx, dy * dy), dz * dz) / kappa

  nt = 100000
  time = 0.0_dp
  flop = 0.0_dp
  elapsed_time = 0.0_dp

  allocate( buf0(nx,ny,nz), buf1(nx,ny,nz) )
  ! the buffers are the mapped variables; f and fn, swapped every step, never appear in a data clause
  f => buf0
  fn => buf1

#if defined(APPLY_FIRST_TOUCH)
  !! first touch
  SOLOMON_OFFLOAD(SOLOMON_CLAUSE_PRIVATE(i,j,k))
  do k=1, nz
    do j=1, ny
      do i=1, nx
        f (i,j,k) = 0.0_sp
        fn(i,j,k) = 0.0_sp
      end do ! i
    end do ! j
  end do ! k
  SOLOMON_END_OFFLOAD
#endif /* defined(APPLY_FIRST_TOUCH) */

  call init(nx, ny, nz, dx, dy, dz, f)

  PRAGMA_ACC_DATA(ACC_CLAUSE_COPY(buf0(1:nx,1:ny,1:nz)), ACC_CLAUSE_CREATE(buf1(1:nx,1:ny,1:nz)))
  call start_timer()

  do icnt=0, nt-1
#if !defined(BENCHMARK_MODE)
    if(mod(icnt,100) == 0) write (*,'("time(",i5,") = ",f7.5)') icnt, time
#endif /* !defined(BENCHMARK_MODE) */

    flop = flop + diffusion3d(nx, ny, nz, dx, dy, dz, dt, kappa, f, fn)

    call swap(f, fn)

    time = time + dt
    if(time + 0.5*dt >= 0.1) exit
  end do

  elapsed_time = get_elapsed_time()

  ! buf0 is copied back at the end of the region; buf1 is not: copy it back so that f refers to
  ! up-to-date data whichever buffer it is associated with after the swaps
  SOLOMON_MEMCPY_D2H(buf1(1:nx,1:ny,1:nz))
  PRAGMA_ACC_END_DATA

#if !defined(BENCHMARK_MODE)
  write(*,'("Time = ",f12.6," [sec]")')           elapsed_time
  write(*,'("Performance = ",f12.6," [GFlops]")') flop/elapsed_time*1.0e-09_dp
  write(*,'("Bandwidth   = ",f12.6," [GB/s]")')    byte_per_flop * flop / elapsed_time * 1.0e-09_dp
#endif /* !defined(BENCHMARK_MODE) */

  ferr = accuracy(time, nx, ny, nz, dx, dy, dz, kappa, f)
#if !defined(BENCHMARK_MODE)
  write(*, '("Error[",i0,"][",i0,"][",i0,"] = ",e13.6)') nx, ny, nz, ferr
#endif /* !defined(BENCHMARK_MODE) */

  write(filename, '(a,i0,a)') "diffusion_" // "benchmark_" // COMPILER, MODEL_ID, ".csv"

  ! every run appends one row; the header goes to a new file
  inquire(file=filename,exist=ex)
  open(newunit=fd,file=filename,status="unknown",action="write",position="append",iostat=iostat)
  if( iostat/=0 ) then
    write(*,'("ERROR: failed to open ",a)') trim(filename)
    stop 1
  end if
  if( .not. ex ) then
    write(fd,'("Model_ID,Optimization_level")',advance='no')
    write(fd,'(",nx,ny,nz")',advance='no')
    write(fd,'(",time[s],Flops,Flop/s,Bytes,B/s,error")')
  end if

  write(fd,'(i0,",",a)',advance='no') MODEL_ID, OPT_LEVEL
  write(fd,'(",",i0)',advance='no') nx
  write(fd,'(",",i0)',advance='no') ny
  write(fd,'(",",i0)',advance='no') nz
  write(fd,'(",",f12.6,",",e13.6,",",e13.6,",",e13.6,",",e13.6,",",e13.6)') &
      elapsed_time, flop, flop/elapsed_time, byte_per_flop*flop, &
      byte_per_flop*flop/elapsed_time, ferr

  close(fd)

  deallocate( buf0, buf1 )

contains

  ! usage: print the usage of this program (as usage() of C/diffusion/main.c)
  subroutine usage()
    character(len=1024) :: exe
    call get_command_argument(0,exe)
    write(error_unit,'("Usage is: ",a," N")') trim(exe)
    write(error_unit,'(a,"N: number of grid points <int>, 3 <= N and N^3 <= ",i0," (N <= 1290)")') achar(9), huge(0)
  end subroutine usage

end program main
