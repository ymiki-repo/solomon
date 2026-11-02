///
/// @file solomon/omp/target/fallback.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief fallback OpenMP target directives to OpenMP directives for multicore CPU
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_TARGET_FALLBACK_HPP)
#define SOLOMON_OMP_TARGET_FALLBACK_HPP

#include "../directive.hpp"

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Variant directives
///

///
/// @brief _Pragma("omp declare target (extended_list)") or _Pragma("omp declare target clause [[,] clause ... ]")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
/// @note use OMP_PASS_LIST(...) to pass extended_list
///
#define PRAGMA_OMP_DECLARE_TARGET(...)

///
/// @brief _Pragma("omp begin declare target [clause [[,] clause] ... ]")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
///
#define PRAGMA_OMP_BEGIN_DECLARE_TARGET(...)

///
/// @brief _Pragma("omp end declare target")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
///
#define PRAGMA_OMP_END_DECLARE_TARGET

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Device directives and construct
///

///
/// @brief _Pragma("omp target data [clause [[,] clause] ... ]")
/// @details Maps variables to a device data environment for the extent of the region.
///
#define PRAGMA_OMP_TARGET_DATA(...)
///
/// @brief finalize the omp target data region
///
#define PRAGMA_OMP_END_TARGET_DATA

///
/// @brief _Pragma("omp target enter data [clause [[,] clause] ... ]")
/// @details Maps variables to a device data environment.
///
#define PRAGMA_OMP_TARGET_ENTER_DATA(...)

///
/// @brief _Pragma("omp target exit data [clause [[,] clause] ... ]")
/// @details Unmaps variables from a device data environment.
///
#define PRAGMA_OMP_TARGET_EXIT_DATA(...)

///
/// @brief _Pragma("omp target [clause [[,] clause] ... ]")
/// @details Map variables to a device data environment and execute the construct on that device.
///
#define PRAGMA_OMP_TARGET(...)
///
/// @brief finalize the ignored omp target region
///
#define PRAGMA_OMP_END_TARGET

///
/// @brief _Pragma("omp target update [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_UPDATE(...)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Combined Constructs and Directives
/// @details The following combined constructs and directives are created following the parameters defined in section 17 of the OpenMP API version 5.2 specification and were explicitly defined in previous versions.
///

///
/// @brief _Pragma("omp parallel [clause [[,] clause] ... ]")
/// @details Creates a team of OpenMP threads that execute the region.
///
#define PRAGMA_OMP_TARGET_PARALLEL(...) PRAGMA_OMP_PARALLEL(__VA_ARGS__)
///
/// @brief finalize the omp target parallel region
///
#define PRAGMA_OMP_END_TARGET_PARALLEL PRAGMA_OMP_END_PARALLEL

///
/// @brief _Pragma("omp parallel for [clause [[,] clause] ... ]")
/// @details Specifies a parallel construct containing a worksharing-loop construct with a canonical loop nest and no other statements.
/// @note clause: Any clause used for parallel or for except the nowait clause.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR(...) PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)
///
/// @brief _Pragma("omp parallel do [clause [[,] clause] ... ]")
/// @details Specifies a parallel construct containing a worksharing-loop construct with a canonical loop nest and no other statements.
/// @note clause: Any clause used for parallel or do except the nowait clause.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO(...) PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)

///
/// @brief _Pragma("omp parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing only one worksharing-loop SIMD construct.
/// @note clause: Any clause used for parallel or for simd except the nowait clause.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...) PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)
///
/// @brief _Pragma("omp parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing only one worksharing-loop SIMD construct.
/// @note clause: Any clause used for parallel or do simd except the nowait clause.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...) PRAGMA_OMP_PARALLEL_DO_SIMD(__VA_ARGS__)

///
/// @brief _Pragma("omp parallel loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing a loop construct with a canonical loop nest and no other statements.
/// @note clause: Any clause used for parallel or loop.
///
#define PRAGMA_OMP_TARGET_PARALLEL_LOOP(...) PRAGMA_OMP_PARALLEL_LOOP(__VA_ARGS__)

///
/// @brief _Pragma("omp simd [clause [[,] clause] ... ]")
/// @details Applied to a loop to indicate that the loop can be transformed into a SIMD loop.
///
#define PRAGMA_OMP_TARGET_SIMD(...) PRAGMA_OMP_SIMD(__VA_ARGS__)
///
/// @brief finalize the omp target simd region
///
#define PRAGMA_OMP_END_TARGET_SIMD PRAGMA_OMP_END_SIMD

///
/// @brief _Pragma("omp teams [clause [[,] clause] ... ]")
/// @details Creates a league of initial teams where the initial thread of each team executes the region.
///
/// @note in C/C++ before OpenMP 6.0, OMP_CLAUSE_IF(...) is dropped here (with the warning where it can be emitted) and does not stop
/// the build as on PRAGMA_OMP_TEAMS(...) (the pseudo tag SOLOMON_INTERNAL_TAG_OMP_TEAMS_IF_FALLBACK; KI-33)
///
#define PRAGMA_OMP_TARGET_TEAMS(...) PRAGMA_OMP(teams SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_TEAMS_IF_FALLBACK), __VA_ARGS__))
///
/// @brief finalize the omp target teams region
///
#define PRAGMA_OMP_END_TARGET_TEAMS PRAGMA_OMP_END_TEAMS

