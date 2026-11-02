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
#include "is_empty.hpp"  // SOLOMON_INTERNAL_IS_EMPTY
#include "sort.hpp"
#include "stringify.hpp"  // SOLOMON_INTERNAL_PICKUP_STOP
#include "va_args.hpp"

///
/// @brief logical or
///
#define SOLOMON_INTERNAL_PICKUP_BITOR(x) SOLOMON_INTERNAL_CLOAK_IMPL_CAT(SOLOMON_INTERNAL_PICKUP_BITOR_, x)
#define SOLOMON_INTERNAL_PICKUP_BITOR_0(y) y
#define SOLOMON_INTERNAL_PICKUP_BITOR_1(y) 1

///
/// @brief flag matching
///
#define SOLOMON_INTERNAL_PICKUP_FLAG_MATCHING(directive_tag, ...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_PICKUP_FLAG_MATCHING_, SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__))(directive_tag, __VA_ARGS__)

#include "impl/check_clause.hpp"   // include SOLOMON_INTERNAL_APPEND_CLAUSES(...) from auto-generated file
#include "impl/pickup_clause.hpp"  // include SOLOMON_INTERNAL_PICKUP_CLAUSE_N(...), SOLOMON_INTERNAL_PICKUP_EXPAND_FLAGS_N(...), SOLOMON_INTERNAL_PICKUP_FLAG_MATCHING_N(...), and SOLOMON_INTERNAL_IS_TAG_COUNT_N from auto-generated file

// the generated stages after the clause check must cover its limit (regenerate them with the same --max: README)
#if (SOLOMON_INTERNAL_SORT_CLAUSE_MAX < SOLOMON_INTERNAL_CHECK_CLAUSE_MAX) || (SOLOMON_INTERNAL_DEDUP_ARGS_MAX < SOLOMON_INTERNAL_CHECK_CLAUSE_MAX) || (defined(SOLOMON_INTERNAL_NEST_MAX) && (SOLOMON_INTERNAL_NEST_MAX < SOLOMON_INTERNAL_CHECK_CLAUSE_MAX))
#error "Solomon: regenerate check_clause.jl, sort_clause.jl, dedup_wait.jl, and nest_map.jl in solomon/util/jl with the same --max (README)"
#endif

#define SOLOMON_INTERNAL_APPEND_MATCHED_CLAUSE_IMPL(NUM, ...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_PICKUP_CLAUSE_, NUM)(__VA_ARGS__)
#define SOLOMON_INTERNAL_APPEND_MATCHED_CLAUSE(...) SOLOMON_INTERNAL_APPEND_MATCHED_CLAUSE_IMPL(__VA_ARGS__)

// N, dtag1, dtag2, ..., dtagN, clause1, Nc1, ctag1_1, ..., ctag1_Nc1, clause2, Nc2, ctag2_1, ..., ctag2_Nc2, ...
#define SOLOMON_INTERNAL_PICKUP_DIRECTIVE_TAGS(...) SOLOMON_INTERNAL_EXTRACT_ARGS(__VA_ARGS__)
#define SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(...) SOLOMON_INTERNAL_RETRIEVE_ARGS(__VA_ARGS__)
#define SOLOMON_INTERNAL_PICKUP_IMPL_CLAUSE_CANDIDATE(clause, ...) clause, SOLOMON_INTERNAL_EXTRACT_ARGS(__VA_ARGS__)
#define SOLOMON_INTERNAL_PICKUP_CLAUSE_CANDIDATE(...) SOLOMON_INTERNAL_PICKUP_IMPL_CLAUSE_CANDIDATE(__VA_ARGS__)
#define SOLOMON_INTERNAL_PICKUP_IMPL_RETRIEVE_CLAUSE_CANDIDATE(clause, ...) SOLOMON_INTERNAL_RETRIEVE_ARGS(__VA_ARGS__)

///
/// @brief 1 when the first element of the clause list is well formed (a Solomon clause: the element is followed by its tag count 0..99), 0 otherwise
/// @details A name that is not a Solomon clause macro (e.g. a misspelled name or raw clause text), and an empty element, are not followed by a tag count.
///
#define SOLOMON_INTERNAL_IS_TAG_COUNT(x) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_PICKUP_WELL_FORMED_, SOLOMON_INTERNAL_PICKUP_COUNT_KIND(x))
#define SOLOMON_INTERNAL_PICKUP_WELL_FORMED_0 0
#define SOLOMON_INTERNAL_PICKUP_WELL_FORMED_1 1
#define SOLOMON_INTERNAL_PICKUP_WELL_FORMED_2 1
#define SOLOMON_INTERNAL_PICKUP_SECOND_IS_COUNT(clause, n, ...) SOLOMON_INTERNAL_IS_TAG_COUNT(n)
#define SOLOMON_INTERNAL_PICKUP_IS_WELL_FORMED(...) SOLOMON_INTERNAL_PICKUP_SECOND_IS_COUNT(__VA_ARGS__, , )
///
/// @brief kind of the first element: 1 for a Solomon clause with tags, 2 for one without tags (no directive takes it), 0 otherwise
///
#define SOLOMON_INTERNAL_PICKUP_COUNT_KIND(x) SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_IS_TAG_COUNT_, x))
#define SOLOMON_INTERNAL_PICKUP_SECOND_KIND(clause, n, ...) SOLOMON_INTERNAL_PICKUP_COUNT_KIND(n)
#define SOLOMON_INTERNAL_PICKUP_KIND(...) SOLOMON_INTERNAL_PICKUP_SECOND_KIND(__VA_ARGS__, , )

