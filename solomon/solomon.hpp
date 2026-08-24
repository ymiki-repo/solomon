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

///
/// @brief version of Solomon
///
#define SOLOMON_VERSION 2.0.0

// accept SOLOMON_-prefixed spellings of the configuration macros (v2.0.0 or later); the unprefixed spellings also keep working
#if defined(SOLOMON_OFFLOAD_BY_OPENACC) && !defined(OFFLOAD_BY_OPENACC)
#define OFFLOAD_BY_OPENACC
#endif  // defined(SOLOMON_OFFLOAD_BY_OPENACC) && !defined(OFFLOAD_BY_OPENACC)
#if defined(SOLOMON_OFFLOAD_BY_OPENACC_PARALLEL) && !defined(OFFLOAD_BY_OPENACC_PARALLEL)
#define OFFLOAD_BY_OPENACC_PARALLEL
#endif  // defined(SOLOMON_OFFLOAD_BY_OPENACC_PARALLEL) && !defined(OFFLOAD_BY_OPENACC_PARALLEL)
#if defined(SOLOMON_OFFLOAD_BY_OPENMP_TARGET) && !defined(OFFLOAD_BY_OPENMP_TARGET)
#define OFFLOAD_BY_OPENMP_TARGET
#endif  // defined(SOLOMON_OFFLOAD_BY_OPENMP_TARGET) && !defined(OFFLOAD_BY_OPENMP_TARGET)
#if defined(SOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE) && !defined(OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE)
#define OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE
#endif  // defined(SOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE) && !defined(OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE)

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
#define SOLOMON_IF_NOT_OFFLOADED(arg)

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

#undef SOLOMON_IF_NOT_OFFLOADED
///
/// @brief "arg" appears only in the fallback mode (when GPU offloading is disabled)
///
#define SOLOMON_IF_NOT_OFFLOADED(arg) arg
#else
// no offloading backend is available
#include "fallback.hpp"

#undef SOLOMON_IF_NOT_OFFLOADED
///
/// @brief "arg" appears only in the fallback mode (when GPU offloading is disabled)
///
#define SOLOMON_IF_NOT_OFFLOADED(arg) arg
#endif  // defined(_OPENMP)

///
/// @brief offload the specified loop
///
#define SOLOMON_OFFLOAD(...) PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)

///
/// @brief finalize the offloading of the specified loop
///
#if !defined(SOLOMON_FORTRAN)
#define SOLOMON_END_OFFLOAD
#else  // !defined(SOLOMON_FORTRAN)
#define SOLOMON_END_OFFLOAD PRAGMA_ACC_END_OFFLOADING_DEFAULT
#endif  // !defined(SOLOMON_FORTRAN)

