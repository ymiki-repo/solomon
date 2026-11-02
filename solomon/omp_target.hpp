///
/// @file solomon/omp_target.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief pragmas to use GPU by OpenMP target directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_TARGET_HPP)
#define SOLOMON_OMP_TARGET_HPP

#if defined(OFFLOAD_BY_OPENMP_TARGET) && defined(_OPENMP) && (_OPENMP >= 201307)
// set OpenMP target environments
#include "omp.hpp"
#include "omp/target/clause.hpp"
#include "omp/target/directive.hpp"
#include "omp/target/runtime.hpp"

// set utilities for OpenMP target
#include "omp/target/utils.hpp"

// load converter from OpenACC to OpenMP target
#include "convert/acc2omp/clause.hpp"
#include "convert/acc2omp/directive.hpp"

// set utilities for OpenACC
#include "acc/utils.hpp"
#endif  // defined(OFFLOAD_BY_OPENMP_TARGET) && defined(_OPENMP) && (_OPENMP >= 201307)

#endif  // !defined(SOLOMON_OMP_TARGET_HPP)