///
/// @brief first element of the list, and 1 when the list has a single element (0 otherwise)
///
#define SOLOMON_INTERNAL_PICKUP_FIRST_(a, ...) a
#define SOLOMON_INTERNAL_PICKUP_FIRST(...) SOLOMON_INTERNAL_PICKUP_FIRST_(__VA_ARGS__, )
#define SOLOMON_INTERNAL_PICKUP_SECOND_(a, b, ...) b
#define SOLOMON_INTERNAL_PICKUP_IS_SINGLE(...) SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_PICKUP_SECOND_(__VA_ARGS__, SOLOMON_INTERNAL_CLOAK_PROBE(~), ~))
#define SOLOMON_INTERNAL_PICKUP_ZERO(...) 0
#define SOLOMON_INTERNAL_PICKUP_ID(...) __VA_ARGS__
#define SOLOMON_INTERNAL_PICKUP_FIRST_IS_EMPTY(...) SOLOMON_INTERNAL_IS_EMPTY(SOLOMON_INTERNAL_PICKUP_FIRST(__VA_ARGS__))

///
/// @brief drop the first element only (a malformed or an empty one; the clauses after it are kept); a single element becomes nothing
///
#define SOLOMON_INTERNAL_PICKUP_DROP_FIRST_(a, ...) __VA_ARGS__
#define SOLOMON_INTERNAL_PICKUP_SKIP_ONE(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_PICKUP_IS_SINGLE(__VA_ARGS__))(SOLOMON_INTERNAL_CLOAK_EAT, SOLOMON_INTERNAL_PICKUP_DROP_FIRST_)(__VA_ARGS__)

///
/// @brief 1 when the list starts with an empty element followed by more elements, 0 otherwise
/// @details Empty elements do not count toward the clause limit: SOLOMON_INTERNAL_PICKUP_DRAIN (impl/check_clause.hpp) drops the leading
/// empty elements at every stage boundary, up to the same number of steps as the clause limit.
///
#define SOLOMON_INTERNAL_PICKUP_LEAD_EMPTY_(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_PICKUP_IS_SINGLE(__VA_ARGS__))(SOLOMON_INTERNAL_PICKUP_ZERO, SOLOMON_INTERNAL_PICKUP_FIRST_IS_EMPTY)(__VA_ARGS__)
#define SOLOMON_INTERNAL_PICKUP_LEAD_EMPTY(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_PICKUP_IS_WELL_FORMED(__VA_ARGS__))(SOLOMON_INTERNAL_PICKUP_ZERO, SOLOMON_INTERNAL_PICKUP_LEAD_EMPTY_)(__VA_ARGS__)

///
/// @brief 1 when nothing is left in the (drained) list: no element or a single empty one; the stage chain stops there
///
#define SOLOMON_INTERNAL_PICKUP_IS_EXHAUSTED(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_PICKUP_IS_SINGLE(__VA_ARGS__))(SOLOMON_INTERNAL_PICKUP_FIRST_IS_EMPTY, SOLOMON_INTERNAL_PICKUP_ZERO)(__VA_ARGS__)

///
/// @brief drop the first candidate (a well-formed clause with its tags, or only the malformed element), then the leading empty elements
///
#define SOLOMON_INTERNAL_PICKUP_RETRIEVE_CLAUSE_CANDIDATE(...) SOLOMON_INTERNAL_PICKUP_DRAIN(SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_PICKUP_IS_WELL_FORMED(__VA_ARGS__))(SOLOMON_INTERNAL_PICKUP_IMPL_RETRIEVE_CLAUSE_CANDIDATE, SOLOMON_INTERNAL_PICKUP_SKIP_ONE)(__VA_ARGS__))
///
/// @brief the arguments of the first stage: the directive tags and the clause list without its leading empty elements
///
#define SOLOMON_INTERNAL_PICKUP_DRAIN_ENTRY(...) SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_PICKUP_DIRECTIVE_TAGS(__VA_ARGS__)) SOLOMON_APPEND_ARGS(SOLOMON_INTERNAL_PICKUP_DRAIN(SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(__VA_ARGS__)))