#if defined(OFFLOAD_BY_OPENACC)
///
/// @brief offload the immediately following loop and distribute it over thread-blocks (OpenACC: _Pragma("acc parallel [...]") _Pragma("acc loop gang [...]"))
/// @note do not pass SOLOMON_CLAUSE_BLOCK, SOLOMON_CLAUSE_THREAD, ACC_CLAUSE_GANG, ACC_CLAUSE_WORKER, or ACC_CLAUSE_VECTOR to SOLOMON_OFFLOAD_OUTER_LOOP(...) because the conflicting clauses are already included in the macro
///
#define SOLOMON_OFFLOAD_OUTER_LOOP(...) PRAGMA_ACC_PARALLEL(__VA_ARGS__) PRAGMA_ACC_LOOP(ACC_CLAUSE_GANG SOLOMON_APPEND_ARGS(__VA_ARGS__))
///
/// @brief parallelize the immediately following loop over threads within a thread-block (OpenACC: _Pragma("acc loop vector [...]"))
/// @note must be used in conjunction with SOLOMON_OFFLOAD_OUTER_LOOP(...) to parallelize the immediately following loop over threads within a thread-block
/// @note do not pass SOLOMON_CLAUSE_BLOCK, SOLOMON_CLAUSE_THREAD, ACC_CLAUSE_GANG, ACC_CLAUSE_WORKER, or ACC_CLAUSE_VECTOR to SOLOMON_PARALLELIZE_INNER_LOOP(...) because the conflicting clauses are already included in the macro
///
#define SOLOMON_PARALLELIZE_INNER_LOOP(...) PRAGMA_ACC_LOOP(ACC_CLAUSE_VECTOR SOLOMON_APPEND_ARGS(__VA_ARGS__))
///
/// @brief finalize the offloading of the outer loop
///
#define SOLOMON_END_OFFLOAD_OUTER_LOOP PRAGMA_ACC_END_PARALLEL
#elif defined(OFFLOAD_BY_OPENMP_TARGET)
///
/// @brief offload the immediately following loop and distribute it over thread-blocks (OpenMP target: _Pragma("omp target teams distribute [...]"))
///
#define SOLOMON_OFFLOAD_OUTER_LOOP(...) PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(__VA_ARGS__)
///
/// @brief parallelize the immediately following loop over threads within a thread-block (OpenMP target: _Pragma("omp parallel for [...]"))
/// @note must be used in conjunction with SOLOMON_OFFLOAD_OUTER_LOOP(...) to parallelize the immediately following loop over threads within a thread-block
///
#define SOLOMON_PARALLELIZE_INNER_LOOP(...) PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)
///
/// @brief finalize the offloading of the outer loop
///
#define SOLOMON_END_OFFLOAD_OUTER_LOOP
#elif defined(_OPENMP)
///
/// @brief parallelize the immediately following loop (OpenMP for multicore CPU: _Pragma("omp parallel for [...]"))
///
#define SOLOMON_OFFLOAD_OUTER_LOOP(...) PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)
///
/// @brief do nothing (OpenMP for multicore CPU: SOLOMON_OFFLOAD_OUTER_LOOP(...) has already parallelized the outer loop; the immediately following loop runs sequentially within each thread)
///
#define SOLOMON_PARALLELIZE_INNER_LOOP(...)
///
/// @brief finalize the offloading of the outer loop
///
#define SOLOMON_END_OFFLOAD_OUTER_LOOP
#else  // defined(_OPENMP)
///
/// @brief offload the immediately following loop (fallback mode: no offloading)
///
#define SOLOMON_OFFLOAD_OUTER_LOOP(...)
///
/// @brief parallelize the immediately following loop (fallback mode: no offloading)
///
#define SOLOMON_PARALLELIZE_INNER_LOOP(...)
///
/// @brief finalize the offloading of the outer loop
///
#define SOLOMON_END_OFFLOAD_OUTER_LOOP
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief indicate parallelism to compiler
///
#define SOLOMON_CLAUSE_INDEPENDENT ACC_CLAUSE_INDEPENDENT

///
/// @brief omit vectorization
///
#define SOLOMON_CLAUSE_SEQUENTIAL ACC_CLAUSE_SEQ

///
/// @brief suggest number of threads per thread-block
///
#define SOLOMON_CLAUSE_NUM_THREADS(n) ACC_CLAUSE_VECTOR_LENGTH(n)

///
/// @brief suggest number of thread-blocks
///
#define SOLOMON_CLAUSE_NUM_BLOCKS(n) ACC_CLAUSE_NUM_GANGS(n)

///
/// @brief deprecated alias of SOLOMON_CLAUSE_NUM_BLOCKS
/// @deprecated NUM_GRIDS(n) was renamed to SOLOMON_CLAUSE_NUM_BLOCKS(n) in v2.0.0
///
#define SOLOMON_CLAUSE_NUM_GRIDS(n) SOLOMON_CLAUSE_NUM_BLOCKS(n)
#if defined(__clang__) && (__clang_major__ >= 14)
#pragma clang deprecated(SOLOMON_CLAUSE_NUM_GRIDS, "SOLOMON_CLAUSE_NUM_GRIDS(n) is a deprecated alias of SOLOMON_CLAUSE_NUM_BLOCKS(n)")
#endif  // defined(__clang__) && (__clang_major__ >= 14)

///
/// @brief suggest parallelization hierarchy: thread(CUDA)/vector(OpenACC)/thread(OpenMP target)
///
#define SOLOMON_CLAUSE_THREAD ACC_CLAUSE_VECTOR

///
/// @brief suggest parallelization hierarchy: block(CUDA)/gang(OpenACC)/teams(OpenMP target)
///
#define SOLOMON_CLAUSE_BLOCK ACC_CLAUSE_GANG

///
/// @brief deprecated alias of SOLOMON_CLAUSE_BLOCK
/// @deprecated AS_GRID was renamed to SOLOMON_CLAUSE_BLOCK in v2.0.0
///
#define SOLOMON_CLAUSE_GRID SOLOMON_CLAUSE_BLOCK
#if defined(__clang__) && (__clang_major__ >= 14)
#pragma clang deprecated(SOLOMON_CLAUSE_GRID, "SOLOMON_CLAUSE_GRID is a deprecated alias of SOLOMON_CLAUSE_BLOCK")
#endif  // defined(__clang__) && (__clang_major__ >= 14)

///
/// @brief collapse tightly-nested loops
///
#define SOLOMON_CLAUSE_COLLAPSE(n) ACC_CLAUSE_COLLAPSE(n)

