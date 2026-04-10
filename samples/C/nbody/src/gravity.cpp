///
/// @file gravity.cpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief calculating gravitational interation (implementation in generic style)
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#include "gravity.hpp"

#include <cmath>
#include <solomon.hpp>
#include <type_traits>

#include "common.hpp"
#include "type.hpp"

void calc_acc(const int Ni, position *ipos, acceleration *iacc, const int Nj, position *jpos, const float eps) {
  OFFLOAD(NUM_THREADS(NTHREADS), AS_INDEPENDENT)
  for (std::remove_const_t<decltype(Ni)> ii = 0; ii < Ni; ii++) {
    // initialization
    position pi = ipos[ii];
    pi.w = eps * eps;  // set squared softening (Plummer softening)
    acceleration ai = {0.0F, 0.0F, 0.0F, 0.0F};
    // force evaluation
    PRAGMA_ACC_LOOP(ACC_CLAUSE_SEQ)
    for (std::remove_const_t<decltype(Nj)> jj = 0; jj < Nj; jj++) {
      // load j-particle
      const position pj = jpos[jj];

      // calculate particle-particle interaction
      position rji;
      rji.x = pj.x - pi.x;
      rji.y = pj.y - pi.y;
      rji.z = pj.z - pi.z;
      const auto r2 = std::fma(rji.z, rji.z, std::fma(rji.y, rji.y, std::fma(rji.x, rji.x, pi.w)));
      rji.w = 1.0F / std::sqrt(r2);  // r^-1
      rji.w *= rji.w * rji.w;        // r^-3
      rji.w *= pj.w;                 // m_j r^-3

      // force accumulation
      ai.x = std::fma(rji.x, rji.w, ai.x);
      ai.y = std::fma(rji.y, rji.w, ai.y);
      ai.z = std::fma(rji.z, rji.w, ai.z);

#ifdef CALCULATE_POTENTIAL
      // gravitational potential
      ai.w = std::fma(r2, rji.w, ai.w);
#endif  // CALCULATE_POTENTIAL
    }
    iacc[ii] = ai;
  }
}

void trim_acc(const int Ni, acceleration *acc, const float newton
#ifdef CALCULATE_POTENTIAL
              ,
              position *pos, const float epsinv
#endif  // CALCULATE_POTENTIAL
) {
  OFFLOAD(AS_INDEPENDENT)
  for (std::remove_const_t<decltype(Ni)> ii = 0; ii < Ni; ii++) {
    // initialization
    acceleration ai = acc[ii];

    ai.x *= newton;
    ai.y *= newton;
    ai.z *= newton;

#ifdef CALCULATE_POTENTIAL
    ai.w = newton * std::fma(epsinv, pos[ii].w, -ai.w);
#endif  // CALCULATE_POTENTIAL

    acc[ii] = ai;
  }
}
