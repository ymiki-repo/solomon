///
/// @file solomon/acc/directive.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief OpenACC directives as preprocessor macros
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_ACC_DIRECTIVE_HPP)
#define SOLOMON_ACC_DIRECTIVE_HPP

#include "../pragma.hpp"       // SOLOMON_INTERNAL_PRAGMA
#include "../util/pickup.hpp"  // SOLOMON_INTERNAL_APPEND_CLAUSES
#include "../util/va_opt.hpp"  // SOLOMON_INTERNAL_USE_VA_OPT
#include "../util/cloak.hpp"   // SOLOMON_INTERNAL_CLOAK_IIF
#include "../util/is_empty.hpp"  // SOLOMON_INTERNAL_IS_EMPTY
#include "clause.hpp"

///
/// @brief 1 when no clause of the required set (pseudo tag, see clause_tag.hpp) is left for an OpenACC data directive, 0 otherwise
/// @details OpenACC 3.3: data (2.6.5), enter data / exit data (2.6.6), host_data (2.8), and update (2.14.4) need at least one of them.
/// A mark of an undefined name or of the clause limit survives every pick, so such a directive keeps its own message.
///
#define SOLOMON_INTERNAL_ACC_NO_REQUIRED(tag, ...) SOLOMON_INTERNAL_IS_EMPTY(SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(tag), __VA_ARGS__))
///
/// @brief stop an OpenACC data directive none of whose clauses is one of its required clauses
/// @details C/C++: a preprocessor / compiler error with the message below; Fortran: the directive with a name that the compiler rejects
///
#if !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC_NOREQ(directive) _Pragma("GCC error \"Solomon: this OpenACC data directive has none of its required clauses (data: copy/copyin/copyout/create/no_create/present/deviceptr/attach/default, enter data: copyin/create/attach, exit data: copyout/delete/detach, host_data: use_device, update: self/host/device)\"")
#else  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC_NOREQ(directive) PRAGMA_ACC(directive SOLOMON_ERROR_NO_REQUIRED_CLAUSE_IN_OPENACC_DATA_DIRECTIVE)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @note directives (chapter 2) in The OpenACC Application Programming Interface 3.3 (https://www.openacc.org/sites/default/files/inline-images/Specification/OpenACC-3.3-final.pdf)
///

///
/// @brief add arguments into _Pragma("acc __VA_ARGS__")
///
#define PRAGMA_ACC(...) SOLOMON_INTERNAL_PRAGMA(acc __VA_ARGS__)

///
/// @brief _Pragma("acc parallel [clause [[,] clause] ... ]")
/// @details A parallel construct launches a number of gangs executing in parallel, where each gang may support multiple workers, each with vector or SIMD operations.
///
#define PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC(parallel SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_O2A_THREAD_LIMIT), __VA_ARGS__))
///
/// @brief finalize the acc parallel region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_PARALLEL
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_PARALLEL PRAGMA_ACC(end parallel)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc kernels [clause [[,] clause] ... ]")
/// @details A kernels construct surrounds loops to be executed on the device, typically as a sequence of kernel operations.
///
#define PRAGMA_ACC_KERNELS(...) PRAGMA_ACC(kernels SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_O2A_THREAD_LIMIT), __VA_ARGS__))
///
/// @brief finalize the acc kernels region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_KERNELS
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_KERNELS PRAGMA_ACC(end kernels)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc serial [clause [[,] clause] ... ]")
/// @details A serial construct surrounds loops or code to be executed serially on the device.
///
#define PRAGMA_ACC_SERIAL(...) PRAGMA_ACC(serial SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_SERIAL), __VA_ARGS__))
///
/// @brief finalize the acc serial region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_SERIAL
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_SERIAL PRAGMA_ACC(end serial)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc data [clause [[,] clause] ... ]")
/// @details A device data construct defines a region of the program within which data is accessible by the device.
///
#define PRAGMA_ACC_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_DATA, __VA_ARGS__))(SOLOMON_INTERNAL_ACC_NOREQ_DATA, SOLOMON_INTERNAL_ACC_DATA_I)(__VA_ARGS__)
#define SOLOMON_INTERNAL_ACC_NOREQ_DATA(...) SOLOMON_INTERNAL_ACC_NOREQ(data)
#define SOLOMON_INTERNAL_ACC_DATA_I(...) PRAGMA_ACC(data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_DATA), __VA_ARGS__))
///
/// @brief finalize the acc data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_DATA PRAGMA_ACC(end data)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc enter data [clause [[,] clause] ... ]")
/// @details An enter data directive is used to allocate and move data to the device memory for the remainder of the program, or until a matching exit data directive deallocates the data.
///
#define PRAGMA_ACC_ENTER_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER, __VA_ARGS__))(SOLOMON_INTERNAL_ACC_NOREQ_ENTER_DATA, SOLOMON_INTERNAL_ACC_ENTER_DATA_I)(__VA_ARGS__)
#define SOLOMON_INTERNAL_ACC_NOREQ_ENTER_DATA(...) SOLOMON_INTERNAL_ACC_NOREQ(enter data)
#define SOLOMON_INTERNAL_ACC_ENTER_DATA_I(...) PRAGMA_ACC(enter data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA), __VA_ARGS__))

