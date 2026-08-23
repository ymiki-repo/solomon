///
/// @file solomon/omp/clause_tag.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief tag about optional clauses for OpenMP directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_CLAUSE_TAG_HPP)
#define SOLOMON_OMP_CLAUSE_TAG_HPP

///
/// @brief matching macro about clause for omp atomic
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_ATOMIC(x) x

///
/// @brief matching macro about clause for omp taskwait
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKWAIT(x) x

///
/// @brief matching macro about clause for omp declare simd
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD(x) x

///
/// @brief matching macro about clause for omp declare target
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET(x) x

///
/// @brief matching macro about clause for omp target
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET(x) x

///
/// @brief matching macro about clause for omp target data
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA(x) x

///
/// @brief matching macro about clause for omp target enter data
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA(x) x

///
/// @brief matching macro about clause for omp target exit data
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA(x) x

///
/// @brief matching macro about clause for omp target update
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE(x) x

///
/// @brief matching macro about clause for omp unroll
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_UNROLL(x) x

///
/// @brief matching macro about clause for omp parallel
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_PARALLEL(x) x

///
/// @brief matching macro about clause for omp teams
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TEAMS(x) x

///
/// @brief matching macro about clause for omp simd
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_SIMD(x) x

///
/// @brief matching macro about clause for omp single
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_SINGLE(x) x

///
/// @brief matching macro about clause for omp sections
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_SECTIONS(x) x

///
/// @brief matching macro about clause for omp for
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_FOR(x) x

///
/// @brief matching macro about clause for omp distribute
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE(x) x

///
/// @brief matching macro about clause for omp loop
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_LOOP(x) x

///
/// @brief matching macro about clause for omp task
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASK(x) x

///
/// @brief matching macro about clause for omp taskloop
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKLOOP(x) x

///
/// @brief matching macro about clause for omp interop
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_INTEROP(x) x

///
/// @brief matching macro about clause for omp target
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET(x) x

///
/// @brief matching macro about clause for omp critical
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_CRITICAL(x) x

///
/// @brief matching macro about clause for omp taskgroup
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKGROUP(x) x

///
/// @brief matching macro about clause for omp taskwait
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKWAIT(x) x

///
/// @brief matching macro about clause for omp flush
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_FLUSH(x) x

///
/// @brief matching macro about clause for omp ordered
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_ORDERED(x) x

///
/// @note target parallel [for / for simd / loop] do not use copyin clause; therefore, special treatment is mandatory
///

///
/// @brief matching macro about copyin clause
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_COPYIN(x) x

///
/// @note parallel [for / sections / for simd ] do not use nowait clause; therefore, special treatment is mandatory
///

///
/// @brief matching macro about nowait clause
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_NOWAIT(x) x

///
/// @note parallel masked taskloop and parallel masked taskloop simd do not use in_reduction clause; therefore, special treatment is mandatory
///

///
/// @brief matching macro about in_reduction clause
///
#define COMPARE_SOLOMON_INTERNAL_TAG_OMP_IN_REDUCTION(x) x

#endif  // !defined(SOLOMON_OMP_CLAUSE_TAG_HPP)
