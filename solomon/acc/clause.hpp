///
/// @file solomon/acc/clause.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief optional clauses for OpenACC directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_ACC_CLAUSE_HPP)
#define SOLOMON_ACC_CLAUSE_HPP

#include "../util/va_args.hpp"
#include "clause_tag.hpp"

///
/// @note Compute Construct Clauses in OpenACC API 2.7 Reference Guide (https://www.openacc.org/sites/default/files/inline-files/API%20Guide%202.7.pdf)
///

///
/// @brief add OpenACC clause "if (condition)" if applicable
/// @details When the condition is nonzero or .TRUE. the compute region will execute on the device; otherwise, the local thread will execute the region.
///
#define ACC_CLAUSE_IF(condition) \
  if (condition) , SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_HOST_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE)

///
/// @brief add OpenACC clause "self [(condition)]" or "self (list)" if applicable
/// @details When the condition is not present or when the condition is nonzero or .TRUE., the compute region will execute on the local device; otherwise, the compute region will execute on the current device.
/// @details Copies the data in list from the device to the local thread. (for update directive)
///
#define ACC_CLAUSE_SELF(...) self(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_UPDATE)

///
/// @brief add OpenACC clause "default (none | present)" if applicable
/// @details none: Prevents the compiler from implicitly determining data attributes for any variable used or assigned in the construct.
/// @details present: Implicitly assume any non-scalar data not specified in a data clause is present.
///
#define ACC_CLAUSE_DEFAULT(mode) default(mode), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA)

///
/// @brief add OpenACC clause "default (none)" if applicable
/// @details Prevents the compiler from implicitly determining data attributes for any variable used or assigned in the construct.
///
#define ACC_CLAUSE_DEFAULT_NONE ACC_CLAUSE_DEFAULT(none)

///
/// @brief add OpenACC clause "default (present)" if applicable
/// @details Implicitly assume any non-scalar data not specified in a data clause is present.
///
#define ACC_CLAUSE_DEFAULT_PRESENT ACC_CLAUSE_DEFAULT(present)

///
/// @brief add OpenACC clause "device_type ([* | device_type_list])" if applicable
/// @details May be followed by any of the clauses below. Clauses following device_type will apply only when compiling for the given device type(s). Clauses following device_type( * ) apply to all devices not named in another device_type clause. May appear more than once with different device types.
///
#define ACC_CLAUSE_DEVICE_TYPE(...) device_type(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_LOOP, SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "async [(expression)]" if applicable
/// @details The compute region executes asynchronously with the local thread on the corresponding async queue. With no argument, the compute region will execute on the default async queue.
///
#define ACC_CLAUSE_ASYNC(...) async(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE, SOLOMON_INTERNAL_TAG_ACC_WAIT)

///
/// @brief add OpenACC clause "wait [(expression_list)]" if applicable
/// @details The compute region will not begin execution until all actions on the corresponding async queue(s) are complete. With no argument, the compute region will wait on all async queues.
///
#define ACC_CLAUSE_WAIT(...) wait(__VA_ARGS__) SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE)

///
/// @brief add OpenACC clause "finalize" if applicable
/// @details Sets the dynamic reference count to zero.
///
#define ACC_CLAUSE_FINALIZE finalize, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA)

///
/// @brief add OpenACC clause "num_gangs (n)" if applicable
/// @details Controls how many parallel gangs are created.
///
#define ACC_CLAUSE_NUM_GANGS(n) num_gangs(n), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS)

///
/// @brief add OpenACC clause "num_workers (n)" if applicable
/// @details Controls how many workers are created in each gang.
///
#define ACC_CLAUSE_NUM_WORKERS(n) num_workers(n), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS)

///
/// @brief add OpenACC clause "vector_length (n)" if applicable
/// @details Controls the vector length on each worker.
///
#define ACC_CLAUSE_VECTOR_LENGTH(n) vector_length(n), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS)

///
/// @brief add OpenACC clause "reduction (operator: list)" if applicable
/// @details A private copy of each variable, array, or subarray in list is allocated for each gang. The values for all gangs are combined with the operator at the end of the parallel region. Array reductions are performed individually for each element. Composite variable reductions are performed individually for each member. Valid C and C++ operators are +, *, max, min, &, |, ^, &&, ||. Appearance of a variable in a reduction clause implies a copy data clause for that variable.
///
#define ACC_CLAUSE_REDUCTION(...) reduction(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_LOOP)

///
/// @brief add OpenACC clause "private (list)" if applicable
/// @details A copy of each variable in list is allocated for each gang.
///
#define ACC_CLAUSE_PRIVATE(...) private(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_LOOP)

