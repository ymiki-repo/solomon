///
/// @file solomon/util/nest_map.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief nested clauses and Solomon map modifiers inside OMP_TARGET_CLAUSE_MAP(...)
/// @details SOLOMON_INTERNAL_NEST_MAP(pfx, raw, ...) is the body of OMP_TARGET_CLAUSE_MAP(...) in every backend.
/// When the arguments start with a nested clause (OMP_TARGET_CLAUSE_TO(list), OMP_TARGET_CLAUSE_FROM(list),
/// OMP_TARGET_CLAUSE_TO_PRESENT(list), OMP_TARGET_CLAUSE_FROM_PRESENT(list), and ACC_CLAUSE_DEVICE / ACC_CLAUSE_HOST)
/// or with Solomon map modifiers (OMP_TARGET_MAP_MODIFIER_*), the modifiers are collected and every nested clause
/// becomes one clause of its own through the backend's rewrites (<pfx>APPLY, <pfx>N_<clause>, <pfx>P_<clause>);
/// otherwise the arguments go to raw(...) unchanged.
///   - A nested clause carries the flag SOLOMON_INTERNAL_TAG_NESTABLE_IN_MAP (or ..._PRESENT) as its FIRST tag, and a
///     modifier carries SOLOMON_INTERNAL_TAG_MODIFIER_IN_MAP; the test calls the third element of the arguments (the
///     first tag of an element) with a magic argument, so user tokens are never pasted.
///   - A non-nested list is scanned from its fourth element (up to SOLOMON_INTERNAL_NEST_MAX elements) by the same
///     call, and a nested clause or a modifier after it stops the map (<pfx>RAWMIX), since the backends that drop a
///     raw map would drop the nested clause with it.
///   - Consequence of these tests: when the third element of a non-nested list, or a later one (up to the 35th), is
///     the name of a function-like macro that cannot be invoked with one argument, or whose expansion with one argument
///     has a comma at the top level, preprocessing fails (move that element to the first or second place, or write an
///     array section).
///   - A Solomon modifier is two elements: a mark that stops the build when the modifier is written outside the map
///     (SOLOMON_INTERNAL_NEST_STOP_OUTSIDE, selected by the directives), then its payload
///     SOLOMON_INTERNAL_NEST_MODIFIER(m) that no directive selects; inside the map the mark is dropped.
///   - An element after the nested clauses that is not a nested clause stops the map (<pfx>MIXED), a nested clause
///     or a modifier after a raw list stops it (<pfx>RAWMIX), modifiers without a nested clause after them stop it as
///     well (<pfx>NOCLAUSE), and so do more modifiers than the stages
///     (<pfx>TOOMANY). These stops are the Solomon errors below (C/C++: a compiler error with a message; Fortran: a
///     SOLOMON_ERROR_* name), and the rest of the map is dropped.
/// The backend supplies <pfx>MODINIT, <pfx>MODADD(mods, ...), <pfx>APPLY(mods, ...), <pfx>N_to / N_from / N_device /
/// N_host, <pfx>P_to / P_from / P_device / P_host, <pfx>MIXED(...), <pfx>RAWMIX(...), <pfx>NOCLAUSE(...),
/// <pfx>TOOMANY(...) and <pfx>SOLOMON_INTERNAL_NEST_MODIFIER(m).
/// The numbers of stages (nested clauses and modifiers) are limited by impl/nest_map.hpp.
///
/// @copyright Copyright (c) 2026 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_NEST_MAP_HPP)
#define SOLOMON_UTIL_NEST_MAP_HPP

#include "cloak.hpp"
#include "impl/nest_map.hpp"  // the stages per nested clause and per modifier from auto-generated file
#include "stringify.hpp"      // SOLOMON_INTERNAL_PICKUP_STOP
#include "va_args.hpp"

