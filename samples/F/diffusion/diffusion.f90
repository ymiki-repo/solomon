#include <solomon.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338328_dp
#endif

module diffusion
  USE_SOLOMON_RUNTIME
  use misc
  implicit none

contains

  function diffusion3d(nx, ny, nz, dx, dy, dz, dt, kappa, f, fn)
    integer,  intent(in)  :: nx, ny, nz
    real(sp), intent(in)  :: dx, dy, dz, dt, kappa
    real(sp), intent(in)  :: f(nx,ny,nz)
    real(sp), intent(out) :: fn(nx,ny,nz)
    real(sp) :: ce,cw,cn,cs,ct,cb,cc
    integer :: i,j,k,ip,im,jp,jm,kp,km
    integer :: w,e,n,s,b,t
    real(dp) :: diffusion3d

    ce = kappa*dt/(dx*dx)
    cw = ce
    cn = kappa*dt/(dy*dy)
    cs = cn
    ct = kappa*dt/(dz*dz)
    cb = ct

    cc = 1.0 - (ce + cw + cn + cs + ct + cb)

    SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_COLLAPSE(3), ACC_CLAUSE_PRESENT(f, fn), \
            SOLOMON_CLAUSE_PRIVATE(i,j,k,ip,im,jp,jm,kp,km,w,e,n,s,b,t))
    do k=1, nz
       do j=1, ny
          do i=1, nx
             ip = MERGE(i+1,i,i+1<=nx)
             im = MERGE(i-1,i,i-1>= 1)
             jp = MERGE(j+1,j,j+1<=ny)
             jm = MERGE(j-1,j,j-1>= 1)
             kp = MERGE(k+1,k,k+1<=nz)
             km = MERGE(k-1,k,k-1>= 1)

             fn(i,j,k) = cc * f(i,j,k) &
                  + ce * f(ip,j,k) + cw * f(im,j,k) &
                  + cn * f(i,jp,k) + cs * f(i,jm,k) &
                  + ct * f(i,j,kp) + cb * f(i,j,km)
          end do ! i
       end do ! j
    end do ! k
    SOLOMON_END_OFFLOAD

    diffusion3d = real(nx*ny*nz,dp)*13.0_dp
  end function diffusion3d


  subroutine init(nx, ny, nz, dx, dy, dz, f)
    integer,  intent(in)  :: nx, ny, nz
    real(sp), intent(in)  :: dx, dy, dz
    real(sp), intent(out) :: f(nx,ny,nz)
    real(sp) :: kx,ky,kz,x,y,z
    integer  :: i,j,k

    kx = 2.0_sp * real(M_PI,sp)
    ky = kx
    kz = kx

    do k=1, nz
       do j = 1, ny
          do i = 1, nx
             x = dx*(real(i-1,sp) + 0.5_sp)
             y = dy*(real(j-1,sp) + 0.5_sp)
             z = dz*(real(k-1,sp) + 0.5_sp)

             f(i,j,k) = 0.125_sp &
                  * (1.0_sp - cos(kx*x)) &
                  * (1.0_sp - cos(ky*y)) &
                  * (1.0_sp - cos(kz*z))
          end do ! i
       end do ! j
    end do ! k
  end subroutine init


  function accuracy(time, nx, ny, nz, dx, dy, dz, kappa, f)
    real(dp), intent(in)  :: time
    integer,  intent(in)  :: nx, ny, nz
    real(sp), intent(in)  :: dx, dy, dz, kappa
    real(sp), intent(in)  :: f(nx,ny,nz)
    real(sp) :: kx,ky,kz,ax,ay,az,x,y,z,f0
    real(dp) :: ferr
    integer  :: i,j,k
    real(dp) :: accuracy

    kx = 2.0_sp*real(M_PI,sp)
    ky = kx
    kz = kx

    ax = exp(-kappa*real(time,sp)*(kx*kx))
    ay = exp(-kappa*real(time,sp)*(ky*ky))
    az = exp(-kappa*real(time,sp)*(kz*kz))

    ferr = 0.0_dp

    do k=1, nz
       do j=1, ny
          do i=1, nx
             x = dx*(real(i-1,sp) + 0.5_sp)
             y = dy*(real(j-1,sp) + 0.5_sp)
             z = dz*(real(k-1,sp) + 0.5_sp)

             f0 = 0.125_sp &
                  * (1.0_sp - ax*cos(kx*x)) &
                  * (1.0_sp - ay*cos(ky*y)) &
                  * (1.0_sp - az*cos(kz*z))

             ferr = ferr + real(f(i,j,k) - f0,dp)**2
          end do ! i
       end do ! j
    end do ! k

    accuracy = sqrt(ferr/real(nx*ny*nz,dp))
  end function accuracy

end module diffusion
