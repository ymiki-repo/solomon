///
/// @file solomon/pragma.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief pragma as macro
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_PRAGMA_HPP)
#define SOLOMON_PRAGMA_HPP

// SOLOMON_INTERNAL_STRINGIFY(x) and SOLOMON_EMIT_WARNING(msg)
#include "util/stringify.hpp"

// the internal names below are not defined in serial execution (serial/fallback.hpp defines them empty);
// the unprefixed v1.x names (PRAGMA, PRAGMA_STR, PRAGMA_STR1, SHOW_PRAGMA) are in short_names.hpp
#if defined(PRINT_GENERATED_PRAGMA)
///
/// @brief print generated pragma
///
#define SOLOMON_INTERNAL_SHOW_PRAGMA(...) _Pragma(SOLOMON_INTERNAL_STRINGIFY(message("#pragma " SOLOMON_INTERNAL_STRINGIFY(__VA_ARGS__))))

///
/// @brief input arguments into _Pragma
///
#define SOLOMON_INTERNAL_PRAGMA(...)        \
  SOLOMON_INTERNAL_SHOW_PRAGMA(__VA_ARGS__) \
  _Pragma(#__VA_ARGS__)
#else  // defined(PRINT_GENERATED_PRAGMA)
///
/// @brief input arguments into _Pragma
///
#define SOLOMON_INTERNAL_PRAGMA(...) _Pragma(#__VA_ARGS__)
#endif  // defined(PRINT_GENERATED_PRAGMA)

#endif  // !defined(SOLOMON_PRAGMA_HPP)
