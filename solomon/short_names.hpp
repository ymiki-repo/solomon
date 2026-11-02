///
/// @file solomon/short_names.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief unprefixed spellings of the user-facing macros for v1.x compatibility, including the spellings that existed only in the pre-release version of v2.0.0 (the fortran branch; "pre-release only" in misc/migrate/MIGRATION.md)
///
/// @note enabled only when SOLOMON_WITH_SHORT_NAMES is defined (default OFF); new code should use the SOLOMON_-prefixed macros
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_SHORT_NAMES_HPP)
#define SOLOMON_SHORT_NAMES_HPP

// directive macros
#define OFFLOAD(...) SOLOMON_OFFLOAD(__VA_ARGS__)
#define END_OFFLOAD SOLOMON_END_OFFLOAD
#define OFFLOAD_OUTER_LOOP(...) SOLOMON_OFFLOAD_OUTER_LOOP(__VA_ARGS__)
#define PARALLELIZE_INNER_LOOP(...) SOLOMON_PARALLELIZE_INNER_LOOP(__VA_ARGS__)
#define END_OFFLOAD_OUTER_LOOP SOLOMON_END_OFFLOAD_OUTER_LOOP
#define IF_NOT_OFFLOADED(arg) SOLOMON_IF_NOT_OFFLOADED(arg)
#define DECLARE_OFFLOADED(...) SOLOMON_DECLARE_OFFLOADED(__VA_ARGS__)
#define DECLARE_OFFLOADED_END SOLOMON_DECLARE_OFFLOADED_END
#define SYNCHRONIZE(...) SOLOMON_SYNCHRONIZE(__VA_ARGS__)
#define WAIT_QUEUE(id) SOLOMON_WAIT_QUEUE(id)
#define ATOMIC(...) SOLOMON_ATOMIC(__VA_ARGS__)
#define ATOMIC_UPDATE SOLOMON_ATOMIC_UPDATE
#define ATOMIC_READ SOLOMON_ATOMIC_READ
#define ATOMIC_WRITE SOLOMON_ATOMIC_WRITE
#define ATOMIC_CAPTURE SOLOMON_ATOMIC_CAPTURE
#define DATA_ACCESS_BY_HOST(...) SOLOMON_DATA_ACCESS_BY_HOST(__VA_ARGS__)
#define DATA_ACCESS_BY_DEVICE(...) SOLOMON_DATA_ACCESS_BY_DEVICE(__VA_ARGS__)
#define USE_DEVICE_DATA_FROM_HOST(...) SOLOMON_USE_DEVICE_DATA_FROM_HOST(__VA_ARGS__)
#define MALLOC_ON_DEVICE(...) SOLOMON_MALLOC_ON_DEVICE(__VA_ARGS__)
#define ALLOCATE_ON_DEVICE(...) SOLOMON_ALLOCATE_ON_DEVICE(__VA_ARGS__)
#define FREE_FROM_DEVICE(...) SOLOMON_FREE_FROM_DEVICE(__VA_ARGS__)
#define DEALLOCATE_ON_DEVICE(...) SOLOMON_DEALLOCATE_ON_DEVICE(__VA_ARGS__)
#define MEMCPY_D2H(...) SOLOMON_MEMCPY_D2H(__VA_ARGS__)
#define MEMCPY_H2D(...) SOLOMON_MEMCPY_H2D(__VA_ARGS__)
#define DECLARE_DATA_ON_DEVICE(...) SOLOMON_DECLARE_DATA_ON_DEVICE(__VA_ARGS__)

