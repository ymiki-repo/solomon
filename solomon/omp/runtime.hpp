///
/// @file solomon/omp/runtime.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief runtime library routines for OpenMP
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_RUNTIME_HPP)
#define SOLOMON_OMP_RUNTIME_HPP

#if !defined(SOLOMON_FORTRAN)
#include <omp.h>
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief use OpenMP runtime library in Fortran
///
#define USE_SOLOMON_RUNTIME use omp_lib
#endif  // !defined(SOLOMON_FORTRAN)

#endif  // !defined(SOLOMON_OMP_RUNTIME_HPP)
