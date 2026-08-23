///
/// @file solomon/omp/clause.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief optional clauses for OpenMP directives and constructs
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_CLAUSE_HPP)
#define SOLOMON_OMP_CLAUSE_HPP

#include "../util/va_args.hpp"
#include "clause_tag.hpp"

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for data environment directives
///

///
/// @brief add OpenMP clause "exclusive (list)" if applicable
///
/// @param[in] list A comma-separated list of file-scope, namespace-scope, or static block-scope variables that do not have incomplete types.
///
#define OMP_CLAUSE_EXCLUSIVE(...) exclusive(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_SCAN)

///
/// @brief add OpenMP clause "inclusive (list)" if applicable
///
/// @param[in] list A comma-separated list of file-scope, namespace-scope, or static block-scope variables that do not have incomplete types.
///
#define OMP_CLAUSE_INCLUSIVE(...) inclusive(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_SCAN)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for variant directives
///

///
/// @brief add OpenMP clause "aligned (list [: alignment])" if applicable
///
/// @param[in] list Declares one or more list items to be aligned to the specified number of bytes.
/// @param[in] alignment Optional constant positive integer expression
///
#define OMP_CLAUSE_ALIGNED(...) aligned(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD, SOLOMON_INTERNAL_TAG_OMP_SIMD)

///
/// @brief add OpenMP clause "inbranch" if applicable
///
#define OMP_CLAUSE_INBRANCH inbranch, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD)

///
/// @brief add OpenMP clause "notinbranch" if applicable
///
#define OMP_CLAUSE_NOTINBRANCH notinbranch, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD)

///
/// @brief add OpenMP clause "simdlen (length)" if applicable
///
/// @param[in] length Specifies the preferred number of iterations to be executed concurrently.
///
#define OMP_CLAUSE_SIMDLEN(length) simdlen(length), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD, SOLOMON_INTERNAL_TAG_OMP_SIMD)

///
/// @brief add OpenMP clause "uniform (list)" if applicable
///
/// @param[in] list Declares arguments to have an invariant value for all concurrent invocations of the function in the execution of a single SIMD loop.
///
#define OMP_CLAUSE_UNIFORM(...) uniform(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for loop transformation constructs
///

///
/// @brief add OpenMP clause "sizes (size_list)" if applicable
///
#define OMP_CLAUSE_SIZES(...) sizes(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TILE)

///
/// @brief add OpenMP clause "full" if applicable
///
#define OMP_CLAUSE_FULL full, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_UNROLL)

///
/// @brief add OpenMP clause "partial [(unroll_factor)]" if applicable
///
#define OMP_CLAUSE_PARTIAL(...) partial(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_UNROLL)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for parallelism constructs
///

///
/// @brief add OpenMP clause "copyin (list)" if applicable
///
#define OMP_CLAUSE_COPYIN(...) copyin(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_COPYIN)

///
/// @brief add OpenMP clause "num_threads (nthreads)" if applicable
///
/// @param[in] nthreads Specifies the number of threads to execute.
///
#define OMP_CLAUSE_NUM_THREADS(nthreads) num_threads(nthreads), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_PARALLEL)

///
/// @brief add OpenMP clause "proc_bind (close | primary | spread)" if applicable
///
/// @param[in] attr close, primary, or spread
/// @remarks close: Instructs the execution environment to assign the threads in the team to places close to the place of the parent thread.
/// @remarks primary: Instructs the execution environment to assign every thread in the team to the same place as the primary thread.
/// @remarks spread: Creates a sparse distribution for a team of T threads among the P places of the parent's place partition.
///
#define OMP_CLAUSE_PROC_BIND(attr) proc_bind(attr), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_PARALLEL)

///
/// @brief add OpenMP clause "num_teams ([lower_bound:] upper_bound)" if applicable
///
/// @param[in] upper_bound
///
#define OMP_CLAUSE_NUM_TEAMS(...) num_teams(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @brief add OpenMP clause "thread_limit (num)" if applicable
///
/// @param[in] num
///
#define OMP_CLAUSE_THREAD_LIMIT(num) thread_limit(num), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TEAMS, SOLOMON_INTERNAL_TAG_OMP_TARGET)

