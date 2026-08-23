///
/// @file solomon/omp/target/clause.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief optional clauses for OpenMP target directives and constructs
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_TARGET_CLAUSE_HPP)
#define SOLOMON_OMP_TARGET_CLAUSE_HPP

#include "../../util/va_args.hpp"
#include "../clause.hpp"

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for variant directives
///

///
/// @brief add OpenMP target clause "aligned (list [: alignment])" if applicable
///
/// @param[in] list Declares one or more list items to be aligned to the specified number of bytes.
/// @param[in] alignment Optional constant positive integer expression
///
#define OMP_TARGET_CLAUSE_ALIGNED(...) OMP_CLAUSE_ALIGNED(__VA_ARGS__)

///
/// @brief add OpenMP target clause "simdlen (length)" if applicable
///
/// @param[in] length Specifies the preferred number of iterations to be executed concurrently.
///
#define OMP_TARGET_CLAUSE_SIMDLEN(length) OMP_CLAUSE_SIMDLEN(length)

///
/// @brief add OpenMP target clause "device_type (host | nohost | any)" if applicable
///
#define OMP_TARGET_CLAUSE_DEVICE_TYPE(type) device_type(type), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET)

///
/// @brief add OpenMP target clause "enter (extended_list)" if applicable
///
/// @param[in] extended_list A comma-separated list of named variables, procedure names, and named common blocks.
///
#define OMP_TARGET_CLAUSE_ENTER(...) enter(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET)

///
/// @brief add OpenMP target clause "indirect [(invoked_by_fptr)]" if applicable
///
/// @param[in] invoked_by_fptr Determines if the procedures in an enter clause may be invoked indirectly.
///
#define OMP_TARGET_CLAUSE_INDIRECT(...) indirect(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET)

///
/// @brief add OpenMP target clause "link (list)" if applicable
///
/// @param[in] list Supports compilation of functions called in a target region that refer to the list items.
///
#define OMP_TARGET_CLAUSE_LINK(...) link(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for parallelism constructs
///

///
/// @brief add OpenMP target clause "copyin (list)" if applicable
///
#define OMP_TARGET_CLAUSE_COPYIN(...) OMP_CLAUSE_COPYIN(__VA_ARGS__)

///
/// @brief add OpenMP target clause "num_threads (nthreads)" if applicable
///
/// @param[in] nthreads Specifies the number of threads to execute.
///
#define OMP_TARGET_CLAUSE_NUM_THREADS(nthreads) OMP_CLAUSE_NUM_THREADS(nthreads)

///
/// @brief add OpenMP target clause "proc_bind (close | primary | spread)" if applicable
///
/// @param[in] attr close, primary, or spread
/// @remarks close: Instructs the execution environment to assign the threads in the team to places close to the place of the parent thread.
/// @remarks primary: Instructs the execution environment to assign every thread in the team to the same place as the primary thread.
/// @remarks spread: Creates a sparse distribution for a team of T threads among the P places of the parent's place partition.
///
#define OMP_TARGET_CLAUSE_PROC_BIND(attr) OMP_CLAUSE_PROC_BIND(attr)

///
/// @brief add OpenMP target clause "num_teams ([lower_bound:] upper_bound)" if applicable
///
/// @param[in] upper_bound
///
#define OMP_TARGET_CLAUSE_NUM_TEAMS(...) OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)

///
/// @brief add OpenMP target clause "thread_limit (num)" if applicable
///
/// @param[in] num
///
#define OMP_TARGET_CLAUSE_THREAD_LIMIT(num) OMP_CLAUSE_THREAD_LIMIT(num)

///
/// @brief add OpenMP target clause "nontemporal (list)" if applicable
///
/// @param[in] list Accesses to the storage locations in list have low temporal locality across the iterations in which those storage locations are accessed.
///
#define OMP_TARGET_CLAUSE_NONTEMPORAL(...) OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)

