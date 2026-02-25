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

#include "../pragma.hpp"       // PRAGMA
#include "../util/pickup.hpp"  // PICKUP_CLAUSES
#include "clause.hpp"

///
/// @note Constructs and Directives in OpenACC API 2.7 Reference Guide (https://www.openacc.org/sites/default/files/inline-files/API%20Guide%202.7.pdf)
///

///
/// @brief add arguments into _Pragma("acc __VA_ARGS__")
///
#define PRAGMA_ACC(...) PRAGMA(acc __VA_ARGS__)

///
/// @brief _Pragma("acc parallel [clause [[,] clause] ... ]")
/// @details A parallel construct launches a number of gangs executing in parallel, where each gang may support multiple workers, each with vector or SIMD operations.
///
#define PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC(parallel APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_PARALLEL), __VA_ARGS__))

///
/// @brief _Pragma("acc kernels [clause [[,] clause] ... ]")
/// @details A kernels construct surrounds loops to be executed on the device, typically as a sequence of kernel operations.
///
#define PRAGMA_ACC_KERNELS(...) PRAGMA_ACC(kernels APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_KERNELS), __VA_ARGS__))

///
/// @brief _Pragma("acc serial [clause [[,] clause] ... ]")
/// @details A serial construct surrounds loops or code to be executed serially on the device.
///
#define PRAGMA_ACC_SERIAL(...) PRAGMA_ACC(serial APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_SERIAL), __VA_ARGS__))

///
/// @brief _Pragma("acc data [clause [[,] clause] ... ]")
/// @details A device data construct defines a region of the program within which data is accessible by the device.
///
#define PRAGMA_ACC_DATA(...) PRAGMA_ACC(data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_DATA), __VA_ARGS__))

///
/// @brief _Pragma("acc enter data [clause [[,] clause] ... ]")
/// @details An enter data directive is used to allocate and move data to the device memory for the remainder of the program, or until a matching exit data directive deallocates the data.
///
#define PRAGMA_ACC_ENTER_DATA(...) PRAGMA_ACC(enter data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_ENTER_DATA), __VA_ARGS__))

///
/// @brief _Pragma("acc exit data [clause [[,] clause] ... ]")
/// @details For data that was created with the enter data directive, the exit data directive moves data from device memory and deallocates the memory.
///
#define PRAGMA_ACC_EXIT_DATA(...) PRAGMA_ACC(exit data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_EXIT_DATA), __VA_ARGS__))

///
/// @brief _Pragma("acc host_data [clause [[,] clause] ... ]")
/// @details A host_data construct makes the address of device data available on the host.
///
#define PRAGMA_ACC_HOST_DATA(...) PRAGMA_ACC(host_data APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_HOST_DATA), __VA_ARGS__))

///
/// @brief _Pragma("acc loop [clause [[,] clause] ... ]")
/// @details A loop construct applies to the immediately following loop or tightly nested loops, and describes the type of device parallelism to use to execute the iterations of the loop.
///
#define PRAGMA_ACC_LOOP(...) PRAGMA_ACC(loop APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_LOOP), __VA_ARGS__))

///
/// @brief _Pragma("acc cache (list)")
/// @details A cache directive may be added at the top of a partitioned loop. The elements or subarrays in the list are cached in the software-managed data cache.
///
#define PRAGMA_ACC_CACHE(...) PRAGMA_ACC(cache(__VA_ARGS__))

///
/// @brief _Pragma("acc atomic [read | write | update | capture]")
/// @details The atomic construct ensures that a specific storage location is accessed or updated atomically, preventing simultaneous, conflicting reading and writing threads.
///
#define PRAGMA_ACC_ATOMIC(...) PRAGMA_ACC(atomic APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_ATOMIC), __VA_ARGS__))

///
/// @brief _Pragma("acc update [clause [[,] clause] ... ]")
/// @details The update directive copies data between the memory for the local thread and the device. An update directive may appear in any data region, including an implicit data region.
///
#define PRAGMA_ACC_UPDATE(...) PRAGMA_ACC(update APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_UPDATE), __VA_ARGS__))

///
/// @brief _Pragma("acc wait [(expression_list)] [clause [[,] clause] ... ]")
/// @details The wait directive causes the local thread to wait for completion of asynchronous device activities, or for asynchronous activities on one async queue to synchronize with one or more other async queues. With no expression, it will wait for all outstanding asynchronous regions or data movement.
///
// #define PRAGMA_ACC_WAIT(...) PRAGMA_ACC(wait APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_WAIT), __VA_ARGS__))
#define PRAGMA_ACC_WAIT(...) PRAGMA_ACC(wait APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_WAIT), ACC_PASS_LIST(__VA_ARGS__)))

///
/// @brief _Pragma("acc routine [clause [[,] clause] ... ]")
/// @details The routine directive tells the compiler to compile a procedure for the device and gives the execution context for calls to the procedure. Such a procedure is called a device routine.
///
#define PRAGMA_ACC_ROUTINE(...) PRAGMA_ACC(routine APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_ROUTINE), __VA_ARGS__))

///
/// @brief _Pragma("acc declare [clause [[,] clause] ... ]")
/// @details A declare directive is used to specify that data is to be allocated in device memory for the duration of the implicit data region of the program or subprogram.
///
#define PRAGMA_ACC_DECLARE(...) PRAGMA_ACC(declare APPEND_CLAUSES(ARGS_WITH_NUM(CLAUSE_TAG_ACC_DECLARE), __VA_ARGS__))

#endif  // !defined(SOLOMON_ACC_DIRECTIVE_HPP)