///
/// @brief add OpenMP clause "nontemporal (list)" if applicable
///
/// @param[in] list Accesses to the storage locations in list have low temporal locality across the iterations in which those storage locations are accessed.
///
#define OMP_CLAUSE_NONTEMPORAL(...) nontemporal(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_SIMD)

///
/// @brief add OpenMP clause "safelen (length)" if applicable
///
/// @param[in] length If used then no two iterations executed concurrently with SIMD instructions can have a greater distance in the logical iteration space than the value of length.
///
#define OMP_CLAUSE_SAFELEN(length) safelen(length), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_SIMD)

///
/// @brief add OpenMP clause "filter (thread_num)" if applicable
///
/// @param[in] thread_num Selects which thread executes.
///
#define OMP_CLAUSE_FILTER(thread_num) filter(thread_num), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_MASKED)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for work-distribution constructs
///

///
/// @brief add OpenMP clause "copyprivate (list)" if applicable
///
/// @param[in] list
///
#define OMP_CLAUSE_COPYPRIVATE(...) copyprivate(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_SINGLE)

///
/// @brief add OpenMP clause "ordered [(n)]" if applicable
///
/// @param[in] n The loops or how many loops to associate with a construct.
///
#define OMP_CLAUSE_ORDERED(...) ordered(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FOR)

///
/// @brief add OpenMP clause "schedule ([modifier] [, modifier] kind [, chunk_size])" if applicable
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
#define OMP_CLAUSE_SCHEDULE(...) schedule(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FOR)

///
/// @brief add OpenMP clause "dist_schedule (kind [, chunk_size])" if applicable
///
/// @param[in] kind static, dynamic, guided, auto, or runtime
/// @remarks static: Iterations are divided into chunks of size chunk_size and assigned to team threads in round-robin fashion in order of thread number.
/// @remarks dynamic: Each thread executes a chunk of iterations then requests another chunk until none remain.
/// @remarks guided: Same as dynamic, except chunk size is different for each chunk, with each successive chunk smaller than the last.
/// @remarks auto: Compiler and/or runtime decides.
/// @remarks runtime: Uses run-sched-var ICV.
///
#define OMP_CLAUSE_DIST_SCHEDULE(...) dist_schedule(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE)

///
/// @brief add OpenMP clause "bind (binding)" if applicable
///
/// @param[in] binding teams, parallel, or thread.
///
#define OMP_CLAUSE_BIND(binding) bind(binding), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_LOOP)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for tasking constructs
///

///
/// @brief add OpenMP clause "affinity ([off_modifier:] locator_list)" if applicable
///
/// @param[in] off_modifier iterator(iterators_definition)
///
#define OMP_CLAUSE_AFFINITY(...) affinity(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK)

///
/// @brief add OpenMP clause "detach (event_handle)" if applicable
/// @details Task does not complete until given event is fulfilled. (Also see omp_fulfilled_event)
///
/// @param[in] event_handle type omp_event_handle_t
///
#define OMP_CLAUSE_DETACH(event_handle) detach(event_handle), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK)

///
/// @brief add OpenMP clause "final (omp_logical_expression)" if applicable
///
/// @param[in] expression The generated task will be a final task if the expression evaluates to true.
///
#define OMP_CLAUSE_FINAL(expression) final(expression), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "mergeable" if applicable
///
#define OMP_CLAUSE_MERGEABLE mergeable, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "priority (value)" if applicable
///
/// @param[in] value Hint to the runtime. Sets max priority value.
///
#define OMP_CLAUSE_PRIORITY(value) priority(value), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "untied" if applicable
/// @details Task is an untied task, meaning any thread in the team can resume the task region after a suspension.
///
#define OMP_CLAUSE_UNTIED untied, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "grainsize ([strict:] grain_size" if applicable
/// @details Causes the number of logical loop iterations assigned to each created task to be greater than or equal to the minimum of the value of the grain-size expression and the number of logical loop iterations, but less than twice the value of the grain-size expression. strict forces use of exact grain size, except for last iteration.
///
#define OMP_CLAUSE_GRAINSIZE(...) grainsize(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "nogroup" if applicable
/// @details Prevents creation of implicit taskgroup region.
///
#define OMP_CLAUSE_NOGROUP nogroup, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "num_tasks ([strict:] num_tasks" if applicable
/// @details Create as many tasks as the minimum of the num_tasks expression and the number of logical loop iterations. strict forces exactly num_tasks tasks to be created.
///
#define OMP_CLAUSE_NUM_TASKS(...) num_tasks(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for interoperability construct
///