///
/// @brief declaration of the specified functions are mapped to device
///
#define SOLOMON_DECLARE_OFFLOADED(...) PRAGMA_ACC_ROUTINE(__VA_ARGS__)

///
/// @brief specify the target procedures by name (OpenACC: routine(name), OpenMP target: declare target(list))
/// @note pass as the first argument of SOLOMON_DECLARE_OFFLOADED(...) (v2.0.0 or later)
///
#define SOLOMON_CLAUSE_TARGETS(...) (__VA_ARGS__), SOLOMON_INTERNAL_ARGS_WITH_NUM(SOLOMON_INTERNAL_TAG_ACC_ROUTINE, SOLOMON_INTERNAL_TAG_OMP_DECLARE_TARGET)

///
/// @brief declaration of the specified functions are mapped to device
/// @note in Fortran, this macro expands to nothing (SOLOMON_DECLARE_OFFLOADED(...) inside a procedure is self-contained); writing it is harmless
///
#define SOLOMON_DECLARE_OFFLOADED_END PRAGMA_OMP_END_DECLARE_TARGET

///
/// @brief declare device-resident variables (OpenACC: declare create(vars), OpenMP target: declare target(vars)) (v2.0.0 or later)
/// @note place at file/module scope (Fortran: in the specification part); no end directive is required
/// @note initialization semantics differ slightly: OpenACC "declare create" allocates without copying initial values, while OpenMP "declare target" copies static initializers; transfer data explicitly (e.g., SOLOMON_MEMCPY_H2D) when initial values matter
///
#if defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_DECLARE_ON_DEVICE(...) PRAGMA_ACC_DECLARE(ACC_CLAUSE_CREATE(__VA_ARGS__))
#elif defined(OFFLOAD_BY_OPENMP_TARGET)
#define SOLOMON_DECLARE_ON_DEVICE(...) PRAGMA_OMP_DECLARE_TARGET(OMP_PASS_LIST(__VA_ARGS__))
#else  // defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_DECLARE_ON_DEVICE(...)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief declare device-resident variables with link semantics (OpenACC: declare link(vars), OpenMP target: declare target link(vars)) (v2.0.0 or later)
///
#if defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_DECLARE_ON_DEVICE_LINKED(...) PRAGMA_ACC_DECLARE(ACC_CLAUSE_LINK(__VA_ARGS__))
#elif defined(OFFLOAD_BY_OPENMP_TARGET)
#define SOLOMON_DECLARE_ON_DEVICE_LINKED(...) PRAGMA_OMP_DECLARE_TARGET(OMP_TARGET_CLAUSE_LINK(__VA_ARGS__))
#else  // defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_DECLARE_ON_DEVICE_LINKED(...)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief launch kernels asynchronously
///
#define SOLOMON_CLAUSE_ASYNC(...) ACC_CLAUSE_ASYNC(__VA_ARGS__)

///
/// @brief launch kernels asynchronously with the specified queue ID
/// @details In OpenACC, SOLOMON_CLAUSE_ASYNC_QUEUE(id) is converted to async(id) and launches work asynchronously on queue id.
///          SOLOMON_WAIT_QUEUE(id) waits for completion of that queue.
///          In OpenMP target directives, explicit queue IDs are not supported; therefore, SOLOMON_CLAUSE_ASYNC_QUEUE(id) is ignored.
///
#if defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_CLAUSE_ASYNC_QUEUE(id) ACC_CLAUSE_ASYNC(id)
#else  // defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_CLAUSE_ASYNC_QUEUE(id)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief offload the immediately following structured block for single-thread execution on the device (OpenACC: acc serial, OpenMP target: omp target without teams) (v2.0.0 or later)
/// @note useful to keep results (e.g., of a reduction) on the device: copy them into device-resident buffers without a round trip of arrays to the host
///
#define SOLOMON_OFFLOAD_SERIAL(...) PRAGMA_ACC_SERIAL(__VA_ARGS__)

///
/// @brief finalize the serial region (v2.0.0 or later)
///
#define SOLOMON_END_OFFLOAD_SERIAL PRAGMA_ACC_END_SERIAL

///
/// @brief synchronize asynchronously launched kernel
///
#define SOLOMON_SYNCHRONIZE(...) PRAGMA_ACC_WAIT(__VA_ARGS__)

///
/// @brief synchronize asynchronously launched kernels with the specified queue ID
/// @details In OpenACC, SOLOMON_WAIT_QUEUE(id) is converted to wait(id) and waits for completion of asynchronously launched kernels on queue id.
///          In OpenMP target directives, explicit queue IDs are not supported; therefore, SOLOMON_WAIT_QUEUE(id) is ignored.
///
#if defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_WAIT_QUEUE(id) PRAGMA_ACC_WAIT(id)
#else  // defined(OFFLOAD_BY_OPENACC)
#define SOLOMON_WAIT_QUEUE(id)
#endif  // defined(OFFLOAD_BY_OPENACC)

