///
/// @file solomon/util/is_empty.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief emptiness test of variadic arguments
/// @details SOLOMON_INTERNAL_IS_EMPTY(...) expands to 1 when the arguments (after macro expansion) are empty and to 0
/// otherwise. The test never pastes the argument itself, so the argument may begin with any token (e.g. "(q)", "*p", "-1").
/// When SOLOMON_INTERNAL_USE_VA_OPT is defined (va_opt.hpp: C23 / C++20 or later, a non-strict mode, or clang-based
/// compilers), the test uses __VA_OPT__ and takes any argument. Otherwise (gcc / g++ in a strict mode with an older
/// version, e.g. -std=c99 or -std=c++17), it follows the comma detection by Jens Gustedt without GNU extensions: commas
/// are looked for in the argument, in TRIGGER argument, in argument () and in TRIGGER argument (); only the empty
/// argument gives the pattern 0001.
/// Limits of the comma detection: an argument that ends with the name of a function-like macro with two or more named
/// parameters stops the preprocessor (the name is followed by "()"; write that name in parentheses, or not at the end);
/// the top-level commas it can see are limited by SOLOMON_INTERNAL_HAS_COMMA (impl/is_empty.hpp; 32 as generated): exactly
/// one more (33 commas) stops the preprocessor, and two or more are taken as a non-empty argument, which they are.
/// SOLOMON_INTERNAL_HAS_EMPTY_ELEMENT(...) expands to 1 when one of the comma-separated elements of non-empty arguments is
/// empty (as "1," or "1, , 2") and to 0 otherwise; it applies SOLOMON_INTERNAL_IS_EMPTY to every element (with the same
/// limits for each element), and does not look at arguments with more elements than SOLOMON_INTERNAL_IE_ANY_k
/// (impl/is_empty.hpp; 0 for them).
///
/// @copyright Copyright (c) 2026 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_IS_EMPTY_HPP)
#define SOLOMON_UTIL_IS_EMPTY_HPP

#include "cloak.hpp"
#include "impl/is_empty.hpp"  // SOLOMON_INTERNAL_HAS_COMMA(...) and SOLOMON_INTERNAL_IE_ANY_k(...) from auto-generated file
#include "va_args.hpp"        // SOLOMON_INTERNAL_NUM_ARGS
#include "va_opt.hpp"         // SOLOMON_INTERNAL_USE_VA_OPT and SOLOMON_INTERNAL_VA_OPT_SELECT(t, f, ...)

#if defined(SOLOMON_INTERNAL_USE_VA_OPT)
///
/// @brief 1 when the variadic arguments are empty, 0 otherwise
///
#define SOLOMON_INTERNAL_IS_EMPTY(...) SOLOMON_INTERNAL_VA_OPT_SELECT(0, 1, __VA_ARGS__)
#else  // defined(SOLOMON_INTERNAL_USE_VA_OPT)
#define SOLOMON_INTERNAL_IE_TRIGGER(...) ,
#define SOLOMON_INTERNAL_IE_PASTE5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4
#define SOLOMON_INTERNAL_IE_CASE(_0, _1, _2, _3) SOLOMON_INTERNAL_HAS_COMMA(SOLOMON_INTERNAL_IE_PASTE5(SOLOMON_INTERNAL_IE_CASE_, _0, _1, _2, _3))
#define SOLOMON_INTERNAL_IE_CASE_0001 ,

///
/// @brief 1 when the variadic arguments are empty, 0 otherwise
///
#define SOLOMON_INTERNAL_IS_EMPTY(...) SOLOMON_INTERNAL_IE_CASE(SOLOMON_INTERNAL_HAS_COMMA(__VA_ARGS__), SOLOMON_INTERNAL_HAS_COMMA(SOLOMON_INTERNAL_IE_TRIGGER __VA_ARGS__), SOLOMON_INTERNAL_HAS_COMMA(__VA_ARGS__()), SOLOMON_INTERNAL_HAS_COMMA(SOLOMON_INTERNAL_IE_TRIGGER __VA_ARGS__()))
#endif  // defined(SOLOMON_INTERNAL_USE_VA_OPT)

///
/// @brief 1 when one of the comma-separated elements of non-empty arguments is empty, 0 otherwise
///
#define SOLOMON_INTERNAL_HAS_EMPTY_ELEMENT(...) SOLOMON_INTERNAL_IE_HAS_EMPTY_N(SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)
#define SOLOMON_INTERNAL_IE_HAS_EMPTY_N(n, ...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_IE_FITS_, n)))(SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_IE_ANY_, n), 0 SOLOMON_INTERNAL_IE_EAT)(__VA_ARGS__)
#define SOLOMON_INTERNAL_IE_EAT(...)

#endif  // !defined(SOLOMON_UTIL_IS_EMPTY_HPP)
