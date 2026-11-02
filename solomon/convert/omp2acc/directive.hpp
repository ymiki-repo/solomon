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

#include "../../util/cloak.hpp"     // SOLOMON_INTERNAL_CLOAK_IIF
#include "../../util/is_empty.hpp"  // SOLOMON_INTERNAL_IS_EMPTY
#include "../../util/va_args.hpp"

///
/// @brief 1 when no clause of the required set (pseudo tag, see acc/clause_tag.hpp) survives the conversion to OpenACC, 0 otherwise
///
#define SOLOMON_INTERNAL_O2A_NO_REQUIRED(tag, ...) SOLOMON_INTERNAL_IS_EMPTY(SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(tag), __VA_ARGS__))

///
/// @brief stop an OpenMP data directive none of whose clauses has an OpenACC counterpart
/// @details C/C++: a preprocessor / compiler error with the message below; Fortran: the OpenACC directive (directive: data, enter data, exit data, update) with a name that the compiler rejects
///
#if !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_O2A_STOP(directive) _Pragma("GCC error \"Solomon: none of the clauses of this OpenMP data directive has an OpenACC counterpart\"")
#define SOLOMON_INTERNAL_O2A_STOP_MIX(...) _Pragma("GCC error \"Solomon: use_device_ptr / use_device_addr cannot be combined with other data clauses (map, has_device_addr, is_device_ptr, defaultmap, ...) in one OpenMP target data directive for the OpenACC backend; split it\"")
#else  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_O2A_STOP(directive) PRAGMA_ACC(directive SOLOMON_ERROR_NO_OPENACC_COUNTERPART_IN_DATA_CLAUSES)
#define SOLOMON_INTERNAL_O2A_STOP_MIX(...) PRAGMA_ACC(data SOLOMON_ERROR_SPLIT_USE_DEVICE_FROM_DATA_CLAUSES)
#endif  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_O2A_STOP_DATA(...) SOLOMON_INTERNAL_O2A_STOP(data)
#define SOLOMON_INTERNAL_O2A_STOP_ENTER(...) SOLOMON_INTERNAL_O2A_STOP(enter data)
#define SOLOMON_INTERNAL_O2A_STOP_EXIT(...) SOLOMON_INTERNAL_O2A_STOP(exit data)
#define SOLOMON_INTERNAL_O2A_STOP_UPDATE(...) SOLOMON_INTERNAL_O2A_STOP(update)

///
/// @brief target data without an OpenACC data clause: host_data when use_device survives, unless the generic map was dropped next to it; otherwise stop
///
#define SOLOMON_INTERNAL_O2A_TD_NODATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_HOST_DATA, __VA_ARGS__))(SOLOMON_INTERNAL_O2A_STOP_DATA, SOLOMON_INTERNAL_O2A_HD_MAP)(__VA_ARGS__)
#define SOLOMON_INTERNAL_O2A_HD_MAP(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_O2A_GENERIC_MAP, __VA_ARGS__))(PRAGMA_ACC_HOST_DATA, SOLOMON_INTERNAL_O2A_STOP_MIX)(__VA_ARGS__)
///
/// @brief target data with an OpenACC data clause: data, unless use_device survives too (stop: split the directive)
/// @details A mark of an undefined name or of the clause limit survives every pick; it is recognized with REQ_NONE (carried by no clause), and then the plain data directive is kept so that the mark stops it with its own message.
///
#define SOLOMON_INTERNAL_O2A_TD_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_HOST_DATA, __VA_ARGS__))(PRAGMA_ACC_DATA, SOLOMON_INTERNAL_O2A_TD_MIX)(__VA_ARGS__)
#define SOLOMON_INTERNAL_O2A_TD_MIX(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_NONE, __VA_ARGS__))(SOLOMON_INTERNAL_O2A_STOP_MIX, PRAGMA_ACC_DATA)(__VA_ARGS__)
///
/// @brief directive (enter data, exit data, update) when a clause of the required set survives, stop otherwise (stop: SOLOMON_INTERNAL_O2A_STOP_*)
///
#define SOLOMON_INTERNAL_O2A_CHECKED(tag, stop, directive, ...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(tag, __VA_ARGS__))(stop, directive)(__VA_ARGS__)

