///
/// @file solomon/omp/target/directive.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief OpenMP target directives as preprocessor macros
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_TARGET_DIRECTIVE_HPP)
#define SOLOMON_OMP_TARGET_DIRECTIVE_HPP

#include "../../util/pickup.hpp"  // SOLOMON_INTERNAL_PICKUP_CLAUSES
#include "../directive.hpp"
#include "clause.hpp"

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Variant directives
///

///
/// @brief _Pragma("omp declare target (extended_list)") or _Pragma("omp declare target clause [[,] clause ... ]")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
/// @note use OMP_PASS_LIST(...) to pass extended_list; clauses which are unavailable for declare target (e.g., converted OpenACC clauses without an OpenMP counterpart) are dropped
///
#define PRAGMA_OMP_DECLARE_TARGET(...) PRAGMA_OMP(declare target SOLOMON_INTERNAL_APPEND_CLAUSES_WITHOUT_SORTING(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET), __VA_ARGS__))

///
/// @brief _Pragma("omp begin declare target [clause [[,] clause] ... ]")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
///
#define PRAGMA_OMP_BEGIN_DECLARE_TARGET(...) PRAGMA_OMP(begin declare target SOLOMON_INTERNAL_APPEND_CLAUSES_WITHOUT_SORTING(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET), __VA_ARGS__))

///
/// @brief _Pragma("omp end declare target")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
/// @note in Fortran, the directive is not required (a list-less "declare target" inside a procedure marks the enclosing procedure); therefore it expands to nothing
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_DECLARE_TARGET PRAGMA_OMP(end declare target)
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_DECLARE_TARGET
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Device directives and construct
///

///
/// @brief _Pragma("omp target data [clause [[,] clause] ... ]")
/// @details Maps variables to a device data environment for the extent of the region.
///
#define PRAGMA_OMP_TARGET_DATA(...) PRAGMA_OMP(target data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA), __VA_ARGS__))
///
/// @brief finalize the omp target data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_DATA PRAGMA_OMP(end target data)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target enter data [clause [[,] clause] ... ]")
/// @details Maps variables to a device data environment.
///
#define PRAGMA_OMP_TARGET_ENTER_DATA(...) PRAGMA_OMP(target enter data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA), __VA_ARGS__))

///
/// @brief _Pragma("omp target exit data [clause [[,] clause] ... ]")
/// @details Unmaps variables from a device data environment.
///
#define PRAGMA_OMP_TARGET_EXIT_DATA(...) PRAGMA_OMP(target exit data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA), __VA_ARGS__))

///
/// @brief _Pragma("omp target [clause [[,] clause] ... ]")
/// @details Map variables to a device data environment and execute the construct on that device.
///
#define PRAGMA_OMP_TARGET(...) PRAGMA_OMP(target SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET), __VA_ARGS__))
///
/// @brief finalize the omp target region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET PRAGMA_OMP(end target)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target update [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_UPDATE(...) PRAGMA_OMP(target update SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Synchronization constructs
///

///
/// @brief _Pragma("omp taskwait [clause [[,] clause] ... ]")
/// @details Specifies a wait on the completion of child tasks of the current task.
///
#define PRAGMA_OMP_TARGET_TASKWAIT(...) PRAGMA_OMP_TASKWAIT(__VA_ARGS__)

///
/// @brief _Pragma("omp atomic [clause [[,] clause] ... ]")
/// @details Ensures a specific storage location is accessed atomically.
///
#define PRAGMA_OMP_TARGET_ATOMIC(...) PRAGMA_OMP_ATOMIC(__VA_ARGS__)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Combined Constructs and Directives
/// @details The following combined constructs and directives are created following the parameters defined in section 17 of the OpenMP API version 5.2 specification and were explicitly defined in previous versions.
///

///
/// @brief _Pragma("omp target parallel [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a parallel construct and no other statements.
/// @note clause: Clauses used for target or parallel except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL(...) PRAGMA_OMP(target parallel SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_PARALLEL), __VA_ARGS__))
///
/// @brief finalize the omp target parallel region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_PARALLEL
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_PARALLEL PRAGMA_OMP(end target parallel)
#endif  // !defined(SOLOMON_FORTRAN)