///
/// @brief add OpenMP target clause "safelen (length)" if applicable
///
/// @param[in] length If used then no two iterations executed concurrently with SIMD instructions can have a greater distance in the logical iteration space than the value of length.
///
#define OMP_TARGET_CLAUSE_SAFELEN(length) OMP_CLAUSE_SAFELEN(length)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for work-distribution constructs
///

///
/// @brief add OpenMP target clause "ordered [(n)]" if applicable
///
/// @param[in] n The loops or how many loops to associate with a construct.
///
#define OMP_TARGET_CLAUSE_ORDERED(...) OMP_CLAUSE_ORDERED(__VA_ARGS__)

///
/// @brief add OpenMP target clause "schedule ([modifier] [, modifier] kind [, chunk_size])" if applicable
///
/// @param[in] kind static, dynamic, guided, auto, or runtime
/// @remarks static: Iterations are divided into chunks of size chunk_size and assigned to team threads in round-robin fashion in order of thread number.
/// @remarks dynamic: Each thread executes a chunk of iterations then requests another chunk until none remain.
/// @remarks guided: Same as dynamic, except chunk size is different for each chunk, with each successive chunk smaller than the last.
/// @remarks auto: Compiler and/or runtime decides.
/// @remarks runtime: Uses run-sched-var ICV.
/// @param[in] modifier monotonic, nonmonotonic, or simd
/// @remarks monotonic: Each thread executes its assigned chunks in increasing logical iteration order. A schedule (static) clause or order clause implies monotonic.
/// @remarks nonmonotonic: Chunks are assigned to threads in any order and the behavior of an application that depends on execution order of the chunks is unspecified.
/// @remarks simd: Ignored when the loop is not associated with a SIMD construct, else new_chunk_size for all except the first and last chunks is chunk_size/simd_width * simd_width (simd_width: implementation-defined value).
///
#define OMP_TARGET_CLAUSE_SCHEDULE(...) OMP_CLAUSE_SCHEDULE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "dist_schedule (kind [, chunk_size])" if applicable
///
/// @param[in] kind static, dynamic, guided, auto, or runtime
/// @remarks static: Iterations are divided into chunks of size chunk_size and assigned to team threads in round-robin fashion in order of thread number.
/// @remarks dynamic: Each thread executes a chunk of iterations then requests another chunk until none remain.
/// @remarks guided: Same as dynamic, except chunk size is different for each chunk, with each successive chunk smaller than the last.
/// @remarks auto: Compiler and/or runtime decides.
/// @remarks runtime: Uses run-sched-var ICV.
///
#define OMP_TARGET_CLAUSE_DIST_SCHEDULE(...) OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "bind (binding)" if applicable
///
/// @param[in] binding teams, parallel, or thread.
///
#define OMP_TARGET_CLAUSE_BIND(binding) OMP_CLAUSE_BIND(binding)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for device directives and construct
///

///
/// @brief add OpenMP target clause "use_device_ptr (list)" if applicable
///
/// @param[in] list A comma-separated list of file-scope, namespace-scope, or static block-scope variables that do not have incomplete types.
///
#define OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...) use_device_ptr(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA)

///
/// @brief add OpenMP target clause "use_device_addr (list)" if applicable
///
/// @param[in] list A comma-separated list of file-scope, namespace-scope, or static block-scope variables that do not have incomplete types.
///
#define OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...) use_device_addr(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA)

///
/// @brief add OpenMP target clause "defaultmap (implicit_behavior [: variable_category])" if applicable
///
/// @param[in] implicit_behavior alloc, default, firstprivate, from, none, present, to, tofrom
/// @param[in] variable_category aggregate, all, pointer, scalar
///
#define OMP_TARGET_CLAUSE_DEFAULTMAP(...) defaultmap(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)

///
/// @brief add OpenMP target clause "defaultmap (none)" if applicable
///
#define OMP_TARGET_CLAUSE_DEFAULTMAP_NONE OMP_TARGET_CLAUSE_DEFAULTMAP(none)