///
/// @brief add OpenMP clause "init ([interop_modifier,] [interop_type,] interop_type: interop_var)" if applicable
///
/// @param[in] interop_modifier prefer_type (preference_list)
/// @param[in] interop_type target, targetsync
///
#define OMP_CLAUSE_INIT(...) init(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_INTEROP)

///
/// @brief add OpenMP clause "use (interop_var)" if applicable
///
#define OMP_CLAUSE_USE(var) use(var), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_INTEROP)

///
/// @note clauses for Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks clauses for synchronization constructs
///

///
/// @brief add OpenMP clause "task_reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
///
/// @param[in] reduction_modifier inscan, task, default
/// @param[in] reduction_identifier +, *, &, |, ^, &&, ||
///
#define OMP_CLAUSE_TASK_REDUCTION(...) task_reduction(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASKGROUP)

///
/// @brief add OpenMP clause "destroy (depend object)" if applicable
///
#define OMP_CLAUSE_DESTROY(...) destroy(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DEPOBJ)

///
/// @brief pass additional list if applicable
///
#define OMP_PASS_LIST(...) (__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET, SOLOMON_INTERNAL_TAG_OMP_FLUSH, SOLOMON_INTERNAL_TAG_OMP_DEPOBJ)

///
/// @brief add OpenMP memory-order-clause "seq_cst" if applicable
///
#define OMP_CLAUSE_SEQ_CST seq_cst, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FLUSH, SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP memory-order-clause "acq_rel" if applicable
///
#define OMP_CLAUSE_ACQ_REL acq_rel, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FLUSH, SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP memory-order-clause "release" if applicable
///
#define OMP_CLAUSE_RELEASE release, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FLUSH, SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP memory-order-clause "acquire" if applicable
///
#define OMP_CLAUSE_ACQUIRE acquire, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FLUSH, SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP memory-order-clause "relaxed" if applicable
///
#define OMP_CLAUSE_RELAXED relaxed, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FLUSH, SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

// FIXME: collision with OMP_CLAUSE_UPDATE AS_OMP_ATOMIC_CLAUSE(update)
// ///
// /// @brief add OpenMP clause "update (task_dependence_type)" if applicable
// /// @details Sets the dependence type of an OpenMP depend object to task_dependence_type.
// ///
// /// @param[in] task_dependence_type in, out, inout, inoutset, mutexinoutset
// ///
// #define OMP_CLAUSE_UPDATE(...) update(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DEPOBJ)

///
/// @brief add OpenMP atomic-clause "read" if applicable
///
#define OMP_CLAUSE_READ read, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP atomic-clause "write" if applicable
///
#define OMP_CLAUSE_WRITE write, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP atomic-clause "update" if applicable
///
#define OMP_CLAUSE_UPDATE update, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP extended-atomic-clause "capture" if applicable
/// @details Capture the value of the variable being updated atomically.
///
#define OMP_CLAUSE_CAPTURE capture, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP extended-atomic-clause "compare" if applicable
/// @details Perform the atomic update conditionally.
///
#define OMP_CLAUSE_COMPARE compare, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP extended-atomic-clause "fail (seq_cst | acquire | relaxed)" if applicable
/// @details Specify the memory ordering requirements for any comparison performed by any atomic conditional update that fails. Its argument overrides any other specified memory ordering.
///
/// @param[in] arg seq_cst (OMP_CLAUSE_SEQ_CST), acquire (OMP_CLAUSE_ACQUIRE), or relaxed (OMP_CLAUSE_RELAXED)
///
#define OMP_CLAUSE_FAIL(...) fail(SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC), __VA_ARGS__)), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP extended-atomic-clause "weak" if applicable
/// @details Specify that the comparison performed by a conditional atomic update may spuriously fail, evaluating to not equal even when the values are equal.
///
#define OMP_CLAUSE_WEAK weak, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP extended-atomic-clause "hint (hint_expression)" if applicable
///
#define OMP_CLAUSE_HINT(expression) hint(expression), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ATOMIC)

