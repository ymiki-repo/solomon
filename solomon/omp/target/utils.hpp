///
/// @file solomon/omp/target/utils.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief utilities to ease OpenMP target implementation
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_TARGET_UTILS_HPP)
#define SOLOMON_OMP_TARGET_UTILS_HPP

///
/// @brief launch offloaded region as default mode
///
#define PRAGMA_OMP_TARGET_LAUNCH_DEFAULT(...) PRAGMA_OMP_TARGET_TEAMS(__VA_ARGS__)

///
/// @brief offload the specified loop as default mode
///
#if !defined(OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE) && (_OPENMP >= 201811)
#define PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(...) PRAGMA_OMP_TARGET_TEAMS_LOOP(__VA_ARGS__)
#else  //! defined(OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE) && (_OPENMP >= 201811)
#define PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(...) PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(__VA_ARGS__)
#endif  //! defined(OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE) && (_OPENMP >= 201811)

///
/// @brief _Pragma("omp target data use_device_ptr (list)")
///
#define PRAGMA_OMP_TARGET_DATA_USE_DEVICE_PTR(...) PRAGMA_OMP_TARGET_DATA(OMP_TARGET_CLAUSE_USE_DEVICE_PTR(__VA_ARGS__))

///
/// @brief _Pragma("omp target data use_device_addr (list)")
///
#define PRAGMA_OMP_TARGET_DATA_USE_DEVICE_ADDR(...) PRAGMA_OMP_TARGET_DATA(OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(__VA_ARGS__))

///
/// @brief _Pragma("omp target update from (list)")
/// @remarks memcpy_D2H
///
#define PRAGMA_OMP_TARGET_UPDATE_FROM(...) PRAGMA_OMP_TARGET_UPDATE(OMP_TARGET_CLAUSE_FROM(__VA_ARGS__))

///
/// @brief _Pragma("omp target update to (list)")
/// @remarks memcpy_H2D
///
#define PRAGMA_OMP_TARGET_UPDATE_TO(...) PRAGMA_OMP_TARGET_UPDATE(OMP_TARGET_CLAUSE_TO(__VA_ARGS__))

///
/// @brief _Pragma("omp target enter data map (alloc: list)")
/// @remarks malloc
///
#define PRAGMA_OMP_TARGET_ENTER_DATA_MAP_ALLOC(...) PRAGMA_OMP_TARGET_ENTER_DATA(OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__))

///
/// @brief _Pragma("omp target enter data map (to: list)")
///
#define PRAGMA_OMP_TARGET_ENTER_DATA_MAP_TO(...) PRAGMA_OMP_TARGET_ENTER_DATA(OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__))

///
/// @brief _Pragma("omp target exit data map (delete: list)")
/// @remarks free
///
#define PRAGMA_OMP_TARGET_EXIT_DATA_MAP_DELETE(...) PRAGMA_OMP_TARGET_EXIT_DATA(OMP_TARGET_CLAUSE_MAP_DELETE(__VA_ARGS__))

///
/// @brief _Pragma("omp target exit data map (from: list)")
/// @remarks free
///
#define PRAGMA_OMP_TARGET_EXIT_DATA_MAP_FROM(...) PRAGMA_OMP_TARGET_EXIT_DATA(OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__))

// ///
// /// @brief offload the specified loop as thread-blocks
// ///
// #if OMP_TARGET_HAS_LOOP_DIRECTIVE
// #define PRAGMA_OMP_TARGET_LOOP(...) PRAGMA_OMP_TARGET(teams loop __VA_ARGS__)
// // #if __AMD__ == 1
// // #if defined(__AMD__)
// #if !defined(__NVCOMPILER)
// // in amdclang++, loop construct cannot include simd clause
// #undef OPTARG_SIMD
// #define OPTARG_SIMD
// #endif  //
// #else   // OMP_TARGET_HAS_LOOP_DIRECTIVE
// #define PRAGMA_OMP_TARGET_LOOP(...) PRAGMA_OMP_TARGET(teams distribute parallel for __VA_ARGS__)
// #endif  // OMP_TARGET_HAS_LOOP_DIRECTIVE

#define PRAGMA_OMP_TARGET_ATOMIC_UPDATE PRAGMA_OMP_TARGET_ATOMIC(OMP_TARGET_CLAUSE_UPDATE)
#define PRAGMA_OMP_TARGET_ATOMIC_READ PRAGMA_OMP_TARGET_ATOMIC(OMP_TARGET_CLAUSE_READ)
#define PRAGMA_OMP_TARGET_ATOMIC_WRITE PRAGMA_OMP_TARGET_ATOMIC(OMP_TARGET_CLAUSE_WRITE)
#define PRAGMA_OMP_TARGET_ATOMIC_CAPTURE PRAGMA_OMP_TARGET_ATOMIC(OMP_TARGET_CLAUSE_CAPTURE)

#endif  // !defined(SOLOMON_OMP_TARGET_UTILS_HPP)
