///
/// @file benchmark.hpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief benchmark settings
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#pragma once

#ifdef BENCHMARK_MODE
#if !defined(NMIN)
// #define NMIN (1024)
// #define NMIN (262144)
// #define NMIN (524288)
// #define NMIN (1048576)
#define NMIN (4194304)
// #define NMIN (8388608)
// #define NMIN (16777216)
// #define NMIN (33554432)
#endif  //! defined(NMIN)
#if !defined(NMAX)
// #define NMAX (262144)
// #define NMAX (524288)
// #define NMAX (1048576)
// #define NMAX (2097152)
#define NMAX (4194304)
// #define NMAX (8388608)
// #define NMAX (16777216)
// #define NMAX (33554432)
#endif  //! defined(NMAX)
#if !defined(NMAX_CRIT)
#define NMAX_CRIT (262144)
// #define NMAX_CRIT (1048576)
#endif  //! defined(NMAX_CRIT)

// #define BENCHMARK_SADDLE_REGION
#ifdef BENCHMARK_SADDLE_REGION
#undef NMIN
#undef NMAX
#undef NMAX_CRIT
#define NMIN_SADDLE (1024)
#define NMAX_SADDLE (524288)
#define NINC_SADDLE (256)
#define NMAX_CRIT (256)
#endif  // BENCHMARK_SADDLE_REGION

#endif  // BENCHMARK_MODE

#ifndef BENCHMARK_MODE
#define CALCULATE_POTENTIAL
#endif  // BENCHMARK_MODE

// 3 subtractions: 3 Flops
// 3 multiplications: 3 Flops
// 6 (+1) FMAs: 12 (+ 2) Flops
// 1 reciprocal square root: 4 Flops (this is assumption, 2 Flops is an
// alternative assumption: this assumption is valid only for A100) -->> 24 or 22
// Flops for w/ or w/o potential calculation, respectively 1 reciprocal square
// root: 3 Flops (this is assumption: this assumption would work for MI100) -->>
// 23 or 21 Flops for w/ or w/o potential calculation, respectively
// H100: throughput is 128 for FP32 add, multiply, FMA
// H100: throughput is 16 for rsqrtf
// H100: 128 / 16 = 8 Flops for rsqrtf would be better assumption
#ifdef CALCULATE_POTENTIAL
#define FLOPS_COUNT (24.0F)
#else  // CALCULATE_POTENTIAL
#define FLOPS_COUNT (22.0F)
#endif  // CALCULATE_POTENTIAL
