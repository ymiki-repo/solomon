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
#define SOLOMON_INTERNAL_PRIORITY_LEVEL_simd 1

// // Priority level 2 (higher) - example clauses
// #define SOLOMON_INTERNAL_PRIORITY_LEVEL_nowait 2
// #define SOLOMON_INTERNAL_PRIORITY_LEVEL_ordered 2

// Priority level 3 (high) - example clauses
// #define SOLOMON_INTERNAL_PRIORITY_LEVEL_some_clause 3

// Default priority level (used when SOLOMON_INTERNAL_PRIORITY_LEVEL_<clause> is not defined)
// This is handled by the probe trick returning 0


///
/// @brief Check if clause has priority > 0 (non-default)
/// Since 0 is default, anything > 0 needs sorting
///
#define SOLOMON_INTERNAL_HAS_PRIORITY(clause) SOLOMON_INTERNAL_CLOAK_NOT(SOLOMON_INTERNAL_IS_PRIORITY_LEVEL_0(clause))

// updated SOLOMON_INTERNAL_APPEND_CLAUSE implementation (works with automatic clause sorting)
#define SOLOMON_INTERNAL_APPEND_CLAUSE_0(clause)
#define SOLOMON_INTERNAL_APPEND_CLAUSE_1(clause) clause,
#define SOLOMON_INTERNAL_APPEND_CLAUSE(flag, clause) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_APPEND_CLAUSE_, flag)(clause)

#define SOLOMON_INTERNAL_REMOVE_ALL_PRIORITIZED_IMPL(...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_REMOVE_ALL_PRIORITIZED_IMPL_, SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)
#define SOLOMON_INTERNAL_REMOVE_ALL_PRIORITIZED(...) SOLOMON_INTERNAL_REMOVE_ALL_PRIORITIZED_IMPL(__VA_ARGS__)

#include "impl/sort_clause.hpp"// include SOLOMON_INTERNAL_SORT_CLAUSES_IMPL_N(...) and their building blocks from auto-generated file

///
/// @brief Main entry point
///
#define SOLOMON_INTERNAL_SORT_CLAUSES(...) SOLOMON_INTERNAL_SORT_CLAUSES_IMPL(__VA_ARGS__)

///
/// @brief Strip trailing comma helper
///
#define SOLOMON_INTERNAL_STRIP_TRAILING_COMMA_IMPL(N, ...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_STRIP_TRAILING_COMMA_, N)(__VA_ARGS__)
#define SOLOMON_INTERNAL_STRIP_TRAILING_COMMA(...) SOLOMON_INTERNAL_STRIP_TRAILING_COMMA_IMPL(SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

///
/// @brief Main entry point with output formatting
///
#define SOLOMON_INTERNAL_SORT_AND_OUTPUT(...) SOLOMON_INTERNAL_STRIP_TRAILING_COMMA(SOLOMON_INTERNAL_SORT_CLAUSES(__VA_ARGS__))

#endif  // !defined(SOLOMON_UTIL_SORT_HPP)
