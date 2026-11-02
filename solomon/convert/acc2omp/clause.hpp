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

#include "../../util/cloak.hpp"       // SOLOMON_INTERNAL_CLOAK_IIF
#include "../../util/dedup_wait.hpp"  // SOLOMON_INTERNAL_WAIT_STOP_EMPTY
#include "../../util/is_empty.hpp"    // SOLOMON_INTERNAL_IS_EMPTY
#include "../../util/missing.hpp"
#include "../../util/stringify.hpp"  // SOLOMON_INTERNAL_PICKUP_STOP
#include "../../util/va_args.hpp"    // SOLOMON_INTERNAL_NUM_ARGS

///
/// @brief add OpenACC clause "if (condition)" if applicable
/// @details replace to OpenMP clause "if (condition)"
///
#define ACC_CLAUSE_IF(condition) OMP_TARGET_CLAUSE_IF(condition)

///
/// @brief add OpenACC clause "self [(condition)]" or "self (list)" if applicable
/// @details replace to OpenMP clause "from (list)" for the update directive (self(list) copies from the device to the local memory)
/// and to "if (target : !(condition))" (Fortran: ".not.(condition)"; no argument: 0 / .false.) for compute constructs (a true condition runs the region on the local device);
/// each element carries the tag of its directive only, so the host fallback drops both
///
#if !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC2OMP_SELF_COND(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_IS_EMPTY(__VA_ARGS__))(0, !(__VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC2OMP_SELF_COND(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_IS_EMPTY(__VA_ARGS__))(.false., .not.(__VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)
/// without arguments it is the compute-construct form only (update needs a list, so an empty self is not counted as its required clause);
/// with two or more arguments it is the update form only (a compute construct takes one condition: the compute form stops)
#define ACC_CLAUSE_SELF(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_IS_EMPTY(__VA_ARGS__))(SOLOMON_INTERNAL_ACC2OMP_SELF_BARE, SOLOMON_INTERNAL_ACC2OMP_SELF_ARGS)(__VA_ARGS__)
#define SOLOMON_INTERNAL_ACC2OMP_SELF_ONE_1 SOLOMON_INTERNAL_CLOAK_PROBE(~)
#define SOLOMON_INTERNAL_ACC2OMP_SELF_ARGS(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_CLOAK_CAT(SOLOMON_INTERNAL_ACC2OMP_SELF_ONE_, SOLOMON_INTERNAL_NUM_ARGS(__VA_ARGS__))))(SOLOMON_INTERNAL_ACC2OMP_SELF_LIST, SOLOMON_INTERNAL_ACC2OMP_SELF_LISTN)(__VA_ARGS__)
// clang-format off: the text of the message is kept as written (permanent)
#define SOLOMON_INTERNAL_ACC2OMP_SELF_LISTN(...) OMP_TARGET_CLAUSE_FROM(__VA_ARGS__), SOLOMON_INTERNAL_PICKUP_STOP(SOLOMON_ERROR_SELF_WITH_LIST_ON_COMPUTE_CONSTRUCT, Solomon: ACC_CLAUSE_SELF with two or more arguments is a list for the update directive; a compute construct takes one condition), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)
// clang-format on
#define SOLOMON_INTERNAL_ACC2OMP_SELF_BARE(...) if (target : SOLOMON_INTERNAL_ACC2OMP_SELF_COND()), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)
#define SOLOMON_INTERNAL_ACC2OMP_SELF_LIST(...) OMP_TARGET_CLAUSE_FROM(__VA_ARGS__), if (target : SOLOMON_INTERNAL_ACC2OMP_SELF_COND(__VA_ARGS__)), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)

///
/// @brief add OpenACC clause "default (none | present)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_DEFAULT(mode) SOLOMON_INTERNAL_MISSING_CLAUSE

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
/// @details ignore the clause (also in routine): the OpenACC argument names device types, while the OpenMP device_type takes host, nohost, or any
///
#define ACC_CLAUSE_DEVICE_TYPE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "async [(expression)]" if applicable
/// @details replace to OpenMP clause "nowait"
///
#define ACC_CLAUSE_ASYNC(...) OMP_TARGET_CLAUSE_NOWAIT

///
/// @brief add OpenACC clause "wait [(expression_list)]" if applicable
/// @details replace to OpenMP clause "depend (in: list)"; without arguments the clause is dropped (no clause waits for all queues in OpenMP)
/// @note An empty argument drops the clause in every language mode (the emptiness test uses __VA_OPT__ where SOLOMON_INTERNAL_USE_VA_OPT is defined, and the portable test elsewhere).
/// An empty element of the list (a stray comma, as in ACC_CLAUSE_WAIT(1, )) stops the build with a Solomon error, as in the OpenACC backend.
///
#define ACC_CLAUSE_WAIT(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_IS_EMPTY(__VA_ARGS__))(SOLOMON_INTERNAL_MISSING_CLAUSE, SOLOMON_INTERNAL_ACC2OMP_WAIT_LIST(__VA_ARGS__))
#define SOLOMON_INTERNAL_ACC2OMP_WAIT_LIST(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_HAS_EMPTY_ELEMENT(__VA_ARGS__))(SOLOMON_INTERNAL_ACC2OMP_WAIT_EMPTY, OMP_TARGET_CLAUSE_DEPEND_IN)(__VA_ARGS__)
// the error takes the place of depend(in : ...), with its tags (OMP_CLAUSE_DEPEND)
#define SOLOMON_INTERNAL_ACC2OMP_WAIT_EMPTY(...) SOLOMON_INTERNAL_WAIT_STOP_EMPTY, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DEPOBJ, SOLOMON_INTERNAL_TAG_OMP_DISPATCH, SOLOMON_INTERNAL_TAG_OMP_INTEROP, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKWAIT)