///
/// @brief compute construct of the OpenMP compute directives (kernels or parallel as PRAGMA_ACC_LAUNCH_DEFAULT)
/// @details with OMP_TARGET_CLAUSE_NUM_THREADS, the vector_length of OMP_TARGET_CLAUSE_THREAD_LIMIT is not picked (one vector_length clause)
///
#define SOLOMON_INTERNAL_O2A_LAUNCH(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_O2A_NUM_THREADS, __VA_ARGS__))(PRAGMA_ACC_LAUNCH_DEFAULT, SOLOMON_INTERNAL_O2A_LAUNCH_NT)(__VA_ARGS__)
#if !defined(OFFLOAD_BY_OPENACC_PARALLEL)
#define SOLOMON_INTERNAL_O2A_LAUNCH_NT(...) PRAGMA_ACC(kernels SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_KERNELS), __VA_ARGS__))
#else  // !defined(OFFLOAD_BY_OPENACC_PARALLEL)
#define SOLOMON_INTERNAL_O2A_LAUNCH_NT(...) PRAGMA_ACC(parallel SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL), __VA_ARGS__))
#endif  // !defined(OFFLOAD_BY_OPENACC_PARALLEL)
///
/// @brief compute construct and loop construct of the OpenMP loop directives
///
#define SOLOMON_INTERNAL_O2A_OFFLOADING(...) SOLOMON_INTERNAL_O2A_LAUNCH(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)
///
/// @brief finalize the OpenMP loop directives (Fortran: the end of the compute construct; the loop construct needs none)
///
#define SOLOMON_INTERNAL_O2A_END_LOOP PRAGMA_ACC_END_OFFLOADING_DEFAULT

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
/// @details replace to _Pragma("acc data [clause [[,] clause] ... ]"), or to _Pragma("acc host_data [clause [[,] clause] ... ]") when only use_device_ptr / use_device_addr (and if) have OpenACC counterparts;
/// stops when no clause has an OpenACC counterpart or when use_device_* is combined with other data clauses
///
#define PRAGMA_OMP_TARGET_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, __VA_ARGS__))(SOLOMON_INTERNAL_O2A_TD_NODATA, SOLOMON_INTERNAL_O2A_TD_DATA)(__VA_ARGS__)

///
/// @brief _Pragma("omp target enter data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc enter data [clause [[,] clause] ... ]"); stops when no clause has an OpenACC counterpart
///
#define PRAGMA_OMP_TARGET_ENTER_DATA(...) SOLOMON_INTERNAL_O2A_CHECKED(SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_O2A_STOP_ENTER, PRAGMA_ACC_ENTER_DATA, __VA_ARGS__)

///
/// @brief _Pragma("omp target exit data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc exit data [clause [[,] clause] ... ]"); stops when no clause has an OpenACC counterpart
///
#define PRAGMA_OMP_TARGET_EXIT_DATA(...) SOLOMON_INTERNAL_O2A_CHECKED(SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_O2A_STOP_EXIT, SOLOMON_INTERNAL_O2A_EXIT_DATA, __VA_ARGS__)
///
/// @brief one finalize clause when OMP_TARGET_CLAUSE_MAP_DELETE is given (OpenMP: the reference count is set to zero; OpenACC: delete without finalize only decrements it)
/// @note finalize applies to every copyout / delete / detach clause of the directive
///
#define SOLOMON_INTERNAL_O2A_EXIT_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_O2A_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_O2A_FINALIZE, __VA_ARGS__))(PRAGMA_ACC_EXIT_DATA, SOLOMON_INTERNAL_O2A_EXIT_DATA_FINALIZE)(__VA_ARGS__)
#define SOLOMON_INTERNAL_O2A_EXIT_DATA_FINALIZE(...) PRAGMA_ACC_EXIT_DATA(__VA_ARGS__, ACC_CLAUSE_FINALIZE)

///
/// @brief _Pragma("omp target [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc serial [clause [[,] clause] ... ]") for single-thread execution on the device (v2.0.0 or later; the previous conversion emitted an invalid directive)
///
#define PRAGMA_OMP_TARGET(...) PRAGMA_ACC_SERIAL(__VA_ARGS__)

///
/// @brief finalize the omp target region
/// @details replace to _Pragma("acc end serial")
///
#define PRAGMA_OMP_END_TARGET PRAGMA_ACC_END_SERIAL

///
/// @brief _Pragma("omp target update [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc update [clause [[,] clause] ... ]"); stops when no clause has an OpenACC counterpart
///
#define PRAGMA_OMP_TARGET_UPDATE(...) SOLOMON_INTERNAL_O2A_CHECKED(SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE, SOLOMON_INTERNAL_O2A_STOP_UPDATE, PRAGMA_ACC_UPDATE, __VA_ARGS__)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Synchronization constructs
///