///
/// @brief _Pragma("omp teams distribute [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute construct and no other statements.
/// @note clause: Any clause used for teams or distribute.
/// @note In fallback mode it becomes _Pragma("omp parallel for [clause [[,] clause] ... ]") (Fortran: parallel do), so that the loop runs on the threads.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...) PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)

///
/// @brief _Pragma("omp teams distribute simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute simd construct and no other statements.
/// @note clause: Any clause used for teams or distribute simd.
/// @note In fallback mode it becomes _Pragma("omp parallel for simd [clause [[,] clause] ... ]") (Fortran: parallel do simd), so that the loop runs on the threads.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...) PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)

///
/// @brief _Pragma("omp parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a loop construct and no other statements.
/// @note clause: Any clause used for teams or loop.
///
#define PRAGMA_OMP_TARGET_TEAMS_LOOP(...) PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)

///
/// @brief _Pragma("omp teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute parallel worksharing-loop construct and no other statements.
/// @note clause: Any clause used for teams or distribute parallel for.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)
///
/// @brief _Pragma("omp teams distribute parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute parallel worksharing-loop construct and no other statements.
/// @note clause: Any clause used for teams or distribute parallel do.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...) PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)

///
/// @brief _Pragma("omp teams distribute parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute parallel for simd construct and no other statements.
/// @note clause: Any clause used for teams or distribute parallel for simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)
///
/// @brief _Pragma("omp teams distribute parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute parallel do simd construct and no other statements.
/// @note clause: Any clause used for teams or distribute parallel do simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...) PRAGMA_OMP_PARALLEL_DO_SIMD(__VA_ARGS__)

///
/// @brief ignore _Pragma("omp taskwait [clause [[,] clause] ... ]") issued for offloaded tasks
/// @details Nothing runs asynchronously on the CPU in fallback mode; PRAGMA_OMP_TASKWAIT(...) (without TARGET) still emits omp taskwait.
///
#define PRAGMA_OMP_TARGET_TASKWAIT(...)

///
/// @brief _Pragma("omp atomic [clause [[,] clause] ... ]")
/// @details Atomic operations inside the parallel loops of fallback mode.
///
#define PRAGMA_OMP_TARGET_ATOMIC(...) PRAGMA_OMP_ATOMIC(__VA_ARGS__)

///
/// @brief the target of ACC_CLAUSE_VECTOR_LENGTH(n) and SOLOMON_CLAUSE_NUM_THREADS(n) in fallback mode: num_threads (n) for
/// the parallel forms and thread_limit (n) for teams (each directive selects one of the two by its tags)
/// @details OMP_TARGET_CLAUSE_THREAD_LIMIT(num) itself stays thread_limit (num), which only teams takes in fallback mode
///
#if defined(SOLOMON_INTERNAL_OMP_TARGET_THREADS_PER_TEAM)
#undef SOLOMON_INTERNAL_OMP_TARGET_THREADS_PER_TEAM
#endif  // defined(SOLOMON_INTERNAL_OMP_TARGET_THREADS_PER_TEAM)
#define SOLOMON_INTERNAL_OMP_TARGET_THREADS_PER_TEAM(num) OMP_CLAUSE_NUM_THREADS(num), OMP_CLAUSE_THREAD_LIMIT(num)

///
/// @brief drop OpenMP target clause "if (target : condition)" in fallback mode
/// @details The directive-name-modifier target names no constituent of the fallback directives (OpenMP 5.2 section 3.4),
/// and a target region whose if clause is false runs on the host (section 13.8), which is what fallback mode does anyway.
/// Use OMP_TARGET_CLAUSE_IF(condition) for a condition that should also apply to the host directive.
///
#if defined(OMP_TARGET_CLAUSE_IF_TARGET)
#undef OMP_TARGET_CLAUSE_IF_TARGET
#endif  // defined(OMP_TARGET_CLAUSE_IF_TARGET)
#define OMP_TARGET_CLAUSE_IF_TARGET(condition) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief finalize the OpenMP loop directives (nothing: the end directive of a loop construct is optional in OpenMP; in the OpenACC backend they close the compute construct in Fortran)
///
#define PRAGMA_OMP_END_TARGET_PARALLEL_DO
#define PRAGMA_OMP_END_TARGET_PARALLEL_DO_SIMD
#define PRAGMA_OMP_END_TARGET_PARALLEL_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD
#define PRAGMA_OMP_END_TARGET_TEAMS_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD

#endif  // !defined(SOLOMON_OMP_TARGET_FALLBACK_HPP)
