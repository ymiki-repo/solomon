///
/// @file solomon/omp/directive.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief OpenMP directives as preprocessor macros
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_OMP_DIRECTIVE_HPP)
#define SOLOMON_OMP_DIRECTIVE_HPP

#include "../pragma.hpp"       // PRAGMA
#include "../util/pickup.hpp"  // PICKUP_CLAUSES
#include "clause.hpp"

///
/// @brief add arguments into _Pragma("omp __VA_ARGS__")
///
#define PRAGMA_OMP(...) PRAGMA(omp __VA_ARGS__)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Data environment directives
///

///
/// @brief _Pragma("omp threadprivate (list)")
/// @details Specifies that variables are replicated, with each thread having its own copy. Each copy of a threadprivate variable is initialized once prior to the first reference to the copy.
///
/// @param[in] list A comma-separated list of file-scope, namespace-scope, or static block-scope variables that do not have incomplete types.
///
#define PRAGMA_OMP_THREADPRIVATE(...) PRAGMA_OMP(threadprivate(__VA_ARGS__))

///
/// @note skip _Pragma("omp declare reduction")
///

///
/// @brief _Pragma("omp scan clause")
/// @details Specifies that scan computations update the list items on each iteration of an enclosing loop nest associated with a worksharing-loop, worksharing-loop SIMD, or simd directive.
///
#define PRAGMA_OMP_SCAN(...) PRAGMA_OMP(scan APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_SCAN), __VA_ARGS__))

///
/// @note skip skip _Pragma("omp declare mapper")
///

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks skip: Memory management directives
///

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Variant directives
///

///
/// @note skip _Pragma("omp [begin] metadirective")
///

///
/// @note skip _Pragma("omp [begin] declare variant")
///

///
/// @note skip _Pragma("omp dispatch")
///

///
/// @brief _Pragma("omp declare simd [clause [[,] clause] ... ]")
/// @details Applied to a function or subroutine to enable creation of one or more versions to process multiple arguments using SIMD instructions from a single invocation in a SIMD loop.
///
#define PRAGMA_OMP_DECLARE_SIMD(...) PRAGMA_OMP(declare simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DECLARE_SIMD), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Informational and utility directives
///

///
/// @note skip _Pragma("omp requires")
///

///
/// @note skip _Pragma("assume"), _Pragma("[begin] assumes")
///

///
/// @note skip _Pragma("omp nothing")
///

///
/// @note skip _Pragma("omp error")
///

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Loop transformation constructs
///

///
/// @brief _Pragma("omp tile clause")
/// @details Tiles one or more loops.
///
#define PRAGMA_OMP_TILE(...) PRAGMA_OMP(tile APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TILE), __VA_ARGS__))

///
/// @brief _Pragma("omp unroll [clause [[,] clause] ... ]")
/// @details Fully or partially unrolls a loop.
///
#define PRAGMA_OMP_UNROLL(...) PRAGMA_OMP(unroll APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_UNROLL), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Parallelism constructs
///

///
/// @brief _Pragma("omp parallel [clause [[,] clause] ... ]")
/// @details Creates a team of OpenMP threads that execute the region.
///
#define PRAGMA_OMP_PARALLEL(...) PRAGMA_OMP(parallel APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_PARALLEL
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_PARALLEL PRAGMA_OMP(end parallel)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp teams [clause [[,] clause] ... ]")
/// @details Creates a league of initial teams where the initial thread of each team executes the region.
///
#define PRAGMA_OMP_TEAMS(...) PRAGMA_OMP(teams APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TEAMS
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_TEAMS PRAGMA_OMP(end teams)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp simd [clause [[,] clause] ... ]")
/// @details Applied to a loop to indicate that the loop can be transformed into a SIMD loop.
///
#define PRAGMA_OMP_SIMD(...) PRAGMA_OMP(simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_SIMD
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_SIMD PRAGMA_OMP(end simd)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp masked [clause [[,] clause] ... ]")
/// @details Specifies a structured block that is executed by a subset of the threads of the current team.
///
#define PRAGMA_OMP_MASKED(...) PRAGMA_OMP(masked APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_MASKED), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Work-distribution constructs
///

///
/// @brief _Pragma("omp single [clause [[,] clause] ... ]")
/// @details Specifies that the associated structured block is executed by only one of the threads in the team.
///
#define PRAGMA_OMP_SINGLE(...) PRAGMA_OMP(single APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_SINGLE), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_SINGLE
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_SINGLE PRAGMA_OMP(end single)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp workshare [clause [[,] clause] ... ]")
/// @details Divides the execution of the enclosed structured block into separate units of work, each executed only once by one thread.
///
#define PRAGMA_OMP_WORKSHARE(...) PRAGMA_OMP(workshare APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_WORKSHARE), __VA_ARGS__))