// the nested clauses must cover the clause limit of one directive (regenerate with the same --max: README)
#if defined(SOLOMON_INTERNAL_CHECK_CLAUSE_MAX) && (SOLOMON_INTERNAL_NEST_MAX < SOLOMON_INTERNAL_CHECK_CLAUSE_MAX)
#error "Solomon: regenerate check_clause.jl, sort_clause.jl, dedup_wait.jl, and nest_map.jl in solomon/util/jl with the same --max (README)"
#endif

// the Solomon errors of a map (C/C++: a compiler error with the message; Fortran: a name that the compiler rejects)
#if defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_NEST_STOP_OUTSIDE(...) SOLOMON_ERROR_MODIFIER_OUTSIDE_MAP(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_STOP_NOCLAUSE SOLOMON_ERROR_MODIFIER_WITHOUT_NESTED_CLAUSE_IN_MAP
#define SOLOMON_INTERNAL_NEST_STOP_MIXED SOLOMON_ERROR_RAW_LIST_AFTER_NESTED_CLAUSE_IN_MAP
#define SOLOMON_INTERNAL_NEST_STOP_RAWMIX SOLOMON_ERROR_NESTED_CLAUSE_OR_MODIFIER_AFTER_RAW_LIST_IN_MAP
#else  // defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_NEST_STOP_OUTSIDE(...) _Pragma("GCC error \"Solomon: a map modifier (OMP_TARGET_MAP_MODIFIER_*) is written outside OMP_TARGET_CLAUSE_MAP(...); write it first inside OMP_TARGET_CLAUSE_MAP(...)\"")
#define SOLOMON_INTERNAL_NEST_STOP_NOCLAUSE _Pragma("GCC error \"Solomon: map modifiers (OMP_TARGET_MAP_MODIFIER_*) in OMP_TARGET_CLAUSE_MAP(...) need a nested clause after them (OMP_TARGET_CLAUSE_TO(...) / OMP_TARGET_CLAUSE_FROM(...))\"")
#define SOLOMON_INTERNAL_NEST_STOP_MIXED _Pragma("GCC error \"Solomon: only nested clauses (OMP_TARGET_CLAUSE_TO(...) / OMP_TARGET_CLAUSE_FROM(...)) may follow a nested clause in OMP_TARGET_CLAUSE_MAP(...); write the other list in another OMP_TARGET_CLAUSE_MAP(...)\"")
#define SOLOMON_INTERNAL_NEST_STOP_RAWMIX _Pragma("GCC error \"Solomon: nested clauses (OMP_TARGET_CLAUSE_TO(...) / OMP_TARGET_CLAUSE_FROM(...)) and map modifiers (OMP_TARGET_MAP_MODIFIER_*) must come first in OMP_TARGET_CLAUSE_MAP(...); write the raw list in another OMP_TARGET_CLAUSE_MAP(...)\"")
#endif  // defined(SOLOMON_FORTRAN)
// clang-format off: the text of the message is kept as written (clang-format would put spaces around the colon) (permanent)
#define SOLOMON_INTERNAL_NEST_STOP_TOOMANY SOLOMON_INTERNAL_PICKUP_STOP(SOLOMON_ERROR_TOO_MANY_MODIFIERS_IN_MAP, Solomon: too many map modifiers (limit SOLOMON_INTERNAL_NEST_MODS_MAX) in one OMP_TARGET_CLAUSE_MAP)
// clang-format on

///
/// @brief the two elements of a Solomon map modifier m (see above); tags: the directives that select the mark
///
#define SOLOMON_INTERNAL_NEST_MODIFIER_ELEMENTS(m, ...) SOLOMON_INTERNAL_NEST_STOP_OUTSIDE(m), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_MODIFIER_IN_MAP, __VA_ARGS__), SOLOMON_INTERNAL_NEST_MODIFIER(m), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_MODIFIER_IN_MAP)

