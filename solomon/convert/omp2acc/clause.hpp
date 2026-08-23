///
/// @file solomon/convert/omp2acc/clause.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief convert OpenMP clauses to OpenACC clauses
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_CONVERT_OMP2ACC_CLAUSE_HPP)
#define SOLOMON_CONVERT_OMP2ACC_CLAUSE_HPP

#include "../../util/missing.hpp"

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for variant directives
///

///
/// @brief add OpenMP target clause "aligned (list [: alignment])" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ALIGNED(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "simdlen (length)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_SIMDLEN(length) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "device_type (host | nohost | any)" if applicable
/// @details replace to OpenACC clause "device_type"
///
#define OMP_TARGET_CLAUSE_DEVICE_TYPE(type) ACC_CLAUSE_DEVICE_TYPE(type)

///
/// @brief add OpenMP target clause "enter (extended_list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ENTER(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "indirect [(invoked_by_fptr)]" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_INDIRECT(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "link (list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_LINK(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for parallelism constructs
///

///
/// @brief add OpenMP target clause "copyin (list)" if applicable
/// @details replace to OpenACC clause "copyin (list)"
///
#define OMP_TARGET_CLAUSE_COPYIN(...) ACC_CLAUSE_COPYIN(__VA_ARGS__)

///
/// @brief add OpenMP target clause "num_threads (nthreads)" if applicable
/// @details replace to OpenACC clause "vector_length (nthreads)"
///
#define OMP_TARGET_CLAUSE_NUM_THREADS(nthreads) ACC_CLAUSE_VECTOR_LENGTH(nthreads)

///
/// @brief add OpenMP target clause "proc_bind (close | primary | spread)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_PROC_BIND(attr) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "num_teams ([lower_bound:] upper_bound)" if applicable
/// @details replace to OpenACC clause "num_gangs (n)"
///
#define OMP_TARGET_CLAUSE_NUM_TEAMS(...) ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)

///
/// @brief add OpenMP target clause "thread_limit (num)" if applicable
/// @details replace to OpenACC clause "vector_length (num)"
///
#define OMP_TARGET_CLAUSE_THREAD_LIMIT(num) ACC_CLAUSE_VECTOR_LENGTH(num)

///
/// @brief add OpenMP target clause "nontemporal (list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_NONTEMPORAL(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "safelen (length)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_SAFELEN(length) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for work-distribution constructs
///

///
/// @brief add OpenMP target clause "ordered [(n)]" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ORDERED(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "schedule ([modifier] [, modifier] kind [, chunk_size])" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_SCHEDULE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "dist_schedule (kind [, chunk_size])" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DIST_SCHEDULE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "bind (binding)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_BIND(binding) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for device directives and construct
///

///
/// @brief add OpenMP target clause "use_device_ptr (list)" if applicable
/// @details replace to OpenACC clause "use_device (list)"
///
#define OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...) ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "use_device_addr (list)" if applicable
/// @details replace to OpenACC clause "use_device (list)"
///
#define OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...) ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "defaultmap (implicit_behavior [: variable_category])" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEFAULTMAP(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "defaultmap (none)" if applicable
/// @details replace to OpenACC clause "default (none)"
///
#define OMP_TARGET_CLAUSE_DEFAULTMAP_NONE ACC_CLAUSE_DEFAULT_NONE

///
/// @brief add OpenMP target clause "defaultmap (present)" if applicable
/// @details replace to OpenACC clause "default (present)"
///
#define OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT ACC_CLAUSE_DEFAULT_PRESENT

///
/// @brief add OpenMP target clause "has_device_addr (list)" if applicable
/// @details replace to OpenACC clause "deviceptr (list)"
///
#define OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...) ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)

///
/// @brief add OpenMP target clause "is_device_ptr (list)" if applicable
/// @details replace to OpenACC clause "deviceptr (list)"
///
#define OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...) ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)

///
/// @brief add OpenMP target clause "uses_allocators ([[alloc_mod,] alloc_mod]: allocator)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_USES_ALLOCATORS(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "from ([motion_modifier[,] [motion_modifier[,] ...]:] locator_list)" if applicable
/// @details replace to OpenACC clause "host (list)"
///
#define OMP_TARGET_CLAUSE_FROM(...) ACC_CLAUSE_HOST(__VA_ARGS__)

///
/// @brief add OpenMP target clause "to ([motion_modifier[,] [motion_modifier[,] ...]:] locator_list)" if applicable
/// @details replace to OpenACC clause "device (list)"
///
#define OMP_TARGET_CLAUSE_TO(...) ACC_CLAUSE_DEVICE(__VA_ARGS__)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for synchronization constructs
///

///
/// @brief pass additional list if applicable
/// @details replace to OpenACC correspondence
///
#define OMP_TARGET_PASS_LIST(...) ACC_PASS_LIST(__VA_ARGS__)

///
/// @brief add OpenMP target memory-order-clause "seq_cst" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_SEQ_CST SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target memory-order-clause "acq_rel" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ACQ_REL SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target memory-order-clause "release" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_RELEASE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target memory-order-clause "acquire" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ACQUIRE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target memory-order-clause "relaxed" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_RELAXED SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target atomic-clause "read" if applicable
/// @details replace to OpenACC clause "read"
///
#define OMP_TARGET_CLAUSE_READ ACC_CLAUSE_READ

///
/// @brief add OpenMP target atomic-clause "write" if applicable
/// @details replace to OpenACC clause "write"
///
#define OMP_TARGET_CLAUSE_WRITE ACC_CLAUSE_WRITE