///
/// @brief _Pragma("omp scope [clause [[,] clause] ... ]")
/// @details Defines a structured block that is executed by all threads in a team but where additional OpenMP operations can be specified.
///
#define PRAGMA_OMP_SCOPE(...) PRAGMA_OMP(scope APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_SCOPE), __VA_ARGS__))

///
/// @brief _Pragma("omp sections [clause [[,] clause] ... ]")
/// @details A non-iterative worksharing construct that contains a set of structured blocks that are to be distributed among and executed by the threads in a team
///
#define PRAGMA_OMP_SECTIONS(...) PRAGMA_OMP(sections APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_SECTIONS), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_SECTIONS
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_SECTIONS PRAGMA_OMP(end sections)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp section")
/// @details A non-iterative worksharing construct that contains a set of structured blocks that are to be distributed among and executed by the threads in a team
///
#define PRAGMA_OMP_SECTION PRAGMA_OMP(section)

///
/// @brief _Pragma("omp for [clause [[,] clause] ... ]")
/// @details Specifies that the iterations of associated loops will be executed in parallel by threads in the team.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DO(...)  PRAGMA_OMP(for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_FOR(...) PRAGMA_OMP(for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_FOR(...) PRAGMA_OMP(do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_DO(...) PRAGMA_OMP(do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp distribute [clause [[,] clause] ... ]")
/// @details Specifies loops which are executed by the initial teams.
///
#define PRAGMA_OMP_DISTRIBUTE(...) PRAGMA_OMP(distribute APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE), __VA_ARGS__))

///
/// @brief _Pragma("omp loop [clause [[,] clause] ... ]")
/// @details Specifies that the iterations of the associated loops may execute concurrently and permits the encountering thread(s) to execute the loop accordingly
///
#define PRAGMA_OMP_LOOP(...) PRAGMA_OMP(loop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_LOOP), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Tasking constructs
///

///
/// @brief _Pragma("omp task [clause [[,] clause] ... ]")
/// @details Defines an explicit task. The data environment of the task is created according to the data-sharing attribute clauses on the task construct, per-data environment ICVs, and any defaults that apply.
///
#define PRAGMA_OMP_TASK(...) PRAGMA_OMP(task APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TASK), __VA_ARGS__))

///
/// @brief _Pragma("omp taskloop [clause [[,] clause] ... ]")
/// @details Specifies that the iterations of one or more associated loops will be executed in parallel using OpenMP tasks.
///
#define PRAGMA_OMP_TASKLOOP(...) PRAGMA_OMP(taskloop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TASKLOOP), __VA_ARGS__))

///
/// @brief _Pragma("omp taskyield")
/// @details Specifies that the current task can be suspended in favor of execution of a different task.
///
#define PRAGMA_OMP_TASKYIELD PRAGMA_OMP(taskyield)

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Interoperability construct
///

///
/// @brief _Pragma("omp interop clause [[[,] clause] ...]")
///
#define PRAGMA_OMP_INTEROP(...) PRAGMA_OMP(interop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_INTEROP), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Synchronization constructs
///

///
/// @brief _Pragma("omp critical [(name) [[,] hint (hint_expression)]]")
/// @details Restricts execution of the associated structured block to a single thread at a time.
///
/// @param[in] hint_expression omp_sync_hint_uncontended, omp_sync_hint_contended, omp_sync_hint_speculative, omp_sync_hint_nonspeculative
///
#define PRAGMA_OMP_CRITICAL(...) PRAGMA_OMP(critical APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_CRITICAL), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_CRITICAL
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_CRITICAL PRAGMA_OMP(end critical)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp barrier")
/// @details Specifies an explicit barrier that prevents any thread in a team from continuing past the barrier until all threads in the team encounter the barrier.
///
#define PRAGMA_OMP_BARRIER PRAGMA_OMP(barrier)

///
/// @brief _Pragma("omp taskgroup [clause [[,] clause] ... ]")
/// @details Specifies a region which a task cannot leave until all its descendant tasks generated inside the dynamic scope of the region have completed.
///
#define PRAGMA_OMP_TASKGROUP(...) PRAGMA_OMP(taskgroup APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TASKGROUP), __VA_ARGS__))

///
/// @brief _Pragma("omp taskwait [clause [[,] clause] ... ]")
/// @details Specifies a wait on the completion of child tasks of the current task.
///
#define PRAGMA_OMP_TASKWAIT(...) PRAGMA_OMP(taskwait APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TASKWAIT), __VA_ARGS__))

///
/// @brief _Pragma("omp flush [memory_order_clause] [(list)]")
/// @details Makes a thread’s temporary view of memory consistent with memory, and enforces an order on the memory operations of the variables.
///
#define PRAGMA_OMP_FLUSH(...) PRAGMA_OMP(flush APPEND_CLAUSES_WITHOUT_SORTING(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FLUSH), OMP_PASS_LIST(__VA_ARGS__)))

