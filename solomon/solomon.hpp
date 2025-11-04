///
/// @file solomon/solomon.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief pragmas to offload operations by directives
///
/// @note please cite Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665, https://doi.org/10.1109/ACCESS.2024.3509380)
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_SOLOMON_HPP)
#define SOLOMON_SOLOMON_HPP

#define SOLOMON_VERSION 1.0.0

// OpenMP for multicore CPU is always available
#if defined(_OPENMP)
#include "omp.hpp"
#endif  // defined(_OPENMP)

// disable unavailable backends
#if defined(OFFLOAD_BY_OPENACC) && !defined(_OPENACC)
// OpenACC is not activated, disable OpenACC as offloading backend
#undef OFFLOAD_BY_OPENACC
#endif  // defined(OFFLOAD_BY_OPENACC) && !defined(_OPENACC)
#if defined(OFFLOAD_BY_OPENMP_TARGET) && !(_OPENMP >= 201307)
// OpenMP 4.0+ is not activated, disable OpenMP target directives as offloading backend
#undef OFFLOAD_BY_OPENMP_TARGET
#endif  // defined(OFFLOAD_BY_OPENMP_TARGET) && !(_OPENMP >= 201307)

// select only one backend for offloading
#if defined(OFFLOAD_BY_OPENACC) && defined(OFFLOAD_BY_OPENMP_TARGET)
#undef OFFLOAD_BY_OPENMP_TARGET
#endif  // defined(OFFLOAD_BY_OPENACC) && defined(OFFLOAD_BY_OPENMP_TARGET)

///
/// @brief "arg" appears only in the fallback mode (when GPU offloading is disabled)
///
#define IF_NOT_OFFLOADED(arg)

// set backend
#if defined(OFFLOAD_BY_OPENACC)
// use OpenACC directives for GPU offloading
#include "acc.hpp"
#elif defined(OFFLOAD_BY_OPENMP_TARGET)
// use OpenMP target directives for GPU offloading
#include "omp_target.hpp"
#elif defined(_OPENMP)
// fallback mode when both OpenACC and OpenMP target are not enabled
#include "fallback.hpp"

#undef IF_NOT_OFFLOADED
///
/// @brief "arg" appears only in the fallback mode (when GPU offloading is disabled)
///
#define IF_NOT_OFFLOADED(arg) arg
#endif  // defined(_OPENMP)

///
/// @brief offload the specified loop
///
#define OFFLOAD(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)

///
/// @brief indicate parallelism to compiler
///
#define AS_INDEPENDENT ACC_CLAUSE_INDEPENDENT

///
/// @brief omit vectorization
///
#define AS_SEQUENTIAL ACC_CLAUSE_SEQ

///
/// @brief suggest number of threads per thread-block
///
#define NUM_THREADS(n) ACC_CLAUSE_VECTOR_LENGTH(n)

///
/// @brief suggest number of thread-blocks
///
#define NUM_BLOCKS(n) ACC_CLAUSE_NUM_WORKERS(n)

///
/// @brief suggest number of grids
///
#define NUM_GRIDS(n) ACC_CLAUSE_NUM_GANGS(n)

///
/// @brief suggest parallelization hierarchy: thread(CUDA)/vector(OpenACC)/SIMD(OpenMP target)
///
#define AS_THREAD PRAGMA_ACC_VECTOR

///
/// @brief suggest parallelization hierarchy: block(CUDA)/worker(OpenACC)/thread(OpenMP target)
///
#define AS_BLOCK PRAGMA_ACC_WORKER

///
/// @brief suggest parallelization hierarchy: grid(CUDA)/gang(OpenACC)/teams(OpenMP target)
///
#define AS_GRID PRAGMA_ACC_GANG

///
/// @brief collapse tightly-nested loops
///
#define COLLAPSE(n) ACC_CLAUSE_COLLAPSE(n)

///
/// @brief declaration of the specified functions are mapped to device
///
#define DECLARE_OFFLOADED(...) PRAGMA_ACC_ROUTINE(__VA_ARGS__)

///
/// @brief declaration of the specified functions are mapped to device
///
#define DECLARE_OFFLOADED_END PRAGMA_OMP_END_DECLARE_TARGET

///
/// @brief launch kernels asynchronously
///
#define AS_ASYNC(...) ACC_CLAUSE_ASYNC(__VA_ARGS__)