///
/// @brief _Pragma("acc exit data [clause [[,] clause] ... ]")
/// @details For data that was created with the enter data directive, the exit data directive moves data from device memory and deallocates the memory.
///
#define PRAGMA_ACC_EXIT_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT, __VA_ARGS__))(SOLOMON_INTERNAL_ACC_NOREQ_EXIT_DATA, SOLOMON_INTERNAL_ACC_EXIT_DATA_I)(__VA_ARGS__)
#define SOLOMON_INTERNAL_ACC_NOREQ_EXIT_DATA(...) SOLOMON_INTERNAL_ACC_NOREQ(exit data)
#define SOLOMON_INTERNAL_ACC_EXIT_DATA_I(...) PRAGMA_ACC(exit data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA), __VA_ARGS__))

///
/// @brief _Pragma("acc host_data [clause [[,] clause] ... ]")
/// @details A host_data construct makes the address of device data available on the host.
///
#define PRAGMA_ACC_HOST_DATA(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_HOST_DATA, __VA_ARGS__))(SOLOMON_INTERNAL_ACC_NOREQ_HOST_DATA, SOLOMON_INTERNAL_ACC_HOST_DATA_I)(__VA_ARGS__)
#define SOLOMON_INTERNAL_ACC_NOREQ_HOST_DATA(...) SOLOMON_INTERNAL_ACC_NOREQ(host_data)
#define SOLOMON_INTERNAL_ACC_HOST_DATA_I(...) PRAGMA_ACC(host_data SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_HOST_DATA), __VA_ARGS__))
///
/// @brief finalize the acc host_data region
///
#if !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_HOST_DATA
#else  // !defined(SOLOMON_FORTRAN)
#define PRAGMA_ACC_END_HOST_DATA PRAGMA_ACC(end host_data)
#endif  // !defined(SOLOMON_FORTRAN)

///
/// @brief _Pragma("acc loop [clause [[,] clause] ... ]")
/// @details A loop construct applies to the immediately following loop or tightly nested loops, and describes the type of device parallelism to use to execute the iterations of the loop.
///
#define PRAGMA_ACC_LOOP(...) PRAGMA_ACC(loop SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP), __VA_ARGS__))

///
/// @brief _Pragma("acc cache (list)")
/// @details A cache directive may be added at the top of a partitioned loop. The elements or subarrays in the list are cached in the software-managed data cache.
///
#define PRAGMA_ACC_CACHE(...) PRAGMA_ACC(cache(__VA_ARGS__))

///
/// @brief _Pragma("acc atomic [read | write | update | capture]")
/// @details The atomic construct ensures that a specific storage location is accessed or updated atomically, preventing simultaneous, conflicting reading and writing threads.
///
#define PRAGMA_ACC_ATOMIC(...) PRAGMA_ACC(atomic SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ATOMIC), __VA_ARGS__))

///
/// @brief _Pragma("acc update [clause [[,] clause] ... ]")
/// @details The update directive copies data between the memory for the local thread and the device. An update directive may appear in any data region, including an implicit data region.
///
#define PRAGMA_ACC_UPDATE(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE, __VA_ARGS__))(SOLOMON_INTERNAL_ACC_NOREQ_UPDATE, SOLOMON_INTERNAL_ACC_UPDATE_I)(__VA_ARGS__)
#define SOLOMON_INTERNAL_ACC_NOREQ_UPDATE(...) SOLOMON_INTERNAL_ACC_NOREQ(update)
#define SOLOMON_INTERNAL_ACC_UPDATE_I(...) PRAGMA_ACC(update SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_UPDATE), __VA_ARGS__))