///
/// @brief _Pragma("omp depobj (depend_object) clause")
/// @details Stand-alone directive that initializes, updates, or destroys an OpenMP depend object.
///
#define PRAGMA_OMP_DEPOBJ(...) PRAGMA_OMP(depobj APPEND_CLAUSES_WITHOUT_SORTING(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DEPOBJ), OMP_PASS_LIST(__VA_ARGS__)))

///
/// @brief _Pragma("omp atomic [clause [[,] clause] ... ]")
/// @details Ensures a specific storage location is accessed atomically.
///
#define PRAGMA_OMP_ATOMIC(...) PRAGMA_OMP(atomic APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_ATOMIC), __VA_ARGS__))

///
/// @brief _Pragma("omp ordered [clause [[,] clause] ... ]")
/// @details Specifies a structured block that is to be executed in loop iteration order in a parallelized loop, or it specifies cross iteration dependences in a doacross loop nest.
///
#define PRAGMA_OMP_ORDERED(...) PRAGMA_OMP(ordered APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_ORDERED), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Cancellation constructs
///

// ///
// /// @brief _Pragma("omp cancel construct_type_clause [[,] if_clause]")
// /// @details Activates cancellation of the innermost enclosing region of the type specified.
// ///
// #define PRAGMA_OMP_CANCEL(...) PRAGMA_OMP(cancel APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_CANCEL), __VA_ARGS__))

// ///
// /// @brief _Pragma("omp cancellation point construct_type_clause")
// /// @details Introduces a user-defined cancellation point at which tasks check if cancellation of the innermost enclosing region of the type specified has been activated.
// ///
// #define PRAGMA_OMP_CANCELLATION_POINT(...) PRAGMA_OMP(cancellation point APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_CANCELLATION_POINT), __VA_ARGS__))

///
/// @note Directives and Constructs in OpenMP 5.2 API Syntax Reference Guide (https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf)
/// @remarks Combined Constructs and Directives
/// @details The following combined constructs and directives are created following the parameters defined in section 17 of the OpenMP API version 5.2 specification and were explicitly defined in previous versions.
///

///
/// @brief _Pragma("omp for simd [clause [[,] clause] ... ]")
/// @details Specifies that the iterations of associated loops will be executed in parallel by threads in the team and the iterations executed by each thread can also be executed concurrently using SIMD instructions.
/// @note clause: Any of the clauses accepted by the simd or for directives.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DO_SIMD(...)  PRAGMA_OMP(for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_NOWAIT, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_FOR_SIMD(...) PRAGMA_OMP(for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_NOWAIT, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DO_SIMD(...) PRAGMA_OMP(do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_NOWAIT, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_FOR_SIMD(...) PRAGMA_OMP(do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_NOWAIT, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp distribute simd [clause [[,] clause] ... ]")
/// @details Specifies a loop that will be distributed across the primary threads of the teams region and executed concurrently using SIMD instructions.
/// @note clause: Any of the clauses accepted by the distribute or simd.
///
#define PRAGMA_OMP_DISTRIBUTE_SIMD(...) PRAGMA_OMP(distribute simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp distribute parallel for [clause [[,] clause] ... ]")
/// @details Specify a loop that can be executed in parallel by multiple threads that are members of multiple teams.
/// @note clause: Any clause used for distribute or parallel for.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_DO(...)  PRAGMA_OMP(distribute parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP(distribute parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_DO(...) PRAGMA_OMP(distribute parallel do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP(distribute parallel do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp distribute parallel for [clause [[,] clause] ... ]")
/// @details Specifies a loop that can be executed concurrently using SIMD instructions in parallel by multiple threads that are members of multiple teams.
/// @note clause: Any clause used for distribute or parallel for simd.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_DO_SIMD(...)  PRAGMA_OMP(distribute parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(distribute parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_DO_SIMD(...) PRAGMA_OMP(distribute parallel do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(distribute parallel do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp taskloop simd [clause [[,] clause] ... ]")
/// @details Specifies that a loop can be executed concurrently using SIMD instructions, and that those iterations will also be executed in parallel using OpenMP tasks.
/// @note clause: Any clause used for simd or taskloop.
///
#define PRAGMA_OMP_TASKLOOP_SIMD(...) PRAGMA_OMP(taskloop simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TASKLOOP, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp parallel for [clause [[,] clause] ... ]")
/// @details Specifies a parallel construct containing a worksharing-loop construct with a canonical loop nest and no other statements.
/// @note clause: Any clause used for parallel or for except the nowait clause.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_PARALLEL_DO(...)  PRAGMA_OMP(parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_PARALLEL_FOR(...) PRAGMA_OMP(parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_PARALLEL_DO(...) PRAGMA_OMP(parallel do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_PARALLEL_FOR(...) PRAGMA_OMP(parallel do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp parallel loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing a loop construct with a canonical loop nest and no other statements.
/// @note clause: Any clause used for parallel or loop.
///
#define PRAGMA_OMP_PARALLEL_LOOP(...) PRAGMA_OMP(parallel loop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_LOOP), __VA_ARGS__))

