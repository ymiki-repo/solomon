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
/// @details acc_device_default is acc_device_nvidia in NVIDIA HPC SDK, and environmental variable (NVCOMPILER_ACC_DEVICE_TYPE) can control it
///
/// @return number of accelerator devices
///
#if __cplusplus > 201103L
static constexpr auto get_num_devices_for_offloading = []() { return acc_get_num_devices(acc_device_default); };
#else  //__cplusplus > 201103L
#define get_num_devices_for_offloading() acc_get_num_devices(acc_device_default)
#endif  //__cplusplus > 201103L

///
/// @brief set default accelerator device
///
/// @param[in] id device ID
///
#if __cplusplus > 201103L
static constexpr auto set_default_device_for_offloading = [](const auto id) { acc_set_device_num(id, acc_device_default); };
#else  //__cplusplus > 201103L
#define set_default_device_for_offloading(id) acc_set_device_num(id, acc_device_default)
#endif  //__cplusplus > 201103L

///
/// @brief get default accelerator device
///
/// @return device ID
///
#if __cplusplus > 201103L
static constexpr auto get_default_device_for_offloading = []() { return acc_get_device_num(acc_device_default); };
#else  //__cplusplus > 201103L
#define get_default_device_for_offloading() acc_get_device_num(acc_device_default)
#endif  //__cplusplus > 201103L

///
/// @brief get ID of CUDA Stream
///
/// @param[in] async identifier of stream or asynchronicity
///
#if __cplusplus > 201103L
static constexpr auto get_offloaded_stream_id = [](const auto async) { acc_get_cuda_stream(async); };
#else  //__cplusplus > 201103L
#define get_offloaded_stream_id(async) acc_get_cuda_stream(async)
#endif  //__cplusplus > 201103L

///
/// @brief get ID of asynchronous CUDA Stream
///
#if __cplusplus > 201103L
static constexpr auto get_offloaded_stream_id_async = []() { get_offloaded_stream_id(acc_async_noval); };
#else  //__cplusplus > 201103L
#define get_offloaded_stream_id_async() get_offloaded_stream_id(acc_async_noval)
#endif  //__cplusplus > 201103L

///
/// @brief get ID of synchronous CUDA Stream
///
#if __cplusplus > 201103L
static constexpr auto get_offloaded_stream_id_sync = []() { get_offloaded_stream_id(acc_async_sync); };
#else  //__cplusplus > 201103L
#define get_offloaded_stream_id_sync() get_offloaded_stream_id(acc_async_sync)
#endif  //__cplusplus > 201103L

#endif  // !defined(SOLOMON_ACC_RUNTIME_HPP)
