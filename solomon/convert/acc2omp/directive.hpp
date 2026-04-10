///
/// @file solomon/convert/acc2omp/directive.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief convert OpenACC directives to OpenMP directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_CONVERT_ACC2OMP_DIRECTIVE_HPP)
#define SOLOMON_CONVERT_ACC2OMP_DIRECTIVE_HPP

///
/// @brief add arguments into _Pragma("acc __VA_ARGS__")
/// @details replace to _Pragma("omp target __VA_ARGS__")
///
#define PRAGMA_ACC(...) PRAGMA_OMP_TARGET(__VA_ARGS__)

///
/// @brief _Pragma("acc parallel [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target teams loop [clause [[,] clause] ... ]") or _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_PARALLEL(...) PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)
///
/// @brief finalize the omp target offloaded region
///
#if defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_PARALLEL
#endif  // defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc kernels [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target teams loop [clause [[,] clause] ... ]") or _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_KERNELS(...) PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)
///
/// @brief finalize the omp target offloaded region
///
#if defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_KERNELS
#endif  // defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc serial [clause [[,] clause] ... ]")
/// @details ignore the construct
///
#define PRAGMA_ACC_SERIAL(...)
///
/// @brief finalize the ignored serial region (acc serial is not supported in OpenMP target backend)
///
#if defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_SERIAL
#endif  // defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target data [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_DATA(...) PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)
///
/// @brief finalize the omp target data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_DATA PRAGMA_OMP_END_TARGET_DATA
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc enter data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target enter data [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_ENTER_DATA(...) PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__)

///
/// @brief _Pragma("acc exit data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target exit data [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_EXIT_DATA(...) PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__)

///
/// @brief _Pragma("acc host_data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target data [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_HOST_DATA(...) PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)
///
/// @brief finalize the omp target data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_HOST_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_HOST_DATA PRAGMA_OMP_END_TARGET_DATA
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc loop [clause [[,] clause] ... ]")
/// @details ignore the construct
///
#define PRAGMA_ACC_LOOP(...)

///
/// @brief _Pragma("acc cache (list)")
/// @details ignore the directive
///
#define PRAGMA_ACC_CACHE(...)

///
/// @brief _Pragma("acc atomic [read | write | update | capture]")
/// @details replace to _Pragma("omp atomic [read | write | update | capture]")
///
#define PRAGMA_ACC_ATOMIC(...) PRAGMA_OMP_TARGET_ATOMIC(__VA_ARGS__)

///
/// @brief _Pragma("acc update [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target update [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_UPDATE(...) PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__)

///
/// @brief _Pragma("acc wait [(expression_list)] [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp taskwait [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_WAIT(...) PRAGMA_OMP_TARGET_TASKWAIT(__VA_ARGS__)

///
/// @brief _Pragma("acc routine [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp declare target [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_ROUTINE(...) PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)
///
/// @brief finalize the omp declare target region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_ROUTINE
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_ROUTINE PRAGMA_OMP_END_DECLARE_TARGET
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc declare [clause [[,] clause] ... ]")
/// @details ignore the directive
///
#define PRAGMA_ACC_DECLARE(...)

#endif  // !defined(SOLOMON_CONVERT_ACC2OMP_DIRECTIVE_HPP)
