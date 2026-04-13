///
/// @file solomon/fallback.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief fallback OpenMP target directives to OpenMP directives for multicore CPU
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_FALLBACK_HPP)
#define SOLOMON_FALLBACK_HPP

#if defined(_OPENMP)
#include "omp.hpp"
#include "omp/runtime.hpp"
#include "omp/target/clause.hpp"
#include "omp/target/fallback.hpp"

// utilities
#include "omp/target/utils.hpp"

// converter from OpenACC to OpenMP target
#include "acc/utils.hpp"
#include "convert/acc2omp/clause.hpp"
#include "convert/acc2omp/directive.hpp"
#else   // defined(_OPENMP)
#if !defined(USE_SOLOMON_RUNTIME)
///
/// @brief empty definition of USE_SOLOMON_RUNTIME when OpenMP is not available
///
#define USE_SOLOMON_RUNTIME
#endif  // !defined(USE_SOLOMON_RUNTIME)
#endif  // defined(_OPENMP)

#endif  // !defined(SOLOMON_FALLBACK_HPP)