///
/// @brief _Pragma("acc wait [(wait-argument)]")
/// @details The wait directive causes the local thread to wait for completion of asynchronous device activities, or for asynchronous activities on one async queue to synchronize with one or more other async queues. With no expression, it will wait for all outstanding asynchronous regions or data movement.
/// The arguments are the queue IDs only (wait-argument); clauses cannot be passed. In the OpenMP target backend the arguments are dropped and "omp taskwait" is emitted; nothing is emitted in fallback mode.
///
/// @note Without arguments it emits "acc wait", in every language mode (gcc / g++ in a strict mode such as -std=c99 or -std=c++17 included).
///
#if defined(SOLOMON_INTERNAL_USE_VA_OPT)
#define PRAGMA_ACC_WAIT(...) PRAGMA_ACC(wait __VA_OPT__(SOLOMON_INTERNAL_APPEND_CLAUSES_WITHOUT_SORTING(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_WAIT), ACC_PASS_LIST(__VA_ARGS__))))
#else  // defined(SOLOMON_INTERNAL_USE_VA_OPT)
#define PRAGMA_ACC_WAIT(...) PRAGMA_ACC(wait SOLOMON_INTERNAL_CLOAK_IF(SOLOMON_INTERNAL_CLOAK_NOT(SOLOMON_INTERNAL_IS_EMPTY(__VA_ARGS__)))(SOLOMON_INTERNAL_APPEND_CLAUSES_WITHOUT_SORTING(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_WAIT), ACC_PASS_LIST(__VA_ARGS__)), ))
#endif  // defined(SOLOMON_INTERNAL_USE_VA_OPT)

///
/// @brief _Pragma("acc routine [clause [[,] clause] ... ]")
/// @details The routine directive tells the compiler to compile a procedure for the device and gives the execution context for calls to the procedure. Such a procedure is called a device routine.
///
#define PRAGMA_ACC_ROUTINE(...) PRAGMA_ACC(routine SOLOMON_INTERNAL_APPEND_CLAUSES_WITHOUT_SORTING(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ROUTINE), __VA_ARGS__))
///
/// @brief finalize the device routine
///
#define PRAGMA_ACC_END_ROUTINE

///
/// @brief _Pragma("acc declare [clause [[,] clause] ... ]")
/// @details A declare directive is used to specify that data is to be allocated in device memory for the duration of the implicit data region of the program or subprogram.
/// The OpenACC specification requires at least one clause in every version (2.6 and 2.7, section 2.13: "declare clause-list",
/// where a clause-list is "a list of one or more clauses" (section 1.4); from 3.0 also an explicit restriction), and every
/// clause of declare is a data clause, so the directive stops when no clause is left, whatever _OPENACC is (as the data
/// directives above).
///
#define PRAGMA_ACC_DECLARE(...) SOLOMON_INTERNAL_CLOAK_IIF(SOLOMON_INTERNAL_ACC_NO_REQUIRED(SOLOMON_INTERNAL_TAG_ACC_DECLARE, __VA_ARGS__))(SOLOMON_INTERNAL_ACC_NOREQ_DECLARE, SOLOMON_INTERNAL_ACC_DECLARE_I)(__VA_ARGS__)
#if !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC_NOREQ_DECLARE(...) _Pragma("GCC error \"Solomon: the OpenACC specification requires at least one clause on a declare directive (copy/copyin/copyout/create/present/deviceptr/device_resident/link), and none is given here\"")
#else  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC_NOREQ_DECLARE(...) PRAGMA_ACC(declare SOLOMON_ERROR_OPENACC_REQUIRES_A_CLAUSE_ON_DECLARE_DIRECTIVE)
#endif  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_INTERNAL_ACC_DECLARE_I(...) PRAGMA_ACC(declare SOLOMON_INTERNAL_APPEND_CLAUSES(SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_DECLARE), __VA_ARGS__))

#endif  // !defined(SOLOMON_ACC_DIRECTIVE_HPP)
