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

#include "../../util/pickup.hpp"  // PICKUP_CLAUSES
#include "../directive.hpp"
#include "clause.hpp"

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Variant directives
///

///
/// @brief _Pragma("omp declare target (extended_list)") or _Pragma("omp declare target clause [[,] clause ... ]")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
/// @note use OMP_PASS_LIST(...) to pass extended_list
///
#define PRAGMA_OMP_DECLARE_TARGET(...) PRAGMA_OMP(declare target APPEND_CLAUSES_WITHOUT_SORTING(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DECLARE_TARGET), OMP_PASS_LIST(__VA_ARGS__)))

///
/// @brief _Pragma("omp begin declare target [clause [[,] clause] ... ]")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
///
#define PRAGMA_OMP_BEGIN_DECLARE_TARGET(...) PRAGMA_OMP(begin declare target APPEND_CLAUSES_WITHOUT_SORTING(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DECLARE_TARGET), OMP_PASS_LIST(__VA_ARGS__)))

///
/// @brief _Pragma("omp end declare target")
/// @details A declarative directive that specifies that variables, functions, and subroutines are mapped to a device.
///
#define PRAGMA_OMP_END_DECLARE_TARGET PRAGMA_OMP(end declare target)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Device directives and construct
///

///
/// @brief _Pragma("omp target data [clause [[,] clause] ... ]")
/// @details Maps variables to a device data environment for the extent of the region.
///
#define PRAGMA_OMP_TARGET_DATA(...) PRAGMA_OMP(target data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET_DATA), __VA_ARGS__))

///
/// @brief _Pragma("omp target enter data [clause [[,] clause] ... ]")
/// @details Maps variables to a device data environment.
///
#define PRAGMA_OMP_TARGET_ENTER_DATA(...) PRAGMA_OMP(target enter data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET_ENTER_DATA), __VA_ARGS__))

///
/// @brief _Pragma("omp target exit data [clause [[,] clause] ... ]")
/// @details Unmaps variables from a device data environment.
///
#define PRAGMA_OMP_TARGET_EXIT_DATA(...) PRAGMA_OMP(target exit data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET_EXIT_DATA), __VA_ARGS__))

///
/// @brief _Pragma("omp target [clause [[,] clause] ... ]")
/// @details Map variables to a device data environment and execute the construct on that device.
///
#define PRAGMA_OMP_TARGET(...) PRAGMA_OMP(target APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET), __VA_ARGS__))

///
/// @brief _Pragma("omp target update [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_UPDATE(...) PRAGMA_OMP(target update APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET_UPDATE), __VA_ARGS__))

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
#define PRAGMA_OMP_TARGET_PARALLEL(...) PRAGMA_OMP(target parallel APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_PARALLEL), __VA_ARGS__))

///
/// @brief _Pragma("omp target parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel for except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR(...) PRAGMA_OMP(target parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))

///
/// @brief _Pragma("omp target parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel for simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(target parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD)__VA_ARGS__))

///
/// @brief _Pragma("omp target parallel loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a parallel loop construct and no other statements.
/// @note clause: Clauses used for target or parallel loop except copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_LOOP(...) PRAGMA_OMP(target parallel loop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_LOOP), __VA_ARGS__))

///
/// @brief _Pragma("omp target simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a simd construct and no other statements.
/// @note clause: Any clause used for target or simd.
///
#define PRAGMA_OMP_TARGET_SIMD(...) PRAGMA_OMP(target simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_SIMD) __VA_ARGS__))

///
/// @brief _Pragma("omp target teams [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams construct and no other statements.
/// @note clause: Any clause used for target or teams.
///
#define PRAGMA_OMP_TARGET_TEAMS(...) PRAGMA_OMP(target teams APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_TEAMS), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams distribute [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute construct and no other statements.
/// @note clause: Any clause used for target or teams distribute.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...) PRAGMA_OMP(target teams distribute APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams distribute simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute simd construct and no other statements.
/// @note clause: Any clause used for target or teams distribute simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...) PRAGMA_OMP(target teams distribute simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams loop construct and no other statements.
/// @note clause: Any clause used for target or teams loop.
///
#define PRAGMA_OMP_TARGET_TEAMS_LOOP(...) PRAGMA_OMP(target teams loop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_LOOP), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel for and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP(target teams distribute parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))

///
/// @brief _Pragma("omp target teams distribute parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(target teams distribute parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TARGET, CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

#endif  // !defined(SOLOMON_OMP_TARGET_DIRECTIVE_HPP)