///
/// @brief mark of a malformed element (not a Solomon clause macro): stops the build and names the element
///
// clang-format off: the text of the message is kept as written (clang-format would put spaces around the colon) (permanent)
#define SOLOMON_INTERNAL_UNKNOWN_CLAUSE(x_) SOLOMON_INTERNAL_PICKUP_STOP(SOLOMON_ERROR_UNDEFINED_CLAUSE x_, Solomon: undefined name in the clause list: x_),
// clang-format on
///
/// @brief mark of a clause without tags (no directive can take it): stops the build and names the clause
///
// clang-format off: the text of the message is kept as written (clang-format would put spaces around the colon) (permanent)
#define SOLOMON_INTERNAL_UNTAGGED_CLAUSE(x_) SOLOMON_INTERNAL_PICKUP_STOP(SOLOMON_ERROR_CLAUSE_WITHOUT_TAGS x_, Solomon: clause without directive tags in the clause list: x_),
// clang-format on

///
/// @brief after the last stage (SOLOMON_INTERNAL_CHECK_CLAUSE_0): a candidate left over (beyond the clause limit) leaves the mark that stops the build
/// @details The rest has already been drained of its leading empty elements; only a single empty element (or nothing) counts as nothing left.
///
#define SOLOMON_INTERNAL_CHECK_CLAUSE_OVERFLOW_MARK(...) SOLOMON_INTERNAL_PICKUP_STOP(SOLOMON_ERROR_TOO_MANY_CLAUSES_FOR_ONE_DIRECTIVE, Solomon: too many clauses for one directive (limit SOLOMON_INTERNAL_CHECK_CLAUSE_MAX)),
#define SOLOMON_INTERNAL_CHECK_CLAUSE_OVERFLOW(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_PICKUP_IS_EXHAUSTED(__VA_ARGS__))(SOLOMON_INTERNAL_CLOAK_EAT, SOLOMON_INTERNAL_CHECK_CLAUSE_OVERFLOW_MARK)()

#define SOLOMON_INTERNAL_COMPARE_0(x) x
#define SOLOMON_INTERNAL_NOT_VOID(...) SOLOMON_INTERNAL_CLOAK_NOT(SOLOMON_INTERNAL_CLOAK_EQUAL(0, SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__)))
#define SOLOMON_INTERNAL_CHECK_CLAUSE_MATCH(...) SOLOMON_INTERNAL_APPEND_MATCHED_CLAUSE(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_PICKUP_DIRECTIVE_TAGS(__VA_ARGS__)), SOLOMON_INTERNAL_PICKUP_CLAUSE_CANDIDATE(SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(__VA_ARGS__)))
///
/// @brief an empty element is skipped silently; any other malformed element leaves the mark, and so does a clause without tags
///
#define SOLOMON_INTERNAL_CHECK_CLAUSE_UNKNOWN_IMPL(clause, ...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_IS_EMPTY(clause))(SOLOMON_INTERNAL_CLOAK_EAT, SOLOMON_INTERNAL_UNKNOWN_CLAUSE)(clause)
#define SOLOMON_INTERNAL_CHECK_CLAUSE_UNKNOWN_SPLIT(...) SOLOMON_INTERNAL_CHECK_CLAUSE_UNKNOWN_IMPL(__VA_ARGS__, )
#define SOLOMON_INTERNAL_CHECK_CLAUSE_UNKNOWN(...) SOLOMON_INTERNAL_CHECK_CLAUSE_UNKNOWN_SPLIT(SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(__VA_ARGS__))
#define SOLOMON_INTERNAL_CHECK_CLAUSE_UNTAGGED_IMPL(clause, ...) SOLOMON_INTERNAL_UNTAGGED_CLAUSE(clause)
#define SOLOMON_INTERNAL_CHECK_CLAUSE_UNTAGGED_SPLIT(...) SOLOMON_INTERNAL_CHECK_CLAUSE_UNTAGGED_IMPL(__VA_ARGS__)
#define SOLOMON_INTERNAL_CHECK_CLAUSE_UNTAGGED(...) SOLOMON_INTERNAL_CHECK_CLAUSE_UNTAGGED_SPLIT(SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(__VA_ARGS__))
#define SOLOMON_INTERNAL_CHECK_CLAUSE_SELECT_0(...) SOLOMON_INTERNAL_CHECK_CLAUSE_UNKNOWN(__VA_ARGS__)
#define SOLOMON_INTERNAL_CHECK_CLAUSE_SELECT_1(...) SOLOMON_INTERNAL_CHECK_CLAUSE_MATCH(__VA_ARGS__)
#define SOLOMON_INTERNAL_CHECK_CLAUSE_SELECT_2(...) SOLOMON_INTERNAL_CHECK_CLAUSE_UNTAGGED(__VA_ARGS__)
#define SOLOMON_INTERNAL_CHECK_CLAUSE_SELECT(...) SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_CHECK_CLAUSE_SELECT_, SOLOMON_INTERNAL_PICKUP_KIND(SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(__VA_ARGS__)))(__VA_ARGS__)
#define SOLOMON_INTERNAL_CHECK_CLAUSE(...) SOLOMON_INTERNAL_CLOAK_IF(SOLOMON_INTERNAL_NOT_VOID(SOLOMON_INTERNAL_PICKUP_CLAUSE_ARGS(__VA_ARGS__)))(SOLOMON_INTERNAL_CHECK_CLAUSE_SELECT(__VA_ARGS__))

#endif  // !defined(SOLOMON_UTIL_PICKUP_HPP)