///
/// @brief add OpenACC clause "firstprivate (list)" if applicable
/// @details A copy of each variable in list is allocated for each gang and initialized with the value of the variable of the local thread.
///
#define ACC_CLAUSE_FIRSTPRIVATE(...) firstprivate(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_SERIAL)

///
/// @note Data Clauses in OpenACC API 2.7 Reference Guide (https://www.openacc.org/sites/default/files/inline-files/API%20Guide%202.7.pdf)
///

///
/// @brief add OpenACC clause "copy (list)" if applicable
/// @details When entering the region, if the data in list is already present in the current device memory, the structured reference count is incremented and that copy is used. Otherwise, it allocates device memory and copies the values from local memory and sets the structured reference count to one. When exiting the region, the structured reference count is decremented. If both reference counts are zero, the data is copied from device memory to local memory and the device memory is deallocated.
///
#define ACC_CLAUSE_COPY(...) copy(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "copyin ([readonly:] list)" if applicable
/// @details When entering the region or at an enter data directive, if the data in list is already present in the current device memory, the appropriate reference count is incremented and that copy is used. Otherwise, it allocates device memory and copies the values from local memory and sets the appropriate reference count to one. When exiting the region the structured reference count is decremented. If both reference counts are zero, the device memory is deallocated. If the readonly modifier appears, the data must not be written within that region.
///
#define ACC_CLAUSE_COPYIN(...) copyin(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "copyout (list)" if applicable
/// @details When entering the region, if the data in list is already present in the current device memory, the structured reference count is incremented and that copy is used. Otherwise, it allocates device memory and sets the structured reference count to one. At an exit data directive with no finalize clause or when exiting the region, the appropriate reference count is decremented. At an exit data directive with a finalize clause, the dynamic reference count is set to zero. In any case, if both reference counts are zero, the data is copied from device memory to local memory and the device memory is deallocated.
///
#define ACC_CLAUSE_COPYOUT(...) copyout(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA, SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "create (list)" if applicable
/// @details When entering the region or at an enter data directive, if the data in list is already present in the current device memory, the appropriate reference count is incremented and that copy is used. Otherwise, it allocates device memory and sets the appropriate reference count to one. When exiting the region, the structured reference count is decremented. If both reference counts are zero, the device memory is deallocated.
///
#define ACC_CLAUSE_CREATE(...) create(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA, SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "no_create (list)" if applicable
/// @details When entering the region, if the data in list is already present in the current device memory, the structured reference count is incremented and that copy is used. Otherwise, no action is performed and any device code in the construct will use the local memory address for that data.
///
#define ACC_CLAUSE_NO_CREATE(...) no_create(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA)

///
/// @brief add OpenACC clause "delete (list)" if applicable
/// @details With no finalize clause, the dynamic reference count is decremented. With a finalize clause, the dynamic reference count is set to zero. In either case, if both reference counts are zero, the device memory is deallocated.
///
#define ACC_CLAUSE_DELETE(...) delete (__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA)

///
/// @brief add OpenACC clause "present (list)" if applicable
/// @details When entering the region, the data must be present in the current device memory, and the structured reference count is incremented. When exiting the region, the structured reference count is decremented.
///
#define ACC_CLAUSE_PRESENT(...) present(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "deviceptr (list)" if applicable
/// @details C and C++; the list entries must be pointer variables that contain device addresses, such as from acc_malloc.
///
#define ACC_CLAUSE_DEVICEPTR(...) deviceptr(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "attach (list)" if applicable
/// @details When entering the region or at an enter data directive, if the pointers in list are already attached to their targets, the attachment count is incremented. Otherwise, it attaches the device pointers to their device targets and sets the attachment count to one. When exiting the region, the attachment count is decremented, and if the count reaches zero, the pointers will be detached.
///
#define ACC_CLAUSE_ATTACH(...) attach(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_PARALLEL, SOLOMON_INTERNAL_TAG_ACC_KERNELS, SOLOMON_INTERNAL_TAG_ACC_SERIAL, SOLOMON_INTERNAL_TAG_ACC_DATA, SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA)

///
/// @brief add OpenACC clause "detach (list)" if applicable
/// @details With no finalize clause, the attachment count for the pointers in list are decremented. With a finalize clause, the attachment counts are set to zero. In either case, if the attachment count becomes zero, the pointers in list will be detached.
///
#define ACC_CLAUSE_DETACH(...) detach(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA)

///
/// @brief add OpenACC clause "use_device (list)" if applicable
/// @details Directs the compiler to use the device address of any entry in list, for instance, when passing a variable to a procedure.
///
#define ACC_CLAUSE_USE_DEVICE(...) use_device(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_HOST_DATA)

