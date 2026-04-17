module misc
  use iso_fortran_env, only: int64
  implicit none

  integer, parameter :: sp = selected_real_kind(6)  ! single precision, float.
  integer, parameter :: dp = selected_real_kind(15) ! double precision, double.
  integer, parameter :: qp = selected_real_kind(33) ! quadruple precision, quad.

  integer(int64) :: t_s

contains

  subroutine swap(f, fn)
    real(sp), pointer, intent(inout) :: f(:,:,:), fn(:,:,:)
    real(sp), pointer :: ftmp(:,:,:)

    ftmp => f
    f => fn
    fn => ftmp
  end subroutine swap

  subroutine start_timer()
    call system_clock(t_s)
  end subroutine start_timer

  double precision function get_elapsed_time()
    integer(int64) :: t_e, t_rate
    call system_clock(t_e, t_rate)
    get_elapsed_time = real(t_e - t_s, dp) / real(t_rate, dp)
  end function get_elapsed_time

end module misc
