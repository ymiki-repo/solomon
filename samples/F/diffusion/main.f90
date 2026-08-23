#include <solomon.hpp>

program main
  USE_SOLOMON_RUNTIME
  use diffusion
  use misc
  implicit none

  real(dp), parameter :: byte_per_flop = real(storage_size(0.0_sp) / 8, dp) * 8.0_dp / 13.0_dp
  integer  :: nx, ny, nz, n
#if defined(APPLY_FIRST_TOUCH)
  integer  :: i, j, k
#endif
  real(sp) :: lx, ly, lz, dx, dy, dz, dt, kappa
  integer  :: nt, icnt
  real(dp) :: time, flop, elapsed_time, ferr
  real(sp), pointer :: f(:,:,:), fn(:,:,:)
  character(len=1024) :: arg, filename
  logical :: ex
  integer :: fd, iostat

  if( command_argument_count() < 1 ) then
     write(*,'("ERROR: insufficient number of input parameters: ", i1)') &
          command_argument_count()
     write(*,'(" (at least ",i1," inputs are required)")') 1

     call get_command_argument(0,arg)
     write(*,'("Usage is: ",a," N")') trim(arg)
     write(*,'("    N: number of grid points <integer>")')
     stop
  end if

  call get_command_argument(1,arg)
  read(arg,*) nx
  ny = nx
  nz = nx
  n  = nx * ny * nz

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

  allocate( f(nx,ny,nz), fn(nx,ny,nz) )

#if defined(APPLY_FIRST_TOUCH)
  !! first touch
  OFFLOAD(AS_PRIVATE(i,j,k))
  do k=1, nz
     do j=1, ny
        do i=1, nx
           f (i,j,k) = 0.0_sp
           fn(i,j,k) = 0.0_sp
        end do ! i
     end do ! j
  end do ! k
  END_OFFLOAD
#endif // !defined(APPLY_FIRST_TOUCH)

  call init(nx, ny, nz, dx, dy, dz, f)

  PRAGMA_ACC_DATA(ACC_CLAUSE_COPY(f(1:nx,1:ny,1:nz)), ACC_CLAUSE_CREATE(fn(1:nx,1:ny,1:nz)))
  call start_timer()

  do icnt=0, nt-1
#if !defined(BENCHMARK_MODE)
     if(mod(icnt,100) == 0) write (*,'("time(",i5,") = ",f7.5)') icnt, time
#endif // !defined(BENCHMARK_MODE)

     flop = flop + diffusion3d(nx, ny, nz, dx, dy, dz, dt, kappa, f, fn)

     call swap(f, fn)

     time = time + dt
     if(time + 0.5*dt >= 0.1) exit
  end do

  elapsed_time = get_elapsed_time()
  PRAGMA_ACC_END_DATA

#if !defined(BENCHMARK_MODE)
  write(*,'("Time = ",f12.6," [sec]")')           elapsed_time
  write(*,'("Performance = ",f12.6," [GFlops]")') flop/elapsed_time*1.0e-09_dp
  write(*,'("Bandwidth   = ",f12.6," [GB/s]")')    byte_per_flop * flop / elapsed_time * 1.0e-09_dp
#endif // !defined(BENCHMARK_MODE)

  ferr = accuracy(time, nx, ny, nz, dx, dy, dz, kappa, f)
#if !defined(BENCHMARK_MODE)
  write(*, '("Error[",i3,"][",i3,"][",i3,"] = ",e13.6)') nx, ny, nz, ferr
#endif // !defined(BENCHMARK_MODE)

  write(filename, '(a,i0,a)') "diffusion_" // "benchmark_" // COMPILER, MODEL_ID, ".csv"

  inquire(file=filename,exist=ex)
  if( ex ) then
     open(newunit=fd,file=filename,status="unknown",action="write",position="append",iostat=iostat)
     if( iostat/=0 ) then
        write(*,'("ERROR: failed to open ",a)') trim(filename)
        stop
     end if
  else
     open(newunit=fd,file=filename,status="unknown",action="write",iostat=iostat)
     if( iostat/=0 ) then
        write(*,'("ERROR: failed to open ",a)') trim(filename)
        stop
     end if
     write(fd,'("Model_ID,Optimization_level")',advance='no')
     write(fd,'(",nx,ny,nz")',advance='no')
     write(fd,'(",time[s],Flops,Flop/s,Bytes,B/s,error")')
  end if

  write(fd,'(i0,",",a)',advance='no') MODEL_ID, OPT_LEVEL
  write(fd,'(",",i3)',advance='no') nx
  write(fd,'(",",i3)',advance='no') ny
  write(fd,'(",",i3)',advance='no') nz
  write(fd,'(",",f12.6,",",e13.6,",",e13.6,",",e13.6,",",e13.6,",",e13.6)') &
       elapsed_time, flop, flop/elapsed_time, byte_per_flop*flop, &
       byte_per_flop*flop/elapsed_time, ferr

  close(fd)

  deallocate( f, fn )
end program main