///
/// @brief add OpenACC clause "finalize" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_FINALIZE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "num_gangs (n)" if applicable
/// @details replace to OpenMP clause "num_teams (n)"
///
#define ACC_CLAUSE_NUM_GANGS(n) OMP_TARGET_CLAUSE_NUM_TEAMS(n)

///
/// @brief add OpenACC clause "num_workers (n)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_NUM_WORKERS(n) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "vector_length (n)" if applicable
/// @details replace to OpenMP clause "thread_limit (n)"
///
#define ACC_CLAUSE_VECTOR_LENGTH(n) SOLOMON_INTERNAL_OMP_TARGET_THREADS_PER_TEAM(n)

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
/// @note data clauses (2.7) in The OpenACC Application Programming Interface 3.3 (https://www.openacc.org/sites/default/files/inline-images/Specification/OpenACC-3.3-final.pdf)
///

///
/// @brief add OpenACC clause "copy (list)" if applicable
/// @details replace to OpenMP clause "map (tofrom: list)"
///
#define ACC_CLAUSE_COPY(...) OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)

///
/// @brief add OpenACC clause "copyin ([readonly:] list)" if applicable
/// @details replace to OpenMP clause "map (to: list)"; the readonly: modifier is for OpenACC only: it remains in the map clause, which the compilers reject
///
#define ACC_CLAUSE_COPYIN(...) OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)

///
/// @brief add OpenACC clause "copyout ([zero:] list)" if applicable
/// @details replace to OpenMP clause "map (from: list)"; the zero: modifier is for OpenACC only: it remains in the map clause, which the compilers reject
///
#define ACC_CLAUSE_COPYOUT(...) OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)

///
/// @brief add OpenACC clause "create ([zero:] list)" if applicable
/// @details replace to OpenMP clause "map (alloc: list)"; the zero: modifier is for OpenACC only: it remains in the map clause, which the compilers reject
///
#define ACC_CLAUSE_CREATE(...) OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)

///
/// @brief add OpenACC clause "no_create (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_NO_CREATE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "delete (list)" if applicable
/// @details replace to OpenMP clause "map (release: list)": both decrement the dynamic reference count (finalize is ignored in OpenMP)
///
#define ACC_CLAUSE_DELETE(...) OMP_TARGET_CLAUSE_MAP_RELEASE(__VA_ARGS__)

///
/// @brief add OpenACC clause "present (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_PRESENT(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "deviceptr (list)" if applicable
/// @details replace to OpenMP clause "is_device_ptr (list)"
///
#define ACC_CLAUSE_DEVICEPTR(...) OMP_TARGET_CLAUSE_IS_DEVICE_PTR(__VA_ARGS__)

///
/// @brief add OpenACC clause "attach (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_ATTACH(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "detach (list)" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_DETACH(...) SOLOMON_INTERNAL_MISSING_CLAUSE

#if defined(_OPENMP) && (_OPENMP >= 201811)
///
/// @brief add OpenACC clause "use_device (list)" if applicable
/// @details replace to OpenMP clause "use_device_addr (list)"
/// @note use_device_addr is introduced in OpenMP 5.0 (_OPENMP = 201811) and use_device_ptr is deprecated
///
#define ACC_CLAUSE_USE_DEVICE(...) OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(__VA_ARGS__)
#else  // defined(_OPENMP) && (_OPENMP >= 201811)
///
/// @brief add OpenACC clause "use_device (list)" if applicable
/// @details replace to OpenMP clause "use_device_ptr (list)"
///
#define ACC_CLAUSE_USE_DEVICE(...) OMP_TARGET_CLAUSE_USE_DEVICE_PTR(__VA_ARGS__)
#endif  // defined(_OPENMP) && (_OPENMP >= 201811)

///
/// @brief add OpenACC clause "if_present" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_IF_PRESENT SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "collapse (n)" if applicable
/// @details replace to OpenMP clause "collapse (n)"
///
#define ACC_CLAUSE_COLLAPSE(n) OMP_TARGET_CLAUSE_COLLAPSE(n)

///
/// @brief add OpenACC clause "seq" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_SEQ SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "auto" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_AUTO SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "independent" if applicable
/// @details replace to OpenMP clause "simd"
///
#define ACC_CLAUSE_INDEPENDENT OMP_TARGET_CLAUSE_SIMD

///
/// @brief add OpenACC clause "tile (expression_list)" if applicable
/// @details ignore the clause (_Pragma("omp tile sizes (size_list)") might be the correspondence)
///
#define ACC_CLAUSE_TILE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "gang" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_GANG SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "worker" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_WORKER SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "vector" if applicable
/// @details ignore the clause
///
#define ACC_CLAUSE_VECTOR SOLOMON_INTERNAL_MISSING_CLAUSE

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
#define ACC_CLAUSE_BIND(arg) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "nohost" if applicable
/// @details ignore the clause: the procedure is also compiled for the host (a declare target directive with only device_type (nohost) is rejected by compilers)
///
#define ACC_CLAUSE_NOHOST SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "device_resident (list)" if applicable
/// @details ignore the clause (declare directive is also ignored)
///
#define ACC_CLAUSE_DEVICE_RESIDENT(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenACC clause "link (list)" if applicable
/// @details ignore the clause (declare directive is also ignored)
///
#define ACC_CLAUSE_LINK(...) SOLOMON_INTERNAL_MISSING_CLAUSE

#endif  // !defined(SOLOMON_CONVERT_ACC2OMP_CLAUSE_HPP)
