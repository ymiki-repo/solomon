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

#if defined(PRINT_GENERATED_PRAGMA)
#define PRAGMA_STR1(x) #x
#define PRAGMA_STR(x) PRAGMA_STR1(x)

#if defined(__clang__) || defined(__GNUC__) || defined(__NVCOMPILER)
#define SOLOMON_EMIT_WARNING(msg) _Pragma(SOLOMON_STRINGIFY(GCC warning msg))
#else//defined(__clang__) || defined(__GNUC__) || defined(__NVCOMPILER)
#define SOLOMON_EMIT_WARNING(msg) _Pragma(SOLOMON_STRINGIFY(message msg))
#endif//defined(__clang__) || defined(__GNUC__) || defined(__NVCOMPILER)

///
/// @brief print generated pragma
///
#define SHOW_PRAGMA(...) _Pragma(PRAGMA_STR(message("#pragma " PRAGMA_STR(__VA_ARGS__))))

///
/// @brief input arguments into _Pragma
///
#define PRAGMA(...)     \
  _Pragma(#__VA_ARGS__) \
      SHOW_PRAGMA(__VA_ARGS__)
#else  // defined(PRINT_GENERATED_PRAGMA)
///
/// @brief input arguments into _Pragma
///
#define PRAGMA(...) _Pragma(#__VA_ARGS__)
#endif  // defined(PRINT_GENERATED_PRAGMA)

#endif  // !defined(SOLOMON_PRAGMA_HPP)
