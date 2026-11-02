///
/// @file solomon/convert/acc2omp/directive.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief convert OpenACC directives to OpenMP directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_CONVERT_ACC2OMP_DIRECTIVE_HPP)
#define SOLOMON_CONVERT_ACC2OMP_DIRECTIVE_HPP

#include "../../util/cloak.hpp"     // SOLOMON_INTERNAL_CLOAK_IIF
#include "../../util/is_empty.hpp"  // SOLOMON_INTERNAL_IS_EMPTY

///
/// @brief 1 when no clause of the required set (pseudo tag, see omp/clause_tag.hpp) survives the conversion to OpenMP target, 0 otherwise
/// @details A data directive of OpenMP 5.2 needs at least one clause of its set; such a directive is not emitted.
///
#define SOLOMON_INTERNAL_ACC2OMP_NO_REQUIRED(tag, ...) SOLOMON_INTERNAL_IS_EMPTY(SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(tag), __VA_ARGS__))

///
/// @brief add arguments into _Pragma("acc __VA_ARGS__")
/// @details replace to _Pragma("omp target __VA_ARGS__")
///
#define PRAGMA_ACC(...) PRAGMA_OMP_TARGET(__VA_ARGS__)

///
/// @brief _Pragma("acc parallel [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target teams loop [clause [[,] clause] ... ]") or _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_PARALLEL(...) PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)
///
/// @brief finalize the omp target offloaded region (nothing is emitted: the combined construct needs no end directive)
///
#define PRAGMA_ACC_END_PARALLEL

///
/// @brief _Pragma("acc kernels [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target teams loop [clause [[,] clause] ... ]") or _Pragma("omp target teams distribute parallel for [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_KERNELS(...) PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)
///
/// @brief finalize the omp target offloaded region (nothing is emitted: the combined construct needs no end directive)
///
#define PRAGMA_ACC_END_KERNELS

///
/// @brief _Pragma("acc serial [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target [clause [[,] clause] ... ]") for single-thread execution on the device (v2.0.0 or later; the construct was ignored before)
///
#define PRAGMA_ACC_SERIAL(...) PRAGMA_OMP_TARGET(__VA_ARGS__)
///
/// @brief finalize the serial region
///
#define PRAGMA_ACC_END_SERIAL PRAGMA_OMP_END_TARGET

///
/// @brief _Pragma("acc data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target data [clause [[,] clause] ... ]")
/// @details C/C++: nothing is emitted when no map / use_device_ptr / use_device_addr remains (OpenMP 5.2 requires one); Fortran keeps the directive because PRAGMA_ACC_END_DATA emits "end target data"
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC2OMP_NO_REQUIRED(SOLOMON_INTERNAL_TAG_OMP_REQ_DATA_ENV, __VA_ARGS__))(, PRAGMA_OMP_TARGET_DATA(__VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_DATA(...) PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)
#endif  // !defined(SOLOMON_FORTRAN)
///
/// @brief finalize the omp target data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_DATA PRAGMA_OMP_END_TARGET_DATA
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc enter data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target enter data [clause [[,] clause] ... ]"); nothing is emitted when no map remains (OpenMP 5.2 requires one)
///
#define PRAGMA_ACC_ENTER_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC2OMP_NO_REQUIRED(SOLOMON_INTERNAL_TAG_OMP_REQ_MAP, __VA_ARGS__))(, PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__))

///
/// @brief _Pragma("acc exit data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target exit data [clause [[,] clause] ... ]"); nothing is emitted when no map remains (OpenMP 5.2 requires one)
///
#define PRAGMA_ACC_EXIT_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC2OMP_NO_REQUIRED(SOLOMON_INTERNAL_TAG_OMP_REQ_MAP, __VA_ARGS__))(, PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__))

///
/// @brief _Pragma("acc host_data [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target data [clause [[,] clause] ... ]")
/// @details C/C++: nothing is emitted when no map / use_device_ptr / use_device_addr remains (OpenMP 5.2 requires one); Fortran keeps the directive because PRAGMA_ACC_END_HOST_DATA emits "end target data"
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_HOST_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC2OMP_NO_REQUIRED(SOLOMON_INTERNAL_TAG_OMP_REQ_DATA_ENV, __VA_ARGS__))(, PRAGMA_OMP_TARGET_DATA(__VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_HOST_DATA(...) PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)
#endif  // !defined(SOLOMON_FORTRAN)
///
/// @brief finalize the omp target data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_HOST_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_HOST_DATA PRAGMA_OMP_END_TARGET_DATA
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc loop [clause [[,] clause] ... ]")
/// @details ignore the construct
///
#define PRAGMA_ACC_LOOP(...)

///
/// @brief _Pragma("acc cache (list)")
/// @details ignore the directive
///
#define PRAGMA_ACC_CACHE(...)

///
/// @brief _Pragma("acc atomic [read | write | update | capture]")
/// @details replace to _Pragma("omp atomic [read | write | update | capture]")
///
#define PRAGMA_ACC_ATOMIC(...) PRAGMA_OMP_TARGET_ATOMIC(__VA_ARGS__)

///
/// @brief _Pragma("acc update [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp target update [clause [[,] clause] ... ]"); nothing is emitted when no to / from remains (OpenMP 5.2 requires one)
///
#define PRAGMA_ACC_UPDATE(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC2OMP_NO_REQUIRED(SOLOMON_INTERNAL_TAG_OMP_REQ_MOTION, __VA_ARGS__))(, PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__))

///
/// @brief _Pragma("acc wait [(expression_list)]")
/// @details replace to _Pragma("omp taskwait"); the queue IDs (any number of them) are dropped
///
#define PRAGMA_ACC_WAIT(...) PRAGMA_OMP_TARGET_TASKWAIT()

///
/// @brief _Pragma("acc routine [clause [[,] clause] ... ]")
/// @details replace to _Pragma("omp declare target [clause [[,] clause] ... ]")
///
#define PRAGMA_ACC_ROUTINE(...) PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)
///
/// @brief finalize the omp declare target region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_ROUTINE
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_ROUTINE PRAGMA_OMP_END_DECLARE_TARGET
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc declare [clause [[,] clause] ... ]")
/// @details ignore the directive
///
#define PRAGMA_ACC_DECLARE(...)

#endif  // !defined(SOLOMON_CONVERT_ACC2OMP_DIRECTIVE_HPP)