///
/// @brief add OpenMP target atomic-clause "update" if applicable
/// @details replace to OpenACC clause "update"
///
#define OMP_TARGET_CLAUSE_UPDATE ACC_CLAUSE_UPDATE

///
/// @brief add OpenMP target extended-atomic-clause "capture" if applicable
/// @details replace to OpenACC clause "capture"
///
#define OMP_TARGET_CLAUSE_CAPTURE ACC_CLAUSE_CAPTURE

///
/// @brief add OpenMP target extended-atomic-clause "compare" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_COMPARE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target extended-atomic-clause "fail (seq_cst | acquire | relaxed)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_FAIL(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target extended-atomic-clause "weak" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_WEAK SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target extended-atomic-clause "hint (hint_expression)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_HINT(expression) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "simd" if applicable
/// @details replace to OpenACC clause "independent"
///
#define OMP_TARGET_CLAUSE_SIMD ACC_CLAUSE_INDEPENDENT

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Data sharing attribute clauses
///

///
/// @brief add OpenMP target clause "default (shared)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEFAULT_SHARED SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "default (firstprivate)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEFAULT_FIRSTPRIVATE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "default (private)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEFAULT_PRIVATE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "default (none)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEFAULT_NONE SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "shared (list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_SHARED(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "private (list)" if applicable
/// @details replace to OpenACC clause "private (list)"
///
#define OMP_TARGET_CLAUSE_PRIVATE(...) ACC_CLAUSE_PRIVATE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "firstprivate (list)" if applicable
/// @details replace to OpenACC clause "firstprivate (list)"
///
#define OMP_TARGET_CLAUSE_FIRSTPRIVATE(...) ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "lastprivate ([lastprivate_modifier:] list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_LASTPRIVATE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "linear (linear_list [: linear_step])" or "linear (linear_list [: linear_modifier [, linear_modifier]])" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_LINEAR(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks allocate clause
///

///
/// @brief add OpenMP target clause "allocate ([allocator:] list)" or "allocate (allocate_modifier [, allocate_modifier]: list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ALLOCATE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks collapse clause
///

///
/// @brief add OpenMP target clause "collapse (n)" if applicable
/// @details replace to OpenACC clause "collapse (n)"
///
#define OMP_TARGET_CLAUSE_COLLAPSE(n) ACC_CLAUSE_COLLAPSE(n)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks depend clause
///

///
/// @brief add OpenMP target clause "depend ([depend_modifier,] dependence_type: locator_list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEPEND(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "depend (in: locator_list)" if applicable
/// @details replace to OpenACC clause "wait (list)"
///
#define OMP_TARGET_CLAUSE_DEPEND_IN(...) ACC_CLAUSE_WAIT(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks device clause
///

///
/// @brief add OpenMP target clause "device ([ancestor | device_num:] device_description)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_DEVICE(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks if clause
///

///
/// @brief add OpenMP target clause "if (condition)" if applicable
/// @details replace to OpenACC clause "if (condition)"
///
#define OMP_TARGET_CLAUSE_IF(condition) ACC_CLAUSE_IF(condition)

///
/// @brief add OpenMP target clause "if (target: condition)" if applicable
/// @details replace to OpenACC clause "if (condition)"
///
#define OMP_TARGET_CLAUSE_IF_TARGET(condition) ACC_CLAUSE_IF(condition)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks map clause
///

///
/// @brief add OpenMP target clause "map ([[map_modifier, [map_modifier, ...]] map_type:] locator_list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_MAP(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "map (alloc: list)" if applicable
/// @details replace to OpenACC clause "create (list)"
///
#define OMP_TARGET_CLAUSE_MAP_ALLOC(...) ACC_CLAUSE_CREATE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (to: list)" if applicable
/// @details replace to OpenACC clause "copyin (list)"
///
#define OMP_TARGET_CLAUSE_MAP_TO(...) ACC_CLAUSE_COPYIN(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (from: list)" if applicable
/// @details replace to OpenACC clause "copyout (list)"
///
#define OMP_TARGET_CLAUSE_MAP_FROM(...) ACC_CLAUSE_COPYOUT(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (tofrom: list)" if applicable
/// @details replace to OpenACC clause "copy (list)"
///
#define OMP_TARGET_CLAUSE_MAP_TOFROM(...) ACC_CLAUSE_COPY(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (release: list)" if applicable
/// @details replace to OpenACC clause "delete (list)"
///
#define OMP_TARGET_CLAUSE_MAP_RELEASE(...) ACC_CLAUSE_DELETE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (delete: list)" if applicable
/// @details replace to OpenACC clause "delete (list)"
///
#define OMP_TARGET_CLAUSE_MAP_DELETE(...) ACC_CLAUSE_DELETE(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks order clause
///

///
/// @brief add OpenMP target clause "order ([order_modifier:] concurrent)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_ORDER(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks nowait clause
///

///
/// @brief add OpenMP target clause "nowait" if applicable
/// @details replace to OpenACC clause "async"
///
#define OMP_TARGET_CLAUSE_NOWAIT ACC_CLAUSE_ASYNC()

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks reduction clause
///

///
/// @brief add OpenMP target clause "reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
/// @details replace to OpenACC clause "reduction"
///
#define OMP_TARGET_CLAUSE_REDUCTION(...) ACC_CLAUSE_REDUCTION(__VA_ARGS__)

///
/// @brief add OpenMP target clause "in_reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
/// @details ignore the clause
///
#define OMP_TARGET_CLAUSE_IN_REDUCTION(...) SOLOMON_INTERNAL_MISSING_CLAUSE

#endif  // !defined(SOLOMON_CONVERT_OMP2ACC_CLAUSE_HPP)
