///
/// @file solomon/util/cloak.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @author Paul Fultz II
/// @brief pickup appropriate clauses for directives
/// @note reference: https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
/// @details implementation is copied from cloak.h developed by Paul Fultz II: available at https://github.com/pfultz2/Cloak (Boost Software License, Version 1.0: https://www.boost.org/LICENSE_1_0.txt)
///
///
#if !defined(SOLOMON_UTIL_CLOAK_HPP)
#define SOLOMON_UTIL_CLOAK_HPP

// copied from cloak.h developed by Paul Fultz II: available at https://github.com/pfultz2/Cloak (Boost Software License, Version 1.0: https://www.boost.org/LICENSE_1_0.txt)
// difference from the original version: rename the name of macros

#define UTIL_CLOAK_CAT(a, ...) _UTIL_CLOAK_CAT(a, __VA_ARGS__)
#define _UTIL_CLOAK_CAT(a, ...) a##__VA_ARGS__

#define UTIL_CLOAK_COMPL(b) _UTIL_CLOAK_CAT(UTIL_CLOAK_COMPL_, b)
#define UTIL_CLOAK_COMPL_0 1
#define UTIL_CLOAK_COMPL_1 0

#define UTIL_CLOAK_BITAND(x) _UTIL_CLOAK_CAT(UTIL_CLOAK_BITAND_, x)
#define UTIL_CLOAK_BITAND_0(y) 0
#define UTIL_CLOAK_BITAND_1(y) y

#define UTIL_CLOAK_BITOR(x) _UTIL_CLOAK_CAT(UTIL_CLOAK_BITOR_, x)
#define UTIL_CLOAK_BITOR_0(y) y
#define UTIL_CLOAK_BITOR_1(y) 1

#define UTIL_CLOAK_CHECK_N(x, n, ...) n
#define UTIL_CLOAK_CHECK(...) UTIL_CLOAK_CHECK_N(__VA_ARGS__, 0, )
#define UTIL_CLOAK_PROBE(x) x, 1,

#define UTIL_CLOAK_IS_PAREN(x) UTIL_CLOAK_CHECK(UTIL_CLOAK_IS_PAREN_PROBE x)
#define UTIL_CLOAK_IS_PAREN_PROBE(...) UTIL_CLOAK_PROBE(~)

#define UTIL_CLOAK_NOT(x) UTIL_CLOAK_CHECK(_UTIL_CLOAK_CAT(UTIL_CLOAK_NOT_, x))
#define UTIL_CLOAK_NOT_0 UTIL_CLOAK_PROBE(~)

#define UTIL_CLOAK_BOOL(x) UTIL_CLOAK_COMPL(UTIL_CLOAK_NOT(x))

#define UTIL_CLOAK_IIF(c) _UTIL_CLOAK_CAT(UTIL_CLOAK_IIF_, c)
#define UTIL_CLOAK_IIF_0(t, ...) __VA_ARGS__
#define UTIL_CLOAK_IIF_1(t, ...) t

#define UTIL_CLOAK_IF(c) UTIL_CLOAK_IIF(UTIL_CLOAK_BOOL(c))

#define UTIL_CLOAK_EAT(...)

#define _UTIL_CLOAK_COMPARE(x, y) UTIL_CLOAK_IS_PAREN(COMPARE_##x(COMPARE_##y)(()))

#define UTIL_CLOAK_IS_COMPARABLE(x) UTIL_CLOAK_IS_PAREN(UTIL_CLOAK_CAT(COMPARE_, x)(()))

#define UTIL_CLOAK_NOT_EQUAL(x, y) UTIL_CLOAK_IIF(UTIL_CLOAK_BITAND(UTIL_CLOAK_IS_COMPARABLE(x))(UTIL_CLOAK_IS_COMPARABLE(y)))(_UTIL_CLOAK_COMPARE, 1 UTIL_CLOAK_EAT)(x, y)

#define UTIL_CLOAK_EQUAL(x, y) UTIL_CLOAK_COMPL(UTIL_CLOAK_NOT_EQUAL(x, y))

#endif  // !defined(SOLOMON_UTIL_CLOAK_HPP)
