///
/// @file solomon/util/va_opt.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief detection of __VA_OPT__ (defines SOLOMON_INTERNAL_USE_VA_OPT when it is used)
/// @details __VA_OPT__ is used when the language version provides it (C23 / C++20 or later), when the compiler is not
/// in a strict ISO mode (__STRICT_ANSI__ is not defined), or with clang-based compilers; it is then probed before use.
/// Otherwise (gcc / g++ in a strict mode with an older version, e.g. -std=c99 or -std=c++17), no macro containing
/// __VA_OPT__ is defined (gcc warns about such a definition under -pedantic), and the users of SOLOMON_INTERNAL_USE_VA_OPT
/// fall back to their implementation without __VA_OPT__.
///
/// @copyright Copyright (c) 2026 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_VA_OPT_HPP)
#define SOLOMON_UTIL_VA_OPT_HPP

#if !defined(__STRICT_ANSI__) || defined(__clang__) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ > 201710L)) || (defined(__cplusplus) && (__cplusplus >= 202002L))
#define SOLOMON_INTERNAL_VA_OPT_THIRD(a, b, c, ...) c
#define SOLOMON_INTERNAL_VA_OPT_CHECK(...) SOLOMON_INTERNAL_VA_OPT_THIRD(__VA_OPT__(, ), 1, 0, )
#if SOLOMON_INTERNAL_VA_OPT_CHECK(?)
#define SOLOMON_INTERNAL_USE_VA_OPT 1
///
/// @brief select t when the variadic arguments are not empty, f otherwise (defined only with SOLOMON_INTERNAL_USE_VA_OPT)
///
#define SOLOMON_INTERNAL_VA_OPT_SELECT(t, f, ...) SOLOMON_INTERNAL_VA_OPT_THIRD(__VA_OPT__(, ), t, f, )
#endif  // SOLOMON_INTERNAL_VA_OPT_CHECK(?)
#endif  // conditions to use __VA_OPT__

#endif  // !defined(SOLOMON_UTIL_VA_OPT_HPP)