///
/// @brief launch kernels asynchronously with the specified queue ID
/// @details In OpenACC, ASYNC_QUEUE(id) is converted to async(id) and launches work asynchronously on queue id.
///          WAIT_QUEUE(id) waits for completion of that queue.
///          In OpenMP target directives, explicit queue IDs are not supported; therefore, ASYNC_QUEUE(id) is ignored.
///
#if defined(OFFLOAD_BY_OPENACC)
#define ASYNC_QUEUE(id) ACC_CLAUSE_ASYNC(id)
#else  // defined(OFFLOAD_BY_OPENACC)
#warning "ASYNC_QUEUE(id) is ignored: explicit queue IDs for asynchronous execution are not available in OpenMP target directives (only supported in OpenACC)."
#define ASYNC_QUEUE(id)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief synchronize asynchronously launched kernel
///
#define SYNCHRONIZE(...) PRAGMA_ACC_WAIT(__VA_ARGS__)

///
/// @brief synchronize asynchronously launched kernels with the specified queue ID
/// @details In OpenACC, WAIT_QUEUE(id) is converted to wait(id) and waits for completion of asynchronously launched kernels on queue id.
///          In OpenMP target directives, explicit queue IDs are not supported; therefore, WAIT_QUEUE(id) is ignored.
///
#if defined(OFFLOAD_BY_OPENACC)
#define WAIT_QUEUE(id) PRAGMA_ACC_WAIT(id)
#else  // defined(OFFLOAD_BY_OPENACC)
#warning "WAIT_QUEUE(id) is ignored: explicit queue IDs for asynchronous execution are not available in OpenMP target directives (only supported in OpenACC)."
#define WAIT_QUEUE(id)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief atomic construct
///
#define ATOMIC(...) PRAGMA_ACC_ATOMIC(__VA_ARGS__)

///
/// @brief atomic construct (default mode: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
///
#define ATOMIC_UPDATE PRAGMA_ACC_ATOMIC_UPDATE

///
/// @brief atomic construct (read: v = x;)
///
#define ATOMIC_READ PRAGMA_ACC_ATOMIC_READ

///
/// @brief atomic construct (write: x = expr;)
///
#define ATOMIC_WRITE PRAGMA_ACC_ATOMIC_WRITE

///
/// @brief atomic construct (v = update-expr, where update-expr is one of: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
///
#define ATOMIC_CAPTURE PRAGMA_ACC_ATOMIC_CAPTURE

///
/// @brief perform reduction
///
#define REDUCTION(...) ACC_CLAUSE_REDUCTION(__VA_ARGS__)

///
/// @brief if clause
///
#define ENABLE_IF(condition) ACC_CLAUSE_IF(condition)

///
/// @brief specify private variables
///
#define AS_PRIVATE(...) ACC_CLAUSE_PRIVATE(__VA_ARGS__)

///
/// @brief specify first private variables
///
#define AS_FIRSTPRIVATE(...) ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)

///
/// @brief makes the address of device data available on the host
///
#define DATA_ACCESS_BY_HOST(...) PRAGMA_ACC_HOST_DATA(__VA_ARGS__)

///
/// @brief defines data accessible by the device
///
#define DATA_ACCESS_BY_DEVICE(...) PRAGMA_ACC_DATA(__VA_ARGS__)

///
/// @brief use device data from host
///
#define USE_DEVICE_DATA_FROM_HOST(...) PRAGMA_ACC_HOST_DATA_USE_DEVICE(__VA_ARGS__)

///
/// @brief specify the pointer is allocated on device
///
#define AS_DEVICE_PTR(...) ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)

///
/// @brief allocate device memory
///
#define MALLOC_ON_DEVICE(...) PRAGMA_ACC_ENTER_DATA_CREATE(__VA_ARGS__)

///
/// @brief release device memory
///
#define FREE_FROM_DEVICE(...) PRAGMA_ACC_EXIT_DATA_DELETE(__VA_ARGS__)

///
/// @brief memcpy from device to host
///
#define MEMCPY_D2H(...) PRAGMA_ACC_UPDATE_HOST(__VA_ARGS__)

///
/// @brief memcpy from host to device
///
#define MEMCPY_H2D(...) PRAGMA_ACC_UPDATE_DEVICE(__VA_ARGS__)

///
/// @brief declaration of the specified variables are mapped to device
///
#define DECLARE_DATA_ON_DEVICE(...) PRAGMA_ACC_DATA_PRESENT(__VA_ARGS__)

///
/// @brief specify variables to be copied (copy from host to device before the computation, copy from device to host after the computation)
///
#define COPY_BEFORE_AND_AFTER_EXEC(...) ACC_CLAUSE_COPY(__VA_ARGS__)

///
/// @brief argument to specify variables to be copied (copy from host to device before the computation)
///
#define COPY_H2D_BEFORE_EXEC(...) ACC_CLAUSE_COPYIN(__VA_ARGS__)

///
/// @brief argument to specify variables to be copied (copy from device to host after the computation)
///
#define COPY_D2H_AFTER_EXEC(...) ACC_CLAUSE_COPYOUT(__VA_ARGS__)

#endif  // !defined(SOLOMON_SOLOMON_HPP)
