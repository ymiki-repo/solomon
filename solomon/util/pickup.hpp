///
/// @file solomon/util/pickup.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief pickup appropriate clauses for directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_PICKUP_HPP)
#define SOLOMON_UTIL_PICKUP_HPP

#include "cloak.hpp"
#include "va_args.hpp"
#include "sort.hpp"

///
/// @brief logical or
///
#define PICKUP_INTERNAL_BITOR(x) _UTIL_CLOAK_CAT(PICKUP_INTERNAL_BITOR_, x)
#define PICKUP_INTERNAL_BITOR_0(y) y
#define PICKUP_INTERNAL_BITOR_1(y) 1

///
/// @brief flag matching
///
#define PICKUP_INTERNAL_FLAG_MATCHING(directive_tag, ...) UTIL_CLOAK_CAT(PICKUP_INTERNAL_FLAG_MATCHING_, NUM_ARGS(__VA_ARGS__))(directive_tag, __VA_ARGS__)

#include "impl/pickup_clause.hpp"// include PICKUP_CLAUSE_N(...), PICKUP_INTERNAL_EXPAND_FLAGS_N(...), and PICKUP_INTERNAL_FLAG_MATCHING_N(...) from auto-generated file
#include "impl/check_clause.hpp"// include APPEND_CLAUSES(...) from auto-generated file

// original APPEND_CLAUSE implementation (works without automatic clause sorting)
// #define APPEND_CLAUSE(flag, clause) UTIL_CLOAK_IF(flag)(clause)

#define _APPEND_MATCHED_CLAUSE(NUM, ...) UTIL_CLOAK_CAT(PICKUP_CLAUSE_, NUM)(__VA_ARGS__)
#define APPEND_MATCHED_CLAUSE(...) _APPEND_MATCHED_CLAUSE(__VA_ARGS__)

// N, dtag1, dtag2, ..., dtagN, clause1, Nc1, ctag1_1, ..., ctag1_Nc1, clause2, Nc2, ctag2_1, ..., ctag2_Nc2, ...
#define PICKUP_INTERNAL_DIRECTIVE_TAGS(...) EXTRACT_ARGS(__VA_ARGS__)
#define PICKUP_INTERNAL_CLAUSE_ARGS(...) RETRIEVE_ARGS(__VA_ARGS__)
#define _PICKUP_INTERNAL_CLAUSE_CANDIDATE(clause, ...) clause, EXTRACT_ARGS(__VA_ARGS__)
#define PICKUP_INTERNAL_CLAUSE_CANDIDATE(...) _PICKUP_INTERNAL_CLAUSE_CANDIDATE(__VA_ARGS__)
#define _PICKUP_INTERNAL_RETRIEVE_CLAUSE_CANDIDATE(clause, ...) RETRIEVE_ARGS(__VA_ARGS__)
#define PICKUP_INTERNAL_RETRIEVE_CLAUSE_CANDIDATE(...) _PICKUP_INTERNAL_RETRIEVE_CLAUSE_CANDIDATE(__VA_ARGS__)

#define COMPARE_0(x) x
#define NOT_VOID(...) UTIL_CLOAK_NOT(UTIL_CLOAK_EQUAL(0, NUM_ARGS(__VA_ARGS__)))
#define CHECK_CLAUSE(...) UTIL_CLOAK_IF(NOT_VOID(PICKUP_INTERNAL_CLAUSE_ARGS(__VA_ARGS__)))(APPEND_MATCHED_CLAUSE(ARGS_WITH_NUM(PICKUP_INTERNAL_DIRECTIVE_TAGS(__VA_ARGS__)), PICKUP_INTERNAL_CLAUSE_CANDIDATE(PICKUP_INTERNAL_CLAUSE_ARGS(__VA_ARGS__))))

#endif  // !defined(SOLOMON_UTIL_PICKUP_HPP)
