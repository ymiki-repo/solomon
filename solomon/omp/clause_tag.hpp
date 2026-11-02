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
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_ATOMIC(x) x

///
/// @brief matching macro about clause for omp taskwait
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKWAIT(x) x

///
/// @brief matching macro about clause for omp declare simd
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD(x) x

///
/// @brief matching macro about clause for omp declare target
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET(x) x

///
/// @brief matching macro about clause for omp target
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET(x) x

///
/// @brief matching macro about clause for omp target data
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA(x) x

///
/// @brief pseudo tags of the clause sets that OpenMP 5.2 requires on the data directives (sections 13.5-13.9)
/// @details REQ_DATA_ENV: map, use_device_ptr or use_device_addr (target data); REQ_MAP: map (target enter / exit data); REQ_MOTION: to or from (target update).
/// They are attached to those clauses only and used to drop an OpenACC data directive converted to OpenMP target when none of them survives.
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_REQ_DATA_ENV(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_REQ_MAP(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_REQ_MOTION(x) x

///
/// @brief matching macro about clause for omp target enter data
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA(x) x

///
/// @brief matching macro about clause for omp target exit data
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA(x) x

///
/// @brief matching macro about clause for omp target update
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE(x) x

///
/// @brief matching macro about clause for omp unroll
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_UNROLL(x) x

///
/// @brief matching macro about clause for omp parallel
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_PARALLEL(x) x

///
/// @brief matching macro about clause for omp teams
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TEAMS(x) x

///
/// @brief matching macro about clause for omp simd
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_SIMD(x) x

///
/// @brief matching macro about clause for omp single
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_SINGLE(x) x

///
/// @brief matching macro about clause for omp sections
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_SECTIONS(x) x

///
/// @brief matching macro about clause for omp for
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_FOR(x) x

///
/// @brief matching macro about clause for omp distribute
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE(x) x

///
/// @brief matching macro about clause for omp loop
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_LOOP(x) x

///
/// @brief matching macro about clause for omp task
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASK(x) x

///
/// @brief matching macro about clause for omp taskloop
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKLOOP(x) x

///
/// @brief matching macro about clause for omp interop
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_INTEROP(x) x

///
/// @brief matching macro about clause for omp critical
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_CRITICAL(x) x

///
/// @brief matching macro about clause for omp taskgroup
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TASKGROUP(x) x

///
/// @brief matching macro about clause for omp flush
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_FLUSH(x) x

///
/// @brief matching macro about clause for omp ordered
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_ORDERED(x) x

///
/// @note target parallel [for / for simd / loop] do not use copyin clause; therefore, special treatment is mandatory
///

///
/// @brief matching macro about copyin clause
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_COPYIN(x) x

///
/// @note parallel [for / sections / for simd ] do not use nowait clause; therefore, special treatment is mandatory
///

///
/// @brief matching macro about nowait clause
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_NOWAIT(x) x

///
/// @note parallel masked taskloop and parallel masked taskloop simd do not use in_reduction clause; therefore, special treatment is mandatory
///

///
/// @brief matching macro about in_reduction clause
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_IN_REDUCTION(x) x

///
/// @brief matching macro about the simd clause: taken by the loop directives without simd in their names (OMP_CLAUSE_SIMD)
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_SIMD_CLAUSE(x) x

///
/// @brief matching macro about the warning for OMP_CLAUSE_IF on the teams directives before OpenMP 6.0 (OMP_CLAUSE_IF)
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TEAMS_IF(x) x

///
/// @brief matching macro about the same warning on PRAGMA_OMP_TARGET_TEAMS in fallback mode (which drops the clause without the stop: KI-33)
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TEAMS_IF_FALLBACK(x) x

///
/// @brief matching macro about clause for omp masked
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_MASKED(x) x

///
/// @brief matching macro about clause for omp scan
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_SCAN(x) x

///
/// @brief matching macro about clause for omp scope
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_SCOPE(x) x

///
/// @brief matching macro about clause for omp tile
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_TILE(x) x

///
/// @brief matching macro about clause for omp workshare
/// @note not defined for NVIDIA HPC SDK in Fortran (nvfortran rejects "workshare nowait", and nowait is the only clause of workshare), so workshare takes no clause there.
/// The compiler is seen through __NVCOMPILER or, when the source goes through the system cpp (fortran.mk, spp.sh), through __SOLOMON_NVCOMPILER__ from their probe.
///
#if !(defined(SOLOMON_FORTRAN) && (defined(__NVCOMPILER) || defined(__SOLOMON_NVCOMPILER__)))
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_WORKSHARE(x) x
#endif  // !(defined(SOLOMON_FORTRAN) && (defined(__NVCOMPILER) || defined(__SOLOMON_NVCOMPILER__)))

///
/// @brief matching macro about clause for omp depobj
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_OMP_DEPOBJ(x) x

#endif  // !defined(SOLOMON_OMP_CLAUSE_TAG_HPP)
