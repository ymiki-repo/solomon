///
/// @file solomon/omp/runtime.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief runtime library routines for OpenMP
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_RUNTIME_HPP)
#define SOLOMON_OMP_RUNTIME_HPP

#if !defined(SOLOMON_FORTRAN)
#include <omp.h>
#else  // !defined(SOLOMON_FORTRAN)
///
/// @brief use OpenMP runtime library in Fortran
///
#define USE_SOLOMON_RUNTIME use omp_lib
#endif  // !defined(SOLOMON_FORTRAN)

// runtime functions in fallback mode (OpenMP for multicore CPU): the OpenMP device routines are used as they are;
// get_offloaded_stream_id* are not defined since no stream of an offloading device is available

///
/// @brief get number of accelerator devices (fallback mode: number of non-host devices that the OpenMP runtime reports)
///
/// @return number of accelerator devices
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_num_devices_for_offloading = []() { return omp_get_num_devices(); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_num_devices_for_offloading() omp_get_num_devices()
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief set default accelerator device (fallback mode: omp_set_default_device)
///
/// @param[in] id device ID
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto set_default_device_for_offloading = [](const auto id) { omp_set_default_device(id); };
#elif !defined(SOLOMON_FORTRAN)
#define set_default_device_for_offloading(id) omp_set_default_device(id)
#else  // !defined(SOLOMON_FORTRAN)
// Fortran: the default integer kind, since gfortran rejects other kinds (nvfortran and ifx accept them)
#define set_default_device_for_offloading(id) omp_set_default_device(int(id))
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief get default accelerator device (fallback mode: omp_get_default_device)
///
/// @return device ID
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_default_device_for_offloading = []() { return omp_get_default_device(); };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_default_device_for_offloading() omp_get_default_device()
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

#endif  // !defined(SOLOMON_OMP_RUNTIME_HPP)