///
/// @brief add OpenMP target clause "defaultmap (present)" if applicable
///
#define OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT OMP_TARGET_CLAUSE_DEFAULTMAP(present)

///
/// @brief add OpenMP target clause "has_device_addr (list)" if applicable
///
/// @param[in] list Indicates that list items already have device addresses, so may be directly accessed from target device. May include array sections.
///
#define OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...) has_device_addr(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)

///
/// @brief add OpenMP target clause "is_device_ptr (list)" if applicable
///
/// @param[in] list Indicates list items are device pointers.
///
#define OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...) is_device_ptr(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)

///
/// @brief add OpenMP target clause "uses_allocators ([[alloc_mod,] alloc_mod]: allocator)" if applicable
/// @details Enables the use of each specified allocator in the region associated with the directive.
///
/// @param[in] alloc_mod memspace(mem_space_handle), traits(traits_array)
/// @remarks mem_space_handle: Variable of memspace_handle_t type
/// @remarks traits_array: Constant array of traits type (omp_alloctrait_t)
///
#define OMP_TARGET_CLAUSE_USES_ALLOCATORS(...) uses_allocators(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET)

///
/// @brief add OpenMP target clause "from ([motion_modifier[,] [motion_modifier[,] ...]:] locator_list)" if applicable
///
/// @param[in] motion_modifier present, mapper (mapper_identifier), iterator (iterators_definition)
///
#define OMP_TARGET_CLAUSE_FROM(...) from(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE)

///
/// @brief add OpenMP target clause "to ([motion_modifier[,] [motion_modifier[,] ...]:] locator_list)" if applicable
///
/// @param[in] motion_modifier present, mapper (mapper_identifier), iterator (iterators_definition)
///
#define OMP_TARGET_CLAUSE_TO(...) to(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for synchronization constructs
///

///
/// @brief pass additional list if applicable
///
#define OMP_TARGET_PASS_LIST(...) OMP_PASS_LIST(__VA_ARGS__)

///
/// @brief add OpenMP target memory-order-clause "seq_cst" if applicable
///
#define OMP_TARGET_CLAUSE_SEQ_CST OMP_CLAUSE_SEQ_CST

///
/// @brief add OpenMP target memory-order-clause "acq_rel" if applicable
///
#define OMP_TARGET_CLAUSE_ACQ_REL OMP_CLAUSE_ACQ_REL

///
/// @brief add OpenMP target memory-order-clause "release" if applicable
///
#define OMP_TARGET_CLAUSE_RELEASE OMP_CLAUSE_RELEASE

///
/// @brief add OpenMP target memory-order-clause "acquire" if applicable
///
#define OMP_TARGET_CLAUSE_ACQUIRE OMP_CLAUSE_ACQUIRE

///
/// @brief add OpenMP target memory-order-clause "relaxed" if applicable
///
#define OMP_TARGET_CLAUSE_RELAXED OMP_CLAUSE_RELAXED

///
/// @brief add OpenMP target atomic-clause "read" if applicable
///
#define OMP_TARGET_CLAUSE_READ OMP_CLAUSE_READ

///
/// @brief add OpenMP target atomic-clause "write" if applicable
///
#define OMP_TARGET_CLAUSE_WRITE OMP_CLAUSE_WRITE

///
/// @brief add OpenMP target atomic-clause "update" if applicable
///
#define OMP_TARGET_CLAUSE_UPDATE OMP_CLAUSE_UPDATE

///
/// @brief add OpenMP target extended-atomic-clause "capture" if applicable
/// @details Capture the value of the variable being updated atomically.
///
#define OMP_TARGET_CLAUSE_CAPTURE OMP_CLAUSE_CAPTURE

///
/// @brief add OpenMP target extended-atomic-clause "compare" if applicable
/// @details Perform the atomic update conditionally.
///
#define OMP_TARGET_CLAUSE_COMPARE OMP_CLAUSE_COMPARE

