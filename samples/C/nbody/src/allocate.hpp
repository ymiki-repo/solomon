///
/// @file allocate.hpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief memory allocation
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#pragma once

#include "type.hpp"

void allocate_Nbody_particles(position **pos, velocity **vel, velocity **vel_tmp, acceleration **acc, const int num);
void release_Nbody_particles(position *pos, velocity *vel, velocity *vel_tmp, acceleration *acc);
