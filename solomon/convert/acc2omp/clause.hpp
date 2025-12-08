///
/// @file solomon/convert/acc2omp/clause.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief convert OpenACC clauses to OpenMP clauses
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_CONVERT_ACC2OMP_CLAUSE_HPP)
#define SOLOMON_CONVERT_ACC2OMP_CLAUSE_HPP

#include "../../util/missing.hpp"

///
/// @brief add OpenACC clause "if (condition)" if applicable
/// @details replace to OpenMP clause "if (condition)"
///
#define ACC_CLAUSE_IF(condition) OMP_TARGET_CLAUSE_IF(condition)

///
/// @brief add OpenACC clause "self [(condition)]" or "self (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_SELF(...) MISSING_CLAUSE

///
/// @brief add OpenACC clause "default (none | present)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_DEFAULT(mode) MISSING_CLAUSE

///
/// @brief add OpenACC clause "default (none)" if applicable
/// @details replace to OpenMP clause "defaultmap (none)"
///
#define ACC_CLAUSE_DEFAULT_NONE OMP_TARGET_CLAUSE_DEFAULTMAP_NONE

///
/// @brief add OpenACC clause "default (present)" if applicable
/// @details replace to OpenMP clause "defaultmap (present)"
///
#define ACC_CLAUSE_DEFAULT_PRESENT OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT

///
/// @brief add OpenACC clause "device_type ([* | device_type_list])" if applicable
/// @details replace to OpenMP clause "device_type (host | nohost | any)"
///
#define ACC_CLAUSE_DEVICE_TYPE(...) OMP_TARGET_CLAUSE_DEVICE_TYPE(__VA_ARGS__)

///
/// @brief add OpenACC clause "async [(expression)]" if applicable
/// @details replace to OpenMP clause "nowait"
///
#define ACC_CLAUSE_ASYNC(...) OMP_TARGET_CLAUSE_NOWAIT

///
/// @brief add OpenACC clause "wait [(expression_list)]" if applicable
/// @details replace to OpenMP clause "depend (in: list)"
///
#define ACC_CLAUSE_WAIT(...) OMP_TARGET_CLAUSE_DEPEND_IN(__VA_ARGS__)

///
/// @brief add OpenACC clause "finalize" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_FINALIZE MISSING_CLAUSE

///
/// @brief add OpenACC clause "num_gangs (n)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_NUM_GANGS(n) MISSING_CLAUSE

///
/// @brief add OpenACC clause "num_workers (n)" if applicable
/// @details replace to OpenMP clause "num_teams (n)"
///
#define ACC_CLAUSE_NUM_WORKERS(n) OMP_TARGET_CLAUSE_NUM_TEAMS(n)

///
/// @brief add OpenACC clause "vector_length (n)" if applicable
/// @details replace to OpenMP clause "thread_limit (n)"
///
#define ACC_CLAUSE_VECTOR_LENGTH(n) OMP_TARGET_CLAUSE_THREAD_LIMIT(n)

///
/// @brief add OpenACC clause "reduction (operator: list)" if applicable
/// @details replace to OpenMP clause "reduction (operator: list)"
///
#define ACC_CLAUSE_REDUCTION(...) OMP_TARGET_CLAUSE_REDUCTION(__VA_ARGS__)

///
/// @brief add OpenACC clause "private (list)" if applicable
/// @details replace to OpenMP clause "private (list)"
///
#define ACC_CLAUSE_PRIVATE(...) OMP_TARGET_CLAUSE_PRIVATE(__VA_ARGS__)

///
/// @brief add OpenACC clause "firstprivate (list)" if applicable
/// @details replace to OpenMP clause "firstprivate (list)"
///
#define ACC_CLAUSE_FIRSTPRIVATE(...) OMP_TARGET_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)

///
/// @note Data Clauses in OpenACC API 2.7 Reference Guide (https://www.openacc.org/sites/default/files/inline-files/API%20Guide%202.7.pdf)
///

///
/// @brief add OpenACC clause "copy (list)" if applicable
/// @details replace to OpenMP clause "map (tofrom: list)"
///
#define ACC_CLAUSE_COPY(...) OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)

///
/// @brief add OpenACC clause "copyin ([readonly:] list)" if applicable
/// @details replace to OpenMP clause "map (to: list)"
///
#define ACC_CLAUSE_COPYIN(...) OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)

///
/// @brief add OpenACC clause "copyout (list)" if applicable
/// @details replace to OpenMP clause "map (from: list)"
///
#define ACC_CLAUSE_COPYOUT(...) OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)

///
/// @brief add OpenACC clause "create (list)" if applicable
/// @details replace to OpenMP clause "map (alloc: list)"
///
#define ACC_CLAUSE_CREATE(...) OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)

///
/// @brief add OpenACC clause "no_create (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_NO_CREATE(...) MISSING_CLAUSE

///
/// @brief add OpenACC clause "delete (list)" if applicable
/// @details replace to OpenMP clause "map (delete: list)"
///
#define ACC_CLAUSE_DELETE(...) OMP_TARGET_CLAUSE_MAP_DELETE(__VA_ARGS__)

///
/// @brief add OpenACC clause "present (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_PRESENT(...) MISSING_CLAUSE

