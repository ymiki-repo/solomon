module misc
  implicit none

  integer, parameter :: sp = selected_real_kind(6)  ! single precision, float.
  integer, parameter :: dp = selected_real_kind(15) ! double precision, double.
  integer, parameter :: qp = selected_real_kind(33) ! quadruple precision, quad.

  real(dp) :: t_s

contains

  subroutine swap(f, fn)
    real(sp), pointer, intent(inout) :: f(:,:,:), fn(:,:,:)
    real(sp), pointer :: ftmp(:,:,:)

    ftmp => f
    f => fn
    fn => ftmp
  end subroutine swap

  subroutine start_timer()
    real(dp) :: omp_get_wtime
    t_s = omp_get_wtime()
  end subroutine start_timer

  double precision function get_elapsed_time()
    real(dp) :: omp_get_wtime
    get_elapsed_time = omp_get_wtime() - t_s
  end function get_elapsed_time

end module misc
