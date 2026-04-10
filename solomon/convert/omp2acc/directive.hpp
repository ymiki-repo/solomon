///
/// @file solomon/convert/omp2acc/directive.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief convert OpenMP directives to OpenACC directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_CONVERT_OMP2ACC_DIRECTIVE_HPP)
#define SOLOMON_CONVERT_OMP2ACC_DIRECTIVE_HPP

#include "../../util/va_args.hpp"

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Variant directives
///

///
/// @brief _Pragma("omp declare target (extended_list)") or _Pragma("omp declare target clause [[,] clause ... ]")
/// @details replace to _Pragma("acc routine [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_DECLARE_TARGET(...) PRAGMA_ACC_ROUTINE(__VA_ARGS__)

///
/// @brief _Pragma("omp begin declare target [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc routine [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_BEGIN_DECLARE_TARGET(...) PRAGMA_ACC_ROUTINE(__VA_ARGS__)

///
/// @brief _Pragma("omp end declare target")
/// @details ignore the directive
///
#define PRAGMA_OMP_END_DECLARE_TARGET

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Device directives and construct
///

///
/// @brief _Pragma("omp target data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc data [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_DATA(...) PRAGMA_ACC_DATA(__VA_ARGS__)

///
/// @brief _Pragma("omp target enter data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc enter data [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_ENTER_DATA(...) PRAGMA_ACC_ENTER_DATA(__VA_ARGS__)

///
/// @brief _Pragma("omp target exit data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc exit data [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_EXIT_DATA(...) PRAGMA_ACC_EXIT_DATA(__VA_ARGS__)

///
/// @brief _Pragma("omp target [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc __VA_ARGS__")
///
#define PRAGMA_OMP_TARGET(...) PRAGMA_ACC(__VA_ARGS__)

///
/// @brief _Pragma("omp target update [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc update [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_UPDATE(...) PRAGMA_ACC_UPDATE(__VA_ARGS__)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Synchronization constructs
///

///
/// @brief _Pragma("omp taskwait [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc wait [(expression_list)] [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_TASKWAIT(...) PRAGMA_ACC_WAIT(__VA_ARGS__)

///
/// @brief _Pragma("omp atomic [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc atomic [clause [[,] clause] ... ]")
///
#define PRAGMA_OMP_TARGET_ATOMIC(...) PRAGMA_ACC_ATOMIC(__VA_ARGS__)

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
#define PRAGMA_OMP_TARGET_PARALLEL(...) PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)
///
/// @brief finalize the acc offloaded region launched as default mode
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_PARALLEL
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_PARALLEL PRAGMA_ACC_END_LAUNCH_DEFAULT
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel for except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)
///
/// @brief _Pragma("omp target parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel do except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)

///
/// @brief _Pragma("omp target parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel for simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...) PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT APPEND_ARGS(__VA_ARGS__))
///
/// @brief _Pragma("omp target parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel do simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...) PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT APPEND_ARGS(__VA_ARGS__))

///
/// @brief _Pragma("omp target parallel loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a parallel loop construct and no other statements.
/// @note clause: Clauses used for target or parallel loop except copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_LOOP(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)

///
/// @brief _Pragma("omp target simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a simd construct and no other statements.
/// @note clause: Any clause used for target or simd.
///
#define PRAGMA_OMP_TARGET_SIMD(...) PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT APPEND_ARGS(__VA_ARGS__))
///
/// @brief finalize the acc offloaded region launched as default mode
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_SIMD
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_SIMD PRAGMA_ACC_END_LAUNCH_DEFAULT
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target teams [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams construct and no other statements.
/// @note clause: Any clause used for target or teams.
///
#define PRAGMA_OMP_TARGET_TEAMS(...) PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)
///
/// @brief finalize the acc offloaded region launched as default mode
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_TEAMS
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TARGET_TEAMS PRAGMA_ACC_END_LAUNCH_DEFAULT
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp target teams distribute [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute construct and no other statements.
/// @note clause: Any clause used for target or teams distribute.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...) PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)

///
/// @brief _Pragma("omp target teams distribute simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute simd construct and no other statements.
/// @note clause: Any clause used for target or teams distribute simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...) PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT APPEND_ARGS(__VA_ARGS__))

///
/// @brief _Pragma("omp target teams loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams loop construct and no other statements.
/// @note clause: Any clause used for target or teams loop.
///
#define PRAGMA_OMP_TARGET_TEAMS_LOOP(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)

///
/// @brief _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel for and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)
///
/// @brief _Pragma("omp target teams distribute parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel do and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)

///
/// @brief _Pragma("omp target teams distribute parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT APPEND_ARGS(__VA_ARGS__))
///
/// @brief _Pragma("omp target teams distribute parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...) PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT APPEND_ARGS(__VA_ARGS__))

#endif  // !defined(SOLOMON_CONVERT_OMP2ACC_DIRECTIVE_HPP)
