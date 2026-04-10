///
/// @file type.hpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief data type
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#pragma once

struct alignas(16) float4 {
  float x;
  float y;
  float z;
  float w;
};
struct float3 {
 public:
  float x;
  float y;
  float z;
};

using position = float4;
using velocity = float3;
using acceleration = float4;
