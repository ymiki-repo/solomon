///
/// @file solomon/util/missing.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief tag for missing clauses
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_UTIL_MISSING_HPP)
#define SOLOMON_UTIL_MISSING_HPP

#include "va_args.hpp"

///
/// @brief matching macro to be ignored
///
#define COMPARE_SOLOMON_INTERNAL_TAG_MISSING(x) x

///
/// @brief missing clause
///
#define SOLOMON_INTERNAL_MISSING_CLAUSE missing, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_MISSING)

#endif  // !defined(SOLOMON_UTIL_MISSING_HPP)