///
/// @brief atomic construct
///
#define SOLOMON_ATOMIC(...) PRAGMA_ACC_ATOMIC(__VA_ARGS__)

///
/// @brief atomic construct (default mode: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
///
#define SOLOMON_ATOMIC_UPDATE PRAGMA_ACC_ATOMIC_UPDATE

///
/// @brief atomic construct (read: v = x;)
///
#define SOLOMON_ATOMIC_READ PRAGMA_ACC_ATOMIC_READ

///
/// @brief atomic construct (write: x = expr;)
///
#define SOLOMON_ATOMIC_WRITE PRAGMA_ACC_ATOMIC_WRITE

///
/// @brief atomic construct (v = update-expr, where update-expr is one of: x++; x--; ++x; --x; x binop= expr; x = x binop expr; or x = expr binop x;)
///
#define SOLOMON_ATOMIC_CAPTURE PRAGMA_ACC_ATOMIC_CAPTURE

///
/// @brief perform reduction
///
#define SOLOMON_CLAUSE_REDUCTION(...) ACC_CLAUSE_REDUCTION(__VA_ARGS__)

///
/// @brief if clause
///
#define SOLOMON_CLAUSE_IF(condition) ACC_CLAUSE_IF(condition)

///
/// @brief specify private variables
///
#define SOLOMON_CLAUSE_PRIVATE(...) ACC_CLAUSE_PRIVATE(__VA_ARGS__)

///
/// @brief specify first private variables
///
#define SOLOMON_CLAUSE_FIRSTPRIVATE(...) ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)

///
/// @brief makes the address of device data available on the host
///
#define SOLOMON_DATA_ACCESS_BY_HOST(...) PRAGMA_ACC_HOST_DATA(__VA_ARGS__)

///
/// @brief defines data accessible by the device
///
#define SOLOMON_DATA_ACCESS_BY_DEVICE(...) PRAGMA_ACC_DATA(__VA_ARGS__)

///
/// @brief use device data from host
///
#define SOLOMON_USE_DEVICE_DATA_FROM_HOST(...) PRAGMA_ACC_HOST_DATA_USE_DEVICE(__VA_ARGS__)

///
/// @brief specify the pointer is allocated on device
///
#define SOLOMON_CLAUSE_DEVICE_PTR(...) ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)

///
/// @brief allocate device memory
///
#define SOLOMON_MALLOC_ON_DEVICE(...) PRAGMA_ACC_ENTER_DATA_CREATE(__VA_ARGS__)
///
/// @brief allocate device memory
///
#define SOLOMON_ALLOCATE_ON_DEVICE(...) SOLOMON_MALLOC_ON_DEVICE(__VA_ARGS__)

///
/// @brief release device memory
///
#define SOLOMON_FREE_FROM_DEVICE(...) PRAGMA_ACC_EXIT_DATA_DELETE(__VA_ARGS__)
///
/// @brief release device memory
///
#define SOLOMON_DEALLOCATE_ON_DEVICE(...) SOLOMON_FREE_FROM_DEVICE(__VA_ARGS__)

///
/// @brief memcpy from device to host
///
#define SOLOMON_MEMCPY_D2H(...) PRAGMA_ACC_UPDATE_HOST(__VA_ARGS__)

///
/// @brief memcpy from host to device
///
#define SOLOMON_MEMCPY_H2D(...) PRAGMA_ACC_UPDATE_DEVICE(__VA_ARGS__)

///
/// @brief declaration of the specified variables are mapped to device
///
#define SOLOMON_DECLARE_DATA_ON_DEVICE(...) PRAGMA_ACC_DATA_PRESENT(__VA_ARGS__)

///
/// @brief specify variables to be copied (copy from host to device before the computation, copy from device to host after the computation)
///
#define SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(...) ACC_CLAUSE_COPY(__VA_ARGS__)

///
/// @brief argument to specify variables to be copied (copy from host to device before the computation)
///
#define SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...) ACC_CLAUSE_COPYIN(__VA_ARGS__)

///
/// @brief argument to specify variables to be copied (copy from device to host after the computation)
///
#define SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(...) ACC_CLAUSE_COPYOUT(__VA_ARGS__)

// unprefixed spellings of the user-facing macros (v1.x compatible; default OFF)
#if defined(SOLOMON_WITH_SHORT_NAMES)
#include "short_names.hpp"
#endif  // defined(SOLOMON_WITH_SHORT_NAMES)

#endif  // !defined(SOLOMON_SOLOMON_HPP)