///
/// @brief add OpenMP clause "threads" if applicable
///
#define OMP_CLAUSE_THREADS threads, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ORDERED)

///
/// @brief add OpenMP clause "simd" if applicable
///
#if defined(__NVCOMPILER)
// extension in NVIDIA HPC SDK: accepts _Pragma("omp target teams loop simd")
#define OMP_CLAUSE_SIMD simd, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ORDERED, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TARGET)
#else  // defined(__NVCOMPILER)
#define OMP_CLAUSE_SIMD simd, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ORDERED, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)
#endif  // #if defined(__NVCOMPILER)

///
/// @brief add OpenMP clause "doacross (dependence_type: [vector])" if applicable
/// @details Identifies cross-iteration dependences that imply additional constraints on the scheduling of loop iterations.
///
/// @param[in] dependence_type source or sink
/// @remarks source: Specifies the satisfaction of cross-iteration dependences that arise from the current iteration. If source is specified, then the vector argument is optional; if vector is omitted, it is assumed to be omp_cur_iteration. At most one doacross clause can be specified on a directive with source as the dependence_type.
/// @remarks sink: Specifies a cross-iteration dependence, where vector indicates the iteration that satisfies the dependence. If vector does not occur in the iteration space, the doacross clause is ignored. If all doacross clauses on an ordered construct are ignored then the construct is ignored.
///
#define OMP_CLAUSE_DOACROSS(...) doacross(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_ORDERED)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Data sharing attribute clauses
///

///
/// @brief add OpenMP clause "default (shared | firstprivate | private | none)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_CLAUSE_DEFAULT(attr) default(attr), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @brief add OpenMP clause "default (shared)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_CLAUSE_DEFAULT_SHARED OMP_CLAUSE_DEFAULT(shared)

///
/// @brief add OpenMP clause "default (firstprivate)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_CLAUSE_DEFAULT_FIRSTPRIVATE OMP_CLAUSE_DEFAULT(firstprivate)

///
/// @brief add OpenMP clause "default (private)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_CLAUSE_DEFAULT_PRIVATE OMP_CLAUSE_DEFAULT(private)

///
/// @brief add OpenMP clause "default (none)" if applicable
/// @details Default data-sharing attributes are disabled. All variables in a construct must be declared inside the construct or appear in a data-sharing attribute clause.
///
#define OMP_CLAUSE_DEFAULT_NONE OMP_CLAUSE_DEFAULT(none)

///
/// @brief add OpenMP clause "shared (list)" if applicable
/// @details Variables in list are shared between threads or explicit tasks executing the construct.
///
#define OMP_CLAUSE_SHARED(...) shared(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @brief add OpenMP clause "private (list)" if applicable
/// @details Creates a new variable for each item in list that is private to each thread or explicit task. The private variable is not given an initial value.
///
#define OMP_CLAUSE_PRIVATE(...) private(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_LOOP, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_SCOPE, SOLOMON_INTERNAL_TAG_OMP_SECTION, SOLOMON_INTERNAL_TAG_OMP_SIMD, SOLOMON_INTERNAL_TAG_OMP_SINGLE, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @brief add OpenMP clause "firstprivate (list)" if applicable
/// @details Declares list items to be private to each thread or explicit task and assigns them the value the original variable has at the time the construct is encountered.
///
#define OMP_CLAUSE_FIRSTPRIVATE(...) firstprivate(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_SCOPE, SOLOMON_INTERNAL_TAG_OMP_SECTION, SOLOMON_INTERNAL_TAG_OMP_SIMD, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @brief add OpenMP clause "lastprivate ([lastprivate_modifier:] list)" if applicable
/// @details After the last loop ends, the variables in list will be copied to the primary thread.
///
/// @param[in] lastprivate_modifier conditional
/// @remarks conditional: Uses the value from the thread that executed the highest index iteration number.
///
#define OMP_CLAUSE_LASTPRIVATE(...) lastprivate(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_LOOP, SOLOMON_INTERNAL_TAG_OMP_SECTION, SOLOMON_INTERNAL_TAG_OMP_SIMD, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @brief add OpenMP clause "linear (linear_list [: linear_step])" or "linear (linear_list [: linear_modifier [, linear_modifier]])" if applicable
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
#define OMP_CLAUSE_LINEAR(...) linear(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DECLARE_SIMD, SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_SIMD)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks allocate clause
///