// flags (pseudo tags placed as the FIRST tag of an element); function-like so that they are probed without pasting
#define SOLOMON_INTERNAL_TAG_NESTABLE_IN_MAP(x) SOLOMON_INTERNAL_NEST_KEY_##x
#define SOLOMON_INTERNAL_TAG_NESTABLE_IN_MAP_PRESENT(x) SOLOMON_INTERNAL_NEST_KEY_##x
#define SOLOMON_INTERNAL_NEST_KEY_SOLOMON_INTERNAL_NEST_MAGIC ~, 1,
#define SOLOMON_INTERNAL_TAG_MODIFIER_IN_MAP(x) SOLOMON_INTERNAL_NEST_MKEY_##x
#define SOLOMON_INTERNAL_NEST_MKEY_SOLOMON_INTERNAL_NEST_MMAGIC ~, 1,
#define SOLOMON_INTERNAL_NEST_END(x) SOLOMON_INTERNAL_NEST_ENDKEY_##x
#define SOLOMON_INTERNAL_NEST_ENDKEY_SOLOMON_INTERNAL_NEST_MAGIC ~, 1,
// probe of the scan of a raw list: a flag of a nested clause or of a modifier, or the end mark (SMAGIC); the end mark alone (EMAGIC)
#define SOLOMON_INTERNAL_NEST_KEY_SOLOMON_INTERNAL_NEST_SMAGIC ~, 1,
#define SOLOMON_INTERNAL_NEST_MKEY_SOLOMON_INTERNAL_NEST_SMAGIC ~, 1,
#define SOLOMON_INTERNAL_NEST_ENDKEY_SOLOMON_INTERNAL_NEST_SMAGIC ~, 1,
#define SOLOMON_INTERNAL_NEST_ENDKEY_SOLOMON_INTERNAL_NEST_EMAGIC ~, 1,
#define SOLOMON_INTERNAL_NEST_IS_ANY(e) SOLOMON_INTERNAL_CLOAK_CHECK(e(SOLOMON_INTERNAL_NEST_SMAGIC))
#define SOLOMON_INTERNAL_NEST_SCAN_HIT(e, ...) SOLOMON_INTERNAL_CLOAK_COMPL(SOLOMON_INTERNAL_CLOAK_CHECK(e(SOLOMON_INTERNAL_NEST_EMAGIC)))

// kind of a nested clause, from its flag (pasted only after the flag was probed): <pfx>N_<clause> or <pfx>P_<clause>;
// the pieces are pasted in one replacement list, so no intermediate token is exposed to the user's macros
#define SOLOMON_INTERNAL_NEST_KIND_SOLOMON_INTERNAL_TAG_NESTABLE_IN_MAP(pfx, ...) SOLOMON_INTERNAL_NEST_PASTE3(pfx, N_, __VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_KIND_SOLOMON_INTERNAL_TAG_NESTABLE_IN_MAP_PRESENT(pfx, ...) SOLOMON_INTERNAL_NEST_PASTE3(pfx, P_, __VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_PASTE3(a, b, ...) a##b##__VA_ARGS__

// own paste for the backend rewrites (they run inside SOLOMON_INTERNAL_CLOAK_CAT, which is disabled there)
#define SOLOMON_INTERNAL_NEST_CAT(a, ...) SOLOMON_INTERNAL_NEST_CAT_I(a, __VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_CAT_I(a, ...) a##__VA_ARGS__
#define SOLOMON_INTERNAL_NEST_E1(a, ...) a
#define SOLOMON_INTERNAL_NEST_E1_X(...) SOLOMON_INTERNAL_NEST_E1(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_E3(a, b, c, ...) c
#define SOLOMON_INTERNAL_NEST_E3_X(...) SOLOMON_INTERNAL_NEST_E3(__VA_ARGS__)