///
/// @brief _Pragma("omp taskwait [clause [[,] clause] ... ]")
/// @details replace to _Pragma("acc wait"): the clauses (depend, nowait) have no OpenACC counterpart and are disregarded; waiting for all the queues covers them
///
#define PRAGMA_OMP_TARGET_TASKWAIT(...) PRAGMA_ACC(wait)

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
#define PRAGMA_OMP_TARGET_PARALLEL(...) SOLOMON_INTERNAL_O2A_LAUNCH(__VA_ARGS__)
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
#define PRAGMA_OMP_TARGET_PARALLEL_FOR(...) SOLOMON_INTERNAL_O2A_OFFLOADING(__VA_ARGS__)
///
/// @brief _Pragma("omp target parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop construct and no other statements.
/// @note clause: Clauses used for target or parallel do except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO(...) SOLOMON_INTERNAL_O2A_OFFLOADING(__VA_ARGS__)

///
/// @brief _Pragma("omp target parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel for simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...) SOLOMON_INTERNAL_O2A_OFFLOADING(ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))
///
/// @brief _Pragma("omp target parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct with a parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Clauses used for target or parallel do simd except for copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...) SOLOMON_INTERNAL_O2A_OFFLOADING(ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))

///
/// @brief _Pragma("omp target parallel loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a parallel loop construct and no other statements.
/// @note clause: Clauses used for target or parallel loop except copyin.
///
#define PRAGMA_OMP_TARGET_PARALLEL_LOOP(...) SOLOMON_INTERNAL_O2A_OFFLOADING(__VA_ARGS__)

///
/// @brief _Pragma("omp target simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a simd construct and no other statements.
/// @note clause: Any clause used for target or simd.
///
/// the loop construct carries the clauses of the loop (as the other OpenMP loop directives with simd)
#define PRAGMA_OMP_TARGET_SIMD(...) SOLOMON_INTERNAL_O2A_OFFLOADING(ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))
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
#define PRAGMA_OMP_TARGET_TEAMS(...) SOLOMON_INTERNAL_O2A_LAUNCH(__VA_ARGS__)
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
/// the loop is shared by the gangs (as SOLOMON_OFFLOAD_OUTER_LOOP); independent as OpenMP distribute (the kernels construct does not assume it)
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...) SOLOMON_INTERNAL_O2A_LAUNCH(__VA_ARGS__) PRAGMA_ACC_LOOP(ACC_CLAUSE_GANG, ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))

///
/// @brief _Pragma("omp target teams distribute simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute simd construct and no other statements.
/// @note clause: Any clause used for target or teams distribute simd.
///
/// the loop construct carries the clauses of the loop (as the other OpenMP loop directives with simd)
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...) SOLOMON_INTERNAL_O2A_OFFLOADING(ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))

///
/// @brief _Pragma("omp target teams loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams loop construct and no other statements.
/// @note clause: Any clause used for target or teams loop.
///
#define PRAGMA_OMP_TARGET_TEAMS_LOOP(...) SOLOMON_INTERNAL_O2A_OFFLOADING(__VA_ARGS__)

///
/// @brief _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel for and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) SOLOMON_INTERNAL_O2A_OFFLOADING(__VA_ARGS__)
///
/// @brief _Pragma("omp target teams distribute parallel do [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing teams distribute parallel do and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...) SOLOMON_INTERNAL_O2A_OFFLOADING(__VA_ARGS__)

///
/// @brief _Pragma("omp target teams distribute parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel for simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) SOLOMON_INTERNAL_O2A_OFFLOADING(ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))
///
/// @brief _Pragma("omp target teams distribute parallel do simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a target construct containing a teams distribute parallel worksharing-loop SIMD construct and no other statements.
/// @note clause: Any clause used for target or teams distribute parallel do simd.
///
#define PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...) SOLOMON_INTERNAL_O2A_OFFLOADING(ACC_CLAUSE_INDEPENDENT SOLOMON_APPEND_ARGS(__VA_ARGS__))

///
/// @brief finalize the OpenMP loop directives converted to OpenACC (Fortran: "!$acc end kernels" or "!$acc end parallel"; nothing in C/C++)
///
#define PRAGMA_OMP_END_TARGET_PARALLEL_DO SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_PARALLEL_DO_SIMD SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_PARALLEL_LOOP SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_LOOP SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO SOLOMON_INTERNAL_O2A_END_LOOP
#define PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD SOLOMON_INTERNAL_O2A_END_LOOP

#endif  // !defined(SOLOMON_CONVERT_OMP2ACC_DIRECTIVE_HPP)
