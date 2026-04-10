///
/// @file solomon/omp/target/runtime.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief runtime library routines for OpenMP target
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_TARGET_RUNTIME_HPP)
#define SOLOMON_OMP_TARGET_RUNTIME_HPP

#if !defined(SOLOMON_FORTRAN)
#include <omp.h>
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief use OpenMP runtime library in Fortran
///
#define USE_SOLOMON_RUNTIME use omp_lib
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief get number of accelerator devices
///
/// @return number of accelerator devices
///
#if __cplusplus > 201103L
static constexpr auto get_num_devices_for_offloading = []() { return omp_get_num_devices(); };
#else  //__cplusplus > 201103L
#define get_num_devices_for_offloading() omp_get_num_devices()
#endif  //__cplusplus > 201103L

///
/// @brief set default accelerator device
///
/// @param[in] id device ID
///
#if __cplusplus > 201103L
static constexpr auto set_default_device_for_offloading = [](const auto id) { omp_set_default_device(id); };
#else  //__cplusplus > 201103L
#define set_default_device_for_offloading(id) omp_set_default_device(id)
#endif  //__cplusplus > 201103L

///
/// @brief get default accelerator device
///
/// @return device ID
///
#if __cplusplus > 201103L
static constexpr auto get_default_device_for_offloading = []() { return omp_get_default_device(); };
#else  //__cplusplus > 201103L
#define get_default_device_for_offloading() omp_get_default_device()
#endif  //__cplusplus > 201103L

// extension by NVIDIA HPC SDK
#if defined(__NVCOMPILER)
///
/// @brief get ID of CUDA Stream
///
/// @param[in] async identifier of stream or asynchronicity
///
#if __cplusplus > 201103L
static constexpr auto get_offloaded_stream_id = [](const auto async) { ompx_get_cuda_stream(get_default_device_for_offloading(), async); };
#else  //__cplusplus > 201103L
#define get_offloaded_stream_id(async) ompx_get_cuda_stream(get_default_device_for_offloading(), async)
// #define get_offloaded_stream_id(async) 0
#endif  //__cplusplus > 201103L

///
/// @brief get ID of asynchronous CUDA Stream
///
#if __cplusplus > 201103L
static constexpr auto get_offloaded_stream_id_async = []() { get_offloaded_stream_id(1); };
#else  //__cplusplus > 201103L
#define get_offloaded_stream_id_async() get_offloaded_stream_id(1)
#endif  //__cplusplus > 201103L

///
/// @brief get ID of synchronous CUDA Stream
///
#if __cplusplus > 201103L
static constexpr auto get_offloaded_stream_id_sync = []() { get_offloaded_stream_id(0); };
#else  //__cplusplus > 201103L
#define get_offloaded_stream_id_sync() get_offloaded_stream_id(0)
#endif  //__cplusplus > 201103L
#endif  // defined(__NVCOMPILER)

#endif  // !defined(SOLOMON_OMP_TARGET_RUNTIME_HPP)