///
/// @brief add OpenMP target extended-atomic-clause "fail (seq_cst | acquire | relaxed)" if applicable
/// @details Specify the memory ordering requirements for any comparison performed by any atomic conditional update that fails. Its argument overrides any other specified memory ordering.
///
/// @param[in] arg seq_cst (OMP_CLAUSE_SEQ_CST), acquire (OMP_CLAUSE_ACQUIRE), or relaxed (OMP_CLAUSE_RELAXED)
///
#define OMP_TARGET_CLAUSE_FAIL(...) OMP_CLAUSE_FAIL(__VA_ARGS__)

///
/// @brief add OpenMP target extended-atomic-clause "weak" if applicable
/// @details Specify that the comparison performed by a conditional atomic update may spuriously fail, evaluating to not equal even when the values are equal.
///
#define OMP_TARGET_CLAUSE_WEAK OMP_CLAUSE_WEAK

///
/// @brief add OpenMP target extended-atomic-clause "hint (hint_expression)" if applicable
///
#define OMP_TARGET_CLAUSE_HINT(expression) OMP_CLAUSE_HINT(expression)

///
/// @brief add OpenMP target clause "simd" if applicable
///
#define OMP_TARGET_CLAUSE_SIMD OMP_CLAUSE_SIMD

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Data sharing attribute clauses
///

///
/// @brief add OpenMP target clause "default (shared)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_TARGET_CLAUSE_DEFAULT_SHARED OMP_CLAUSE_DEFAULT_SHARED

///
/// @brief add OpenMP target clause "default (firstprivate)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_TARGET_CLAUSE_DEFAULT_FIRSTPRIVATE OMP_CLAUSE_DEFAULT_FIRSTPRIVATE

///
/// @brief add OpenMP target clause "default (private)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_TARGET_CLAUSE_DEFAULT_PRIVATE OMP_CLAUSE_DEFAULT_PRIVATE

///
/// @brief add OpenMP target clause "default (none)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_TARGET_CLAUSE_DEFAULT_NONE OMP_CLAUSE_DEFAULT_NONE

///
/// @brief add OpenMP target clause "shared (list)" if applicable
/// @details Variables in list are shared between threads or explicit tasks executing the construct.
///
#define OMP_TARGET_CLAUSE_SHARED(...) OMP_CLAUSE_SHARED(__VA_ARGS__)

///
/// @brief add OpenMP target clause "private (list)" if applicable
/// @details Creates a new variable for each item in list that is private to each thread or explicit task. The private variable is not given an initial value.
///
#define OMP_TARGET_CLAUSE_PRIVATE(...) OMP_CLAUSE_PRIVATE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "firstprivate (list)" if applicable
/// @details Declares list items to be private to each thread or explicit task and assigns them the value the original variable has at the time the construct is encountered.
///
#define OMP_TARGET_CLAUSE_FIRSTPRIVATE(...) OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "lastprivate ([lastprivate_modifier:] list)" if applicable
/// @details After the last loop ends, the variables in list will be copied to the primary thread.
///
/// @param[in] lastprivate_modifier conditional
/// @remarks conditional: Uses the value from the thread that executed the highest index iteration number.
///
#define OMP_TARGET_CLAUSE_LASTPRIVATE(...) OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "linear (linear_list [: linear_step])" or "linear (linear_list [: linear_modifier [, linear_modifier]])" if applicable
/// @details Declares each linear-list item to have a linear value or address with respect to the iteration space of the loop.
///
/// @param[in] linear_list list (or for declare simd argument_list)
/// @param[in] linear_step OpenMP integer expression (1 is default)
/// @param[in] linear_modifier: step(linear_step), linear_type_modifier
/// @param[in] linear_type_modifier val, ref, uval (val is default)
/// @remarks val: The value is linear
/// @remarks ref: The address is linear
/// @remarks uval: The value is linear, may not be modified
/// @remarks The ref and uval modifiers may only be specified for a linear clause on the declare simd directive, and only for arguments that are passed by reference.
///
#define OMP_TARGET_CLAUSE_LINEAR(...) OMP_CLAUSE_LINEAR(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks allocate clause
///

