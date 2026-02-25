///
/// @file solomon/acc/utils.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief utilities to ease OpenACC implementation
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_ACC_UTILS_HPP)
#define SOLOMON_ACC_UTILS_HPP

///
/// @brief _Pragma("acc kernels [clause [[,] clause] ... ]") _Pragma("acc loop [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_KERNELS_LOOP(...) PRAGMA_ACC_KERNELS(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)

///
/// @brief _Pragma("acc parallel [clause [[,] clause] ... ]") _Pragma("acc loop [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_PARALLEL_LOOP(...) PRAGMA_ACC_PARALLEL(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)

///
/// @brief launch offloaded region as default mode
///
#if !defined(OFFLOAD_BY_OPENACC_PARALLEL)
#define PRAGMA_ACC_LAUNCH_DEFAULT(...) PRAGMA_ACC_KERNELS(__VA_ARGS__)
#else  //! defined(OFFLOAD_BY_OPENACC_PARALLEL)
#define PRAGMA_ACC_LAUNCH_DEFAULT(...) PRAGMA_ACC_PARALLEL(__VA_ARGS__)
#endif  //! defined(OFFLOAD_BY_OPENACC_PARALLEL)

///
/// @brief offload the specified loop as default mode
///
#define PRAGMA_ACC_OFFLOADING_DEFAULT(...) PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)

///
/// @brief _Pragma("acc host_data use_device (list)")
///
#define PRAGMA_ACC_HOST_DATA_USE_DEVICE(...) PRAGMA_ACC_HOST_DATA(ACC_CLAUSE_USE_DEVICE(__VA_ARGS__))

///
/// @brief _Pragma("acc update host (list)")
///
#define PRAGMA_ACC_UPDATE_HOST(...) PRAGMA_ACC_UPDATE(ACC_CLAUSE_HOST(__VA_ARGS__))

///
/// @brief _Pragma("acc update device (list)")
///
#define PRAGMA_ACC_UPDATE_DEVICE(...) PRAGMA_ACC_UPDATE(ACC_CLAUSE_DEVICE(__VA_ARGS__))

///
/// @brief _Pragma("acc enter data create (list)")
///
#define PRAGMA_ACC_ENTER_DATA_CREATE(...) PRAGMA_ACC_ENTER_DATA(ACC_CLAUSE_CREATE(__VA_ARGS__))

///
/// @brief _Pragma("acc exit data delete (list)")
///
#define PRAGMA_ACC_EXIT_DATA_DELETE(...) PRAGMA_ACC_EXIT_DATA(ACC_CLAUSE_DELETE(__VA_ARGS__))

///
/// @brief _Pragma("acc data present (list)")
///
#if defined(OFFLOAD_BY_OPENACC)
#define PRAGMA_ACC_DATA_PRESENT(...) PRAGMA_ACC_DATA(ACC_CLAUSE_PRESENT(__VA_ARGS__))
#else  // defined(OFFLOAD_BY_OPENACC)
#define PRAGMA_ACC_DATA_PRESENT(...)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief _Pragma("acc enter data copyin (list)")
///
#define PRAGMA_ACC_ENTER_DATA_COPYIN(...) PRAGMA_ACC_ENTER_DATA(ACC_CLAUSE_COPYIN(__VA_ARGS__))

///
/// @brief _Pragma("acc exit data copyout (list)")
///
#define PRAGMA_ACC_EXIT_DATA_COPYOUT(...) PRAGMA_ACC_EXIT_DATA(ACC_CLAUSE_COPYOUT(__VA_ARGS__))

#define PRAGMA_ACC_ATOMIC_UPDATE PRAGMA_ACC_ATOMIC(ACC_CLAUSE_UPDATE)
#define PRAGMA_ACC_ATOMIC_READ PRAGMA_ACC_ATOMIC(ACC_CLAUSE_READ)
#define PRAGMA_ACC_ATOMIC_WRITE PRAGMA_ACC_ATOMIC(ACC_CLAUSE_WRITE)
#define PRAGMA_ACC_ATOMIC_CAPTURE PRAGMA_ACC_ATOMIC(ACC_CLAUSE_CAPTURE)

#endif  // !defined(SOLOMON_ACC_UTILS_HPP)