// 1 when the first element is a nested clause / a Solomon modifier / the end mark, 0 otherwise
#define SOLOMON_INTERNAL_NEST_IS_NESTED(...) SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_NEST_E3_X(__VA_ARGS__, ~, ~, ~)(SOLOMON_INTERNAL_NEST_MAGIC))
#define SOLOMON_INTERNAL_NEST_IS_MOD(...) SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_NEST_E3_X(__VA_ARGS__, ~, ~, ~)(SOLOMON_INTERNAL_NEST_MMAGIC))
#define SOLOMON_INTERNAL_NEST_IS_END(...) SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_NEST_E1_X(__VA_ARGS__, ~)(SOLOMON_INTERNAL_NEST_MAGIC))

#define SOLOMON_INTERNAL_NEST_EAT(...)
#define SOLOMON_INTERNAL_NEST_RAW(pfx, raw, ...) raw(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_MIXED(pfx, raw, mods, ...) SOLOMON_INTERNAL_CLOAK_CAT(pfx, MIXED)(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_NOCLAUSE(pfx, raw, mods, ...) SOLOMON_INTERNAL_CLOAK_CAT(pfx, NOCLAUSE)(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_TOOMANY(pfx, raw, mods, ...) SOLOMON_INTERNAL_CLOAK_CAT(pfx, TOOMANY)(__VA_ARGS__)

///
/// @brief entry: raw(args...) unless the arguments start with a nested clause or a Solomon modifier
///
#define SOLOMON_INTERNAL_NEST_MAP(pfx, raw, ...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_CLOAK_BITOR(SOLOMON_INTERNAL_NEST_IS_NESTED(__VA_ARGS__))(SOLOMON_INTERNAL_NEST_IS_MOD(__VA_ARGS__)))(SOLOMON_INTERNAL_NEST_BEGIN, SOLOMON_INTERNAL_NEST_RAWCHK)(pfx, raw, __VA_ARGS__)
// a raw list: stops when a nested clause or a modifier follows it; the flag of the first one is the fourth element or later
// (raw element, clause text, number of tags, flag), so the scan starts there (the third element is probed above) and looks
// at up to SOLOMON_INTERNAL_NEST_MAX elements (SCAN_k in impl/nest_map.hpp)
#define SOLOMON_INTERNAL_NEST_RAWCHK(pfx, raw, ...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_NEST_SCAN(~, SOLOMON_INTERNAL_NEST_DROP3(__VA_ARGS__, SOLOMON_INTERNAL_NEST_END, SOLOMON_INTERNAL_NEST_END, SOLOMON_INTERNAL_NEST_END)))(SOLOMON_INTERNAL_NEST_RAWMIX, SOLOMON_INTERNAL_NEST_RAW)(pfx, raw, __VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_DROP3(a, b, c, ...) __VA_ARGS__
#define SOLOMON_INTERNAL_NEST_SCAN(...) SOLOMON_INTERNAL_NEST_SCAN_1(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_RAWMIX(pfx, raw, ...) SOLOMON_INTERNAL_CLOAK_CAT(pfx, RAWMIX)(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_BEGIN(pfx, raw, ...) SOLOMON_INTERNAL_NEST_MODS_1(pfx, raw, SOLOMON_INTERNAL_CLOAK_CAT(pfx, MODINIT), __VA_ARGS__, SOLOMON_INTERNAL_NEST_END)

// after the modifiers, a nested clause must follow
#define SOLOMON_INTERNAL_NEST_FIRST(pfx, raw, mods, ...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_NEST_IS_NESTED(__VA_ARGS__))(SOLOMON_INTERNAL_NEST_STEP_1, SOLOMON_INTERNAL_NEST_NOCLAUSE)(pfx, raw, mods, __VA_ARGS__)

// one nested clause: <pfx>APPLY(mods, <pfx><kind><clause tokens>)
#define SOLOMON_INTERNAL_NEST_ONE(pfx, mods, c, f) SOLOMON_INTERNAL_CLOAK_CAT(pfx, APPLY)(mods, SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_NEST_KIND_, f)(pfx, c))

#endif  // !defined(SOLOMON_UTIL_NEST_MAP_HPP)