#if !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel for except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR(...) PRAGMA_OMP(target parallel for SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_FOR), __VA_ARGS__))
///
/// @brief _Pragma("omp target parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel for except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO(...) PRAGMA_OMP_TARGET_PARALLEL_FOR(__VA_ARGS__)
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel do except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO(...) PRAGMA_OMP(target parallel do SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_FOR), __VA_ARGS__))
///
/// @brief _Pragma("omp target parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel do except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR(...) PRAGMA_OMP_TARGET_PARALLEL_DO(__VA_ARGS__)
#endif  // !defined(SOLOMON_FORTRAN)

#if !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel for simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(target parallel for simd SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_SIMD), __VA_ARGS__))
///
/// @brief _Pragma("omp target parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel for simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...) PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(__VA_ARGS__)
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel do simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...) PRAGMA_OMP(target parallel do simd SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_SIMD), __VA_ARGS__))
///
/// @brief _Pragma("omp target parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel do simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...) PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(__VA_ARGS__)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target parallel loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a parallel loop construct and no other statements.
/// @note clause: Clauses used for target or parallel loop except copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_LOOP(...) PRAGMA_OMP(target parallel loop SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_LOOP), __VA_ARGS__))

///
/// @brief _Pragma("omp target simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a simd construct and no other statements.
/// @note clause: Any clause used for target or simd.
///
#define PRAGMA_OMP_TARGET_SIMD(...) PRAGMA_OMP(target simd SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_SIMD) __VA_ARGS__))
///
/// @brief finalize the omp target simd region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_SIMD
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_SIMD PRAGMA_OMP(end target simd)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target teams [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams construct and no other statements.
/// @note clause: Any clause used for target or teams.
///
#define PRAGMA_OMP_TARGET_TEAMS(...) PRAGMA_OMP(target teams SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS), __VA_ARGS__))
///
/// @brief finalize the omp target teams region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_TEAMS
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_TEAMS PRAGMA_OMP(end target teams)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target teams distribute [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute construct and no other statements.
/// @note clause: Any clause used for target or teams distribute.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...) PRAGMA_OMP(target teams distribute SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams distribute simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute simd construct and no other statements.
/// @note clause: Any clause used for target or teams distribute simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...) PRAGMA_OMP(target teams distribute simd SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams loop construct and no other statements.
/// @note clause: Any clause used for target or teams loop.
///
#define PRAGMA_OMP_TARGET_TEAMS_LOOP(...) PRAGMA_OMP(target teams loop SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_LOOP), __VA_ARGS__))

#if !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel for and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP(target teams distribute parallel for SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_COPYIN, SOLOMON_INTERNAL_TAG_OMP_FOR), __VA_ARGS__))
///
/// @brief _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel for and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...) PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(__VA_ARGS__)
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target teams distribute parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel do and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...) PRAGMA_OMP(target teams distribute parallel do SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_COPYIN, SOLOMON_INTERNAL_TAG_OMP_FOR), __VA_ARGS__))
///
/// @brief _Pragma("omp target teams distribute parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel do and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(__VA_ARGS__)
#endif  // !defined(SOLOMON_FORTRAN)

#if !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target teams distribute parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(target teams distribute parallel for simd SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_COPYIN, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_SIMD), __VA_ARGS__))
///
/// @brief _Pragma("omp target teams distribute parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...) PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(__VA_ARGS__)
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief _Pragma("omp target teams distribute parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...) PRAGMA_OMP(target teams distribute parallel do simd SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_COPYIN, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_SIMD), __VA_ARGS__))
///
/// @brief _Pragma("omp target teams distribute parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(__VA_ARGS__)
#endif  // !defined(SOLOMON_FORTRAN)

#endif  // !defined(SOLOMON_OMP_TARGET_DIRECTIVE_HPP)
