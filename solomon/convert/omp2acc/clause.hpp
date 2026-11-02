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
#include "../../util/nest_map.hpp"  // SOLOMON_INTERNAL_NEST_MAP

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
/// @details nohost: replace to OpenACC clause "nohost"; host, any: ignore the clause (OpenACC compiles both versions by default).
/// The OpenACC device_type clause selects the device of the following clauses and is a different clause; any other argument stops the build with a message that names it
///
// one more level, so that a macro given as the argument is expanded before it is pasted
#define OMP_TARGET_CLAUSE_DEVICE_TYPE(type) SOLOMON_INTERNAL_O2A_DEVICE_TYPE(type)
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE(type) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_CLOAK_CHECK(SOLOMON_INTERNAL_O2A_DEVICE_TYPE_IS_##type))(SOLOMON_INTERNAL_O2A_DEVICE_TYPE_##type, SOLOMON_INTERNAL_O2A_DEVICE_TYPE_UNKNOWN(type))
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_IS_nohost SOLOMON_INTERNAL_CLOAK_PROBE(~)
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_IS_host SOLOMON_INTERNAL_CLOAK_PROBE(~)
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_IS_any SOLOMON_INTERNAL_CLOAK_PROBE(~)
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_nohost ACC_CLAUSE_NOHOST
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_host SOLOMON_INTERNAL_MISSING_CLAUSE
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_any SOLOMON_INTERNAL_MISSING_CLAUSE
// the mark of any other argument is taken by every OpenACC directive, so that it stops wherever it is written
// clang-format off: the text of the message is kept as written (clang-format would put spaces around the colon) (permanent)
#define SOLOMON_INTERNAL_O2A_DEVICE_TYPE_UNKNOWN(type) SOLOMON_INTERNAL_PICKUP_STOP(SOLOMON_ERROR_UNKNOWN_DEVICE_TYPE type, Solomon: the argument of OMP_TARGET_CLAUSE_DEVICE_TYPE must be host / nohost / any: type), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_LOOP, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_HOST_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_ATOMIC, SOLOMON_INTERNAL_TAG_ACC_WAIT, SOLOMON_INTERNAL_TAG_ACC_ROUTINE, SOLOMON_INTERNAL_TAG_ACC_DECLARE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
// clang-format on

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
/// @details ignore the clause: OpenACC has no threadprivate, and its copyin is a transfer to the device (the OpenMP
/// target backend drops copyin from the combined target directives as well)
///
#define OMP_TARGET_CLAUSE_COPYIN(...) SOLOMON_INTERNAL_MISSING_CLAUSE

///
/// @brief add OpenMP target clause "num_threads (nthreads)" if applicable
/// @details replace to OpenACC clause "vector_length (nthreads)"
///
#define OMP_TARGET_CLAUSE_NUM_THREADS(nthreads) vector_length(nthreads), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_O2A_NUM_THREADS)

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
/// carries O2A_THREAD_LIMIT instead of the compute tags: PRAGMA_ACC_PARALLEL / _KERNELS pick it, the OpenMP compute directives only without num_threads
#define OMP_TARGET_CLAUSE_THREAD_LIMIT(num) vector_length(num), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_O2A_THREAD_LIMIT)

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
/// @details replace to OpenACC clause "wait" without arguments (waits for all async queues); the locator list is dropped
///
#define OMP_TARGET_CLAUSE_DEPEND_IN(...) wait, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE)

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
/// @details ignore the clause unless it holds nested clauses; the dropped element carries a pseudo tag so that PRAGMA_OMP_TARGET_DATA can stop when it is combined with use_device_ptr / use_device_addr.
/// Nested OMP_TARGET_CLAUSE_TO(list) / OMP_TARGET_CLAUSE_FROM(list) become copyin(list) / copyout(list) each; with the
/// present modifier (OMP_TARGET_MAP_MODIFIER_PRESENT, or OMP_TARGET_CLAUSE_TO_PRESENT / _FROM_PRESENT) they become
/// present(list) on data and compute constructs and stop on enter data / exit data; close is dropped; always, mapper
/// and iterator stop; the stops are Solomon errors (solomon/util/nest_map.hpp).
///
#define OMP_TARGET_CLAUSE_MAP(...) SOLOMON_INTERNAL_NEST_MAP(SOLOMON_INTERNAL_NEST_ACC_, SOLOMON_INTERNAL_NEST_ACC_DROP, __VA_ARGS__)

// state of the collected modifiers (digits, which no user macro can replace): 0 (none or close), 1 (present),
// 2 (no OpenACC counterpart); kind of one modifier: 1 (present), 2 (always, mapper, iterator), 3 (close)
#define SOLOMON_INTERNAL_NEST_ACC_MODINIT 0
#define SOLOMON_INTERNAL_NEST_ACC_MODADD(s, v) SOLOMON_INTERNAL_NEST_CAT(SOLOMON_INTERNAL_NEST_ACC_COMB_, SOLOMON_INTERNAL_NEST_CAT(s, v))
#define SOLOMON_INTERNAL_NEST_ACC_SOLOMON_INTERNAL_NEST_MODIFIER(...) SOLOMON_INTERNAL_NEST_CAT(SOLOMON_INTERNAL_NEST_ACC_MODK_, __VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_MODK_present 1
#define SOLOMON_INTERNAL_NEST_ACC_MODK_close 3
#define SOLOMON_INTERNAL_NEST_ACC_MODK_always 2
#define SOLOMON_INTERNAL_NEST_ACC_MODK_mapper(...) 2
#define SOLOMON_INTERNAL_NEST_ACC_MODK_iterator(...) 2
#define SOLOMON_INTERNAL_NEST_ACC_COMB_03 0
#define SOLOMON_INTERNAL_NEST_ACC_COMB_01 1
#define SOLOMON_INTERNAL_NEST_ACC_COMB_02 2
#define SOLOMON_INTERNAL_NEST_ACC_COMB_13 1
#define SOLOMON_INTERNAL_NEST_ACC_COMB_11 1
#define SOLOMON_INTERNAL_NEST_ACC_COMB_12 2
#define SOLOMON_INTERNAL_NEST_ACC_COMB_23 2
#define SOLOMON_INTERNAL_NEST_ACC_COMB_21 2
#define SOLOMON_INTERNAL_NEST_ACC_COMB_22 2
// direction of a nested clause: 4 (to the device), 5 (from the device), 6 / 7 (the same with its own present modifier)
#define SOLOMON_INTERNAL_NEST_ACC_N_device(...) 4, __VA_ARGS__
#define SOLOMON_INTERNAL_NEST_ACC_N_host(...) 5, __VA_ARGS__
#define SOLOMON_INTERNAL_NEST_ACC_P_device(...) 6, __VA_ARGS__
#define SOLOMON_INTERNAL_NEST_ACC_P_host(...) 7, __VA_ARGS__
// one nested clause with the state of the modifiers: DO_<state><direction>
#define SOLOMON_INTERNAL_NEST_ACC_APPLY(s, ...) SOLOMON_INTERNAL_NEST_ACC_APPLY_(s, __VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_APPLY_(s, d, ...) SOLOMON_INTERNAL_NEST_CAT(SOLOMON_INTERNAL_NEST_ACC_DO_, SOLOMON_INTERNAL_NEST_CAT(s, d))(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_04(...) ACC_CLAUSE_COPYIN(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_05(...) ACC_CLAUSE_COPYOUT(__VA_ARGS__)
#if defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_NEST_ACC_STOP_PRESENT(...) SOLOMON_ERROR_PRESENT_MODIFIER_ON_ENTER_OR_EXIT_DATA_IN_OPENACC(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_STOP_MODIFIER(...) SOLOMON_ERROR_MAP_MODIFIER_WITHOUT_OPENACC_COUNTERPART(__VA_ARGS__)
#else  // defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_NEST_ACC_STOP_PRESENT(...) _Pragma("GCC error \"Solomon: the present map modifier (OMP_TARGET_MAP_MODIFIER_PRESENT, OMP_TARGET_CLAUSE_TO_PRESENT / _FROM_PRESENT) has no OpenACC counterpart on enter data / exit data\"")
#define SOLOMON_INTERNAL_NEST_ACC_STOP_MODIFIER(...) _Pragma("GCC error \"Solomon: the map modifiers always, mapper and iterator (OMP_TARGET_MAP_MODIFIER_ALWAYS / _MAPPER / _ITERATOR) have no OpenACC counterpart\"")
#endif  // defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_NEST_ACC_PRESENT_(...) ACC_CLAUSE_PRESENT(__VA_ARGS__), SOLOMON_INTERNAL_NEST_ACC_STOP_PRESENT(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT)
#define SOLOMON_INTERNAL_NEST_ACC_DO_14(...) SOLOMON_INTERNAL_NEST_ACC_PRESENT_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_06(...) SOLOMON_INTERNAL_NEST_ACC_PRESENT_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_16(...) SOLOMON_INTERNAL_NEST_ACC_PRESENT_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_15(...) SOLOMON_INTERNAL_NEST_ACC_PRESENT_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_07(...) SOLOMON_INTERNAL_NEST_ACC_PRESENT_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_17(...) SOLOMON_INTERNAL_NEST_ACC_PRESENT_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_STOP_(...) SOLOMON_INTERNAL_NEST_ACC_STOP_MODIFIER(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
#define SOLOMON_INTERNAL_NEST_ACC_DO_24(...) SOLOMON_INTERNAL_NEST_ACC_STOP_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_25(...) SOLOMON_INTERNAL_NEST_ACC_STOP_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_26(...) SOLOMON_INTERNAL_NEST_ACC_STOP_(__VA_ARGS__)
#define SOLOMON_INTERNAL_NEST_ACC_DO_27(...) SOLOMON_INTERNAL_NEST_ACC_STOP_(__VA_ARGS__)
// forms that stop (the rest of the map is dropped): modifiers without a nested clause, a raw list after a nested clause,
// a nested clause or a modifier after a raw list, too many modifiers
#define SOLOMON_INTERNAL_NEST_ACC_NOCLAUSE(...) SOLOMON_INTERNAL_NEST_STOP_NOCLAUSE, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
#define SOLOMON_INTERNAL_NEST_ACC_MIXED(...) SOLOMON_INTERNAL_NEST_STOP_MIXED, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
#define SOLOMON_INTERNAL_NEST_ACC_RAWMIX(...) SOLOMON_INTERNAL_NEST_STOP_RAWMIX, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
#define SOLOMON_INTERNAL_NEST_ACC_TOOMANY(...) SOLOMON_INTERNAL_NEST_STOP_TOOMANY, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
// a map without nested clauses is dropped; the element keeps the pseudo tag of a generic map (see PRAGMA_OMP_TARGET_DATA)
#define SOLOMON_INTERNAL_NEST_ACC_DROP(...) missing, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_MISSING, SOLOMON_INTERNAL_TAG_ACC_O2A_GENERIC_MAP)

///
/// @brief map modifiers for OMP_TARGET_CLAUSE_MAP(modifier, ..., OMP_TARGET_CLAUSE_TO(list), ...)
/// @details Written outside OMP_TARGET_CLAUSE_MAP(...), they stop the build with a Solomon error (solomon/util/nest_map.hpp).
///
#define SOLOMON_INTERNAL_OMP_MODIFIER(...) SOLOMON_INTERNAL_NEST_MODIFIER_ELEMENTS(__VA_ARGS__, SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE)
#define OMP_TARGET_MAP_MODIFIER_ALWAYS SOLOMON_INTERNAL_OMP_MODIFIER(always)
#define OMP_TARGET_MAP_MODIFIER_CLOSE SOLOMON_INTERNAL_OMP_MODIFIER(close)
#define OMP_TARGET_MAP_MODIFIER_PRESENT SOLOMON_INTERNAL_OMP_MODIFIER(present)
#define OMP_TARGET_MAP_MODIFIER_MAPPER(...) SOLOMON_INTERNAL_OMP_MODIFIER(mapper(__VA_ARGS__))
#define OMP_TARGET_MAP_MODIFIER_ITERATOR(...) SOLOMON_INTERNAL_OMP_MODIFIER(iterator(__VA_ARGS__))

///
/// @brief add OpenMP target clause "to (present: list)" / "from (present: list)": device(list) / host(list) on update
///
#define OMP_TARGET_CLAUSE_TO_PRESENT(...) SOLOMON_INTERNAL_ACC_DEVICE_PRESENT(__VA_ARGS__)
#define OMP_TARGET_CLAUSE_FROM_PRESENT(...) SOLOMON_INTERNAL_ACC_HOST_PRESENT(__VA_ARGS__)

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
#define OMP_TARGET_CLAUSE_MAP_DELETE(...) delete (__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, SOLOMON_INTERNAL_TAG_ACC_O2A_FINALIZE)

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
