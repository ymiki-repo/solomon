///
/// @file gen_ic.cpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief generate initial-condition on GPU
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#include "gen_ic.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <boost/math/constants/constants.hpp>  // boost::math::constants
#include <random>                              // std::random_device
#include <type_traits>                         // std::remove_reference_t

#include "type.hpp"

///
/// @brief set initial condition (uniform sphere)
///
/// @param[in] num number of N-body particles
/// @param[out] pos position of N-body particles
/// @param[out] vel velocity of N-body particles
/// @param[in] Mtot total mass of the system
/// @param[in] rad radius of the initial sphere
/// @param[in] virial virial ratio of the initial condition
/// @param[in] newton gravitational constant in the computational unit
///
void set_uniform_sphere(const int num, position *pos, velocity *vel, const float Mtot, const float rad, const float virial, const float newton = 1.0F) {
  // set velocity dispersion
  const auto sigma = std::sqrt(1.2F * newton * Mtot * virial / rad);
  const auto sig1d = sigma / std::sqrt(3.0F);

  // prepare pseudo random numbers
  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());
  std::uniform_real_distribution<decltype((*pos).x)> dist_uni(0.0F, 1.0F);
  std::normal_distribution<decltype((*vel).x)> dist_nrm(0.0F, 1.0F);

  // distribute N-body particles
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    const auto mass = (ii < num) ? (Mtot / static_cast<decltype(Mtot)>(num)) : 0.0F;  // set massless particle to remove if statements in gravity calculation

    const auto rr = rad * std::cbrt(dist_uni(engine));
    const auto prj = 2.0F * dist_uni(engine) - 1.0F;
    const auto RR = rr * std::sqrt(1.0F - prj * prj);
    const auto theta = boost::math::constants::two_pi<decltype((*pos).x)>() * dist_uni(engine);
    auto pi = position{};
    pi.x = RR * std::cos(theta);
    pi.y = RR * std::sin(theta);
    pi.z = rr * prj;
    pi.w = mass;

    // set particle velocity
    auto vi = velocity{};
    vi.x = sig1d * dist_nrm(engine);
    vi.y = sig1d * dist_nrm(engine);
    vi.z = sig1d * dist_nrm(engine);

    pos[ii] = pi;
    vel[ii] = vi;
  }

  // shift center-of-mass and remove bulk velocity
  auto cx = 0.0;
  auto cy = 0.0;
  auto cz = 0.0;
  auto vx = 0.0;
  auto vy = 0.0;
  auto vz = 0.0;
#pragma omp parallel for reduction(+ : cx, cy, cz, vx, vy, vz)
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    // evaluate center-of-mass
    const auto pi = pos[ii];
    const auto mi = static_cast<double>(pi.w);
    cx += mi * static_cast<double>(pi.x);
    cy += mi * static_cast<double>(pi.y);
    cz += mi * static_cast<double>(pi.z);

    // evaluate bulk velocity
    const auto vi = vel[ii];
    vx += mi * static_cast<double>(vi.x);
    vy += mi * static_cast<double>(vi.y);
    vz += mi * static_cast<double>(vi.z);
  }
  const auto M_inv = 1.0 / static_cast<double>(Mtot);
  cx *= M_inv;
  cy *= M_inv;
  cz *= M_inv;
  vx *= M_inv;
  vy *= M_inv;
  vz *= M_inv;
#pragma omp parallel for
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    auto pi = pos[ii];
    pi.x -= cx;
    pi.y -= cy;
    pi.z -= cz;
    pos[ii] = pi;
    auto vi = vel[ii];
    vi.x -= vx;
    vi.y -= vy;
    vi.z -= vz;
    vel[ii] = vi;
  }
}