///
/// @brief add OpenACC clause "deviceptr (list)" if applicable
/// @details replace to OpenMP clause "is_device_ptr (list)"
///
#define ACC_CLAUSE_DEVICEPTR(...) OMP_TARGET_CLAUSE_IS_DEVICE_PTR(__VA_ARGS__)

///
/// @brief add OpenACC clause "attach (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_ATTACH(...) MISSING_CLAUSE

///
/// @brief add OpenACC clause "detach (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_DETACH(...) MISSING_CLAUSE

#if _OPENMP >= 201811
///
/// @brief add OpenACC clause "use_device (list)" if applicable
/// @details replace to OpenMP clause "use_device_addr (list)"
/// @note use_device_addr is introduced in OpenMP 5.0 (_OPENMP = 201811) and use_device_ptr is deprecated
///
#define ACC_CLAUSE_USE_DEVICE(...) OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(__VA_ARGS__)
#else  // _OPENMP >= 201811
///
/// @brief add OpenACC clause "use_device (list)" if applicable
/// @details replace to OpenMP clause "use_device_ptr (list)"
///
#define ACC_CLAUSE_USE_DEVICE(...) OMP_TARGET_CLAUSE_USE_DEVICE_PTR(__VA_ARGS__)
#endif  // _OPENMP >= 201811

///
/// @brief add OpenACC clause "if_present" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_IF_PRESENT MISSING_CLAUSE

///
/// @brief add OpenACC clause "collapse (n)" if applicable
/// @details replace to OpenMP clause "collapse (n)"
///
#define ACC_CLAUSE_COLLAPSE(n) OMP_TARGET_CLAUSE_COLLAPSE(n)

///
/// @brief add OpenACC clause "seq" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_SEQ MISSING_CLAUSE

///
/// @brief add OpenACC clause "auto" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_AUTO MISSING_CLAUSE

///
/// @brief add OpenACC clause "independent" if applicable
/// @details replace to OpenMP clause "simd"
///
#define ACC_CLAUSE_INDEPENDENT OMP_TARGET_CLAUSE_SIMD

///
/// @brief add OpenACC clause "tile (expression_list)" if applicable
/// @details ignore the clause (_Pragma("omp tile sizes (size_list)") might be the correspondence)
///
#define ACC_CLAUSE_TILE(...) MISSING_CLAUSE

///
/// @brief add OpenACC clause "gang" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_GANG MISSING_CLAUSE

///
/// @brief add OpenACC clause "worker" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_WORKER MISSING_CLAUSE

///
/// @brief add OpenACC clause "vector" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_VECTOR MISSING_CLAUSE

///
/// @brief add OpenACC clause "read" if applicable (read: v = x;)
/// @details replace to OpenMP clause "read"
///
#define ACC_CLAUSE_READ OMP_TARGET_CLAUSE_READ

///
/// @brief add OpenACC clause "write" if applicable (write: x = expr;)
/// @details replace to OpenMP clause "write"
///
#define ACC_CLAUSE_WRITE OMP_TARGET_CLAUSE_WRITE

///
/// @brief add OpenACC clause "update" if applicable (default mode: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
/// @details replace to OpenMP clause "update"
///
#define ACC_CLAUSE_UPDATE OMP_TARGET_CLAUSE_UPDATE

///
/// @brief add OpenACC clause "capture" if applicable (v = update-expr, where update-expr is one of: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
/// @details replace to OpenMP clause "capture"
///
#define ACC_CLAUSE_CAPTURE OMP_TARGET_CLAUSE_CAPTURE

///
/// @brief add OpenACC clause "host (list)" if applicable
/// @details replace to OpenMP clause "from (list)"
///
#define ACC_CLAUSE_HOST(...) OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)

///
/// @brief add OpenACC clause "device (list)" if applicable
/// @details replace to OpenMP clause "to (list)"
///
#define ACC_CLAUSE_DEVICE(...) OMP_TARGET_CLAUSE_TO(__VA_ARGS__)

///
/// @brief pass additional list if applicable
/// @details replace to OpenMP correspondence
///
#define ACC_PASS_LIST(...) OMP_TARGET_PASS_LIST(__VA_ARGS__)

///
/// @brief add OpenACC clause "bind (name or string)" if applicable
/// @details ignore the clause (_Pragma("acc routine [clause [[,] clause]...]") ==>> _Pragma("omp declare target [clause [[,] clause]...]"): enter, indirect, link are candidates)
///
#define ACC_CLAUSE_BIND(arg) MISSING_CLAUSE

///
/// @brief add OpenACC clause "nohost" if applicable
/// @details replace to OpenMP clause "device_type (nohost)"
///
#define ACC_CLAUSE_NOHOST OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)

///
/// @brief add OpenACC clause "device_resident (list)" if applicable
/// @details ignore the clause (declare directive is also ignored)
///
#define ACC_CLAUSE_DEVICE_RESIDENT(...) AS_MISSING_CLAUSE(missing)

///
/// @brief add OpenACC clause "link (list)" if applicable
/// @details ignore the clause (declare directive is also ignored)
///
#define ACC_CLAUSE_LINK(...) MISSING_CLAUSE

#endif  // !defined(SOLOMON_CONVERT_ACC2OMP_CLAUSE_HPP)