///
/// @brief add OpenMP target clause "allocate ([allocator:] list)" or "allocate (allocate_modifier [, allocate_modifier]: list)" if applicable
///
/// @param[in] allocate_modifier allocator(allocator), align(alignment)
/// @param[in] allocator an expression of "type omp_allocator_handle_t"
/// @param[in] alignment a constant positive integer power of 2
///
#define OMP_TARGET_CLAUSE_ALLOCATE(...) OMP_CLAUSE_ALLOCATE(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks collapse clause
///

///
/// @brief add OpenMP target clause "collapse (n)" if applicable
/// @details A constant positive integer expression that specifies how many loops are associated with the construct.
///
/// @param[in] n A constant positive integer expression that specifies how many loops are associated with the construct.
///
#define OMP_TARGET_CLAUSE_COLLAPSE(n) OMP_CLAUSE_COLLAPSE(n)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks depend clause
///

///
/// @brief add OpenMP target clause "depend ([depend_modifier,] dependence_type: locator_list)" if applicable
/// @details Enforces additional constraints on the scheduling of tasks or loop iterations, establishing dependences only between sibling tasks or between loop iterations.
///
/// @param[in] depend_modifier iterator(iterators_definition)
/// @param[in] dependence_type in, out, inout, mutexinoutset, inoutset, depobj
/// @remarks in: The generated task will be a dependent task of all previously generated sibling tasks that reference at least one of the list items in an out or inout dependence-type list.
/// @remarks out and inout: The generated task will be a dependent task of all previously generated sibling tasks that reference at least one of the list items in an in, out, mutexinoutset, inout, or inoutset dependence-type list.
/// @remarks mutexinoutset: If the storage location of at least one of the list items is the same as that of a list item appearing in a depend clause with an in, out, inout, or inoutset dependence-type on a construct from which a sibling task was previously generated, then the generated task will be a dependent task of that sibling task. If the storage location of at least one of the list items is the same as that of a list item appearing in a depend clause with a mutexinoutset dependence-type on a construct from which a sibling task was previously generated, then the sibling tasks will be mutually exclusive tasks.
/// @remarks inoutset: If the storage location of at least one of the list items matches the storage location of a list item appearing in a depend clause with an in, out, inout, or mutexinoutset dependence-type on a construct from which a sibling task was previously generated, then the generated task will be a dependent task of that sibling task.
/// @remarks depobj: The task dependences are derived from the depend clause specified in the depobj constructs that initialized dependences represented by the depend objects specified in the depend clause as if the depend clauses of the depobj constructs were specified in the current construct.
///
#define OMP_TARGET_CLAUSE_DEPEND(...) OMP_CLAUSE_DEPEND(__VA_ARGS__)

///
/// @brief add OpenMP target clause "depend (in: locator_list)" if applicable
///
#define OMP_TARGET_CLAUSE_DEPEND_IN(...) OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks device clause
///

///
/// @brief add OpenMP target clause "device ([ancestor | device_num:] device_description)" if applicable
/// @details Identifies the target device that is associated with a device construct.
///
/// @param[in] device_description An expression of type integer that refers to the device number or, if ancestor modifier is specified, must be 1.
///
#define OMP_TARGET_CLAUSE_DEVICE(...) device(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISPATCH, SOLOMON_INTERNAL_TAG_OMP_INTEROP, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks if clause
///

///
/// @brief add OpenMP target clause "if ([directive_name_modifier:] omp_logical_expression)" if applicable
/// @details The effect of the if clause depends on the construct to which it is applied. For combined or composite constructs, it only applies to the semantics of the construct named in the directive-name-modifier if one is specified. If no modifier is specified for a combined or composite construct then the if clause applies to all constructs to which an if clause can apply.
///
#define OMP_TARGET_CLAUSE_IF(condition) OMP_CLAUSE_IF(condition)