///
/// @brief _Pragma("omp parallel sections [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing a sections construct and no other statements.
/// @note clause: Any clause used for parallel or sections except the nowait clause.
///
#define PRAGMA_OMP_PARALLEL_SECTIONS(...) PRAGMA_OMP(parallel sections APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_SECTIONS), __VA_ARGS__))
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_PARALLEL_SECTIONS
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_END_PARALLEL_SECTIONS PRAGMA_OMP(end parallel sections)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp parallel for simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing only one worksharing-loop SIMD construct.
/// @note clause: Any clause used for parallel or for simd except the nowait clause.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_PARALLEL_DO_SIMD(...)   PRAGMA_OMP(parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_PARALLEL_FOR_SIMD(...)  PRAGMA_OMP(parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_PARALLEL_DO_SIMD(...) PRAGMA_OMP(parallel do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(parallel do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp masked taskloop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a masked construct containing a taskloop construct and no other statements.
/// @note clause: Any clause used for taskloop or masked.
///
#define PRAGMA_OMP_MASKED_TASKLOOP(...) PRAGMA_OMP(masked taskloop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_MASKED, CLAUSE_TAG_OMP_TASKLOOP, CLAUSE_TAG_OMP_IN_REDUCTION), __VA_ARGS__))

///
/// @brief _Pragma("omp masked taskloop simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a masked construct containing a taskloop simd construct and no other statements.
/// @note clause: Any clause used for masked or taskloop simd.
///
#define PRAGMA_OMP_MASKED_TASKLOOP_SIMD(...) PRAGMA_OMP(masked taskloop simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_MASKED, CLAUSE_TAG_OMP_TASKLOOP, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp parallel masked taskloop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing a masked taskloop construct and no other statements.
/// @note clause: Any clause used for parallel or masked taskloop except the in_reduction clause.
///
#define PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP(...) PRAGMA_OMP(parallel masked taskloop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_MASKED, CLAUSE_TAG_OMP_TASKLOOP), __VA_ARGS__))

///
/// @brief _Pragma("omp parallel masked taskloop simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a parallel construct containing a masked taskloop simd construct and no other statements.
/// @note clause: Any clause used for masked taskloop simd or parallel except the in_reduction clause.
///
#define PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP_SIMD(...) PRAGMA_OMP(parallel masked taskloop simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_MASKED, CLAUSE_TAG_OMP_TASKLOOP, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp teams distribute [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute construct and no other statements.
/// @note clause: Any clause used for teams or distribute.
///
#define PRAGMA_OMP_TEAMS_DISTRIBUTE(...) PRAGMA_OMP(teams distribute APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE), __VA_ARGS__))

///
/// @brief _Pragma("omp teams distribute simd [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute simd construct and no other statements.
/// @note clause: Any clause used for teams or distribute simd.
///
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(...) PRAGMA_OMP(teams distribute simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))

///
/// @brief _Pragma("omp teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute parallel worksharing-loop construct and no other statements.
/// @note clause: Any clause used for teams or distribute parallel for.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO(...)  PRAGMA_OMP(teams distribute parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP(teams distribute parallel for APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO(...) PRAGMA_OMP(teams distribute parallel do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR(...) PRAGMA_OMP(teams distribute parallel do APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp teams distribute parallel for [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a distribute parallel for simd construct and no other statements.
/// @note clause: Any clause used for teams or distribute parallel for simd.
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...)  PRAGMA_OMP(teams distribute parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(teams distribute parallel for simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...) PRAGMA_OMP(teams distribute parallel do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#define PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...) PRAGMA_OMP(teams distribute parallel do simd APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_DISTRIBUTE, CLAUSE_TAG_OMP_PARALLEL, CLAUSE_TAG_OMP_COPYIN, CLAUSE_TAG_OMP_FOR, CLAUSE_TAG_OMP_SIMD), __VA_ARGS__))
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("omp teams loop [clause [[,] clause] ... ]")
/// @details Shortcut for specifying a teams construct containing a loop construct and no other statements.
/// @note clause: Any clause used for teams or loop.
///
#define PRAGMA_OMP_TEAMS_LOOP(...) PRAGMA_OMP(teams loop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_OMP_TEAMS, CLAUSE_TAG_OMP_LOOP), __VA_ARGS__))

#endif  // !defined(SOLOMON_OMP_DIRECTIVE_HPP)
