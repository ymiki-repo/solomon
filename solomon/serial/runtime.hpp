///
/// @file solomon/serial/runtime.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief runtime functions for serial execution (neither OpenACC nor OpenMP is available)
///
/// @note no accelerator device is available: get_num_devices_for_offloading() and get_default_device_for_offloading() return 0, and set_default_device_for_offloading(id) does nothing
/// @note get_offloaded_stream_id* are not defined since no stream of an offloading device is available
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_SERIAL_RUNTIME_HPP)
#define SOLOMON_SERIAL_RUNTIME_HPP

///
/// @brief get number of accelerator devices (serial execution: always 0)
///
/// @return 0
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_num_devices_for_offloading = []() { return 0; };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_num_devices_for_offloading() 0
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief set default accelerator device (serial execution: do nothing)
/// @details C/C++ evaluates the argument and discards it; Fortran calls the intrinsic get_command_argument(int(id)) without output arguments, which has no effect, so that "call set_default_device_for_offloading(id)" stays a valid statement for an integer id of any kind
///
/// @param[in] id device ID (ignored)
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto set_default_device_for_offloading = [](const auto id) { static_cast<void>(id); };
#elif !defined(SOLOMON_FORTRAN)
#define set_default_device_for_offloading(id) ((void)(id))
#else  // !defined(SOLOMON_FORTRAN)
#define set_default_device_for_offloading(id) get_command_argument(int(id))
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

///
/// @brief get default accelerator device (serial execution: always 0)
///
/// @return 0
///
#if defined(__cplusplus) && (__cplusplus >= 201703L)
static constexpr auto get_default_device_for_offloading = []() { return 0; };
#else  // defined(__cplusplus) && (__cplusplus >= 201703L)
#define get_default_device_for_offloading() 0
#endif  // defined(__cplusplus) && (__cplusplus >= 201703L)

#endif  // !defined(SOLOMON_SERIAL_RUNTIME_HPP)