///
/// @brief add OpenMP target clause "if ([directive_name_modifier:] omp_logical_expression)" if applicable
/// @details The effect of the if clause depends on the construct to which it is applied. For combined or composite constructs, it only applies to the semantics of the construct named in the directive-name-modifier if one is specified. If no modifier is specified for a combined or composite construct then the if clause applies to all constructs to which an if clause can apply.
///
#define OMP_TARGET_CLAUSE_IF_TARGET(condition) OMP_CLAUSE_IF(target : condition)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks map clause
///

///
/// @brief add OpenMP target clause "map ([[map_modifier, [map_modifier, ...]] map_type:] locator_list)" if applicable
/// @details Maps data from the task’s environment to the device environment.
///
/// @param[in] map_type alloc, to, from, tofrom, release, delete
/// @param[in] map_modifier always, close, present, mapper(mapper-id), iterator(iterators-definition)
///
#define OMP_TARGET_CLAUSE_MAP(...) OMP_CLAUSE_MAP(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (alloc: list)" if applicable
///
#define OMP_TARGET_CLAUSE_MAP_ALLOC(...) OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (to: list)" if applicable
///
#define OMP_TARGET_CLAUSE_MAP_TO(...) OMP_CLAUSE_MAP_TO(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (from: list)" if applicable
///
#define OMP_TARGET_CLAUSE_MAP_FROM(...) OMP_CLAUSE_MAP_FROM(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (tofrom: list)" if applicable
///
#define OMP_TARGET_CLAUSE_MAP_TOFROM(...) OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (release: list)" if applicable
///
#define OMP_TARGET_CLAUSE_MAP_RELEASE(...) OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)

///
/// @brief add OpenMP target clause "map (delete: list)" if applicable
///
#define OMP_TARGET_CLAUSE_MAP_DELETE(...) OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks order clause
///

///
/// @brief add OpenMP target clause "order ([order_modifier:] concurrent)" if applicable
/// @details Specifies an expected order of execution for the iterations of the associated loops of a loop-associated directive.
///
/// @param[in] order_modifier reproducible, unconstrained
///
#define OMP_TARGET_CLAUSE_ORDER(...) OMP_CLAUSE_ORDER(__VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks nowait clause
///

///
/// @brief add OpenMP target clause "nowait" if applicable
/// @details Overrides any synchronization that would otherwise occur at the end of a construct. It can also specify that an interoperability requirement set includes the nowait property. If the construct includes an implicit barrier, the nowait clause specifies that the barrier will not occur.
/// @remarks parallel for, parallel sections, or parallel for simd cannot use nowait clause
///
#define OMP_TARGET_CLAUSE_NOWAIT OMP_CLAUSE_NOWAIT

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks reduction clause
///

///
/// @brief add OpenMP target clause "reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
/// @details Specifies a reduction-identifier and one or more list items.
///
/// @param[in] reduction_modifier inscan, task, default
/// @param[in] reduction_identifier +, *, &, |, ^, &&, ||
///
#define OMP_TARGET_CLAUSE_REDUCTION(...) OMP_CLAUSE_REDUCTION(__VA_ARGS__)

///
/// @brief add OpenMP target clause "in_reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
/// @details Specifies a reduction-identifier and one or more list items.
/// @remarks parallel masked taskloop, parallel masked taskloop simd cannot use nowait clause
///
/// @param[in] reduction_modifier inscan, task, default
/// @param[in] reduction_identifier +, *, &, |, ^, &&, ||
///
#define OMP_TARGET_CLAUSE_IN_REDUCTION(...) OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)

#endif  // !defined(SOLOMON_OMP_TARGET_CLAUSE_HPP)
