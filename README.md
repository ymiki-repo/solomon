# Solomon: Simple Off-LOading Macros Orchestrating multiple Notations

[English](README.md) | [日本語](README_jp.md)

## Summary

* Preprocessor macros to switch OpenACC and OpenMP target directives
* See [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) for detail
* Please cite [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380)
* Released under the MIT license, see LICENSE.txt
* Copyright (c) 2024 Yohei MIKI

## How to use

### How to develop codes using Solomon

1. Include Solomon

   ```c++
   #include <solomon.hpp>
   ```

2. Insert offloading macros instead of OpenACC or OpenMP target directives

   * For beginners, we recommend to use intuitive notations like `OFFLOAD(...)`
   * Experienced developers of OpenACC or OpenMP target will prefer OpenACC/OpenMP-like notations
     * In OpenMP-like notation, only notations like `PRAGMA_OMP_TARGET_*` or `OMP_TARGET_CLAUSE_*` are converted to OpenACC backend (e.g., `PRAGMA_OMP_ATOMIC(...)` will be transted as `_Pragma("omp atomic __VA_ARGS__")`)
     * We strongly recommend not to adopt `PRAGMA_OMP_TARGET_DATA(...)` in your codes
       * Alternative notations are `DATA_ACCESS_BY_DEVICE(...)` or `PRAGMA_ACC_DATA(...)` for data accessed by device (GPU), and `DATA_ACCESS_BY_HOST(...)` or `PRAGMA_ACC_HOST_DATA(...)` for data accessed by host (CPU)
     * In OpenACC-like notation, inserting `DECLARE_OFFLOADED_END` or `PRAGMA_OMP_END_DECLARE_TARGET` is required when you insert `PRAGMA_ACC_ROUTINE(...)` (for proper translation to OpenMP target offloading)
   * `IF_NOT_OFFLOADED(arg)` is available to hide directives when GPU offloading is enabled
     * <details><summary> Example: `arg` appears only in fallback mode (when GPU offloading is disabled (both OpenACC and OpenMP target are not enabled))</summary>

       ```c++
       OFFLOAD()
       for(int i = 0; i < num; i++){
         IF_NOT_OFFLOADED(PRAGMA_OMP_SIMD())
         for(int j = 0; j < 16; j++){
           // computation
         }
       }
       ```

       * Output in OpenACC backend

         ```c++
         _Pragma("acc kernels")
         _Pragma("acc loop")
         for(int i = 0; i < num; i++){

           for(int j = 0; j < 16; j++){
             // computation
           }
         }
         ```

       * Output in OpenMP target backend

         ```c++
         _Pragma("omp target teams loop")
         for(int i = 0; i < num; i++){

           for(int j = 0; j < 16; j++){
             // computation
           }
         }
         ```

       * Output in fallback mode

         ```c++
         _Pragma("omp parallel for")
         for(int i = 0; i < num; i++){
           _Pragma("omp simd")
           for(int j = 0; j < 16; j++){
             // computation
           }
         }
         ```

     </details>

   * Optional clauses must be passed as comma-separated notation as

      ```c++
      OFFLOAD(AS_INDEPENDENT, ACC_CLAUSE_VECTOR_LENGTH(128), OMP_TARGET_CLAUSE_COLLAPSE(3))
      ```

      * Mixture of intuitive and OpenACC/OpenMP-like notations are enabled
      * `AS_INDEPENDENT` (or the correspondences: `ACC_CLAUSE_INDEPENDENT` and `OMP_TARGET_CLAUSE_SIMD`) must be specified at the head of all optional clauses
      * Solomon automatically drops incompatible clauses
   * We encourage the adoption of combined macros (instead of individual macros separately) for better conversion between OpenACC and OpenMP target

     | recommended implementations | corresponding implementation (not recommended) |
     | ---- | ---- |
     | **`OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` | <br> `PRAGMA_ACC_KERNELS(...) PRAGMA_ACC_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC_LOOP(...)` |
     | **`DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `PRAGMA_ACC_DATA(ACC_CLAUSE_PRESENT(...))` |
     | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` |

   * Macros for asynchronous kernel execution and synchronization are provided below. Please use them according to your needs
     * To improve compatibility between OpenACC and OpenMP target, it is recommended to use the intuitive notation provided by Solomon for asynchronous execution and synchronization
     * If you wish to perform asynchronous execution in a backend-independent manner, use `AS_ASYNC(...)` and `SYNCHRONIZE(...)`. Note that queue IDs may be ignored depending in on the backend
     * If you with to perform fine-grained asynchronous operations with specific queue IDs, use `ASYNC_QUEUE(id)` and `WAIT_QUEUE(id)`. Note that asynchronous execution may not occur depending on the backend

     | Available macros | output | offloading backend | note |
     | ---- | ---- | ---- | ---- |
     | **`AS_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP | Enables asynchronous execution in both backends <br> Queue IDs can be specified in OpenACC <br> Queue IDs are ignored in OpenMP |
     | **`SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `_Pragma("acc wait __VA_ARGS__")` <br> `_Pragma("omp taskwait __VA_ARGS__")` | <br> OpenACC <br> OpenMP | Performs synchronization for both backends. Should be used in correspondence with `AS_ASYNC(...)` |
     | **`ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` <br> N/A (disregarded in OpenMP backend) | <br> OpenACC <br> OpenMP | Performs asynchronous execution with a specified queue ID in OpenACC only <br> Queue ID specification is mandatory <br> Ignored in OpenMP because queue-specific asynchronous execution is not supported |
     | **`WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `wait(id)` <br> N/A (disregarded in OpenMP backend) | <br> OpenACC <br> OpenMP | Performs synchronization with a specified queue ID in OpenACC only. Should be used in correspondence with `ASYNC_QUEUE(id)` <br> Queue ID specification is mandatory <br> Ignored in OpenMP because queue-specific synchronization is not supported |

### How to compile codes using Solomon

* Enable OpenACC or OpenMP target by compiler option
* Specify the path to solomon (the path where `solomon.hpp` exists) as `-I/path/to/solomon`
* Add compilation flags to specify the expected behabior of Solomon

  | compilation flag | offloading backend | note |
  | ---- | ---- | ---- |
  | `-DOFFLOAD_BY_OPENACC` | OpenACC | use `kernels` construct in default |
  | `-DOFFLOAD_BY_OPENACC -DOFFLOAD_BY_OPENACC_PARALLEL` | OpenACC | use `parallel` construct in default |
  | `-DOFFLOAD_BY_OPENMP_TARGET` | OpenMP target | use `loop` directive in default |
  | `-DOFFLOAD_BY_OPENMP_TARGET -DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | OpenMP target | use `distribute` directive in default |
  | | fallback mode | thread-parallelization for multicore CPUs using OpenMP |