///
/// @brief add OpenMP clause "allocate ([allocator:] list)" or "allocate (allocate_modifier [, allocate_modifier]: list)" if applicable
///
/// @param[in] allocate_modifier allocator(allocator), align(alignment)
/// @param[in] allocator an expression of "type omp_allocator_handle_t"
/// @param[in] alignment a constant positive integer power of 2
///
#define OMP_CLAUSE_ALLOCATE(...) allocate(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_SCOPE, SOLOMON_INTERNAL_TAG_OMP_SECTION, SOLOMON_INTERNAL_TAG_OMP_SINGLE, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKGROUP, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks collapse clause
///

///
/// @brief add OpenMP clause "collapse (n)" if applicable
/// @details A constant positive integer expression that specifies how many loops are associated with the construct.
///
/// @param[in] n A constant positive integer expression that specifies how many loops are associated with the construct.
///
#define OMP_CLAUSE_COLLAPSE(n) collapse(n), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_LOOP, SOLOMON_INTERNAL_TAG_OMP_SIMD, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks depend clause
///

///
/// @brief add OpenMP clause "depend ([depend_modifier,] dependence_type: locator_list)" if applicable
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
#define OMP_CLAUSE_DEPEND(...) depend(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DEPOBJ, SOLOMON_INTERNAL_TAG_OMP_DISPATCH, SOLOMON_INTERNAL_TAG_OMP_INTEROP, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKWAIT)

///
/// @brief add OpenMP clause "depend (in: locator_list)" if applicable
///
#define OMP_CLAUSE_DEPEND_IN(...) OMP_CLAUSE_DEPEND(in : __VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks if clause
///

///
/// @brief add OpenMP clause "if ([directive_name_modifier:] omp_logical_expression)" if applicable
/// @details The effect of the if clause depends on the construct to which it is applied. For combined or composite constructs, it only applies to the semantics of the construct named in the directive-name-modifier if one is specified. If no modifier is specified for a combined or composite construct then the if clause applies to all constructs to which an if clause can apply.
///
#define OMP_CLAUSE_IF(...) \
  if (__VA_ARGS__) , SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_CANCEL, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_SIMD, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE, SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks map clause
///

///
/// @brief add OpenMP clause "map ([[map_modifier, [map_modifier, ...]] map_type:] locator_list)" if applicable
/// @details Maps data from the task's environment to the device environment.
///
/// @param[in] map_type alloc, to, from, tofrom, release, delete
/// @param[in] map_modifier always, close, present, mapper(mapper-id), iterator(iterators-definition)
///
#define OMP_CLAUSE_MAP(...) map(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TARGET_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA, SOLOMON_INTERNAL_TAG_OMP_DECLARE_MAPPER)

///
/// @brief add OpenMP clause "map (tofrom: alloc)" if applicable
///
#define OMP_CLAUSE_MAP_ALLOC(...) OMP_CLAUSE_MAP(alloc : __VA_ARGS__)

///
/// @brief add OpenMP clause "map (to: list)" if applicable
///
#define OMP_CLAUSE_MAP_TO(...) OMP_CLAUSE_MAP(to : __VA_ARGS__)

