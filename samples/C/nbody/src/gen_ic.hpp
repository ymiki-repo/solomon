///
/// @file gen_ic.hpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief generate initial-condition on host CPU
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#pragma once

#include "type.hpp"

void set_uniform_sphere(const int num, position *pos, velocity *vel, const float Mtot, const float rad, const float virial, const float newton);
