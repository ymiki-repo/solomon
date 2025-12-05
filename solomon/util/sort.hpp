///
/// @file solomon/util/sort.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief Sort clauses with priority order
///
/// @note Priority levels:
///       0 = default (lowest priority)
///       1 = highest priority
///       2 = higher priority
///       3 = high priority
///       (larger numbers = lower priority)
///
/// @copyright Copyright (c) 2025 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_SORT_HPP)
#define SOLOMON_UTIL_SORT_HPP

#include "va_args.hpp"
#include "cloak.hpp"

///
/// @brief Define priority levels for clauses
/// 0 = default (lowest priority)
/// 1 = highest priority (e.g., simd)
/// 2 = higher priority
/// 3 = high priority
///

// Priority level 1 (highest) - simd and equivalents
#define PRIORITY_LEVEL_simd 1

// // Priority level 2 (higher) - example clauses
// #define PRIORITY_LEVEL_nowait 2
// #define PRIORITY_LEVEL_ordered 2

// Priority level 3 (high) - example clauses
// #define PRIORITY_LEVEL_some_clause 3

// Default priority level (used when PRIORITY_LEVEL_<clause> is not defined)
// This is handled by the probe trick returning 0


///
/// @brief Check if clause has priority > 0 (non-default)
/// Since 0 is default, anything > 0 needs sorting
///
#define HAS_PRIORITY(clause) UTIL_CLOAK_NOT(IS_PRIORITY_LEVEL_0(clause))

// updated APPEND_CLAUSE implementation (works with automatic clause sorting)
#define APPEND_CLAUSE_0(clause)
#define APPEND_CLAUSE_1(clause) clause,
#define APPEND_CLAUSE(flag, clause) UTIL_CLOAK_CAT(APPEND_CLAUSE_, flag)(clause)

#define REMOVE_ALL_PRIORITIZED_IMPL(...) UTIL_CLOAK_CAT(REMOVE_ALL_PRIORITIZED_IMPL_, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)
#define REMOVE_ALL_PRIORITIZED(...) REMOVE_ALL_PRIORITIZED_IMPL(__VA_ARGS__)

#include "impl/sort_clause.hpp"// include SORT_CLAUSES_IMPL_N(...) and their building blocks from auto-generated file

///
/// @brief Main entry point
///
#define SORT_CLAUSES(...) SORT_CLAUSES_IMPL(__VA_ARGS__)

///
/// @brief Strip trailing comma helper
///
#define _STRIP_TRAILING_COMMA(N, ...) UTIL_CLOAK_CAT(STRIP_TRAILING_COMMA_, N)(__VA_ARGS__)
#define STRIP_TRAILING_COMMA(...) _STRIP_TRAILING_COMMA(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

///
/// @brief Main entry point with output formatting
///
#define SORT_AND_OUTPUT(...) STRIP_TRAILING_COMMA(SORT_CLAUSES(__VA_ARGS__))

#endif  // !defined(SOLOMON_UTIL_SORT_HPP)
