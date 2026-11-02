///
/// @file solomon/acc/runtime.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief runtime library routines for OpenACC
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_ACC_RUNTIME_HPP)
#define SOLOMON_ACC_RUNTIME_HPP

#if !defined(SOLOMON_FORTRAN)
#include <openacc.h>
#else  // defined(SOLOMON_FORTRAN)
///
/// @brief use OpenACC runtime library in Fortran
///
#define USE_SOLOMON_RUNTIME use openacc
#endif  // defined(SOLOMON_FORTRAN)

///
/// @brief get number of accelerator devices
/// @details acc_device_default is the default device type: an NVIDIA GPU in NVIDIA HPC SDK (-acc), which the environment variable NVCOMPILER_ACC_DEVICE_TYPE (NVIDIA, MULTICORE, or HOST) can change
///
/// @return number of accelerator devices
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_num_devices_for_offloading = []() { return acc_get_num_devices(acc_device_default); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_num_devices_for_offloading() acc_get_num_devices(acc_device_default)
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief set default accelerator device
///
/// @param[in] id device ID
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto set_default_device_for_offloading = [](const auto id) { acc_set_device_num(id, acc_device_default); };
#elif !defined(SOLOMON_FORTRAN)
#define set_default_device_for_offloading(id) acc_set_device_num(id, acc_device_default)
#else  // !defined(SOLOMON_FORTRAN)
// Fortran: the default integer kind, since gfortran and nvfortran reject other kinds (integer(8), integer(2))
#define set_default_device_for_offloading(id) acc_set_device_num(int(id), acc_device_default)
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief get default accelerator device
///
/// @return device ID
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_default_device_for_offloading = []() { return acc_get_device_num(acc_device_default); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_default_device_for_offloading() acc_get_device_num(acc_device_default)
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief get ID of CUDA Stream
///
/// @param[in] async identifier of stream or asynchronicity
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_offloaded_stream_id = [](const auto async) { return acc_get_cuda_stream(async); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_offloaded_stream_id(async) acc_get_cuda_stream(async)
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief get ID of asynchronous CUDA Stream
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_offloaded_stream_id_async = []() { return get_offloaded_stream_id(acc_async_noval); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_offloaded_stream_id_async() get_offloaded_stream_id(acc_async_noval)
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief get ID of synchronous CUDA Stream
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_offloaded_stream_id_sync = []() { return get_offloaded_stream_id(acc_async_sync); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_offloaded_stream_id_sync() get_offloaded_stream_id(acc_async_sync)
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

#endif  // !defined(SOLOMON_ACC_RUNTIME_HPP)