///
/// @brief add OpenACC clause "if_present" if applicable
/// @details If any entry in list is not present in the current device memory, the device address will not be used and no error will result.
///
#define ACC_CLAUSE_IF_PRESENT if_present, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_HOST_DATA, SOLOMON_INTERNAL_TAG_ACC_UPDATE)

///
/// @brief add OpenACC clause "collapse (n)" if applicable
/// @details Applies the associated directive to the following n tightly nested loops.
///
#define ACC_CLAUSE_COLLAPSE(n) collapse(n), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP)

///
/// @brief add OpenACC clause "seq" if applicable
/// @details Executes the loop or loops sequentially.
///
#define ACC_CLAUSE_SEQ seq, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP, SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "auto" if applicable
/// @details Instructs the compiler to analyze the loop or loops to determine whether it can be safely executed in parallel, and if so, to apply gang, worker or vector parallelism.
///
#define ACC_CLAUSE_AUTO auto, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP)

///
/// @brief add OpenACC clause "independent" if applicable
/// @details Specifies that the loop iterations are data-independent and can be executed in parallel, overriding compiler dependence analysis.
///
#define ACC_CLAUSE_INDEPENDENT independent, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP)

///
/// @brief add OpenACC clause "tile (expression_list)" if applicable
/// @details With n expressions, specifies that the following n tightly nested loops should be split into n outer tile loops and n inner element loops, where the trip counts of the element loops are taken from the expression. The first entry applies to the innermost element loop. May be combined with one or two of gang, worker and vector clauses.
///
#define ACC_CLAUSE_TILE(...) tile(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP)

///
/// @brief add OpenACC clause "gang" if applicable
/// @details Shares the iterations of the loop or loops across the gangs of the parallel region.
///
#define ACC_CLAUSE_GANG gang, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP, SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "worker" if applicable
/// @details Shares the iterations of the loop or loops across the workers of the gang.
///
#define ACC_CLAUSE_WORKER worker, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP, SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "vector" if applicable
/// @details Executes the iterations of the loop or loops in SIMD or vector mode.
///
#define ACC_CLAUSE_VECTOR vector, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_LOOP, SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "read" if applicable (read: v = x;)
///
#define ACC_CLAUSE_READ read, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ATOMIC)

///
/// @brief add OpenACC clause "write" if applicable (write: x = expr;)
///
#define ACC_CLAUSE_WRITE write, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ATOMIC)

///
/// @brief add OpenACC clause "update" if applicable (default mode: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
///
#define ACC_CLAUSE_UPDATE update, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ATOMIC)

///
/// @brief add OpenACC clause "capture" if applicable (v = update-expr, where update-expr is one of: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
///
#define ACC_CLAUSE_CAPTURE capture, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ATOMIC)

///
/// @brief add OpenACC clause "host (list)" if applicable
/// @details Copies the data in list from the device to the local thread.
///
#define ACC_CLAUSE_HOST(...) host(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_UPDATE)

///
/// @brief add OpenACC clause "device (list)" if applicable
/// @details Copies the data in list from the local thread to the device.
///
#define ACC_CLAUSE_DEVICE(...) device(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_UPDATE)

///
/// @brief pass additional list if applicable
///
#define ACC_PASS_LIST(...) (__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_WAIT)

///
/// @brief add OpenACC clause "bind (name or string)" if applicable
/// @details bind (name): Specifies an alternate procedure name to use when compiling or calling the procedure on the device.
/// @details bind (string): Specifies a quoted string to use for the name when compiling or calling the procedure on the device.
///
#define ACC_CLAUSE_BIND(arg) bind(arg), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "nohost" if applicable
/// @details Specifies that a host version of the procedure should not be compiled.
///
#define ACC_CLAUSE_NOHOST nohost, SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ROUTINE)

///
/// @brief add OpenACC clause "device_resident (list)" if applicable
/// @details Specifies that the variables in list are to be allocated on the device for the duration of the implicit data region.
///
#define ACC_CLAUSE_DEVICE_RESIDENT(...) device_resident(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_DECLARE)

///
/// @brief add OpenACC clause "link (list)" if applicable
/// @details For large global static data objects, specifies that a global link for each object in list is to be statically allocated on the device. Device memory for the object will be allocated when the object appears in a data clause, and the global link will be assigned.
///
#define ACC_CLAUSE_LINK(...) link(__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_DECLARE)

#endif  // !defined(SOLOMON_ACC_CLAUSE_HPP)