* Adding the compile flag `-DPRINT_GENERATED_PRAGMA` allows you to output the actually generated directives as compile-time messages
  * In LLVM, this is treated as a warning, so when specifying `-Werror`, also pass `-Wno-error=pragma-messages` to prevent these messages from being treated as errors
* See examples: [Makefile for nbody](samples/nbody/Makefile) and [Makefile for diffusion](samples/diffusion/Makefile)

### How to extend capability of Solomon

* Solomon accepts up to 32 clause candidates per directive
  * If the current limitation (32) does not fit your implementation, increase the value as follows

    ```sh
    cd solomon/util # you will find jl/ and pickup.hpp in the directory
    julia jl/check_clause.jl --max 64 >> pickup.hpp # example to reset the limitation as 64
    # edit pickup.hpp appropriately (remove old CHECK_CLAUSE_* and APPEND_CLAUSE, and use new CHECK_CLAUSE_* and APPEND_CLAUSE)
    ```

  * Similar limitations exsit for some internal macros, and you can also increase such limitations

## Samples

### nbody: sample of compute-intensive application

* [Collisionless $N$-body simulation based on direct method](samples/nbody/)
  * Implementation using intuitive notation: `samples/nbody/src/[nbody gravity].cpp`
  * Implementation using OpenACC-like notation: `samples/nbody/src/[nbody gravity]_acc.cpp`
  * Implementation using OpenMP-like notation: `samples/nbody/src/[nbody gravity]_omp.cpp`

### diffusion: sample of memory-intensive application

