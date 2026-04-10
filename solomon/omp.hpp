///
/// @file solomon/omp.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief pragmas to use OpenMP directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_HPP)
#define SOLOMON_OMP_HPP

#include "omp/clause.hpp"
#include "omp/directive.hpp"

///
/// @brief _Pragma("omp atomic update")
///
#define PRAGMA_OMP_ATOMIC_UPDATE PRAGMA_OMP_ATOMIC(OMP_CLAUSE_UPDATE)
///
/// @brief _Pragma("omp atomic read")
///
#define PRAGMA_OMP_ATOMIC_READ PRAGMA_OMP_ATOMIC(OMP_CLAUSE_READ)
///
/// @brief _Pragma("omp atomic write")
///
#define PRAGMA_OMP_ATOMIC_WRITE PRAGMA_OMP_ATOMIC(OMP_CLAUSE_WRITE)
///
/// @brief _Pragma("omp atomic capture")
///
#define PRAGMA_OMP_ATOMIC_CAPTURE PRAGMA_OMP_ATOMIC(OMP_CLAUSE_CAPTURE)

#endif  // !defined(SOLOMON_OMP_HPP)
