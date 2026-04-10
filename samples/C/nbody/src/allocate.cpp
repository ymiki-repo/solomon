///
/// @file allocate.cpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief memory allocation
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#include "allocate.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <type_traits>

#include "common.hpp"
#include "type.hpp"

static inline size_t round_up(const size_t org, const size_t unit) {
  const size_t mod = org % unit;
  return ((mod == 0) ? org : (org + unit - mod));
}

void allocate_Nbody_particles(position **pos, velocity **vel, velocity **vel_tmp, acceleration **acc, const int num) {
  size_t size = round_up(num, NTHREADS);

  *pos = new std::remove_reference_t<decltype(**pos)>[size];
  *vel = new std::remove_reference_t<decltype(**vel)>[size];
  *vel_tmp = new std::remove_reference_t<decltype(**vel_tmp)>[size];
  *acc = new std::remove_reference_t<decltype(**acc)>[size];

  // zero-clear arrays (for safety of massless particles)
  constexpr std::remove_reference_t<decltype(**pos)> pos_zero = {0.0, 0.0, 0.0, 0.0};
  constexpr std::remove_reference_t<decltype(**vel)> vel_zero = {0.0, 0.0, 0.0};
  constexpr std::remove_reference_t<decltype(**acc)> acc_zero = {0.0, 0.0, 0.0, 0.0};
#pragma omp parallel for
  for (std::remove_const_t<decltype(size)> ii = 0; ii < size; ii++) {
    (*pos)[ii] = pos_zero;
    (*vel)[ii] = vel_zero;
    (*vel_tmp)[ii] = vel_zero;
    (*acc)[ii] = acc_zero;
  }
}

void release_Nbody_particles(position *pos, velocity *vel, velocity *vel_tmp, acceleration *acc) {
  delete[] pos;
  delete[] vel;
  delete[] vel_tmp;
  delete[] acc;
}
