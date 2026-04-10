///
/// @file gravity.hpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief calculating gravitational interaction
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#pragma once

#include "benchmark.hpp"
#include "type.hpp"

void calc_acc(const int Ni, position *ipos, acceleration *iacc, const int Nj, position *jpos, const float eps);

void trim_acc(const int Ni, acceleration *acc, const float newton
#ifdef CALCULATE_POTENTIAL
              ,
              position *pos, const float epsinv
#endif  // CALCULATE_POTENTIAL
);
