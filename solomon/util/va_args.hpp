///
/// @file solomon/util/va_args.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @author Francesco Pretto (for implementation of SOLOMON_INTERNAL_NUM_ARGS_IMPL and SOLOMON_INTERNAL_NUM_ARGS)
/// @brief manipulation of variadic arguments
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_VA_ARGS_HPP)
#define SOLOMON_UTIL_VA_ARGS_HPP

#if __cplusplus >= 202002L
// __VA_OPT__ is C++20 feature (https://cpprefjp.github.io/lang/cpp20/va_opt.html)
#define SOLOMON_APPEND_ARGS(...) __VA_OPT__(, ) __VA_ARGS__
#else  //__cplusplus >= 202002L
// fallback implementation: C++17 or lower do not have __VA_OPT__
// #if defined(__GNUC__)
// use GCC extension
#define SOLOMON_APPEND_ARGS(...) , ##__VA_ARGS__
// #endif  // defined(__GNUC__)
#endif  //__cplusplus >= 202002L

#include "impl/num_args.hpp"// include SOLOMON_INTERNAL_NUM_ARGS(...) from auto-generated file
#include "impl/retrieve_args.hpp"// include SOLOMON_INTERNAL_EXTRACT_ARGS_N(...) and SOLOMON_INTERNAL_RETRIEVE_ARGS_N(...) from auto-generated file

#define SOLOMON_INTERNAL_ARGS_WITH_NUM(...) SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__) SOLOMON_APPEND_ARGS(__VA_ARGS__)

#define SOLOMON_INTERNAL_EXTRACT_ARGS_IMPL(N, ...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_EXTRACT_ARGS_, N)(__VA_ARGS__)
///
/// @brief extract first N arguments
///
#define SOLOMON_INTERNAL_EXTRACT_ARGS(...) SOLOMON_INTERNAL_EXTRACT_ARGS_IMPL(__VA_ARGS__)

#define SOLOMON_INTERNAL_RETRIEVE_ARGS_IMPL(N, ...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_RETRIEVE_ARGS_, N)(__VA_ARGS__)
///
/// @brief retrieve (N + 1)-th and latter arguments
///
#define SOLOMON_INTERNAL_RETRIEVE_ARGS(...) SOLOMON_INTERNAL_RETRIEVE_ARGS_IMPL(__VA_ARGS__)

#endif  // !defined(SOLOMON_UTIL_VA_ARGS_HPP)