* [Diffusion equation in 3D](samples/diffusion/)
  * Original implementation in OpenACC is available at [GitHub](https://github.com/hoshino-UTokyo/lecture_openacc) (developed by Tetsuya Hoshino at Nagoya University)
  * Some trivial optimizations and refactoring are added

## Available APIs in Solomon

### Available directives

* <details><summary>Computation</summary>

  | input | output | backend |
  | ---- | ---- | ---- |
  | **`OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | <br> `_Pragma("acc kernels __VA_ARGS__") _Pragma("acc loop __VA_ARGS__")` <br> `_Pragma("acc parallel __VA_ARGS__") _Pragma("acc loop __VA_ARGS__")` <br> `_Pragma("omp target teams loop __VA_ARGS__")` <br> `_Pragma("omp target teams distribute parallel for __VA_ARGS__")` | <br> OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP (loop) <br> OpenMP (distribute) |
  | **`SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `_Pragma("acc wait __VA_ARGS__")` <br> `_Pragma("omp taskwait __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `_Pragma("acc wait id")` | <br> OpenACC (only) |
  | **`DECLARE_OFFLOADED(...)`** <br> `PRAGMA_ACC_ROUTINE(...)` <br> `PRAGMA_OMP_DECLARE_TARGET(...)` | <br> `_Pragma("acc routine __VA_ARGS__")` <br> `_Pragma("omp declare target __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`DECLARE_OFFLOADED_END`** <br> `PRAGMA_OMP_END_DECLARE_TARGET` | <br> `_Pragma("omp end declare target")` | <br> OpenMP (only) |
  | **`ATOMIC(...)`** <br> `PRAGMA_ACC_ATOMIC(...)` <br> `PRAGMA_OMP_TARGET_ATOMIC(...)` | <br> `_Pragma("acc atomic __VA_ARGS__")` <br> `_Pragma("omp atomic __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_UPDATE`** <br> `PRAGMA_ACC_ATOMIC_UPDATE` <br> `PRAGMA_OMP_TARGET_ATOMIC_UPDATE` | <br> `_Pragma("acc atomic update")` <br> `_Pragma("omp atomic update")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_READ`** <br> `PRAGMA_ACC_ATOMIC_READ` <br> `PRAGMA_OMP_TARGET_ATOMIC_READ` | <br> `_Pragma("acc atomic read")` <br> `_Pragma("omp atomic read")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_WRITE`** <br> `PRAGMA_ACC_ATOMIC_WRITE` <br> `PRAGMA_OMP_TARGET_ATOMIC_WRITE` | <br> `_Pragma("acc atomic write")` <br> `_Pragma("omp atomic write")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_CAPTURE`** <br> `PRAGMA_ACC_ATOMIC_CAPTURE` <br> `PRAGMA_OMP_TARGET_ATOMIC_CAPTURE` | <br> `_Pragma("acc atomic capture")` <br> `_Pragma("omp atomic capture")` | <br> OpenACC <br> OpenMP |

  </details>

  * <details><summary>Abstraction macros</summary>

    | input | intermediate macro | backend |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_LAUNCH_DEFAULT(...)` <br> `PRAGMA_OMP_TARGET_LAUNCH_DEFAULT(...)` | `PRAGMA_ACC_KERNELS(__VA_ARGS__)` <br> `PRAGMA_ACC_PARALLEL(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS(__VA_ARGS__)` | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP |
    | `PRAGMA_ACC_OFFLOADING_DEFAULT(...)` <br> `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(...)` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(__VA_ARGS__)` | OpenACC <br> OpenMP (loop) <br> OpenMP (distribute) |

    </details>

  * <details><summary>OpenACC directives</summary>

    | input | output | counterpart in OpenMP backend |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_PARALLEL(...)` | `_Pragma("acc parallel __VA_ARGS__")` | `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)` |
    | `PRAGMA_ACC_KERNELS(...)` | `_Pragma("acc kernels __VA_ARGS__")` | `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)` |
    | `PRAGMA_ACC_SERIAL(...)` | `_Pragma("acc serial __VA_ARGS__")` | N/A (disregarded in OpenMP backend) |
    | `PRAGMA_ACC_LOOP(...)` | `_Pragma("acc loop __VA_ARGS__")` | N/A (disregarded in OpenMP backend) |
    | `PRAGMA_ACC_CACHE(...)` | `_Pragma("acc cache(__VA_ARGS__)")` | N/A (disregarded in OpenMP backend) |
    | `PRAGMA_ACC_ATOMIC(...)` | `_Pragma("acc atomic __VA_ARGS__")` | `PRAGMA_OMP_TARGET_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_ACC_WAIT(...)` | `_Pragma("acc wait __VA_ARGS__")` | `PRAGMA_OMP_TARGET_TASKWAIT(__VA_ARGS__)` |
    | `PRAGMA_ACC_ROUTINE(...)` | `_Pragma("acc routine __VA_ARGS__")` | `PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)` |
    | `PRAGMA_ACC_DECLARE(...)` | `_Pragma("acc declare __VA_ARGS__")` | N/A (disregarded in OpenMP backend) |

    </details>

  * <details><summary>OpenMP target directives</summary>

    | input | output | counterpart in OpenACC backend | counterpart in fallback mode (CPU execution without offloading) |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET(...)` | `_Pragma("omp target __VA_ARGS__")` | `PRAGMA_ACC(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_PARALLEL(...)` | `_Pragma("omp target parallel __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_FOR(...)` | `_Pragma("omp target parallel for __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp target parallel for simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_LOOP(...)` | `_Pragma("omp target parallel loop __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_SIMD(...)` | `_Pragma("omp target simd __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS(...)` | `_Pragma("omp target teams __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` | `_Pragma("omp target teams distribute __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` | `_Pragma("omp target teams distribute simd __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` | `_Pragma("omp target teams loop __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp target teams distribute parallel for __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp target teams distribute parallel for simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_ATOMIC(...)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` | `PRAGMA_ACC_ATOMIC(__VA_ARGS__)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` | `PRAGMA_ACC_WAIT(__VA_ARGS__)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` |
    | `PRAGMA_OMP_DECLARE_TARGET(...)` | `_Pragma("omp declare target __VA_ARGS__")` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_BEGIN_DECLARE_TARGET(...)` | `_Pragma("omp begin declare target __VA_ARGS__")` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_END_DECLARE_TARGET` | `_Pragma("omp end declare target")` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) |

    </details>

  * <details><summary>OpenMP directives</summary>

    | input | output |
    | ---- | ---- |
    | `PRAGMA_OMP_THREADPRIVATE(...)` | `_Pragma("omp threadprivate(__VA_ARGS__)")` |
    | `PRAGMA_OMP_SCAN(...)` | `_Pragma("omp scan __VA_ARGS__")` |
    | `PRAGMA_OMP_DECLARE_SIMD(...)` | `_Pragma("omp declare simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TILE(...)` | `_Pragma("omp tile __VA_ARGS__")` |
    | `PRAGMA_OMP_UNROLL(...)` | `_Pragma("omp unroll __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL(...)` | `_Pragma("omp parallel __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS(...)` | `_Pragma("omp teams __VA_ARGS__")` |
    | `PRAGMA_OMP_SIMD(...)` | `_Pragma("omp simd __VA_ARGS__")` |
    | `PRAGMA_OMP_MASKED(...)` | `_Pragma("omp masked __VA_ARGS__")` |
    | `PRAGMA_OMP_SINGLE(...)` | `_Pragma("omp single __VA_ARGS__")` |
    | `PRAGMA_OMP_WORKSHARE(...)` | `_Pragma("omp workshare __VA_ARGS__")` |
    | `PRAGMA_OMP_SCOPE(...)` | `_Pragma("omp scope __VA_ARGS__")` |
    | `PRAGMA_OMP_SECTIONS(...)` | `_Pragma("omp sections __VA_ARGS__")` |
    | `PRAGMA_OMP_SECTION` | `_Pragma("omp section")` |
    | `PRAGMA_OMP_FOR(...)` | `_Pragma("omp for __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE(...)` | `_Pragma("omp distribute __VA_ARGS__")` |
    | `PRAGMA_OMP_LOOP(...)` | `_Pragma("omp loop __VA_ARGS__")` |
    | `PRAGMA_OMP_TASK(...)` | `_Pragma("omp task __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKLOOP(...)` | `_Pragma("omp taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKYIELD` | `_Pragma("omp taskyield")` |
    | `PRAGMA_OMP_INTEROP(...)` | `_Pragma("omp interop __VA_ARGS__")` |
    | `PRAGMA_OMP_CRITICAL(...)` | `_Pragma("omp critical __VA_ARGS__")` |
    | `PRAGMA_OMP_BARRIER` | `_Pragma("omp barrier")` |
    | `PRAGMA_OMP_TASKGROUP(...)` | `_Pragma("omp taskgroup __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKWAIT(...)` | `_Pragma("omp taskwait __VA_ARGS__")` |
    | `PRAGMA_OMP_FLUSH(...)` | `_Pragma("omp flush __VA_ARGS__")` |
    | `PRAGMA_OMP_DEPOBJ(...)` | `_Pragma("omp depobj __VA_ARGS__")` |
    | `PRAGMA_OMP_ATOMIC(...)` | `_Pragma("omp atomic __VA_ARGS__")` |
    | `PRAGMA_OMP_ORDERED(...)` | `_Pragma("omp ordered __VA_ARGS__")` |
    | `PRAGMA_OMP_FOR_SIMD(...)` | `_Pragma("omp for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE_SIMD(...)` | `_Pragma("omp distribute simd __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp distribute parallel for __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp distribute parallel for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKLOOP_SIMD(...)` | `_Pragma("omp taskloop simd __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_FOR(...)` | `_Pragma("omp parallel for __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_LOOP(...)` | `_Pragma("omp parallel loop __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_SECTIONS(...)` | `_Pragma("omp parallel sections __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp parallel for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_MASKED_TASKLOOP(...)` | `_Pragma("omp masked taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_MASKED_TASKLOOP_SIMD(...)` | `_Pragma("omp masked taskloop simd __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP(...)` | `_Pragma("omp parallel masked taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP_SIMD(...)` | `_Pragma("omp parallel masked taskloop simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE(...)` | `_Pragma("omp teams distribute __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(...)` | `_Pragma("omp teams distribute simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp teams distribute parallel for __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp teams distribute parallel for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_LOOP(...)` | `_Pragma("omp teams loop __VA_ARGS__")` |

    </details>