///
/// @brief add OpenMP clause "map (from: list)" if applicable
///
#define OMP_CLAUSE_MAP_FROM(...) OMP_CLAUSE_MAP(from : __VA_ARGS__)

///
/// @brief add OpenMP clause "map (tofrom: list)" if applicable
///
#define OMP_CLAUSE_MAP_TOFROM(...) OMP_CLAUSE_MAP(tofrom : __VA_ARGS__)

///
/// @brief add OpenMP clause "map (release: list)" if applicable
///
#define OMP_CLAUSE_MAP_RELEASE(...) OMP_CLAUSE_MAP(release : __VA_ARGS__)

///
/// @brief add OpenMP clause "map (delete: list)" if applicable
///
#define OMP_CLAUSE_MAP_DELETE(...) OMP_CLAUSE_MAP(delete : __VA_ARGS__)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks order clause
///

///
/// @brief add OpenMP clause "order ([order_modifier:] concurrent)" if applicable
/// @details Specifies an expected order of execution for the iterations of the associated loops of a loop-associated directive.
///
/// @param[in] order_modifier reproducible, unconstrained
///
#define OMP_CLAUSE_ORDER(...) order(__VA_ARGS__ concurrent), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISTRIBUTE, SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_LOOP, SOLOMON_INTERNAL_TAG_OMP_SIMD)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks nowait clause
///

///
/// @brief add OpenMP clause "nowait" if applicable
/// @details Overrides any synchronization that would otherwise occur at the end of a construct. It can also specify that an interoperability requirement set includes the nowait property. If the construct includes an implicit barrier, the nowait clause specifies that the barrier will not occur.
/// @remarks parallel for, parallel sections, or parallel for simd cannot use nowait clause
///
#define OMP_CLAUSE_NOWAIT nowait, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_DISPATCH, SOLOMON_INTERNAL_TAG_OMP_INTEROP, SOLOMON_INTERNAL_TAG_OMP_SCOPE, SOLOMON_INTERNAL_TAG_OMP_SINGLE, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_TARGET_ENTER_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_EXIT_DATA, SOLOMON_INTERNAL_TAG_OMP_TARGET_UPDATE, SOLOMON_INTERNAL_TAG_OMP_TASKWAIT, SOLOMON_INTERNAL_TAG_OMP_WORKSHARE, SOLOMON_INTERNAL_TAG_OMP_NOWAIT)

///
/// @note Clauses in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks reduction clause
///

///
/// @brief add OpenMP clause "reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
/// @details Specifies a reduction-identifier and one or more list items.
///
/// @param[in] reduction_modifier inscan, task, default
/// @param[in] reduction_identifier +, *, &, |, ^, &&, ||
///
#define OMP_CLAUSE_REDUCTION(...) reduction(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_FOR, SOLOMON_INTERNAL_TAG_OMP_LOOP, SOLOMON_INTERNAL_TAG_OMP_PARALLEL, SOLOMON_INTERNAL_TAG_OMP_SCOPE, SOLOMON_INTERNAL_TAG_OMP_SECTION, SOLOMON_INTERNAL_TAG_OMP_SIMD, SOLOMON_INTERNAL_TAG_OMP_TASKLOOP, SOLOMON_INTERNAL_TAG_OMP_TEAMS)

///
/// @brief add OpenMP clause "in_reduction ([reduction_modifier,] reduction_identifier: list)" if applicable
/// @details Specifies a reduction-identifier and one or more list items.
/// @remarks parallel masked taskloop, parallel masked taskloop simd cannot use nowait clause
///
/// @param[in] reduction_modifier inscan, task, default
/// @param[in] reduction_identifier +, *, &, |, ^, &&, ||
///
#define OMP_CLAUSE_IN_REDUCTION(...) in_reduction(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_OMP_TASK, SOLOMON_INTERNAL_TAG_OMP_TARGET, SOLOMON_INTERNAL_TAG_OMP_IN_REDUCTION)

#endif  // !defined(SOLOMON_OMP_CLAUSE_HPP)