// clause tokens
#define AS_INDEPENDENT SOLOMON_CLAUSE_INDEPENDENT
#define AS_SEQUENTIAL SOLOMON_CLAUSE_SEQUENTIAL
#define NUM_THREADS(n) SOLOMON_CLAUSE_NUM_THREADS(n)
#define NUM_BLOCKS(n) SOLOMON_CLAUSE_NUM_BLOCKS(n)
#define AS_THREAD SOLOMON_CLAUSE_THREAD
#define AS_BLOCK SOLOMON_CLAUSE_BLOCK
#define COLLAPSE(n) SOLOMON_CLAUSE_COLLAPSE(n)
#define AS_ASYNC(...) SOLOMON_CLAUSE_ASYNC(__VA_ARGS__)
#define ASYNC_QUEUE(id) SOLOMON_CLAUSE_ASYNC_QUEUE(id)
#define REDUCTION(...) SOLOMON_CLAUSE_REDUCTION(__VA_ARGS__)
#define ENABLE_IF(condition) SOLOMON_CLAUSE_IF(condition)
#define AS_PRIVATE(...) SOLOMON_CLAUSE_PRIVATE(__VA_ARGS__)
#define AS_FIRSTPRIVATE(...) SOLOMON_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)
#define AS_DEVICE_PTR(...) SOLOMON_CLAUSE_DEVICE_PTR(__VA_ARGS__)
#define COPY_BEFORE_AND_AFTER_EXEC(...) SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(__VA_ARGS__)
#define COPY_H2D_BEFORE_EXEC(...) SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(__VA_ARGS__)
#define COPY_D2H_AFTER_EXEC(...) SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(__VA_ARGS__)

///
/// @brief deprecated alias of NUM_BLOCKS
/// @deprecated NUM_GRIDS(n) was renamed to NUM_BLOCKS(n) in v2.0.0
///
#define NUM_GRIDS(n) SOLOMON_CLAUSE_NUM_BLOCKS(n)
#if defined(__clang__) && (__clang_major__ >= 14)
#pragma clang deprecated(NUM_GRIDS, "NUM_GRIDS(n) was renamed to NUM_BLOCKS(n) in v2.0.0")
#endif  // defined(__clang__) && (__clang_major__ >= 14)

///
/// @brief deprecated alias of AS_BLOCK
/// @deprecated AS_GRID was renamed to AS_BLOCK in v2.0.0
///
#define AS_GRID SOLOMON_CLAUSE_BLOCK
#if defined(__clang__) && (__clang_major__ >= 14)
#pragma clang deprecated(AS_GRID, "AS_GRID was renamed to AS_BLOCK in v2.0.0")
#endif  // defined(__clang__) && (__clang_major__ >= 14)

// semi-public utility
#define APPEND_ARGS(...) SOLOMON_APPEND_ARGS(__VA_ARGS__)

// v1.x helpers to build pragmas (internal SOLOMON_INTERNAL_* names since v2.0.0); defined with the v1.x bodies,
// not as aliases, so that the arguments are stringified as written
#if defined(PRINT_GENERATED_PRAGMA)
#define PRAGMA_STR1(x) #x
#define PRAGMA_STR(x) PRAGMA_STR1(x)
#endif  // defined(PRINT_GENERATED_PRAGMA)
#if defined(SOLOMON_SERIAL_FALLBACK_HPP)
// serial execution: no pragma is emitted (as SOLOMON_INTERNAL_PRAGMA)
#if defined(PRINT_GENERATED_PRAGMA)
#define SHOW_PRAGMA(...)
#endif  // defined(PRINT_GENERATED_PRAGMA)
#define PRAGMA(...)
#else  // defined(SOLOMON_SERIAL_FALLBACK_HPP)
#if defined(PRINT_GENERATED_PRAGMA)
///
/// @brief print generated pragma
///
#define SHOW_PRAGMA(...) _Pragma(PRAGMA_STR(message("#pragma " PRAGMA_STR(__VA_ARGS__))))

///
/// @brief input arguments into _Pragma
///
#define PRAGMA(...)        \
  SHOW_PRAGMA(__VA_ARGS__) \
  _Pragma(#__VA_ARGS__)
#else  // defined(PRINT_GENERATED_PRAGMA)
///
/// @brief input arguments into _Pragma
///
#define PRAGMA(...) _Pragma(#__VA_ARGS__)
#endif  // defined(PRINT_GENERATED_PRAGMA)
#endif  // defined(SOLOMON_SERIAL_FALLBACK_HPP)

#endif  // !defined(SOLOMON_SHORT_NAMES_HPP)