* <details><summary>Memory, data transfer</summary>

  | input | output | backend |
  | ---- | ---- | ---- |
  | **`MALLOC_ON_DEVICE(...)`** <br> `PRAGMA_ACC_ENTER_DATA_CREATE(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_ALLOC(...)` | <br> `_Pragma("acc enter data create(__VA_ARGS__)")` <br> `_Pragma("omp target enter data map(alloc: __VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`FREE_FROM_DEVICE(...)`** <br> `PRAGMA_ACC_EXIT_DATA_DELETE(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_DELETE(...)` | <br> `_Pragma("acc exit data delete(__VA_ARGS__)")` <br> `_Pragma("omp target exit data map(delete: __VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`MEMCPY_D2H(...)`** <br> `PRAGMA_ACC_UPDATE_HOST(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_FROM(...)` | <br> `_Pragma("acc update host(__VA_ARGS__)")` <br> `_Pragma("omp target update from(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`MEMCPY_H2D(...)`** <br> `PRAGMA_ACC_UPDATE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_TO(...)` | <br> `_Pragma("acc update device(__VA_ARGS__)")` <br> `_Pragma("omp target update to(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `_Pragma("acc enter data __VA_ARGS__")` <br> `_Pragma("omp target enter data __VA_ARGS__")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA_COPYIN(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_TO(...)` | `_Pragma("acc enter data copyin(__VA_ARGS__)")` <br> `_Pragma("omp target enter data map(to: __VA_ARGS__)")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `_Pragma("acc exit data __VA_ARGS__")` <br> `_Pragma("omp target exit data __VA_ARGS__")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA_COPYOUT(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_FROM(...)` | `_Pragma("acc exit data copyout(__VA_ARGS__)")` <br> `_Pragma("omp target exit data map(from: __VA_ARGS__)")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_UPDATE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE(...)` | `_Pragma("acc update __VA_ARGS__")` <br> `_Pragma("omp target update __VA_ARGS__")` | OpenACC <br> OpenMP |
  | **`DATA_ACCESS_BY_DEVICE(...)`** <br> `PRAGMA_ACC_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `_Pragma("acc data __VA_ARGS__")` <br> `_Pragma("omp target data __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`DATA_ACCESS_BY_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `_Pragma("acc host_data __VA_ARGS__")` <br> `_Pragma("omp target data __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`USE_DEVICE_DATA_FROM_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA_USE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_DATA_USE_DEVICE_PTR(...)` | <br> `_Pragma("acc host_data use_device(__VA_ARGS__)")` <br> `_Pragma("omp target data use_device_ptr(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `_Pragma("acc data present(__VA_ARGS__)")` | <br> OpenACC (only) |

  </details>

  * <details><summary>OpenACC directives</summary>

    | input | output | counterpart in OpenMP backend |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_DATA(...)` | `_Pragma("acc data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_ENTER_DATA(...)` | `_Pragma("acc enter data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_EXIT_DATA(...)` | `_Pragma("acc exit data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_HOST_DATA(...)` | `_Pragma("acc host_data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_UPDATE(...)` | `_Pragma("acc update __VA_ARGS__")` | `PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__)` |

    </details>

  * <details><summary>OpenMP target directives</summary>

    | input | output | counterpart in OpenACC backend | counterpart in fallback mode (CPU execution without offloading) |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET_DATA(...)` | `_Pragma("omp target data __VA_ARGS__")` | `PRAGMA_ACC_DATA(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `_Pragma("omp target enter data __VA_ARGS__")` | `PRAGMA_ACC_ENTER_DATA(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `_Pragma("omp target exit data __VA_ARGS__")` | `PRAGMA_ACC_EXIT_DATA(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_UPDATE(...)` | `_Pragma("omp target update __VA_ARGS__")` | `PRAGMA_ACC_UPDATE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |

    </details>

### Available clauses

* <details><summary>Intuitive notation</summary>

  | input | output | backend |
  | ---- | ---- | ---- |
  | **`AS_INDEPENDENT`** <br> `ACC_CLAUSE_INDEPENDENT` <br> `OMP_TARGET_CLAUSE_SIMD` | <br> `independent` <br> `simd` | <br> OpenACC <br> OpenMP |
  | **`AS_SEQUENTIAL`** <br> `ACC_CLAUSE_SEQ` | <br> `seq` | <br> OpenACC (only) |
  | **`NUM_THREADS(n)`** <br> `ACC_CLAUSE_VECTOR_LENGTH(n)` <br> `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | <br> `vector_length(n)` <br> `thread_limit(n)` | <br> OpenACC <br> OpenMP |
  | **`NUM_BLOCKS(n)`** <br> `ACC_CLAUSE_NUM_WORKERS(n)` <br> `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | <br> `num_workers(n)` <br> `num_teams(n)` | <br> OpenACC <br> OpenMP |
  | **`NUM_GRIDS(n)`** <br> `ACC_CLAUSE_NUM_GANGS(n)` | <br> `num_gang(n)` | <br> OpenACC (only) |
  | **`AS_THREAD`** <br> `ACC_CLAUSE_VECTOR` | <br> `vector` | <br> OpenACC (only) |
  | **`AS_BLOCK`** <br> `ACC_CLAUSE_WORKER` | <br> `worker` | <br> OpenACC (only) |
  | **`AS_GRID`** <br> `ACC_CLAUSE_GANG` | <br> `gang` | <br> OpenACC (only) |
  | **`COLLAPSE(n)`** <br> `ACC_CLAUSE_COLLAPSE(n)` <br> `OMP_TARGET_CLAUSE_COLLAPSE(n)` | <br> `collapse(n)` <br> `collapse(n)` | <br> OpenACC <br> OpenMP |
  | **`AS_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP |
  | **`ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` | <br> OpenACC (only) |
  | **`REDUCTION(...)`** <br> `ACC_CLAUSE_REDUCTION(...)` <br> `OMP_TARGET_CLAUSE_REDUCTION(...)` | <br> `reduction(__VA_ARGS__)` <br> `reduction(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`ENABLE_IF(condition)`** <br> `ACC_CLAUSE_IF(condition)` <br> `OMP_TARGET_CLAUSE_IF(condition)` | <br> `if(condition)` <br> `if(condition)` | <br> OpenACC <br> OpenMP |
  | **`AS_PRIVATE(...)`** <br> `ACC_CLAUSE_PRIVATE(...)` <br> `OMP_TARGET_CLAUSE_PRIVATE(...)` | <br> `private(__VA_ARGS__)` <br> `private(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`AS_FIRSTPRIVATE(...)`** <br> `ACC_CLAUSE_FIRSTPRIVATE(...)` <br> `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | <br> `firstprivate(__VA_ARGS__)` <br> `firstprivate(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`AS_DEVICE_PTR(...)`** <br> `ACC_CLAUSE_DEVICEPTR(...)` <br> `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | <br> `deviceptr(__VA_ARGS__)` <br> `is_device_ptr(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`COPY_BEFORE_AND_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPY(...)` <br> `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | <br> `copy(__VA_ARGS__)` <br> `map(tofrom: __VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`COPY_H2D_BEFORE_EXEC(...)`** <br> `ACC_CLAUSE_COPYIN(...)` <br> `OMP_TARGET_CLAUSE_MAP_TO(...)` | <br> `copyin(__VA_ARGS__)` <br> `map(to: __VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`COPY_D2H_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPYOUT(...)` <br> `OMP_TARGET_CLAUSE_MAP_FROM(...)` | <br> `copyout(__VA_ARGS__)` <br> `map(from: __VA_ARGS__)` | <br> OpenACC <br> OpenMP |

  </details>

* <details><summary>OpenACC clauses</summary>

  | input | output | counterpart in OpenMP backend | note |
  | ---- | ---- | ---- | ---- |
  | `ACC_CLAUSE_IF(condition)` | `if(condition)` | `OMP_TARGET_CLAUSE_IF(condition)` ||
  | `ACC_CLAUSE_SELF(...)` | `self(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_DEFAULT(mode)` | `default(mode)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_DEFAULT_NONE` | `default(none)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` ||
  | `ACC_CLAUSE_DEFAULT_PRESENT` | `default(present)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` ||
  | `ACC_CLAUSE_DEVICE_TYPE(...)` | `device_type(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_DEVICE_TYPE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_ASYNC(...)` | `async(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_NOWAIT` ||
  | `ACC_CLAUSE_WAIT(...)` | `wait(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_DEPEND_IN(__VA_ARGS__)` ||
  | `ACC_CLAUSE_FINALIZE` | `finalize` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_NUM_GANGS(n)` | `num_gangs(n)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_NUM_WORKERS(n)` | `num_workers(n)` | `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` ||
  | `ACC_CLAUSE_VECTOR_LENGTH(n)` | `vector_length(n)` | `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` ||
  | `ACC_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_REDUCTION(__VA_ARGS__)` ||
  | `ACC_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_PRIVATE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_COPY(...)` | `copy(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)` ||
  | `ACC_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)` ||
  | `ACC_CLAUSE_COPYOUT(...)` | `copyout(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)` ||
  | `ACC_CLAUSE_CREATE(...)` | `create(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)` ||
  | `ACC_CLAUSE_NO_CREATE(...)` | `no_create(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_DELETE(...)` | `delete(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_DELETE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_PRESENT(...)` | `present(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_DEVICEPTR(...)` | `deviceptr(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(__VA_ARGS__)` ||
  | `ACC_CLAUSE_ATTACH(...)` | `attach(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_DETACH(...)` | `detach(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_USE_DEVICE(...)` | `use_device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(__VA_ARGS__)` ||
  | `ACC_CLAUSE_IF_PRESENT` | `if_present` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_COLLAPSE(n)` | `collapse(n)` | `OMP_TARGET_CLAUSE_COLLAPSE(n)` ||
  | `ACC_CLAUSE_SEQ` | `seq` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_AUTO` | `auto` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_INDEPENDENT` | `independent` | `OMP_TARGET_CLAUSE_SIMD` ||
  | `ACC_CLAUSE_TILE(...)` | `tile(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_GANG` | `gang` | N/A (disregarded in OpenMP backend) | notation as `gang(n)` is N/A |
  | `ACC_CLAUSE_WORKER` | `worker` | N/A (disregarded in OpenMP backend) | notation as `worker(n)` is N/A |
  | `ACC_CLAUSE_VECTOR` | `vector` | N/A (disregarded in OpenMP backend) | notation as `vector(n)` is N/A |
  | `ACC_CLAUSE_READ` | `read` | `OMP_TARGET_CLAUSE_READ` ||
  | `ACC_CLAUSE_WRITE` | `write` | `OMP_TARGET_CLAUSE_WRITE` ||
  | `ACC_CLAUSE_UPDATE` | `update` | `OMP_TARGET_CLAUSE_UPDATE` ||
  | `ACC_CLAUSE_CAPTURE` | `capture` | `OMP_TARGET_CLAUSE_CAPTURE` ||
  | `ACC_CLAUSE_HOST(...)` | `host(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)` ||
  | `ACC_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_TO(__VA_ARGS__)` ||
  | `ACC_PASS_LIST(...)` | `(__VA_ARGS__)` | `OMP_TARGET_PASS_LIST(__VA_ARGS__)` ||
  | `ACC_CLAUSE_BIND(arg)` | `bind(arg)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_NOHOST` | `nohost` | `OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` ||
  | `ACC_CLAUSE_DEVICE_RESIDENT(...)` | `device_resident(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||
  | `ACC_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) ||

  </details>

* <details><summary>OpenMP target clauses</summary>

  | input | output | counterpart in OpenACC backend | counterpart in fallback mode (CPU execution without offloading) |
  | ---- | ---- | ---- | ---- |
  | `OMP_TARGET_CLAUSE_ALIGNED(...)` | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SIMDLEN(length)` | `OMP_CLAUSE_SIMDLEN(length)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SIMDLEN(length)` |
  | `OMP_TARGET_CLAUSE_DEVICE_TYPE(type)` | `device_type(type)` | `ACC_CLAUSE_DEVICE_TYPE(type)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_ENTER(...)` | `enter(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_INDIRECT(...)` | `indirect(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_COPYIN(...)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_NUM_THREADS(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `ACC_CLAUSE_VECTOR_LENGTH(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` |
  | `OMP_TARGET_CLAUSE_PROC_BIND(attr)` | `OMP_CLAUSE_PROC_BIND(attr)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_PROC_BIND(attr)` |
  | `OMP_TARGET_CLAUSE_NUM_TEAMS(...)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | `ACC_CLAUSE_NUM_WORKERS(__VA_ARGS__)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | `ACC_CLAUSE_VECTOR_LENGTH(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` |
  | `OMP_TARGET_CLAUSE_NONTEMPORAL(...)` | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SAFELEN(length)` | `OMP_CLAUSE_SAFELEN(length)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SAFELEN(length)` |
  | `OMP_TARGET_CLAUSE_ORDERED(...)` | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SCHEDULE(...)` | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_DIST_SCHEDULE(...)` | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_BIND(binding)` | `OMP_CLAUSE_BIND(binding)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_BIND(binding)` |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...)` | `use_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...)` | `use_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP(...)` | `defaultmap(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | `OMP_TARGET_CLAUSE_DEFAULTMAP(none)` | `ACC_CLAUSE_DEFAULT_NONE` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | `OMP_TARGET_CLAUSE_DEFAULTMAP(present)` | `ACC_CLAUSE_DEFAULT_PRESENT` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...)` | `has_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | `is_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_USES_ALLOCATORS(...)` | `uses_allocators(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_FROM(...)` | `from(__VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_CLAUSE_TO(...)` | `to(__VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
  | `OMP_TARGET_PASS_LIST(...)` | `OMP_PASS_LIST(__VA_ARGS__)` | `ACC_PASS_LIST(__VA_ARGS__)` | `OMP_PASS_LIST(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SEQ_CST` | `OMP_CLAUSE_SEQ_CST` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SEQ_CST` |
  | `OMP_TARGET_CLAUSE_ACQ_REL` | `OMP_CLAUSE_ACQ_REL` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ACQ_REL` |
  | `OMP_TARGET_CLAUSE_RELEASE` | `OMP_CLAUSE_RELEASE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_RELEASE` |
  | `OMP_TARGET_CLAUSE_ACQUIRE` | `OMP_CLAUSE_ACQUIRE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ACQUIRE` |
  | `OMP_TARGET_CLAUSE_RELAXED` | `OMP_CLAUSE_RELAXED` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_RELAXED` |
  | `OMP_TARGET_CLAUSE_READ` | `OMP_CLAUSE_READ` | `ACC_CLAUSE_READ` | `OMP_CLAUSE_READ` |
  | `OMP_TARGET_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` | `ACC_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` |
  | `OMP_TARGET_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` | `ACC_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` |
  | `OMP_TARGET_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` | `ACC_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` |
  | `OMP_TARGET_CLAUSE_COMPARE` | `OMP_CLAUSE_COMPARE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_COMPARE` |
  | `OMP_TARGET_CLAUSE_FAIL(...)` | `OMP_CLAUSE_FAIL(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_FAIL(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_WEAK` | `OMP_CLAUSE_WEAK` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_WEAK` |
  | `OMP_TARGET_CLAUSE_HINT(expression)` | `OMP_CLAUSE_HINT(expression)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_HINT(expression)` |
  | `OMP_TARGET_CLAUSE_SIMD` | `OMP_CLAUSE_SIMD` | `ACC_CLAUSE_INDEPENDENT` | `OMP_CLAUSE_SIMD` |
  | `OMP_TARGET_CLAUSE_DEFAULT_SHARED` | `OMP_CLAUSE_DEFAULT_SHARED` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_SHARED` |
  | `OMP_TARGET_CLAUSE_DEFAULT_FIRSTPRIVATE` | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` |
  | `OMP_TARGET_CLAUSE_DEFAULT_PRIVATE` | `OMP_CLAUSE_DEFAULT_PRIVATE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_PRIVATE` |
  | `OMP_TARGET_CLAUSE_DEFAULT_NONE` | `OMP_CLAUSE_DEFAULT_NONE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_NONE` |
  | `OMP_TARGET_CLAUSE_SHARED(...)` | `OMP_CLAUSE_SHARED(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SHARED(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_PRIVATE(...)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_PRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_LASTPRIVATE(...)` | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_LINEAR(...)` | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_ALLOCATE(...)` | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | `ACC_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` |
  | `OMP_TARGET_CLAUSE_DEPEND(...)` | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | `ACC_CLAUSE_WAIT(__VA_ARGS__)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `device(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` |
  | `OMP_TARGET_CLAUSE_IF_TARGET(condition)` | `OMP_CLAUSE_IF(target : condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(target : condition)` |
  | `OMP_TARGET_CLAUSE_MAP(...)` | `OMP_CLAUSE_MAP(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_MAP(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | `ACC_CLAUSE_CREATE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | `ACC_CLAUSE_COPYOUT(__VA_ARGS__)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | `ACC_CLAUSE_COPY(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_ORDER(...)` | `OMP_CLAUSE_ORDER(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ORDER(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_NOWAIT` | `OMP_CLAUSE_NOWAIT` | `ACC_CLAUSE_ASYNC()` | `OMP_CLAUSE_NOWAIT` |
  | `OMP_TARGET_CLAUSE_REDUCTION(...)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` | `ACC_CLAUSE_REDUCTION(__VA_ARGS__)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_IN_REDUCTION(...)` | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` |

  </details>

* <details><summary>OpenMP clauses</summary>

  | input | output |
  | ---- | ---- |
  | `OMP_CLAUSE_EXCLUSIVE(...)` | `exclusive(__VA_ARGS__)` |
  | `OMP_CLAUSE_INCLUSIVE(...)` | `inclusive(__VA_ARGS__)` |
  | `OMP_CLAUSE_ALIGNED(...)` | `aligned(__VA_ARGS__)` |
  | `OMP_CLAUSE_INBRANCH` | `inbranch` |
  | `OMP_CLAUSE_NOTINBRANCH` | `notinbranch` |
  | `OMP_CLAUSE_SIMDLEN(length)` | `simdlen(length)` |
  | `OMP_CLAUSE_UNIFORM(...)` | `uniform(__VA_ARGS__)` |
  | `OMP_CLAUSE_SIZES(...)` | `sizes(__VA_ARGS__)` |
  | `OMP_CLAUSE_FULL` | `full` |
  | `OMP_CLAUSE_PARTIAL(...)` | `partial(__VA_ARGS__)` |
  | `OMP_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` |
  | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `num_threads(nthreads)` |
  | `OMP_CLAUSE_PROC_BIND(attr)` | `proc_bind(attr)` |
  | `OMP_CLAUSE_NUM_TEAMS(...)` | `num_teams(__VA_ARGS__)` |
  | `OMP_CLAUSE_THREAD_LIMIT(num)` | `thread_limit(num)` |
  | `OMP_CLAUSE_NONTEMPORAL(...)` | `nontemporal(__VA_ARGS__)` |
  | `OMP_CLAUSE_SAFELEN(length)` | `safelen(length)` |
  | `OMP_CLAUSE_FILTER(thread_num)` | `filter(thread_num)` |
  | `OMP_CLAUSE_COPYPRIVATE(...)` | `copyprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_ORDERED(...)` | `ordered(__VA_ARGS__)` |
  | `OMP_CLAUSE_SCHEDULE(...)` | `schedule(__VA_ARGS__)` |
  | `OMP_CLAUSE_DIST_SCHEDULE(...)` | `dist_schedule(__VA_ARGS__)` |
  | `OMP_CLAUSE_BIND(binding)` | `bind(binding)` |
  | `OMP_CLAUSE_AFFINITY(...)` | `affinity(__VA_ARGS__)` |
  | `OMP_CLAUSE_DETACH(event_handle)` | `detach(event_handle)` |
  | `OMP_CLAUSE_FINAL(expression)` | `final(expression)` |
  | `OMP_CLAUSE_MERGEABLE` | `mergeable` |
  | `OMP_CLAUSE_PRIORITY(value)` | `priority(value)` |
  | `OMP_CLAUSE_UNTIED` | `untied` |
  | `OMP_CLAUSE_GRAINSIZE(...)` | `grainsize(__VA_ARGS__)` |
  | `OMP_CLAUSE_NOGROUP` | `nogroup` |
  | `OMP_CLAUSE_NUM_TASKS(...)` | `num_tasks(__VA_ARGS__)` |
  | `OMP_CLAUSE_INIT(...)` | `init(__VA_ARGS__)` |
  | `OMP_CLAUSE_USE(var)` | `use(var)` |
  | `OMP_CLAUSE_TASK_REDUCTION(...)` | `task_reduction(__VA_ARGS__)` |
  | `OMP_CLAUSE_DESTROY(...)` | `destroy(__VA_ARGS__)` |
  | `OMP_PASS_LIST(...)` | `(__VA_ARGS__)` |
  | `OMP_CLAUSE_SEQ_CST` | `seq_cst` |
  | `OMP_CLAUSE_ACQ_REL` | `acq_rel` |
  | `OMP_CLAUSE_RELEASE` | `release` |
  | `OMP_CLAUSE_ACQUIRE` | `acquire` |
  | `OMP_CLAUSE_RELAXED` | `relaxed` |
  | `OMP_CLAUSE_READ` | `read` |
  | `OMP_CLAUSE_WRITE` | `write` |
  | `OMP_CLAUSE_UPDATE` | `update` |
  | `OMP_CLAUSE_CAPTURE` | `capture` |
  | `OMP_CLAUSE_COMPARE` | `compare` |
  | `OMP_CLAUSE_FAIL(...)` | `fail(__VA_ARGS__)` |
  | `OMP_CLAUSE_WEAK` | `weak` |
  | `OMP_CLAUSE_HINT(expression)` | `hint(expression)` |
  | `OMP_CLAUSE_THREADS` | `threads` |
  | `OMP_CLAUSE_SIMD` | `simd` |
  | `OMP_CLAUSE_DOACROSS(...)` | `doacross(__VA_ARGS__)` |
  | `OMP_CLAUSE_DEFAULT(attr)` | `default(attr)` |
  | `OMP_CLAUSE_DEFAULT_SHARED` | `OMP_CLAUSE_DEFAULT(shared)` |
  | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | `OMP_CLAUSE_DEFAULT(firstprivate)` |
  | `OMP_CLAUSE_DEFAULT_PRIVATE` | `OMP_CLAUSE_DEFAULT(private)` |
  | `OMP_CLAUSE_DEFAULT_NONE` | `OMP_CLAUSE_DEFAULT(none)` |
  | `OMP_CLAUSE_SHARED(...)` | `shared(__VA_ARGS__)` |
  | `OMP_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` |
  | `OMP_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_LASTPRIVATE(...)` | `lastprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_LINEAR(...)` | `linear(__VA_ARGS__)` |
  | `OMP_CLAUSE_ALLOCATE(...)` | `allocate(__VA_ARGS__)` |
  | `OMP_CLAUSE_COLLAPSE(n)` | `collapse(n)` |
  | `OMP_CLAUSE_DEPEND(...)` | `depend(__VA_ARGS__)` |
  | `OMP_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND(in : __VA_ARGS__)` |
  | `OMP_CLAUSE_IF(...)` | `if(__VA_ARGS__)` |
  | `OMP_CLAUSE_MAP(...)` | `map(__VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP(alloc : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP(to : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP(from : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP(tofrom : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP(release : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP(delete : __VA_ARGS__)` |
  | `OMP_CLAUSE_ORDER(...)` | `order(__VA_ARGS__ concurrent)` |
  | `OMP_CLAUSE_NOWAIT` | `nowait` |
  | `OMP_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` |
  | `OMP_CLAUSE_IN_REDUCTION(...)` | `in_reduction(__VA_ARGS__)` |

  </details>

