///
/// @file solomon/util/stringify.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief stringification, compile-time warning, and the stop of the build from inside a directive, defined in every configuration (serial execution included)
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_STRINGIFY_HPP)
#define SOLOMON_UTIL_STRINGIFY_HPP

///
/// @brief stringify the argument as written
///
#define SOLOMON_INTERNAL_STRINGIFY_IMPL(x) #x

///
/// @brief stringify the argument after macro expansion
///
#define SOLOMON_INTERNAL_STRINGIFY(x) SOLOMON_INTERNAL_STRINGIFY_IMPL(x)

///
/// @brief stop the build from inside a directive
/// @details C/C++: a preprocessor / compiler error with the message (msg: tokens without commas, stringified after macro expansion);
/// Fortran (cpp does not execute a nested _Pragma): the name (name: starts with SOLOMON_ERROR_) that the Fortran compiler rejects
///
#if defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_PICKUP_STOP(name, msg) name
#else  // defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_PICKUP_STOP(name, msg) _Pragma(SOLOMON_INTERNAL_STRINGIFY(GCC error SOLOMON_INTERNAL_STRINGIFY(msg)))
#endif  // defined(SOLOMON_FORTRAN)

#if defined(PRINT_GENERATED_PRAGMA)
///
/// @brief emit a compile-time warning (enabled with PRINT_GENERATED_PRAGMA)
///
#if defined(__clang__) || defined(__GNUC__) || defined(__NVCOMPILER)
#define SOLOMON_EMIT_WARNING(msg) _Pragma(SOLOMON_INTERNAL_STRINGIFY(GCC warning msg))
#else  // defined(__clang__) || defined(__GNUC__) || defined(__NVCOMPILER)
#define SOLOMON_EMIT_WARNING(msg) _Pragma(SOLOMON_INTERNAL_STRINGIFY(message msg))
#endif  // defined(__clang__) || defined(__GNUC__) || defined(__NVCOMPILER)
#endif  // defined(PRINT_GENERATED_PRAGMA)

#endif  // !defined(SOLOMON_UTIL_STRINGIFY_HPP)
