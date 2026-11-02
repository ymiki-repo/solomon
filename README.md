# Solomon: Simple Off-LOading Macros Orchestrating multiple Notations

| | 日本語 | English |
| ---- | ---- | ---- |
| C/C++ | [README](README_jp.md) | [this file](README.md) |
| Fortran (**added in v2.0.0**) | [README](README_f90_jp.md) | [README](README_f90.md) |

## Summary

* Preprocessor macros to switch OpenACC and OpenMP target directives
* Current version: v2.0.0 (defined as `SOLOMON_VERSION` in `solomon/solomon.hpp`)
* See [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) for detail
* Please cite [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380)
* Released under the MIT license, see LICENSE.txt
* Copyright (c) 2024 Yohei MIKI

## Table of contents

* [Summary](#summary)
* [Significance](#significance)
  * [Background](#background)
  * [Development Policy and Features](#development-policy-and-features)
* [Changelog](#changelog)
  * [v2.0.0 (2026-11-04)](#v200-2026-11-04)
* [How to use](#how-to-use)
  * [How to develop codes using Solomon](#how-to-develop-codes-using-solomon)
  * [How to compile codes using Solomon](#how-to-compile-codes-using-solomon)
  * [Backward compatibility with v1.x (v2.0.0 or later)](#backward-compatibility-with-v1x-v200-or-later)
  * [How to extend capability of Solomon (Code Generators)](#how-to-extend-capability-of-solomon-code-generators)
  * [Profiler tags (NVTX, rocTX, ITT)](#profiler-tags-nvtx-roctx-itt)
  * [Editor support (syntax highlighting and clang-format)](#editor-support-syntax-highlighting-and-clang-format)
* [Samples](#samples)
  * [nbody: sample of compute-intensive application](#nbody-sample-of-compute-intensive-application)
  * [diffusion: sample of memory-intensive application](#diffusion-sample-of-memory-intensive-application)
  * [declare: sample of the declare directives](#declare-sample-of-the-declare-directives)
  * [Running the samples](#running-the-samples)
* [Available APIs in Solomon](#available-apis-in-solomon)
  * [Available directives](#available-directives)
  * [Available clauses](#available-clauses)
  * [Available runtime functions](#available-runtime-functions)
* [Acknowledgment](#acknowledgment)

## Significance

### Background

* GPU offloading using directives requires less porting effort compared to GPU offloading using low-level development environments such as CUDA/HIP/SYCL
* OpenACC and OpenMP target are typical directives used for GPU offloading, but there is a trade-off between the two:
  * OpenACC
    * It is a widely used directive, and because it was developed earlier, its features and documentation are more extensive
    * Since AMD and Intel do not provide compilers that support OpenACC, it is practically a directive tailored for NVIDIA GPUs
      * Note that there is OpenACC support for AMD GPUs via the HPE Cray compiler, among others
  * OpenMP target
    * All major GPU vendors (NVIDIA, AMD, Intel) provide compilers that support OpenMP target
    * It does not yet support all the features available in OpenACC
* For directive-based GPU offloading, you must decide which directive to use while taking these trade-offs into account

### Development Policy and Features

* To release from vendor lock-in, the development of Solomon aims to achieve the following:
  * Utilize both OpenACC and OpenMP target through a unified syntax using a directive-like notation
  * Reduce the learning curve for both beginners in directive-based GPU offloading and developers already accustomed to OpenACC or OpenMP target implementations
* Solomon provides functionality up to the expansion into directives using preprocessor macros
  * Since actual binary generation is delegated to the GPU vendors' compilers, you directly benefit from compiler performance improvements and feature extensions
  * Because it is simply a collection of macros, even if development/updates stagnate, it will not have adverse effects such as breaking user code
  * It is also easy for users to add their own macros on the user side

## Changelog

### v2.0.0 (2026-11-04)

* **Fortran support**: the same macros as C/C++ now work in Fortran codes through the `spp.sh` / `fortran.mk` preprocessing ([README for Fortran](README_f90.md))
* **Namespace cleanup**: all user-facing macros were renamed with the `SOLOMON_` / `SOLOMON_CLAUSE_` prefixes; the v1.x spellings keep working with `-DSOLOMON_WITH_SHORT_NAMES`, and a migration script is bundled; the helpers `PRAGMA(...)` / `PRAGMA_STR(x)` / `PRAGMA_STR1(x)` / `SHOW_PRAGMA(...)` became internal and their v1.x names are defined only with `-DSOLOMON_WITH_SHORT_NAMES` ([Backward compatibility](#backward-compatibility-with-v1x-v200-or-later))
* **Two-level simple notation**: the parallelism hierarchy follows CUDA's block / thread; `SOLOMON_OFFLOAD_OUTER_LOOP(...)` / `SOLOMON_PARALLELIZE_INNER_LOOP(...)` and `SOLOMON_CLAUSE_NUM_BLOCKS(n)` / `SOLOMON_CLAUSE_NUM_THREADS(n)` express the gang↔teams / vector↔thread correspondence (`SOLOMON_CLAUSE_BLOCK` / `SOLOMON_CLAUSE_THREAD` are OpenACC-only hints, ignored in OpenMP target)
* **Meaning of the block clauses in OpenACC** (changed from v1.x): `SOLOMON_CLAUSE_BLOCK` / `SOLOMON_CLAUSE_NUM_BLOCKS(n)` (v1.x `AS_BLOCK` / `NUM_BLOCKS(n)`) now emit `gang` / `num_gangs(n)` instead of `worker` / `num_workers(n)` in the OpenACC backend, and `SOLOMON_CLAUSE_NUM_GRIDS(n)` (v1.x `NUM_GRIDS(n)`) now emits `num_teams(n)` in the OpenMP target backend as well ([Backward compatibility](#backward-compatibility-with-v1x-v200-or-later))
* **Serial fallback**: when neither OpenACC nor OpenMP is enabled, every directive is removed and the code compiles as serial code (manually adding `-D_OPENMP` is no longer needed); `SOLOMON_APPEND_ARGS(...)` and its v1.x name `APPEND_ARGS(...)` also expand to nothing there, dropping their arguments
* **Device-resident variables**: `SOLOMON_DECLARE_ON_DEVICE(...)` (`SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` for link semantics)
* **Serial offloaded regions**: `SOLOMON_OFFLOAD_SERIAL` (mutual conversion between `acc serial` and a bare `omp target`)
* **Improved offloaded-function declarations**: fixed the broken output of `SOLOMON_DECLARE_OFFLOADED` and added the named form `SOLOMON_CLAUSE_TARGETS(...)`
* **Vendor-neutral profiler tags**: `-DSOLOMON_TAGGED_PROFILE(_AUTO)` emits NVTX / rocTX / ITT tags ([Profiler tags](#profiler-tags-nvtx-roctx-itt))
* **Line numbers of automatic profiler tags with GCC**: `spp.sh` and `fortran.mk` pass `-ftrack-macro-expansion=0` to GCC's `cpp`, since GCC's preprocessor can write a wrong line number in the later tags of a long file; for C/C++ built with g++ / gcc, see [Profiler tags](#profiler-tags-nvtx-roctx-itt)
* **Editor support**: syntax highlighting for vim / emacs / VS Code and a `.clang-format` are bundled ([Editor support](#editor-support-syntax-highlighting-and-clang-format))
* **Sample job scripts**: the recipes are shared in `sh/common/`, and each system only needs its settings in `sh/<system>/config.sh` ([Running the samples](#running-the-samples)); `verify.sh` judges the results (a job with a wrong result fails), and a job script stops when another job script is running in the same sample directory or when files of a previous run are left
* **Fortran preprocessing tools**: `spp.sh` and `fortran.mk` ask the compiler itself, with the flags as given, whether OpenACC / OpenMP is enabled and how it reads the source, and stop with an error when the compiler does not answer (e.g., an unknown flag) or reads the source as fixed form; the source must be the last argument of `spp.sh` ([README for Fortran](README_f90.md))
* **Migration script**: by default it rewrites only the sources that include `solomon.hpp` (`--all` rewrites every source given) ([Backward compatibility](#backward-compatibility-with-v1x-v200-or-later))
* **Waiting for queues**: `SOLOMON_CLAUSE_WAIT_QUEUE(...)` makes a construct wait for the given queues before it starts (OpenACC `wait(...)`; ignored in OpenMP target)
* **Merging `wait` clauses**: the `wait` clauses of one directive are merged per `device_type` segment following OpenACC (`wait` and `wait(1)` become `wait`; `wait(1)` and `wait(2)` become `wait(1, 2)`; `devnum:` / `queues:` waits are kept)
* **Clauses only `simd` takes, in the Solomon notation**: `SOLOMON_CLAUSE_SIMDLEN(length)`, `SOLOMON_CLAUSE_SAFELEN(length)`, `SOLOMON_CLAUSE_ALIGNED(...)`, `SOLOMON_CLAUSE_NONTEMPORAL(...)`, and `SOLOMON_CLAUSE_IF_SIMD(condition)` are emitted where `SOLOMON_CLAUSE_INDEPENDENT` adds `simd` (OpenMP only); the ones that NVIDIA HPC SDK (nvc / nvc++) rejects are dropped on purpose with that compiler ([Available clauses](#available-clauses))
* **Correction of the v1.1.0 entry on clause sorting**: the automatic sorting moves only `SOLOMON_CLAUSE_INDEPENDENT` (and its equivalents) to the front: `simd` in the OpenMP backends and `independent` in OpenACC (v2.0.0 or later; v1.x moved it in the OpenMP backends only), and a second one is dropped. Every other clause keeps the written order, so the generated directives can depend on the input order (the order of clauses is not significant in OpenACC)
* **Runtime functions**: the six functions to choose the device and to get the stream (`get_num_devices_for_offloading()` and the like, present since v1.0.0) are now documented ([Available runtime functions](#available-runtime-functions)). The three device functions are now also defined in fallback mode (the OpenMP routines) and serial mode (`0` / nothing), where the build failed; `get_offloaded_stream_id*` returned no value in C++14 or later, and now return the stream
* **Bug fixes** (the output of the directives below changes; codes that relied on the old output may behave differently):
  * `ACC_CLAUSE_WAIT(...)` dropped the `wait` clause and every clause written after it; they are now emitted, so a dropped `async` and the like take effect (codes that do not wait explicitly may behave differently). `OMP_TARGET_CLAUSE_DEPEND_IN(...)` now becomes `wait` without arguments (waits for all async queues) in the OpenACC backend. An empty argument of `ACC_CLAUSE_WAIT(...)` (a stray comma, as in `ACC_CLAUSE_WAIT(1, )`) stops the build with a Solomon error in the OpenACC and OpenMP target backends
  * The clauses written after `SOLOMON_CLAUSE_ASYNC_QUEUE(...)` were dropped in the backends other than OpenACC; they are now kept
  * `async` / `wait` / `device_type` given to the data construct and `device_type` given to `update` were dropped; they are now emitted (OpenACC 3.2 or later; GCC 13 rejects `async` / `wait` on the data construct). A dropped `async` takes effect and the copyout at the exit of the data construct becomes asynchronous, so codes that read the results on the host without a wait may behave differently
  * `SOLOMON_CLAUSE_ASYNC()` / `ACC_CLAUSE_ASYNC()` without arguments, and `OMP_TARGET_CLAUSE_NOWAIT` in the OpenACC backend, emitted `async()`, which compilers other than the C/C++ compilers of NVIDIA HPC SDK rejected; they now emit `async`
  * `ACC_CLAUSE_SELF(...)` given to `PRAGMA_ACC_UPDATE` was dropped in the OpenMP target backend, so the transfer from the device to the host silently disappeared; it now becomes `from(...)`
  * `ACC_CLAUSE_SELF(...)` given to a compute construct was dropped in the OpenMP target backend, so the region ran on the device instead of the host; it now becomes `if(target: !(condition))` (an `if` clause on the same directive stops the OpenMP target compilers). `ACC_CLAUSE_SELF(...)` with two or more arguments (the list of `PRAGMA_ACC_UPDATE(...)`) given to a compute construct stops the build with a Solomon error in the OpenMP target backend
  * OpenMP data directives in the OpenACC backend became OpenACC directives without clauses, which silently changed the meaning or were rejected by compilers; target data with only `use_device_*` now becomes `acc host_data`, and the other forms without an OpenACC counterpart stop with a Solomon error
  * In the OpenMP target backend, an OpenACC data directive (`data`, `host_data`, `enter data`, `exit data`, `update`) none of whose clauses becomes a clause the OpenMP directive requires (`map` / `use_device_*`, or `to` / `from` for `update`) became an OpenMP data directive without its required clause, which most compilers rejected; it is no longer generated
  * `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` / `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_FROM(...))`, the form listed in this README, emitted a broken `map` clause since v1.x; each nested clause now becomes `map(to : ...)` / `map(from : ...)` (OpenACC: `copyin(...)` / `copyout(...)`). Map modifiers can be written as `OMP_TARGET_MAP_MODIFIER_*` in front of the nested clauses, and `OMP_TARGET_CLAUSE_TO_PRESENT(...)` / `OMP_TARGET_CLAUSE_FROM_PRESENT(...)` were added for `update`. A raw list in front of the nested clauses or the modifiers (e.g., `OMP_TARGET_CLAUSE_MAP(a, OMP_TARGET_CLAUSE_TO(b))`) stops the build with a Solomon error (checked up to the 35th element). Migration: if the third or a later element of a non-nested list is the name of a function-like macro that cannot be invoked with one argument (e.g., `#define fm2(x, y) ...` and `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`), preprocessing now fails, and if it is the name of one whose expansion with one argument has a comma at the top level (e.g., `#define fc1(x) x, x`), the build stops with a Solomon error that names internal macros. Place that element first or second (e.g., `OMP_TARGET_CLAUSE_MAP(fm2, a, b)`), or write it as an array section (`fm2[0:n]`)
  * In fallback mode (gcc / clang with `-fopenmp` and the like), `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` / `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` now become `omp parallel for` / `omp parallel for simd`; loops that ran on one team (in effect sequentially) now run on threads, so a data race in such a loop shows up
  * In fallback mode, `OMP_TARGET_CLAUSE_IF_TARGET(...)` is now dropped; it emitted `if (target : ...)`, which compilers other than nvfortran rejected
  * `PRAGMA_OMP_TARGET_SIMD(...)` dropped all of its clauses in the OpenMP target backend since v1.x; they are now emitted
  * A name that is not a Solomon clause macro in a clause list (e.g., a misspelled name) was silently dropped together with the clauses written after it; it now stops the build with a Solomon error (Fortran: a name that the compiler rejects is left in the directive)
  * The 32nd clause of a directive was not expanded (an internal macro name was left, which the compilers rejected), and the candidates from the 33rd on were dropped: when 32 or more clauses were selected, an internal macro name was left, but a list mixing clauses that do not fit the directive (e.g., 31 `ACC_CLAUSE_COLLAPSE(...)` and 2 `ACC_CLAUSE_PRIVATE(...)` given to `PRAGMA_ACC_PARALLEL(...)`) and the directives without clause sorting (e.g., the 33rd `ACC_CLAUSE_BIND(...)` of `PRAGMA_ACC_ROUTINE(...)`) silently lost the rest. Now 32 clauses are emitted and 33 or more candidates stop the build with a Solomon error; empty arguments are not counted
  * In the OpenACC backend, an OpenACC data directive none of whose clauses is one of its required clauses (e.g., only `ACC_CLAUSE_IF(...)`) was emitted with those clauses only (e.g., `acc data if (n)`): g++ 13 accepted `data` / `host_data` silently and rejected `enter data` / `exit data` / `update`, gfortran 13 accepted `data` / `host_data` / `enter data` and rejected `update`, and nvc++ 26.9 rejected them; it now stops with a Solomon error
  * `PRAGMA_ACC_DECLARE(...)` none of whose clauses is a declare clause stops with a Solomon error whatever `_OPENACC` is: every version of OpenACC requires at least one clause on a declare directive (2.6 and 2.7, section 2.13 "declare clause-list"; 3.0 and later also as a restriction)
  * Misused map modifiers (`OMP_TARGET_MAP_MODIFIER_*` outside `OMP_TARGET_CLAUSE_MAP(...)`, modifiers without a nested clause, more than 6 modifiers) left internal names in the directive, and clang silently dropped the `map` clause; they now stop with a Solomon error. More than 32 arguments of the `wait` clauses merged into one also stop with a Solomon error
  * `SOLOMON_SYNCHRONIZE()` / `PRAGMA_ACC_WAIT()` (and `PRAGMA_OMP_TARGET_TASKWAIT()` in the OpenACC backend) without arguments emitted `acc wait (),`, and `PRAGMA_OMP_FLUSH()` without arguments emitted `omp flush (),`, in v1.1.0 (`main` as of 2026-03-21) with every compiler and language mode measured (gcc / g++, clang / clang++, and the C/C++ compilers of NVIDIA HPC SDK, in GNU and strict modes alike; `PRAGMA_OMP_FLUSH()` with OpenMP enabled); gcc / g++ and clang rejected both, and the C/C++ compilers of NVIDIA HPC SDK accepted `acc wait (),` but rejected `omp flush (),` (v1.1.0 as published on 2025-12-05 emitted `acc wait` / `omp flush`; the update of v1.1.0 in 2026-02 changed the former to `acc wait ()`, and the one in 2026-03 brought both broken forms). They now emit `acc wait` / `omp flush` (in the strict modes of gcc / g++ this brings the restriction of the next item)
  * With gcc / g++ in a strict mode before C23 / C++20 (`-std=c99`, `-std=c++17`, ...), `ACC_CLAUSE_WAIT(...)`, `ACC_CLAUSE_ASYNC(...)`, `ACC_CLAUSE_SELF(...)`, `SOLOMON_SYNCHRONIZE(...)`, `PRAGMA_ACC_WAIT(...)`, and `PRAGMA_OMP_FLUSH(...)` whose arguments end with the name of a function-like macro with two or more named parameters (e.g., `ACC_CLAUSE_WAIT(fm2)` after `#define fm2(a, b) ...`), which v1.1.0 preprocessed without an error, now stop the preprocessing: put that name in parentheses or not at the end, or use a GNU mode or C23 / C++20 (the other configurations are not affected); the same six macros with exactly 34 arguments also stop there
  * `ACC_CLAUSE_SELF()` without arguments given to `PRAGMA_ACC_UPDATE(...)` was counted as a required clause, so an update with no other clause became `update self()` in OpenACC (g++ rejected it, and nvc++ accepted it) and `target update` without clauses in OpenMP target (g++ and clang++ rejected it, and nvc++ 26.9 accepted it); it now stops with the Solomon error of the required clauses in OpenACC and is not generated in OpenMP target
  * In the OpenACC backend, `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)`, `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)`, and `PRAGMA_OMP_TARGET_SIMD(...)` emitted no `loop` construct, so with `OFFLOAD_BY_OPENACC_PARALLEL` every gang ran the whole loop (wrong results without the compiler's auto-parallelization) and the loop clauses (e.g., `collapse`) were dropped; they now emit `loop independent gang` / `loop independent`. Loops that ran redundantly or on one gang now run in parallel, so a data race in such a loop shows up
  * `OMP_TARGET_CLAUSE_MAP_DELETE(...)` became `delete` without `finalize` in the OpenACC backend, which only decrements the reference count (the data stayed on the device and later transfers were skipped); the exit data directive now gets `finalize`, which applies to every `copyout` / `delete` of that directive (write `OMP_TARGET_CLAUSE_MAP_DELETE(...)` in an exit data directive of its own when the other variables were entered more than once). If you added `ACC_CLAUSE_FINALIZE` to such a directive as a workaround, remove it: `finalize finalize` is rejected by GCC 13
  * In OpenMP backends, `ACC_CLAUSE_DELETE(...)` (and therefore `PRAGMA_ACC_EXIT_DATA_DELETE(...)` and `SOLOMON_FREE_FROM_DEVICE(...)`) emits `map(release: ...)` instead of `map(delete: ...)`: it decrements the reference count as in OpenACC instead of removing the data
  * In the OpenACC backend, `PRAGMA_OMP_TARGET_TASKWAIT(...)` passed its clauses to `acc wait` (internal names left in the directive); it now emits `acc wait`, which waits for all the queues
  * In fallback mode, `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` no longer becomes `num_threads(num)`, which duplicated `num_threads` together with `OMP_TARGET_CLAUSE_NUM_THREADS(...)`; `SOLOMON_CLAUSE_NUM_THREADS(n)` and `ACC_CLAUSE_VECTOR_LENGTH(n)` still become `num_threads(n)`, except in `PRAGMA_OMP_TARGET_TEAMS(...)`, which takes them as `thread_limit(n)`
  * In OpenMP backends, `ACC_CLAUSE_DEVICE_TYPE(...)` and `ACC_CLAUSE_NOHOST` are dropped also in `PRAGMA_ACC_ROUTINE(...)` / `SOLOMON_DECLARE_OFFLOADED(...)`; they emitted a `declare target` that compilers reject. In the OpenACC backend, `OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` becomes `nohost`, and `host` / `any` are dropped; another word stops the build with a Solomon error
  * `OMP_TARGET_CLAUSE_INDIRECT()`, `OMP_TARGET_CLAUSE_ORDERED()`, `OMP_CLAUSE_ORDERED()`, `OMP_CLAUSE_PARTIAL()`, and `OMP_CLAUSE_DESTROY()` without an argument emit `indirect` / `ordered` / `partial` / `destroy` instead of `indirect()` / `ordered()` / `partial()` / `destroy()`
  * `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` (and `_SIMD`) drops `copyin` like the other target combined directives
  * Host OpenMP directives silently dropped clauses that they take: the clauses of `PRAGMA_OMP_MASKED(...)`, `PRAGMA_OMP_SCAN(...)`, `PRAGMA_OMP_SCOPE(...)`, `PRAGMA_OMP_TILE(...)`, `PRAGMA_OMP_WORKSHARE(...)`, and `PRAGMA_OMP_DEPOBJ(...)` (and the combined forms with `masked`), the clauses of `sections` (including `nowait`), `in_reduction` of `taskloop`, `firstprivate` of `single`, `nowait` of `for`, the name and `hint` of `critical`, and `destroy` of `interop`; they are now emitted. `firstprivate` of `simd` and `linear` of `distribute`, which the compilers reject there, are now dropped. Inputs that passed because the clause or name vanished and now stop: `firstprivate` / `allocate` on `PRAGMA_OMP_SCOPE(...)` (OpenMP 5.2; Clang 22 and Intel 2026.1 reject them), `PRAGMA_OMP_CRITICAL(OMP_CLAUSE_HINT(h))` without a name (rejected by GCC, Clang, and Intel compilers), and a bare name such as `PRAGMA_OMP_DEPOBJ(o)` (a Solomon error; pass `OMP_PASS_LIST(o)`). `OMP_CLAUSE_DEPOBJ_UPDATE(...)` was added for the `update` clause of `PRAGMA_OMP_DEPOBJ(...)`; `OMP_CLAUSE_UPDATE` (the clause of `atomic`) given there becomes `update` without an argument, which the compilers reject, instead of vanishing
  * Passing a Solomon clause macro to `PRAGMA_OMP_FLUSH(...)` as the first argument put its expansion inside the parentheses; it now stops with a Solomon error (after the first argument the expansion still stays in the list and the compiler rejects it). With that check, a second argument that starts with `::`, `/`, or `(` stops the preprocessing: put such a name first
  * `-DPRINT_GENERATED_PRAGMA` now emits the message before the directive, so that loop directives also compile with GCC (v1.x put it between the directive and the loop)
  * In fallback mode, `SOLOMON_SYNCHRONIZE()` / `PRAGMA_ACC_WAIT(...)` (v1.x `SYNCHRONIZE()`) left the undefined name `PRAGMA_OMP_TARGET_TASKWAIT(...)` in v1.1.0, and `PRAGMA_OMP_TARGET_TASKWAIT(...)` left internal names, which stopped the build; none of them emits anything now
  * `OMP_CLAUSE_SIMD` (and `SOLOMON_CLAUSE_INDEPENDENT` / `OMP_TARGET_CLAUSE_SIMD` in the OpenMP backends) given to a directive whose name has `simd` (e.g., `PRAGMA_OMP_FOR_SIMD(...)`) is dropped instead of emitting `simd simd`, which GCC, Clang, and Intel compilers reject; with NVIDIA HPC SDK (nvc / nvc++), the directives with `target` in their names still get it (see `SOLOMON_CLAUSE_INDEPENDENT`)
  * `PRAGMA_OMP_ORDERED(...)` emits `OMP_CLAUSE_DEPEND(source)` / `OMP_CLAUSE_DEPEND(sink: ...)` for the compilers that reject `doacross` (Clang 23 and Intel 2026.1); nvc++ 26.9 ignores them with a warning and gives wrong results (see `OMP_CLAUSE_DOACROSS(...)`)
  * `OMP_CLAUSE_IF(...)` given to `PRAGMA_OMP_TEAMS(...)`, `..._TEAMS_DISTRIBUTE(...)`, or `..._TEAMS_LOOP(...)` (`if` on `teams` is a clause of OpenMP 6.0) is emitted only when `_OPENMP >= 202411`; before OpenMP 6.0, Clang-based compilers and gcc (C) drop it with a warning, and g++ stops with a Solomon error, while NVIDIA HPC SDK (nvc / nvc++, which has no host `teams`) drops it silently; in Fortran it is emitted as written in every version (a valid form of OpenMP 6.0, but compilers before 6.0 may reject it: gfortran 13 and nvfortran 26.9 reject it, and ifx 2026.1 accepts it and runs it correctly) (see `OMP_CLAUSE_IF(...)`)
  * With `-pedantic`, clang-based compilers no longer warn that Solomon uses the GNU extension `, ##__VA_ARGS__` where `__VA_OPT__` is available (see the note on `-pedantic` in [Backward compatibility](#backward-compatibility-with-v1x-v200-or-later))
  * `OMP_TARGET_CLAUSE_COPYIN(...)` (the OpenMP `copyin`, which copies `threadprivate` variables) became the OpenACC data clause `copyin` (a transfer to the device) in the OpenACC backend; it is now dropped there, as the OpenMP target backend drops it from the combined target directives (the table of OpenMP target clauses shows how to write a transfer)
  * In OpenACC, the `_SIMD` target directives and `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` given `SOLOMON_CLAUSE_INDEPENDENT` or `OMP_TARGET_CLAUSE_SIMD` emitted two `independent` clauses (GCC and Clang reject them); a directive now gets one `independent` clause (one `simd` clause in the OpenMP backends), placed first
* **Samples**: nbody writes the velocities at t = 0 into the snapshot 0 and computes `E(0)`, the reference of `collapse_error.dat`, from them (v1.x used the velocities half a time step earlier, so `E(0)` changes slightly); the diffusion `./run` takes exactly one argument (`padding/`: one or two) with 3 <= N; the Makefiles take `SOLOMON_DIR` also from the environment and stop when Solomon is not found there, and `OPT_LEVEL` other than 0 to 4 stops `make` ([Building the samples by hand](#building-the-samples-by-hand))

<details>
<summary>v1.1.0 (2025-12-05)</summary>

This entry is a summary of v1.x written in v2.0.0, and it includes the updates of `main` up to 2026-03-21 (`94ae137`).

* Added automatic sorting of clauses (the generated directives no longer depend on the input order)
* Added `ASYNC_QUEUE(id)` / `WAIT_QUEUE(id)` for asynchronous execution
* Added `PRINT_GENERATED_PRAGMA` (prints the generated directives as compile-time messages)
* Switched to `use_device_addr` from the deprecated `use_device_ptr`
* Bug fixes: `PRAGMA_ACC_WAIT`, `AS_THREAD`/`AS_BLOCK`/`AS_GRID`, missing OpenACC atomic directives, and others

(entries use the v1.x spellings of that time)

</details>

<details>
<summary>v1.0.0 (2024-08-22)</summary>

* Initial release: macros switching OpenACC and OpenMP target directives from a single source (the version published in [Miki & Hanawa 2024, IEEE Access](https://doi.org/10.1109/ACCESS.2024.3509380))
* Mutual conversion between the OpenACC and OpenMP target notations (acc2omp / omp2acc) and the simple notation
* Bundled samples (nbody, diffusion) and code generators

</details>

## How to use

### How to develop codes using Solomon

1. Include Solomon

   ```c++
   #include <solomon.hpp>
   ```

2. Insert offloading macros instead of OpenACC or OpenMP target directives

   * For beginners, we recommend to use intuitive notations like `SOLOMON_OFFLOAD(...)`
   * Experienced developers of OpenACC or OpenMP target will prefer OpenACC/OpenMP-like notations
     * In OpenMP-like notation, only notations like `PRAGMA_OMP_TARGET_*` or `OMP_TARGET_CLAUSE_*` are converted to OpenACC backend (e.g., `PRAGMA_OMP_ATOMIC(...)` will be translated as `_Pragma("omp atomic __VA_ARGS__")`)
       * OpenMP directives for the CPU (those without `TARGET`) are emitted as they are even in the OpenACC backend. They are meant for calling offloaded regions from OpenMP threads on the host (e.g., `SOLOMON_OFFLOAD(...)` inside `PRAGMA_OMP_PARALLEL(...)`), not for mixing OpenMP directives for the CPU into the offloaded regions
       * In that use, enable OpenMP as well as OpenACC (with NVIDIA HPC SDK, add `-mp` to `-acc`); without it, these macros are left undefined and the compilation fails
     * We strongly recommend not to adopt `PRAGMA_OMP_TARGET_DATA(...)` in your codes
       * In the OpenACC backend, `PRAGMA_OMP_TARGET_DATA(...)` with only `OMP_TARGET_CLAUSE_USE_DEVICE_PTR/ADDR(...)` (and `if`) becomes `host_data use_device(...)`. An OpenMP data directive none of whose clauses has an OpenACC counterpart (e.g., only the generic `OMP_TARGET_CLAUSE_MAP(...)`), or a target data directive that combines `use_device_*` with other data clauses (including the generic `OMP_TARGET_CLAUSE_MAP(...)`), stops with a Solomon error; split it into nested directives (v2.0.0 or later). `OMP_TARGET_CLAUSE_DEVICE(...)` is disregarded (OpenACC uses the current device)
       * Alternative notations are `SOLOMON_DATA_ACCESS_BY_DEVICE(...)` or `PRAGMA_ACC_DATA(...)` for data accessed by device (GPU), and `SOLOMON_DATA_ACCESS_BY_HOST(...)` or `PRAGMA_ACC_HOST_DATA(...)` for data accessed by host (CPU)
     * We strongly recommend not to adopt `PRAGMA_ACC_DECLARE(...)` in your codes (it is not converted to the OpenMP target backend)
       * Alternative notations are `SOLOMON_DECLARE_ON_DEVICE(...)` for device-resident variables and `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` for link semantics (v2.0.0 or later)
     * In OpenACC-like notation, inserting `SOLOMON_DECLARE_OFFLOADED_END` or `PRAGMA_OMP_END_DECLARE_TARGET` is required when you insert `PRAGMA_ACC_ROUTINE(...)` or the pair form of `SOLOMON_DECLARE_OFFLOADED(...)` (for proper translation to OpenMP target offloading); do not insert it after the named form with `SOLOMON_CLAUSE_TARGETS(...)` (the compilation fails)
   * `SOLOMON_IF_NOT_OFFLOADED(arg)` is available to hide directives when GPU offloading is enabled
     * <details><summary> Example: `arg` appears only in fallback mode (when GPU offloading is disabled (both OpenACC and OpenMP target are not enabled))</summary>

       ```c++
       SOLOMON_OFFLOAD()
       for(int i = 0; i < num; i++){
         SOLOMON_IF_NOT_OFFLOADED(PRAGMA_OMP_SIMD())
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
      SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, ACC_CLAUSE_VECTOR_LENGTH(128), OMP_TARGET_CLAUSE_COLLAPSE(3))
      ```

      * Mixture of intuitive and OpenACC/OpenMP-like notations are enabled
      * ~~`SOLOMON_CLAUSE_INDEPENDENT` (or the correspondences: `ACC_CLAUSE_INDEPENDENT` and `OMP_TARGET_CLAUSE_SIMD`) must be specified at the head of all optional clauses~~
        * **[UPDATE v1.1.0]** This constraint is now automatically handled. In the OpenMP backends, where `simd` is a part of the directive name, Solomon moves `SOLOMON_CLAUSE_INDEPENDENT` (and its equivalents) to the front, regardless of where you write them in your code; in OpenACC, `independent` is moved to the front as well (v2.0.0 or later; the order of clauses is not significant there, OpenACC 3.3, Section 2.1). Only one `simd` / `independent` clause is emitted, so writing `SOLOMON_CLAUSE_INDEPENDENT` where the directive already adds it (e.g., `PRAGMA_OMP_TARGET_SIMD(...)`) is harmless (with NVIDIA HPC SDK, an OpenMP target directive whose name has `simd` may still carry a `simd` clause, which it accepts). Clauses written after `ACC_CLAUSE_DEVICE_TYPE(...)` apply to the given devices, so they are neither moved nor merged (OpenACC 3.3, Section 2.4). The other clauses keep the written order in every backend
        * The clauses only `simd` takes (`OMP_CLAUSE_SIMDLEN(...)`, `OMP_CLAUSE_SAFELEN(...)`, `OMP_CLAUSE_ALIGNED(...)`, `OMP_CLAUSE_NONTEMPORAL(...)`) are dropped by a directive whose name has no `simd`, even when the sorting adds `simd`; `OMP_CLAUSE_IF(simd: ...)` is kept (`if` is also a clause of the other constructs), so the compiler rejects it where no `simd` is added (always in the `teams loop` form). In the Solomon notation, write `SOLOMON_CLAUSE_SIMDLEN(length)`, `SOLOMON_CLAUSE_SAFELEN(length)`, `SOLOMON_CLAUSE_ALIGNED(...)`, `SOLOMON_CLAUSE_NONTEMPORAL(...)`, or `SOLOMON_CLAUSE_IF_SIMD(condition)` together with `SOLOMON_CLAUSE_INDEPENDENT` instead: they go wherever `simd` goes and are dropped where it does not (see the table of the intuitive notation)
      * Solomon automatically drops incompatible clauses
        * In the OpenMP target backend, an OpenACC data directive (`data`, `host_data`, `enter data`, `exit data`, `update`) none of whose clauses becomes a clause the OpenMP directive requires (`map` / `use_device_*`, or `to` / `from` for `update`) (e.g., only `present`, or only `if`) is not generated at all (v2.0.0 or later)
        * In the OpenACC backend, an OpenACC data directive (`data`, `host_data`, `enter data`, `exit data`, `update`) none of whose clauses is one of its required clauses (e.g., only `ACC_CLAUSE_IF(...)`) stops with a Solomon error (v2.0.0 or later)
        * `PRAGMA_ACC_DECLARE(...)` none of whose clauses is a declare clause (`copy`, `copyin`, `copyout`, `create`, `present`, `deviceptr`, `device_resident`, `link`) stops with `Solomon: the OpenACC specification requires at least one clause on a declare directive (...)` (Fortran: `SOLOMON_ERROR_OPENACC_REQUIRES_A_CLAUSE_ON_DECLARE_DIRECTIVE`) whatever `_OPENACC` is (v2.0.0 or later): every version of OpenACC requires at least one clause (2.6 and 2.7, section 2.13 "declare clause-list"; 3.0 and later also as a restriction)
        * Misused Solomon map modifiers also stop the build (v2.0.0 or later): an `OMP_TARGET_MAP_MODIFIER_*` written outside `OMP_TARGET_CLAUSE_MAP(...)`, modifiers without a nested clause after them, a list other than nested clauses after a nested clause, and more than 6 modifiers in one `OMP_TARGET_CLAUSE_MAP(...)`. GCC, Clang, and Intel compilers (C/C++) report `error: Solomon: ...`; NVIDIA HPC SDK (C/C++) reports `invalid text in pragma`; in Fortran, a name starting with `SOLOMON_ERROR_` is left in the directive and the Fortran compiler reports a syntax error there
        * Only Solomon clause macros are accepted as clauses (v2.0.0 or later). A name that is not a Solomon clause macro (e.g., a misspelled `SOLOMON_CLAUSE_*` or raw clause text such as `collapse(2)`) stops the build: GCC, Clang, and Intel compilers (C/C++) report `error: Solomon: undefined name in the clause list: <name>`; NVIDIA HPC SDK (C/C++) reports `invalid text in pragma` at the line of the macro; in Fortran, `SOLOMON_ERROR_UNDEFINED_CLAUSE` is placed before the name and the Fortran compiler reports a syntax error there. An element that begins with an operator, a string, a number, or a parenthesis (e.g., `*p`, `(x)`, or `SOLOMON_CLAUSE_TARGETS(...)`) placed right after such a name may instead stop the preprocessor with a token-pasting error. A missing comma between two clause macros also stops with `undefined name in the clause list`, which then shows a number and internal names (`SOLOMON_INTERNAL_TAG_*`) instead of the clauses. Serial builds (neither OpenACC nor OpenMP enabled) do not check the names. Empty arguments are ignored (they are not counted toward the clause limit). `OMP_CLAUSE_*` are for the host OpenMP directives (`PRAGMA_OMP_*`) and are not meant for OpenACC directives: in the OpenACC backend without OpenMP they stop as undefined names (for clauses used in both OpenACC and OpenMP target, use `OMP_TARGET_CLAUSE_*` or `SOLOMON_CLAUSE_*`). In C/C++, an undefined name followed by integer elements (e.g., `async, 1`) may be taken for a clause and vanish, or, when Solomon clause macros follow them, stop the build with a message that names an internal name (e.g., `SOLOMON_INTERNAL_TAG_ACC_PARALLEL`) instead of the name you wrote; write Solomon clause macros only
      * The emptiness test: `ACC_CLAUSE_WAIT(...)`, `ACC_CLAUSE_ASYNC(...)` and `ACC_CLAUSE_SELF(...)` (and the clauses of the same names in the other notations, `OMP_CLAUSE_ORDERED(...)`, `OMP_CLAUSE_PARTIAL(...)`, `OMP_CLAUSE_DESTROY(...)`, and `OMP_TARGET_CLAUSE_INDIRECT(...)`, and the directives `SOLOMON_SYNCHRONIZE(...)`, `PRAGMA_ACC_WAIT(...)`, and `PRAGMA_OMP_FLUSH(...)`) test whether their arguments are empty. With gcc / g++ in a strict mode before C23 / C++20 (`-std=c99`, `-std=c++17`, ...), preprocessing stops when the last token of the arguments is the name of a function-like macro with two or more named parameters (e.g., `fm2` after `#define fm2(a, b) ...`), and when there are exactly 34 arguments (33 top-level commas; 33 arguments, and 35 or more, pass): put that name in parentheses or not at the end, give another number of arguments, or use a GNU mode or C23 / C++20. Other configurations (GNU modes, clang-based compilers, C23 / C++20, Fortran) have no such limit
   * We encourage the adoption of combined macros (instead of individual macros separately) for better conversion between OpenACC and OpenMP target

     | recommended implementations | corresponding implementation (not recommended) |
     | ---- | ---- |
     | **`SOLOMON_OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` | <br> `PRAGMA_ACC_KERNELS(...) PRAGMA_ACC_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC_LOOP(...)` |
     | **`SOLOMON_DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `PRAGMA_ACC_DATA(ACC_CLAUSE_PRESENT(...))` |
     | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` |

     * Do not mix raw clause text and Solomon macros in one `OMP_TARGET_CLAUSE_MAP(...)`: write map-type modifiers as `OMP_TARGET_MAP_MODIFIER_*` before the nested clauses (e.g., `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_MAP_MODIFIER_PRESENT, OMP_TARGET_CLAUSE_TO(a))`), not as `OMP_TARGET_CLAUSE_MAP(present, OMP_TARGET_CLAUSE_TO(a))` or `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(present : a))`. Such mixtures are not supported. Nested clauses and modifiers must come first: a raw list in front of them stops the build with a Solomon error (v2.0.0 or later; checked up to the 35th element)

   * If you wish to give finer-grained instructions than `SOLOMON_OFFLOAD(...)` (i.e., to attach directives to individual loops in a loop nest), the macros below are available
     * To improve compatibility between OpenACC and OpenMP target, the use of the combined macros is recommended (direct implementations using OpenACC/OpenMP-style notations are not guaranteed to be converted correctly to the other backend due to design differences between the two sets of directives)

     | Available macros | output | offloading backend | note |
     | ---- | ---- | ---- | ---- |
     | **`SOLOMON_OFFLOAD_OUTER_LOOP(...)`** | `_Pragma("acc parallel [...]") _Pragma("acc loop gang [...]")` <br> `_Pragma("omp target teams distribute [...]")` <br> `_Pragma("omp parallel for [...]")` | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) | specify just before the outer loop of a loop nest <br> suggest the number of threads and thread-blocks via `SOLOMON_CLAUSE_NUM_THREADS(n)` and `SOLOMON_CLAUSE_NUM_BLOCKS(n)` <br> do not pass `SOLOMON_CLAUSE_BLOCK`, `ACC_CLAUSE_GANG`, and similar clauses, which would duplicate the clauses already embedded in the macro |
     | **`SOLOMON_PARALLELIZE_INNER_LOOP(...)`** | `_Pragma("acc loop vector [...]")` <br> `_Pragma("omp parallel for [...]")` <br> disregarded (the outer loop is already parallelized) | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) | specify just before the inner loop of a loop nest <br> do not pass `SOLOMON_CLAUSE_THREAD`, `ACC_CLAUSE_VECTOR`, and similar clauses, which would duplicate the clauses already embedded in the macro |

     ```c++
     SOLOMON_OFFLOAD_OUTER_LOOP(SOLOMON_CLAUSE_NUM_BLOCKS(16384), SOLOMON_CLAUSE_NUM_THREADS(256))
     for(int i = 0; i < N_out; i++){
       // common computation
       SOLOMON_PARALLELIZE_INNER_LOOP()
       for(int j = 0; j < N_in; j++){
         // further computation
       }
     }
     ```

   * Macros for asynchronous kernel execution and synchronization are provided below. Please use them according to your needs
     * To improve compatibility between OpenACC and OpenMP target, it is recommended to use the intuitive notation provided by Solomon for asynchronous execution and synchronization
     * If you wish to perform asynchronous execution in a backend-independent manner, use `SOLOMON_CLAUSE_ASYNC(...)` and `SOLOMON_SYNCHRONIZE(...)`. Note that queue IDs may be ignored depending on the backend
     * If you wish to perform fine-grained asynchronous operations with specific queue IDs, use `SOLOMON_CLAUSE_ASYNC_QUEUE(id)` and `SOLOMON_WAIT_QUEUE(id)`. Note that asynchronous execution may not occur depending on the backend
     * To make a construct wait for specific queues before it starts, add `SOLOMON_CLAUSE_WAIT_QUEUE(...)` (paired with `SOLOMON_CLAUSE_ASYNC_QUEUE(id)`). To wait for all queues before the next launch, put `SOLOMON_SYNCHRONIZE()` before it

     | Available macros | output | offloading backend | note |
     | ---- | ---- | ---- | ---- |
     | **`SOLOMON_CLAUSE_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP | Enables asynchronous execution in both backends <br> Queue IDs can be specified in OpenACC <br> Queue IDs are ignored in OpenMP <br> With no argument, `async` without parentheses is generated in OpenACC (the same as `async(acc_async_noval)`) |
     | **`SOLOMON_SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `_Pragma("acc wait __VA_ARGS__")` <br> `_Pragma("omp taskwait __VA_ARGS__")` | <br> OpenACC <br> OpenMP | Performs synchronization for both backends. Should be used in correspondence with `SOLOMON_CLAUSE_ASYNC(...)` <br> OpenACC (`SOLOMON_SYNCHRONIZE`, `PRAGMA_ACC_WAIT`): `acc wait` without arguments and `acc wait (__VA_ARGS__)` with arguments; the arguments are queue IDs only (wait-argument), and clauses cannot be passed. <br> OpenMP target (`SOLOMON_SYNCHRONIZE`, `PRAGMA_ACC_WAIT`): the arguments are ignored whatever their number, and `omp taskwait` is emitted <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)`: in OpenMP target the arguments (clauses) are passed to `omp taskwait` as they are, and queue IDs such as `PRAGMA_OMP_TARGET_TASKWAIT(1, 2)` stop the preprocessing; in the OpenACC backend it becomes `acc wait` <br> Fallback mode (CPU execution without offloading): none of the three emits anything |
     | **`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` <br> N/A (disregarded in OpenMP backend) | <br> OpenACC <br> OpenMP | Performs asynchronous execution with a specified queue ID in OpenACC only <br> Queue ID specification is mandatory <br> Ignored in OpenMP because queue-specific asynchronous execution is not supported |
     | **`SOLOMON_CLAUSE_WAIT_QUEUE(...)`** <br> `ACC_CLAUSE_WAIT(...)` | <br> `wait(__VA_ARGS__)` <br> N/A (disregarded in OpenMP backend) | <br> OpenACC <br> OpenMP | The construct starts after the work on the specified queue IDs is complete, in OpenACC only (v2.0.0 or later) <br> Pass one queue ID or more (unlike `SOLOMON_WAIT_QUEUE(id)`, a list is accepted); an empty argument becomes `wait` (all queues) in OpenACC, but to wait for all queues putting `SOLOMON_SYNCHRONIZE()` before the construct is recommended <br> Ignored in OpenMP; combined with `SOLOMON_CLAUSE_ASYNC(...)` it does not wait in OpenMP target <br> Several `wait` clauses of one directive are merged (see `ACC_CLAUSE_WAIT(...)`) |
     | **`SOLOMON_WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `acc wait (id)` <br> N/A (disregarded in OpenMP backend) | <br> OpenACC <br> OpenMP | Performs synchronization with a specified queue ID in OpenACC only. Should be used in correspondence with `SOLOMON_CLAUSE_ASYNC_QUEUE(id)` <br> Queue ID specification is mandatory <br> Ignored in OpenMP because queue-specific synchronization is not supported |

### How to compile codes using Solomon

* Enable OpenACC or OpenMP target by compiler option
  * Check the options to enable OpenACC or OpenMP target features for the compiler you are using:
    * The option for GPU offloading using OpenACC with the NVIDIA HPC SDK is `-acc=gpu -gpu=[target GPU architecture]`
    * The option for GPU offloading using OpenMP target with the NVIDIA HPC SDK is `-mp=gpu -gpu=[target GPU architecture]`
    * The option for GPU offloading using OpenMP target with AMD ROCm is `-fopenmp --offload-arch=[target GPU architecture]`
    * The option for GPU offloading using OpenMP target with Intel oneAPI is `-fiopenmp -fopenmp-targets=spir64_gen -Xs "-device [target GPU architecture]"`

* Check the Solomon path (the directory containing `solomon.hpp`) and specify it during compilation using an option like `-I/path/to/solomon`
  * Both relative paths from the compilation directory (e.g., `../../../solomon`) and absolute paths (e.g., `/usr/local/solomon/include`) are accepted
  * For the diffusion equation sample code, the relative path to the Solomon header file `solomon/solomon.hpp` from the compilation directory (`samples/C/diffusion/`, where `samples/C/diffusion/Makefile` is located) is `../../../solomon`

* Add compilation flags to specify the expected behavior of Solomon

  | compilation flag | offloading backend | note |
  | ---- | ---- | ---- |
  | `-DSOLOMON_OFFLOAD_BY_OPENACC` | OpenACC | use `kernels` construct in default |
  | `-DSOLOMON_OFFLOAD_BY_OPENACC -DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` | OpenACC | use `parallel` construct in default |
  | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET` | OpenMP target | use `loop` directive in default when the compiler's `_OPENMP` is 201811 (OpenMP 5.0) or later; below that (e.g., GCC 13) `distribute` is used |
  | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | OpenMP target | use `distribute` directive in default |
  | | fallback mode | thread-parallelization for multicore CPUs using OpenMP |
  | | serial mode | when neither OpenACC nor OpenMP is enabled, all directives are removed and the code compiles as a serial program (v2.0.0 or later) |

  * `-DSOLOMON_OFFLOAD_BY_*` takes effect only together with the compiler's flag that enables it (`-acc` / `-fopenacc` for OpenACC, `-mp=gpu` / `-fopenmp` and the like for OpenMP target, which needs `_OPENMP` 201307 (OpenMP 4.0) or later); without it, the request is withdrawn without a warning and the code is built in fallback mode (OpenMP enabled) or serial mode
  * After `solomon.hpp`, `SOLOMON_OFFLOAD_BY_OPENACC` / `SOLOMON_OFFLOAD_BY_OPENMP_TARGET` remain defined only for the backend in use (e.g., `-DSOLOMON_OFFLOAD_BY_OPENACC` without the compiler's OpenACC flag leaves it undefined); the unprefixed `OFFLOAD_BY_OPENACC` / `OFFLOAD_BY_OPENMP_TARGET` are set the same way whichever spelling was given, so test them to find the backend in use. `SOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` / `SOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` take effect, and remain defined after `solomon.hpp`, only when the backend they belong to is in use; their unprefixed spellings `OFFLOAD_BY_OPENACC_PARALLEL` / `OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE`, whether given directly or defined by `solomon.hpp` from the prefixed ones, remain defined whatever the backend is (as in v1.x), so they do not tell the backend in use

* Adding the compile flag `-DPRINT_GENERATED_PRAGMA` outputs each generated directive as a compile-time message (`#pragma message`, placed just before the directive): GCC prints it as a note, and LLVM-based compilers (clang, icx / icpx) as a warning; the NVIDIA HPC SDK prints nothing (26.9)
  * With LLVM-based compilers, the message stays a warning under `-Werror`; to say so explicitly, pass `-Wno-error=#pragma-messages` (the option `-Wno-error=pragma-messages` does not exist and stops the build)
* See examples of the flags: [Makefile for nbody](samples/C/nbody/Makefile) and [Makefile for diffusion](samples/C/diffusion/Makefile)

### Backward compatibility with v1.x (v2.0.0 or later)

* In v2.0.0, all user-facing macros were renamed to avoid name collisions with user codes and other libraries:
  * directive macros now carry the `SOLOMON_` prefix (e.g., `OFFLOAD(...)` is now `SOLOMON_OFFLOAD(...)`)
  * clause tokens now carry the `SOLOMON_CLAUSE_` prefix (e.g., `AS_INDEPENDENT` is now `SOLOMON_CLAUSE_INDEPENDENT`, and `COLLAPSE(n)` is now `SOLOMON_CLAUSE_COLLAPSE(n)`)
* Codes written for v1.x keep working by defining `SOLOMON_WITH_SHORT_NAMES` (e.g., adding `-DSOLOMON_WITH_SHORT_NAMES` to the compilation flags), which enables the unprefixed v1.x spellings (default: OFF)
* The helpers to build `_Pragma`, `PRAGMA(...)`, `PRAGMA_STR(x)`, `PRAGMA_STR1(x)`, and `SHOW_PRAGMA(...)`, became internal in v2.0.0 (renamed with the `SOLOMON_INTERNAL_` prefix) and are not defined by default; defining `SOLOMON_WITH_SHORT_NAMES` enables the v1.x name `PRAGMA(...)`, and `PRAGMA_STR(x)`, `PRAGMA_STR1(x)`, and `SHOW_PRAGMA(...)` are defined only when `PRINT_GENERATED_PRAGMA` is also defined (as in v1.x). If your own directive macros are built on them, copy their definitions (`solomon/short_names.hpp`) into your code
* In serial execution (neither OpenACC nor OpenMP is enabled), `SOLOMON_APPEND_ARGS(...)` and its v1.x name `APPEND_ARGS(...)` expand to nothing, so their arguments are dropped (e.g., `f(0 SOLOMON_APPEND_ARGS(1, 2))` becomes `f(0)`)
* `SOLOMON_APPEND_ARGS()` with no argument expands to nothing, except with gcc / g++ in a strict mode before C23 / C++20 (`-std=c99`, `-std=c++17`, ...), where it keeps the comma (`f(0 SOLOMON_APPEND_ARGS())` becomes `f(0 ,)`): use a GNU mode or C23 / C++20 there
* With `-pedantic`, no Solomon macro warns in C23 / C++20 (`-std=c2x`, `-std=c++20`) or later, nor in the GNU modes of gcc / g++. Before C23 / C++20, each directive macro may warn that a variadic macro got no argument for `...`: clang-based compilers (clang, icx / icpx) in every mode (`-Wno-variadic-macro-arguments-omitted` silences it), and gcc / g++ in a strict mode (`-std=c99`, `-std=c++17`, ...; no option silences it: use a GNU mode or C23 / C++20 with `-pedantic -Werror`)
* `OMP_TARGET_CLAUSE_MAP(...)`: if the third or a later element of a non-nested list is the name of a function-like macro that cannot be invoked with one argument (e.g., `#define fm2(x, y) ...` and `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`), preprocessing now fails, and if it is the name of one whose expansion with one argument has a comma at the top level (e.g., `#define fc1(x) x, x`), the build stops with a Solomon error that names internal macros; place that element first or second, or write it as an array section (`fm2[0:n]`)
* The configuration macros have new spellings `-DSOLOMON_OFFLOAD_BY_*`; the unprefixed `-DOFFLOAD_BY_*` spellings keep working unconditionally (defining `SOLOMON_WITH_SHORT_NAMES` is not required for them)
* `AS_BLOCK` / `NUM_BLOCKS(n)` (`SOLOMON_CLAUSE_BLOCK` / `SOLOMON_CLAUSE_NUM_BLOCKS(n)`) changed their meaning in the OpenACC backend: they now emit `gang` / `num_gangs(n)` (v1.x: `worker` / `num_workers(n)`), so that a block is a gang in OpenACC and a team in OpenMP target. Codes keep compiling, but their parallelization in OpenACC changes; to keep the v1.x meaning in OpenACC, write `ACC_CLAUSE_WORKER` / `ACC_CLAUSE_NUM_WORKERS(n)` instead. Likewise, `NUM_GRIDS(n)` (`SOLOMON_CLAUSE_NUM_GRIDS(n)`, an alias of `SOLOMON_CLAUSE_NUM_BLOCKS(n)`) now emits `num_teams(n)` in the OpenMP target backend as well (v1.x: dropped), so `NUM_GRIDS(n)` together with `NUM_BLOCKS(m)` emits the same clause twice (an example is in [misc/migrate/MIGRATION.md](misc/migrate/MIGRATION.md)). The migration script prints a note for the files where it rewrites them
* The complete old-to-new correspondence table is available in [misc/migrate/MIGRATION.md](misc/migrate/MIGRATION.md)
* A migration script `misc/migrate/solomon_migrate_v1_to_v2.sh` is bundled: it rewrites v1.x sources (and build scripts passed explicitly as file arguments) to the v2.0.0 spellings; dry-run by default, `--apply` rewrites in place with backups (`*.v1.bak`). By default only the sources that include `solomon.hpp` are rewritten (the others, including your headers that use the v1.x names without including `solomon.hpp`, are skipped with a note; `--all` rewrites them as well). It needs bash and GNU sed, and was tested with bash 4 or later and the GNU tools

### How to extend capability of Solomon (Code Generators)

* Solomon accepts up to 32 clause candidates per directive (clauses that do not fit the directive and names that are not Solomon clause macros are counted; empty arguments are not counted). More candidates stop the build (v2.0.0 or later): GCC, Clang, and Intel compilers (C/C++) report `error: Solomon: too many clauses for one directive (limit 32)`; NVIDIA HPC SDK (C/C++) reports `invalid text in pragma` at the line of the macro; in Fortran, `SOLOMON_ERROR_TOO_MANY_CLAUSES_FOR_ONE_DIRECTIVE` is left in the directive and the Fortran compiler reports a syntax error there
  * Some clauses count as two candidates: `ACC_CLAUSE_SELF(...)` with an argument in the OpenMP target backend (in fallback mode, only on compute constructs; `ACC_CLAUSE_SELF()` without an argument counts as one), and, in the OpenACC backend, a nested clause of `OMP_TARGET_CLAUSE_MAP(...)` with `present` (after `OMP_TARGET_MAP_MODIFIER_PRESENT`, or a nested `OMP_TARGET_CLAUSE_TO_PRESENT(...)` / `OMP_TARGET_CLAUSE_FROM_PRESENT(...)`; written directly in an update directive they count as one); `SOLOMON_CLAUSE_IF(...)` (and the other `if` clauses) also counts as two in the OpenMP backends of C/C++ before OpenMP 6.0, and `SOLOMON_CLAUSE_NUM_THREADS(...)` (and `ACC_CLAUSE_VECTOR_LENGTH(...)`) in fallback mode
  * In the OpenACC backend, `SOLOMON_OFFLOAD_OUTER_LOOP(...)` and `SOLOMON_PARALLELIZE_INNER_LOOP(...)` accept 31 clauses, since `gang` / `vector` is added to `loop`; likewise `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` accepts 30 (`gang` and `independent` are added), and an exit data directive with `OMP_TARGET_CLAUSE_MAP_DELETE(...)` accepts 31 (`finalize` is added)
  * Empty arguments in a row followed by other arguments count as one candidate per 33 of them (33 to 65 in a row count as one, 66 to 98 as two, ...)
  * The arguments of the `wait` clauses of one directive merged into one are limited to 32 in total; more stop the build with `error: Solomon: too many arguments of the wait clauses merged into one (limit 32)` (Fortran: `SOLOMON_ERROR_TOO_MANY_MERGED_WAIT_ARGUMENTS`). The arguments written in one `wait` clause are not merged and have no such limit
  * If the current limitation (32) does not fit your implementation, regenerate the four generators below with the same value (`dedup_wait.jl` also raises the limit of the merged `wait` arguments, and `nest_map.jl` the number of nested clauses in one `OMP_TARGET_CLAUSE_MAP(...)`). The generators are Julia scripts that need the package `ArgParse` (`julia -e 'import Pkg; Pkg.add("ArgParse")'`)

    ```sh
    cd solomon/util # This directory contains jl/*.jl
    julia jl/check_clause.jl --max 64 # Example to increase the limit to 64
    julia jl/sort_clause.jl --max 64 --level 1
    julia jl/dedup_wait.jl --max 64
    julia jl/nest_map.jl --max 64 --mods 6
    ```

  * The generators write into `solomon/util/impl/` from whichever directory they are run. An invalid value (e.g., `--max 0`) stops a generator with exit code 1 without writing a header. When `sort_clause.jl`, `dedup_wait.jl`, or `nest_map.jl` was generated with a smaller value than `check_clause.jl`, the build stops with `#error "Solomon: regenerate check_clause.jl, sort_clause.jl, dedup_wait.jl, and nest_map.jl in solomon/util/jl with the same --max (README)"` (v2.0.0 or later)
  * Similar limitations exist for other internal macros, which can also be increased using the same procedure
  * Available code generators are all located in `solomon/util/jl/`:

  | Generator | Purpose | Default Maximum |
  | --------- | ------- | --------------- |
  | `check_clause.jl` | Generate clause checking macros | 32 |
  | `num_args.jl` | Generate input counting macros | 1024 |
  | `pickup_clause.jl` | Generate clause filtering macros | 99 |
  | `retrieve_args.jl` | Generate input filtering macros | 128 |
  | `sort_clause.jl` | Generate clause sorting macros | 32 |
  | `dedup_wait.jl` | Generate macros merging the `wait` clauses of one directive | 32 |
  | `is_empty.jl` | Generate comma detection macros for the emptiness test | 32 |
  | `nest_map.jl` | Generate macros for the nested clauses of `OMP_TARGET_CLAUSE_MAP(...)` | 32 |

### Profiler tags (NVTX, rocTX, ITT)

Solomon can emit vendor-neutral profiler tags (v2.0.0 or later). The backend (NVIDIA NVTX, AMD rocTX, or Intel ITT) is selected automatically from the compiler in use, so vendor names never appear in your code; to force one, define `SOLOMON_TAGGED_PROFILE_WITH_NVTX`, `SOLOMON_TAGGED_PROFILE_WITH_ROCTX`, or `SOLOMON_TAGGED_PROFILE_WITH_ITT`. Only the NVIDIA HPC SDK (and `nvcc`) and the Intel compilers are recognized automatically (NVTX and ITT). AMD compilers define no identification macro in the host-side pass, so rocTX cannot be auto-selected; always define `SOLOMON_TAGGED_PROFILE_WITH_ROCTX` explicitly there. GCC and Clang (including the other LLVM-based compilers) are not recognized either: define `..._WITH_*` with them.

* `-DSOLOMON_TAGGED_PROFILE` enables the manual tagging macros (write them without trailing semicolons, like the other Solomon macros):
  * `SOLOMON_PROFILE_RANGE_BEGIN("name")` / `SOLOMON_PROFILE_RANGE_END`: a named range
  * `SOLOMON_PROFILE_MARK("name")`: an instantaneous event
* `-DSOLOMON_TAGGED_PROFILE_AUTO` additionally tags the Solomon directive macros automatically with `"FILE:LINE MACRO"` strings:
  * data-movement and synchronization macros (`SOLOMON_MEMCPY_*`, `SOLOMON_MALLOC_ON_DEVICE`, `SOLOMON_FREE_FROM_DEVICE`, `SOLOMON_SYNCHRONIZE`, ...) get ranges covering the operation
  * kernel-launching loop macros (`SOLOMON_OFFLOAD`, `SOLOMON_OFFLOAD_OUTER_LOOP`) get instantaneous marks only, since a macro cannot see the end of the following loop; use the manual range macros (or the profiler's own kernel trace) to measure kernel execution time
  * `SOLOMON_OFFLOAD_SERIAL`/`SOLOMON_END_OFFLOAD_SERIAL` get a real range; do not leave the serial region with `continue` / `break` to an outer loop (`break` leaves the range open)
  * the automatic tags keep each Solomon directive macro a single statement, so that the macro can be the body of `if` / `else` / a loop without braces; this also holds for the stand-alone directives such as `SOLOMON_MEMCPY_*` and `SOLOMON_SYNCHRONIZE`, which cannot be such a body without the tags (a `#pragma` alone is not a statement; the loop macros can be one with or without the tags); then clang-based compilers (clang, icx / icpx) by default and gcc / g++ with `-Wall` warn `-Wdangling-else` around an `if` without braces, which stops a `-Werror` build: add `-Wno-dangling-else`, or put braces
  * with g++ / gcc 14 or older, the line numbers of the later automatic tags of a translation unit with many Solomon directives can show a line of the Solomon header instead (the macro names and the directives stay right); adding the GCC-only option `-ftrack-macro-expansion=0` to the C/C++ compile flags fixes it (clang and the other compilers reject the option, so add it for GCC only); for Fortran, `spp.sh` and `fortran.mk` add it automatically when `cpp` is GCC's
* linking: NVTX needs no extra flags with the NVIDIA HPC SDK offloading flags (`-acc=gpu`, `-mp=gpu`, `-cuda`); without them (e.g., `-mp` for the host), add `-cudalib=nvtx3`. When the header of the selected backend is not found, the compilation stops with an error that tells what to add; for rocTX, add `-I$ROCM_PATH/include` when compiling and `-L$ROCM_PATH/lib -lroctx64` when linking; for ITT, add `-I$VTUNE_PROFILER_DIR/sdk/include` when compiling and `-L$VTUNE_PROFILER_DIR/sdk/lib64 -littnotify` when linking (`VTUNE_PROFILER_DIR` is set by VTune's `env/vars.sh`)
* Fortran: additionally compile the bundled helper with the same compiler/flags and link its object, e.g., `nvc -c -mp=gpu -DSOLOMON_TAGGED_PROFILE /path/to/solomon/profile/solomon_profile.c` (without the define, the helper becomes no-ops); `FILE` in automatic tags is the path of the source as given to `spp.sh` or `make` (v2.0.0 or later; a tag line longer than 132 columns is continued with `&`)

### Editor support (syntax highlighting and clang-format)

Since Solomon provides plain preprocessor macros, editors do not highlight them by default. Configuration files that make the `SOLOMON_*` macros and the two-level notations (`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_*`) look like directives are bundled under `misc/editor/` (v2.0.0 or later):

* vim / neovim: copy (or symlink) `misc/editor/vim/after/syntax/{c,cpp,fortran}.vim` into `~/.vim/after/syntax/` (neovim: `~/.config/nvim/after/syntax/`)
* emacs: add `(load "/path/to/solomon/misc/editor/emacs/solomon-highlight.el")` to your `init.el`
* VS Code: copy (or symlink) the folder `misc/editor/vscode/solomon-highlight/` into `~/.vscode/extensions/` and restart VS Code
  * this is a build-free local injection grammar (two plain-text files); VS Code cannot apply regex-based highlighting through `settings.json` alone, so this folder is loaded as an unpacked local extension (nothing is published to the marketplace)
  * the names are highlighted in directives and code, not inside comments and strings
* clang-format: the repository root `.clang-format` registers the Solomon directive macros in `StatementMacros`/`AttributeMacros` so that formatting does not mangle directive lines; it is a reference setting (not enforced), so copy the macro lists into your own `.clang-format`

## Samples

### nbody: sample of compute-intensive application

* [Collisionless $N$-body simulation based on direct method](samples/C/nbody/)
  * Implementation using intuitive notation: `samples/C/nbody/src/[nbody gravity].cpp`
  * Implementation using OpenACC-like notation: `samples/C/nbody/src/[nbody gravity]_acc.cpp`
  * Implementation using OpenMP-like notation: `samples/C/nbody/src/[nbody gravity]_omp.cpp`

### diffusion: sample of memory-intensive application

* [Diffusion equation in 3D](samples/C/diffusion/)
  * Original implementation in OpenACC is available at [GitHub](https://github.com/hoshino-UTokyo/lecture_openacc) (developed by Tetsuya Hoshino at Nagoya University)
  * Some trivial optimizations and refactoring are added

### declare: sample of the declare directives

* [Functions called in offloaded regions and variables resident on the device](samples/C/declare/)
  * Uses `SOLOMON_DECLARE_OFFLOADED(...)` in the pair form (closed by `SOLOMON_DECLARE_OFFLOADED_END`) and in the named form with `SOLOMON_CLAUSE_TARGETS(...)`, and `SOLOMON_DECLARE_ON_DEVICE(...)`
  * Prints `1000000.0` and exits with 0 when the result is correct; `sbatch sh/whirlpool/verify.sh` in the sample directory builds and runs the four models with the NVIDIA HPC SDK
  * When offloading, it also checks that the host copy of the variable declared by `SOLOMON_DECLARE_ON_DEVICE(...)` is not overwritten by the device (define `SHARED_HOST_DEVICE_MEMORY` to skip this check with unified memory or host fallback)
  * The NVIDIA HPC SDK (26.9) makes functions defined in the same file callable on the device even without the declaration, so the sample detects a missing `SOLOMON_DECLARE_ON_DEVICE(...)` but not a missing function declaration with that compiler

### Running the samples

* Each sample (`samples/C/nbody/`, `samples/C/diffusion/`, `samples/C/diffusion/padding/`, `samples/C/declare/`, `samples/F/diffusion/`, `samples/F/declare/`) has its job scripts under `sh/`
  * `sh/common/`: the recipes shared by all systems (`verify.sh` checks the results of every model; `iterate.sh`, `survey.sh`, and `scaling.sh` measure the performance); the other scripts there (`guard.sh`, and `setup.sh` / `sweep.sh` in some samples) are parts that the recipes read, and no job script runs them
  * `sh/<system>/` (`local`, `whirlpool`, `miyabi-g`, `mercury`; each sample has the systems it was run on, and the declare samples have `whirlpool` only): `config.sh` holds the settings of the system (compiler, GPU vendor and architecture, module names), and each job script holds the scheduler directives and runs the recipe of the same name in `sh/common/`
* Run (or submit) the job scripts in the sample directory: e.g., `cd samples/C/diffusion && sbatch sh/whirlpool/verify.sh`, or `bash sh/local/verify.sh` without a job scheduler
  * `verify.sh` judges the results and the job fails when one is wrong (v2.0.0 or later): diffusion requires `Error < 2.0e-4` for every size (32, 37, 64, ...; 37 takes an odd number of time steps, so that a result left one time step behind fails), and nbody requires `0 < |E(t=10)/E(0) - 1| <= 1.0e-2`
  * The job scripts take no arguments (their settings are in `config.sh`). When `config.sh` is not in the current directory, the job scripts for Slurm and PBS move to `SLURM_SUBMIT_DIR` / `PBS_O_WORKDIR` first
  * With `DETECT_NUMA=1`, the job scripts set `CUDA_VISIBLE_DEVICES` (`ROCR_VISIBLE_DEVICES`) to `GPU_ID` of `config.sh`, but keep a value that is already set, e.g., by the job scheduler or in your shell (they print a line then, and `GPU_ID` is not used)
  * Each run of the diffusion samples is limited to `RUN_TIMEOUT` of `config.sh` (600s by default; a duration of `timeout`, e.g., 10m), and a longer run counts as a failure (`timed out after RUN_TIMEOUT=...`)
  * The job scripts need bash 4.1 or later, Environment Modules, `timeout` (GNU coreutils), and `numactl` (each recipe checks the commands it uses at the start and stops when one is not found); `flock` (util-linux) holds the lock below when it is found (without it they run without the lock, with a warning)
  * nbody: `verify.sh` draws the figures with gnuplot when it is found (a failed plot is reported as a warning, not as a failure)
  * Do not submit two job scripts (e.g., `verify.sh` and `iterate.sh`) in the same sample directory at the same time: the second one stops with `another job script is running` (it holds `sh/.lock`); use a copy of the sample directory instead (a copy outside `samples/C/` needs the path to Solomon: export `SOLOMON_DIR=/path/to/solomon` so that the job script sees it)
  * A job script stops when products of an earlier run are left in the sample directory; move them out and run again (they are not removed)
* The plotting scripts (`samples/C/nbody/jl/`, `samples/C/diffusion/jl/`) are meant for the author's measurements: they assume the directory layout and the system names of the author's data, and `jl/package.jl` updates the default Julia environment (it calls `Pkg.update()`); run them in a Julia environment of their own
* To add a system, copy `sh/local/` (or the directory of a system with the same job scheduler) to `sh/<your-system>/`, then change the values in `config.sh` and the scheduler directives
* On Miyabi-G, the group to be charged is written in the `#PBS -W group_list=` line of each job script in `sh/miyabi-g/` (scheduler directives cannot read shell variables) and in `PBS_GROUP` of `samples/C/nbody/sh/miyabi-g/config.sh` (the `module use` path refers to it); to change all of them at once, run the following in the top directory of Solomon after replacing `YOUR_GROUP` with your group: `sed -i -E 's/^(#PBS -W group_list=|PBS_GROUP=).*/\1YOUR_GROUP/' samples/*/*/sh/miyabi-g/*.sh`

### Building the samples by hand

* Run `make` in the sample directory. nbody builds `bin/nbody_pragma`, `bin/acc_pragma`, and `bin/omp_pragma` with nvc++ and creates `bin/obj/` and the output directories `dat/`, `fig/`, `log/` (no `make dir` is needed); diffusion (`samples/C/diffusion/`, `samples/C/diffusion/padding/`) and declare build `./run` with nvc. The default target GPU is `GPU_ARCH=90` (H100); pass another one on the command line (e.g., `make GPU_ARCH=120`)
* Select another compiler with `AMDCLANG=1` or `ICPX=1` (e.g., `make AMDCLANG=1 GPU_ARCH=gfx90a`, `make ICPX=1 GPU_ARCH=pvc`): `GPU_ARCH` has no default for AMD and Intel GPUs, at most one compiler can be selected, OpenACC (`USE_OPENACC=1`) needs the NVIDIA HPC SDK, and `ACPP=1` is not supported yet. With `NVHPC=0` and no other compiler, `$(CXX)` (nbody) or `$(CC)` (diffusion, declare) builds a host binary without offloading
* `SOLOMON_DIR` (default `../../../solomon`) is the directory `solomon/` of Solomon; give it on the command line or in the environment when the sample directory is elsewhere (e.g., `make SOLOMON_DIR=/path/to/solomon`); an exported `SOLOMON_DIR` is used as well. The path must not contain a space, a quote, `$`, the backquote, or any of `\ ( ) ; & | < >`, and `make` stops when `solomon.hpp` is not found there
* nbody includes a header of Boost (`src/gen_ic.cpp`): when the Boost headers are not in the search path of the compiler, give their directory in `BOOST_INC` (the Makefile passes `-isystem=$(BOOST_INC)`); the job scripts load the module `BOOST_MODULE` of `config.sh` for it
* Changing a make variable or a header (also of Solomon) rebuilds the objects and the executables; `make clean` is not needed. Only the values of the variables are compared (they are kept in `bin/obj/settings.txt` or `settings.txt`, removed by `make clean`): after switching to another compiler of the same name (another module or `PATH`), run `make clean`
* diffusion: run `./run N` (`./run N [padding]` in `padding/`), where `N` is the number of grid points per dimension (3 <= N and N^3 must fit in `int`, that is N <= 1290; with padding, N^2 (N + padding) must fit in `int`); a missing, extra, or invalid argument prints the usage and exits with 1. `OPT_LEVEL` takes 0 (no optimization flag), 1 (`-O1`), 2 (`-O2`), 3 (`-O3`), or 4 (default: `-Ofast` with nvc, `-O3 -ffast-math` otherwise); another value stops `make` with an error. `USE_MANAGED=1` / `USE_UNIFIED=1` pass `-gpu=mem:managed` / `-gpu=mem:unified` (`-gpu=managed` / `-gpu=unified` with an NVIDIA HPC SDK that does not know `mem:`)
* diffusion: each run appends one row to `diffusion_benchmark_<compiler><MODEL_ID>.csv` in the current directory, where `<compiler>` is the file name of `CC` (e.g., `nvc`); runs started at the same time in one directory add their rows without losing any. The base and the padding versions write the same file name with a different number of columns: do not run both in one directory. `Bandwidth` (`B/s` in the CSV) is the nominal traffic of 8 accesses of 4 bytes per cell over the elapsed time, without the reuse by caches; it can exceed the memory bandwidth of the device
* nbody: `BENCHMARK=1` measures the gravity calculation for `N = SET_NMIN, 2 SET_NMIN, ..., SET_NMAX`, repeating each `N` `SET_NCRIT / min(N, SET_NCRIT)` times (default `SET_NCRIT` 262144), and writes `log/<nbody|nbody_acc|nbody_omp>_benchmark_<compiler><MODEL_ID>.csv`, where `<compiler>` is the file name of `CXX` (e.g., `nvc++`). The snapshots `dat/collapse_snp<NNN>.dat` have 10 columns: `x y z vx vy vz ax ay az pot`
* declare: on the host (e.g., gcc `-fopenmp` without a GPU) or with unified memory, the check of the device-resident variable fails with a message that names these cases; define `SHARED_HOST_DEVICE_MEMORY` there (e.g., `make NVHPC=0 CC='gcc -std=c99 -fopenmp -DSHARED_HOST_DEVICE_MEMORY'`)

## Available APIs in Solomon

### Available directives

* <details><summary>Computation</summary>

  | input | output | backend |
  | ---- | ---- | ---- |
  | **`SOLOMON_OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | <br> `_Pragma("acc kernels __VA_ARGS__") _Pragma("acc loop __VA_ARGS__")` <br> `_Pragma("acc parallel __VA_ARGS__") _Pragma("acc loop __VA_ARGS__")` <br> `_Pragma("omp target teams loop __VA_ARGS__")` <br> `_Pragma("omp target teams distribute parallel for __VA_ARGS__")` | <br> OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP (loop: `_OPENMP` 201811 or later) <br> OpenMP (distribute) |
  | **`SOLOMON_OFFLOAD_OUTER_LOOP(...)`** | `_Pragma("acc parallel __VA_ARGS__") _Pragma("acc loop gang __VA_ARGS__")` <br> `_Pragma("omp target teams distribute __VA_ARGS__")` <br> `_Pragma("omp parallel for __VA_ARGS__")` | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) |
  | **`SOLOMON_PARALLELIZE_INNER_LOOP(...)`** | `_Pragma("acc loop vector __VA_ARGS__")` <br> `_Pragma("omp parallel for __VA_ARGS__")` <br> disregarded (the outer loop is already parallelized) | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) |
  | **`SOLOMON_OFFLOAD_SERIAL(...)`** <br> `PRAGMA_ACC_SERIAL(...)` <br> `PRAGMA_OMP_TARGET(...)` | <br> `_Pragma("acc serial __VA_ARGS__")` <br> `_Pragma("omp target __VA_ARGS__")` | <br> OpenACC <br> OpenMP <br> offload the immediately following structured block for single-thread execution on the device; useful to keep reduction results on the device (v2.0.0 or later) |
  | **`SOLOMON_END_OFFLOAD_SERIAL`** | (nothing in C/C++) | OpenACC/OpenMP <br> finalize the serial region (v2.0.0 or later) |
  | **`SOLOMON_END_OFFLOAD`** | (nothing in C/C++) | OpenACC/OpenMP <br> closes `SOLOMON_OFFLOAD(...)` in Fortran (see the Fortran README); C/C++ codes may keep it, e.g., in a source shared with Fortran (the C sample `samples/C/declare/` uses it) |
  | **`SOLOMON_END_OFFLOAD_OUTER_LOOP`** | (nothing in C/C++) | OpenACC/OpenMP <br> closes `SOLOMON_OFFLOAD_OUTER_LOOP(...)` in Fortran (see the Fortran README); C/C++ codes may keep it, e.g., in a source shared with Fortran |
  | **`SOLOMON_SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `_Pragma("acc wait __VA_ARGS__")` (queue IDs only) <br> `_Pragma("omp taskwait")` (`SOLOMON_SYNCHRONIZE`, `PRAGMA_ACC_WAIT`: the arguments are ignored) <br> `_Pragma("omp taskwait __VA_ARGS__")` (`PRAGMA_OMP_TARGET_TASKWAIT`: the clauses are passed) | <br> OpenACC <br> OpenMP <br> OpenMP |
  | **`SOLOMON_WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `_Pragma("acc wait (id)")` | <br> OpenACC (only) |
  | **`SOLOMON_DECLARE_OFFLOADED(...)`** <br> `PRAGMA_ACC_ROUTINE(...)` <br> `PRAGMA_OMP_DECLARE_TARGET(...)` | <br> `_Pragma("acc routine __VA_ARGS__")` <br> `_Pragma("omp declare target __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_DECLARE_OFFLOADED_END`** <br> `PRAGMA_OMP_END_DECLARE_TARGET` | <br> `_Pragma("omp end declare target")` | <br> OpenMP (only) <br> required only for the pair form of `SOLOMON_DECLARE_OFFLOADED(...)` (do not write it after the named form with `SOLOMON_CLAUSE_TARGETS(...)`); in Fortran it expands to nothing (v2.0.0 or later) |
  | **`SOLOMON_CLAUSE_TARGETS(...)`** | `(__VA_ARGS__)` | OpenACC/OpenMP <br> specify the target procedures by name: `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(func), ...)` expands to `_Pragma("acc routine (func) ...")` / `_Pragma("omp declare target (func)")`; self-contained, no end directive (v2.0.0 or later); `SOLOMON_CLAUSE_TARGETS(...)` may be written at any position among the arguments, and the name list always comes right after the directive name |
  | **`SOLOMON_DECLARE_ON_DEVICE(...)`** | `_Pragma("acc declare create(__VA_ARGS__)")` <br> `_Pragma("omp declare target (__VA_ARGS__)")` | OpenACC <br> OpenMP <br> declare device-resident variables at file scope; no end directive (v2.0.0 or later) |
  | **`SOLOMON_DECLARE_ON_DEVICE_LINKED(...)`** | `_Pragma("acc declare link(__VA_ARGS__)")` <br> `_Pragma("omp declare target link(__VA_ARGS__)")` | OpenACC <br> OpenMP <br> declare device-resident variables with link semantics (v2.0.0 or later); the NVIDIA HPC SDK (26.9) rejects `link` of `declare target` in OpenMP target (`not supported in NVIDIA subset`) |
  | **`SOLOMON_ATOMIC(...)`** <br> `PRAGMA_ACC_ATOMIC(...)` <br> `PRAGMA_OMP_TARGET_ATOMIC(...)` | <br> `_Pragma("acc atomic __VA_ARGS__")` <br> `_Pragma("omp atomic __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_UPDATE`** <br> `PRAGMA_ACC_ATOMIC_UPDATE` <br> `PRAGMA_OMP_TARGET_ATOMIC_UPDATE` | <br> `_Pragma("acc atomic update")` <br> `_Pragma("omp atomic update")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_READ`** <br> `PRAGMA_ACC_ATOMIC_READ` <br> `PRAGMA_OMP_TARGET_ATOMIC_READ` | <br> `_Pragma("acc atomic read")` <br> `_Pragma("omp atomic read")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_WRITE`** <br> `PRAGMA_ACC_ATOMIC_WRITE` <br> `PRAGMA_OMP_TARGET_ATOMIC_WRITE` | <br> `_Pragma("acc atomic write")` <br> `_Pragma("omp atomic write")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_CAPTURE`** <br> `PRAGMA_ACC_ATOMIC_CAPTURE` <br> `PRAGMA_OMP_TARGET_ATOMIC_CAPTURE` | <br> `_Pragma("acc atomic capture")` <br> `_Pragma("omp atomic capture")` | <br> OpenACC <br> OpenMP |

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
    | `PRAGMA_ACC_SERIAL(...)` | `_Pragma("acc serial __VA_ARGS__")` | `PRAGMA_OMP_TARGET(__VA_ARGS__)` (v2.0.0 or later) |
    | `PRAGMA_ACC_LOOP(...)` | `_Pragma("acc loop __VA_ARGS__")` | N/A (disregarded in OpenMP backend) |
    | `PRAGMA_ACC_CACHE(...)` | `_Pragma("acc cache(__VA_ARGS__)")` | N/A (disregarded in OpenMP backend) |
    | `PRAGMA_ACC_ATOMIC(...)` | `_Pragma("acc atomic __VA_ARGS__")` | `PRAGMA_OMP_TARGET_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_ACC_WAIT(...)` | `_Pragma("acc wait __VA_ARGS__")` | `PRAGMA_OMP_TARGET_TASKWAIT()` |
    | `PRAGMA_ACC_ROUTINE(...)` | `_Pragma("acc routine __VA_ARGS__")` | `PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)` |
    | `PRAGMA_ACC_DECLARE(...)` | `_Pragma("acc declare __VA_ARGS__")` | N/A (disregarded in OpenMP backend; use `SOLOMON_DECLARE_ON_DEVICE(...)` instead) |

    </details>

  * <details><summary>OpenMP target directives</summary>

    | input | output | counterpart in OpenACC backend | counterpart in fallback mode (CPU execution without offloading) |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET(...)` | `_Pragma("omp target __VA_ARGS__")` | `PRAGMA_ACC_SERIAL(__VA_ARGS__)` (v2.0.0 or later) | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_PARALLEL(...)` | `_Pragma("omp target parallel __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_FOR(...)` | `_Pragma("omp target parallel for __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp target parallel for simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_LOOP(...)` | `_Pragma("omp target parallel loop __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_SIMD(...)` | `_Pragma("omp target simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` (v2.0.0 or later) | `PRAGMA_OMP_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS(...)` | `_Pragma("omp target teams __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS(__VA_ARGS__)` (stays teams in fallback mode: NVIDIA HPC SDK (`-mp`) rejects it, and gcc / clang run the block in parallel only when the number of teams is given) |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` | `_Pragma("omp target teams distribute __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__) PRAGMA_ACC_LOOP(ACC_CLAUSE_GANG, ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` (as `SOLOMON_OFFLOAD_OUTER_LOOP(...)`; do not pass `ACC_CLAUSE_GANG`; v2.0.0 or later) | `PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` | `_Pragma("omp target teams distribute simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` (v2.0.0 or later) | `PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` | `_Pragma("omp target teams loop __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp target teams distribute parallel for __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp target teams distribute parallel for simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_ATOMIC(...)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` | `PRAGMA_ACC_ATOMIC(__VA_ARGS__)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` | `PRAGMA_ACC(wait)`: `acc wait` (the clauses are disregarded; v2.0.0 or later) | N/A (disregarded in fallback mode) |
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
    | `PRAGMA_OMP_SCOPE(...)` | `_Pragma("omp scope __VA_ARGS__")` <br> `firstprivate` and `allocate` on `scope` are OpenMP 5.2 features; Clang 22 and Intel 2026.1 reject them |
    | `PRAGMA_OMP_SECTIONS(...)` | `_Pragma("omp sections __VA_ARGS__")` |
    | `PRAGMA_OMP_SECTION` | `_Pragma("omp section")` |
    | `PRAGMA_OMP_FOR(...)` | `_Pragma("omp for __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE(...)` | `_Pragma("omp distribute __VA_ARGS__")` |
    | `PRAGMA_OMP_LOOP(...)` | `_Pragma("omp loop __VA_ARGS__")` |
    | `PRAGMA_OMP_TASK(...)` | `_Pragma("omp task __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKLOOP(...)` | `_Pragma("omp taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKYIELD` | `_Pragma("omp taskyield")` |
    | `PRAGMA_OMP_INTEROP(...)` | `_Pragma("omp interop __VA_ARGS__")` |
    | `PRAGMA_OMP_CRITICAL(...)` | `_Pragma("omp critical __VA_ARGS__")` <br> Pass the name with `OMP_PASS_LIST(name)`: `PRAGMA_OMP_CRITICAL(OMP_PASS_LIST(name), OMP_CLAUSE_HINT(hint))` → `critical (name) hint(hint)` (the name comes first wherever it is written) <br> `OMP_CLAUSE_HINT` other than `omp_sync_hint_none` needs a name (OpenMP 5.2, Section 15.2); without a name, GCC, Clang and Intel compilers reject it |
    | `PRAGMA_OMP_BARRIER` | `_Pragma("omp barrier")` |
    | `PRAGMA_OMP_TASKGROUP(...)` | `_Pragma("omp taskgroup __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKWAIT(...)` | `_Pragma("omp taskwait __VA_ARGS__")` |
    | `PRAGMA_OMP_FLUSH(...)` | `_Pragma("omp flush")` without arguments; with arguments the list follows in parentheses, `(__VA_ARGS__)` (`__VA_ARGS__` is a list of variables only; a memory-order clause cannot be passed: a Solomon clause macro (e.g. `OMP_CLAUSE_ACQ_REL`) as the first argument stops the build with `Solomon: PRAGMA_OMP_FLUSH takes a list of variables; a memory-order clause cannot be passed`; after the first argument its expansion stays in the list and the compiler rejects it. The second argument must not start with `::`, `/`, or `(`: that check pastes a token to it and the preprocessing stops; put such a name first, or write `PRAGMA_OMP(flush (...))`) |
    | `PRAGMA_OMP_DEPOBJ(...)` | `_Pragma("omp depobj __VA_ARGS__")` <br> pass the depend object first with `OMP_PASS_LIST(obj)`, then `OMP_CLAUSE_DEPEND(...)`, `OMP_CLAUSE_DESTROY(...)`, or `OMP_CLAUSE_DEPOBJ_UPDATE(...)`: `omp depobj (obj) depend(...)`; a bare name stops as an undefined name; `OMP_CLAUSE_UPDATE` is the clause of `atomic` and stops the build there (the compiler rejects `update` without an argument) |
    | `PRAGMA_OMP_ATOMIC(...)` | `_Pragma("omp atomic __VA_ARGS__")` |
    | `PRAGMA_OMP_ATOMIC_UPDATE` | `_Pragma("omp atomic update")` |
    | `PRAGMA_OMP_ATOMIC_READ` | `_Pragma("omp atomic read")` |
    | `PRAGMA_OMP_ATOMIC_WRITE` | `_Pragma("omp atomic write")` |
    | `PRAGMA_OMP_ATOMIC_CAPTURE` | `_Pragma("omp atomic capture")` |
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
  | **`SOLOMON_MALLOC_ON_DEVICE(...)`** <br> `PRAGMA_ACC_ENTER_DATA_CREATE(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_ALLOC(...)` | <br> `_Pragma("acc enter data create(__VA_ARGS__)")` <br> `_Pragma("omp target enter data map(alloc: __VA_ARGS__)")` | <br> OpenACC <br> OpenMP <br> `SOLOMON_ALLOCATE_ON_DEVICE(...)` (listed in the Fortran README) is the same macro |
  | **`SOLOMON_FREE_FROM_DEVICE(...)`** <br> `PRAGMA_ACC_EXIT_DATA_DELETE(...)` | <br> `_Pragma("acc exit data delete(__VA_ARGS__)")` <br> `_Pragma("omp target exit data map(release: __VA_ARGS__)")` | <br> OpenACC <br> OpenMP <br> decrements the reference count of the data on the device (the data stays while another `SOLOMON_MALLOC_ON_DEVICE(...)` holds it); `SOLOMON_DEALLOCATE_ON_DEVICE(...)` (listed in the Fortran README) is the same macro |
  | `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_DELETE(...)` | `_Pragma("acc exit data delete(__VA_ARGS__) finalize")` <br> `_Pragma("omp target exit data map(delete: __VA_ARGS__)")` | OpenACC <br> OpenMP <br> removes the data from the device regardless of the reference count |
  | **`SOLOMON_MEMCPY_D2H(...)`** <br> `PRAGMA_ACC_UPDATE_HOST(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_FROM(...)` | <br> `_Pragma("acc update host(__VA_ARGS__)")` <br> `_Pragma("omp target update from(__VA_ARGS__)")` <br> (clauses such as async or device_type cannot be passed: the arguments go inside the parentheses; give clauses to PRAGMA_ACC_UPDATE) | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_MEMCPY_H2D(...)`** <br> `PRAGMA_ACC_UPDATE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_TO(...)` | <br> `_Pragma("acc update device(__VA_ARGS__)")` <br> `_Pragma("omp target update to(__VA_ARGS__)")` <br> (clauses such as async or device_type cannot be passed: the arguments go inside the parentheses; give clauses to PRAGMA_ACC_UPDATE) | <br> OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `_Pragma("acc enter data __VA_ARGS__")` <br> `_Pragma("omp target enter data __VA_ARGS__")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA_COPYIN(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_TO(...)` | `_Pragma("acc enter data copyin(__VA_ARGS__)")` <br> `_Pragma("omp target enter data map(to: __VA_ARGS__)")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `_Pragma("acc exit data __VA_ARGS__")` <br> `_Pragma("omp target exit data __VA_ARGS__")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA_COPYOUT(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_FROM(...)` | `_Pragma("acc exit data copyout(__VA_ARGS__)")` <br> `_Pragma("omp target exit data map(from: __VA_ARGS__)")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_UPDATE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE(...)` | `_Pragma("acc update __VA_ARGS__")` <br> `_Pragma("omp target update __VA_ARGS__")` | OpenACC <br> OpenMP <br> For data updates, prefer `SOLOMON_MEMCPY_H2D(list)` / `SOLOMON_MEMCPY_D2H(list)`. If clauses are needed, use `PRAGMA_OMP_TARGET_UPDATE(OMP_TARGET_CLAUSE_TO(list))` / `PRAGMA_OMP_TARGET_UPDATE(OMP_TARGET_CLAUSE_FROM(list))`. Do not wrap them in `OMP_TARGET_CLAUSE_MAP(...)`: the update directive has no `map` clause, and the wrapped clause is dropped |
  | **`SOLOMON_DATA_ACCESS_BY_DEVICE(...)`** <br> `PRAGMA_ACC_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `_Pragma("acc data __VA_ARGS__")` <br> N/A when no clause becomes `map` or `use_device_*`; otherwise `_Pragma("omp target data __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_DATA_ACCESS_BY_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `_Pragma("acc host_data __VA_ARGS__")` <br> N/A when no clause becomes `map` or `use_device_*`; otherwise `_Pragma("omp target data __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_USE_DEVICE_DATA_FROM_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA_USE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_DATA_USE_DEVICE_ADDR(...)` | <br> `_Pragma("acc host_data use_device(__VA_ARGS__)")` <br> `_Pragma("omp target data use_device_addr(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `_Pragma("acc data present(__VA_ARGS__)")` | <br> OpenACC (only) |

  </details>

  * <details><summary>OpenACC directives</summary>

    | input | output | counterpart in OpenMP backend |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_DATA(...)` | `_Pragma("acc data __VA_ARGS__")` | N/A when no clause becomes `map` or `use_device_*`; otherwise `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_ENTER_DATA(...)` | `_Pragma("acc enter data __VA_ARGS__")` | N/A when no clause becomes `map`; otherwise `PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_EXIT_DATA(...)` | `_Pragma("acc exit data __VA_ARGS__")` | N/A when no clause becomes `map`; otherwise `PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_HOST_DATA(...)` | `_Pragma("acc host_data __VA_ARGS__")` | N/A when no clause becomes `map` or `use_device_*`; otherwise `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_UPDATE(...)` | `_Pragma("acc update __VA_ARGS__")` | N/A when no clause becomes `to` / `from`; otherwise `PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__)` |

    </details>

  * <details><summary>OpenMP target directives</summary>

    | input | output | counterpart in OpenACC backend | counterpart in fallback mode (CPU execution without offloading) |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET_DATA(...)` | `_Pragma("omp target data __VA_ARGS__")` | N/A (stops with a Solomon error) when no clause has an OpenACC counterpart; otherwise `PRAGMA_ACC_DATA(...)` or `PRAGMA_ACC_HOST_DATA(...)` (only `use_device_*`) | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `_Pragma("omp target enter data __VA_ARGS__")` | N/A (stops with a Solomon error) when no clause has an OpenACC counterpart; otherwise `PRAGMA_ACC_ENTER_DATA(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `_Pragma("omp target exit data __VA_ARGS__")` | N/A (stops with a Solomon error) when no clause has an OpenACC counterpart; otherwise `PRAGMA_ACC_EXIT_DATA(__VA_ARGS__)` | N/A (disregarded in fallback mode) |
    | `PRAGMA_OMP_TARGET_UPDATE(...)` | `_Pragma("omp target update __VA_ARGS__")` | N/A (stops with a Solomon error) when no clause has an OpenACC counterpart; otherwise `PRAGMA_ACC_UPDATE(__VA_ARGS__)` | N/A (disregarded in fallback mode) |

    </details>

### Available clauses

* <details><summary>Intuitive notation</summary>

  | input | output | backend | note |
  | ---- | ---- | ---- | --- |
  | **`SOLOMON_CLAUSE_INDEPENDENT`** <br> `ACC_CLAUSE_INDEPENDENT` <br> `OMP_TARGET_CLAUSE_SIMD` | <br> `independent` <br> `simd` | <br> OpenACC <br> OpenMP | `simd` is not added in the `teams loop` form, since `loop` takes no `simd`. With NVIDIA HPC SDK (nvc / nvc++), the directives with `target` in their names also get `simd`, as an extension of that compiler (e.g. `SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT)` gives `target teams loop simd`; also `target teams simd`, and, for the `target` ones of the `*_SIMD` directives, `target ... simd simd`), which nvc / nvc++ accept; they are not OpenMP forms |
  | **`SOLOMON_CLAUSE_SIMDLEN(length)`** <br> `OMP_CLAUSE_SIMDLEN(length)` | <br> `simdlen(length)` | <br> OpenMP (only) | emitted only where `SOLOMON_CLAUSE_INDEPENDENT` adds `simd` (e.g., `SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_SIMDLEN(8))` gives `target teams distribute parallel for simd simdlen(8)`, and `parallel for simd simdlen(8)` in fallback mode), and ignored where no `simd` is added: in OpenACC, in the `teams loop` form, and in serial mode. The loop directives whose names have `simd` (e.g., `PRAGMA_OMP_PARALLEL_FOR_SIMD(...)`) also take it, but `PRAGMA_OMP_DECLARE_SIMD(...)` drops it silently (write the OpenMP notation there, e.g., `OMP_CLAUSE_SIMDLEN(length)`). It is not `vector_length` of OpenACC: give the number of threads per thread-block with `SOLOMON_CLAUSE_NUM_THREADS(n)`. Pass `SOLOMON_CLAUSE_INDEPENDENT` as well: without it, the clause stays on a directive without `simd`, which compilers reject |
  | **`SOLOMON_CLAUSE_SAFELEN(length)`** <br> `OMP_CLAUSE_SAFELEN(length)` | <br> `safelen(length)` | <br> OpenMP (only) | as `SOLOMON_CLAUSE_SIMDLEN(length)`. The iterations are still distributed over threads or teams before `simd`, so `safelen` does not make a loop with a dependence between iterations safe here; write such an inner loop with `PRAGMA_OMP_SIMD(OMP_CLAUSE_SAFELEN(n))` |
  | **`SOLOMON_CLAUSE_ALIGNED(...)`** <br> `OMP_CLAUSE_ALIGNED(...)` | <br> `aligned(__VA_ARGS__)` | <br> OpenMP (only) | as `SOLOMON_CLAUSE_SIMDLEN(length)`; with NVIDIA HPC SDK (nvc / nvc++), dropped on purpose in the OpenMP target backend, since the compiler rejects `aligned` on the `target` directives (a hint: the results do not change) |
  | **`SOLOMON_CLAUSE_NONTEMPORAL(...)`** <br> `OMP_CLAUSE_NONTEMPORAL(...)` | <br> `nontemporal(__VA_ARGS__)` | <br> OpenMP (only) | as `SOLOMON_CLAUSE_SIMDLEN(length)`; dropped on purpose with NVIDIA HPC SDK (nvc / nvc++), which rejects `nontemporal` (a hint: the results do not change) |
  | **`SOLOMON_CLAUSE_IF_SIMD(condition)`** <br> `OMP_CLAUSE_IF(simd: condition)` | <br> `if(simd: condition)` | <br> OpenMP (only) | as `SOLOMON_CLAUSE_SIMDLEN(length)`; when the condition is false, the loop runs without SIMD (one iteration at a time). Dropped on purpose with NVIDIA HPC SDK (nvc / nvc++), which rejects `if` with the `simd` modifier (the loop may then use SIMD also when the condition is false; the results do not change) |
  | **`SOLOMON_CLAUSE_SEQUENTIAL`** <br> `ACC_CLAUSE_SEQ` | <br> `seq` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_NUM_THREADS(n)`** <br> `ACC_CLAUSE_VECTOR_LENGTH(n)` <br> `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | <br> `vector_length(n)` <br> `thread_limit(n)` | <br> OpenACC <br> OpenMP | `SOLOMON_CLAUSE_NUM_THREADS(n)` and `ACC_CLAUSE_VECTOR_LENGTH(n)` become `num_threads(n)` in fallback mode, i.e., the number of CPU threads (a thread-block size for GPUs may exceed the number of cores), and `thread_limit(n)` for `PRAGMA_OMP_TARGET_TEAMS(...)`; `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` stays `thread_limit(n)`, which only `PRAGMA_OMP_TARGET_TEAMS(...)` takes there; do not give `OMP_TARGET_CLAUSE_NUM_THREADS(...)` to the same directive |
  | **`SOLOMON_CLAUSE_NUM_BLOCKS(n)`** <br> `ACC_CLAUSE_NUM_GANGS(n)` <br> `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | <br> `num_gangs(n)` <br> `num_teams(n)` | <br> OpenACC <br> OpenMP | changed from `ACC_CLAUSE_NUM_WORKERS(n)` to `ACC_CLAUSE_NUM_GANGS(n)` in v2.0.0 |
  | ~~`SOLOMON_CLAUSE_NUM_GRIDS(n)`~~ <br> ~~`ACC_CLAUSE_NUM_GANGS(n)`~~ <br> ~~`OMP_TARGET_CLAUSE_NUM_TEAMS(n)`~~ | <br> ~~`num_gangs(n)`~~ <br> ~~`num_teams(n)`~~ | <br> OpenACC <br> OpenMP | deprecated in v2.0.0; use `SOLOMON_CLAUSE_NUM_BLOCKS(n)` instead |
  | **`SOLOMON_CLAUSE_THREAD`** <br> `ACC_CLAUSE_VECTOR` | <br> `vector` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_BLOCK`** <br> `ACC_CLAUSE_GANG` | <br> `gang` | <br> OpenACC (only) | changed from `ACC_CLAUSE_WORKER` to `ACC_CLAUSE_GANG` in v2.0.0 |
  | ~~`SOLOMON_CLAUSE_GRID`~~ <br> ~~`ACC_CLAUSE_GANG`~~ | <br> ~~`gang`~~ | <br> OpenACC (only) | deprecated in v2.0.0; use `SOLOMON_CLAUSE_BLOCK` instead |
  | **`SOLOMON_CLAUSE_COLLAPSE(n)`** <br> `ACC_CLAUSE_COLLAPSE(n)` <br> `OMP_TARGET_CLAUSE_COLLAPSE(n)` | <br> `collapse(n)` <br> `collapse(n)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async` without arguments; with an argument it follows in parentheses, `(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP | With no argument, `async` without parentheses is generated in OpenACC (the same as `async(acc_async_noval)`) |
  | **`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_WAIT_QUEUE(...)`** <br> `ACC_CLAUSE_WAIT(...)` | <br> `wait` without arguments; with arguments the list follows in parentheses, `(__VA_ARGS__)` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_REDUCTION(...)`** <br> `ACC_CLAUSE_REDUCTION(...)` <br> `OMP_TARGET_CLAUSE_REDUCTION(...)` | <br> `reduction(__VA_ARGS__)` <br> `reduction(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_IF(condition)`** <br> `ACC_CLAUSE_IF(condition)` <br> `OMP_TARGET_CLAUSE_IF(condition)` | <br> `if(condition)` <br> `if(condition)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_PRIVATE(...)`** <br> `ACC_CLAUSE_PRIVATE(...)` <br> `OMP_TARGET_CLAUSE_PRIVATE(...)` | <br> `private(__VA_ARGS__)` <br> `private(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_FIRSTPRIVATE(...)`** <br> `ACC_CLAUSE_FIRSTPRIVATE(...)` <br> `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | <br> `firstprivate(__VA_ARGS__)` <br> `firstprivate(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_DEVICE_PTR(...)`** <br> `ACC_CLAUSE_DEVICEPTR(...)` <br> `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | <br> `deviceptr(__VA_ARGS__)` <br> `is_device_ptr(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPY(...)` <br> `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | <br> `copy(__VA_ARGS__)` <br> `map(tofrom: __VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)`** <br> `ACC_CLAUSE_COPYIN(...)` <br> `OMP_TARGET_CLAUSE_MAP_TO(...)` | <br> `copyin(__VA_ARGS__)` <br> `map(to: __VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPYOUT(...)` <br> `OMP_TARGET_CLAUSE_MAP_FROM(...)` | <br> `copyout(__VA_ARGS__)` <br> `map(from: __VA_ARGS__)` | <br> OpenACC <br> OpenMP | |

  </details>

* <details><summary>OpenACC clauses</summary>

  | input | output | counterpart in OpenMP backend | note |
  | ---- | ---- | ---- | ---- |
  | `ACC_CLAUSE_IF(condition)` | `if(condition)` | `OMP_TARGET_CLAUSE_IF(condition)` | |
  | `ACC_CLAUSE_SELF(...)` | `self` without arguments; with an argument it follows in parentheses, `(__VA_ARGS__)` | `if (target : 0)` without arguments and `if(target: !(condition))` with an argument on compute constructs (an `if` clause on the same directive is not allowed in the OpenMP target backend); `OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)` on `PRAGMA_ACC_UPDATE`, where `ACC_CLAUSE_SELF()` without arguments is dropped | With no argument, `self` without parentheses is generated in OpenACC. NVIDIA HPC SDK 26.9 and GCC 13 do not accept `self` on compute constructs (OpenACC 2.7) <br> `ACC_CLAUSE_SELF()` without arguments is not a required clause of `PRAGMA_ACC_UPDATE(...)`: without another clause, the update stops with the Solomon error of the required clauses in OpenACC, and is not generated in OpenMP target (v2.0.0 or later) <br> With two or more arguments it is the list of `PRAGMA_ACC_UPDATE(...)`; given to a compute construct, it stops the build with a Solomon error in the OpenMP target backend (`self` of a compute construct takes one condition) |
  | `ACC_CLAUSE_DEFAULT(mode)` | `default(mode)` | N/A (disregarded in OpenMP backend) | On the data construct (`PRAGMA_ACC_DATA(...)`), GCC 13 rejects `default`, and NVIDIA HPC SDK 26.9 rejects it without another data clause |
  | `ACC_CLAUSE_DEFAULT_NONE` | `default(none)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | On the data construct, GCC 13 rejects `default`, and NVIDIA HPC SDK 26.9 rejects it without another data clause |
  | `ACC_CLAUSE_DEFAULT_PRESENT` | `default(present)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | On the data construct, GCC 13 rejects `default`, and NVIDIA HPC SDK 26.9 rejects it without another data clause <br> In the OpenMP target backend, GCC 13 and nvc++ 26.9 (`-mp=gpu`, `not supported yet`) reject `defaultmap(present)` (Clang 23 and Intel 2026.1 accept it); with nvc++, write `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_MAP_MODIFIER_PRESENT, ...)` instead (GCC 13 rejects that too) |
  | `ACC_CLAUSE_DEVICE_TYPE(...)` | `device_type(__VA_ARGS__)` | N/A (disregarded in OpenMP backends) | Also given to the data construct and `update` (NVIDIA HPC SDK 26.9 Fortran rejects it there) <br> In the macros that give one clause list to two directives (`SOLOMON_OFFLOAD`, `SOLOMON_OFFLOAD_OUTER_LOOP`, `PRAGMA_ACC_KERNELS_LOOP` / `PRAGMA_ACC_PARALLEL_LOOP`, and the compute directives of `PRAGMA_OMP_TARGET_*` in the OpenACC backend), the list goes to both the compute construct and the `loop` construct, so the clauses after `device_type` are device-specific in both (as in `kernels loop`) <br> After `device_type`, write only clauses that may follow `device_type` in the directive that receives them (OpenACC 3.3): compute constructs take `async`, `wait`, `num_gangs`, `num_workers`, `vector_length`; `loop` takes `collapse`, `gang`, `worker`, `vector`, `seq`, `independent`, `auto`, `tile`; the data construct and `update` take `async`, `wait`; `routine` takes `gang`, `worker`, `vector`, `seq`, `bind`. Write the other clauses (`private`, `reduction`, `copyin`, ...) before any `device_type`. The two-directive macros give each clause only to the directive that takes it, so clauses of both lists may follow `device_type`: `SOLOMON_OFFLOAD(SOLOMON_CLAUSE_COLLAPSE(2), ACC_CLAUSE_DEVICE_TYPE(nvidia), ACC_CLAUSE_NUM_GANGS(4))` gives `kernels device_type(nvidia) num_gangs(4)` and `loop collapse(2) device_type(nvidia)` <br> In the OpenMP backends (OpenMP target and fallback mode), `device_type` is dropped and the clauses after it apply to all devices (also in `PRAGMA_ACC_ROUTINE(...)` / `SOLOMON_DECLARE_OFFLOADED(...)`) |
  | `ACC_CLAUSE_ASYNC(...)` | `async` without arguments; with an argument it follows in parentheses, `(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_NOWAIT` | Also given to the data construct (`PRAGMA_ACC_DATA`, `SOLOMON_DATA_ACCESS_BY_DEVICE`; OpenACC 3.2 or later, GCC 13 rejects it); `target data` of OpenMP target gets nothing (it has no `nowait` / `depend`), so put `SOLOMON_SYNCHRONIZE()` before the data construct when a wait is needed <br> `async` on the data construct makes its own transfers (including the copyout at its exit) asynchronous: wait before the host reads the results after the data construct <br> With no argument, `async` without parentheses is generated in OpenACC (the same as `async(acc_async_noval)`) <br> With gcc / g++ in a strict mode before C23 / C++20, see the note on the emptiness test in [How to develop codes using Solomon](#how-to-develop-codes-using-solomon) |
  | `ACC_CLAUSE_WAIT(...)` | `wait` without arguments; with arguments the list follows in parentheses, `(__VA_ARGS__)` | N/A without arguments (the clause is dropped); `OMP_TARGET_CLAUSE_DEPEND_IN(__VA_ARGS__)` with arguments | Without arguments, OpenACC gets `wait` (waits for all async queues); OpenMP target ignores it, so put `SOLOMON_SYNCHRONIZE()` before the construct when a wait is needed <br> In OpenMP target the arguments become the list of `depend(in : ...)`, so pass variables; queue IDs (integers) work only in OpenACC. For backend-independent asynchronous execution, `SOLOMON_CLAUSE_ASYNC(...)` and `SOLOMON_SYNCHRONIZE(...)` are recommended; for a wait for queues independent of the backend, use `SOLOMON_CLAUSE_WAIT_QUEUE(...)` <br> The `wait` clauses of one directive are merged per `device_type` segment: with a `wait` without arguments, the waits for the current device become that one `wait`; otherwise the queue lists become one `wait(..., ...)`; waits with `devnum:` / `queues:` are kept. An empty `wait()` is the same as `wait`. When writing two or more `wait` clauses, start each argument with a name, an integer or a parenthesis (e.g. `(*p)`). The merged arguments are limited to 32; more stop the build with `error: Solomon: too many arguments of the wait clauses merged into one (limit 32)` (see [How to extend capability of Solomon](#how-to-extend-capability-of-solomon-code-generators)); the arguments written in one `wait` clause are not merged and have no such limit <br> An empty element of the arguments (a stray comma, as in `ACC_CLAUSE_WAIT(1, )` or `ACC_CLAUSE_WAIT(, 2)`) stops the build with `Solomon: ACC_CLAUSE_WAIT(...) has an empty argument (...); remove the extra comma, and write ACC_CLAUSE_WAIT() to wait without a list` (Fortran: `SOLOMON_ERROR_EMPTY_ARGUMENT_IN_ACC_CLAUSE_WAIT`) in the OpenACC and OpenMP target backends (v2.0.0 or later; up to 33 arguments are checked) <br> Also given to the data construct (`PRAGMA_ACC_DATA`, `SOLOMON_DATA_ACCESS_BY_DEVICE`; OpenACC 3.2 or later, GCC 13 rejects it); `target data` of OpenMP target gets nothing (it has no `nowait` / `depend`), so put `SOLOMON_SYNCHRONIZE()` before the data construct when a wait is needed |
  | `ACC_CLAUSE_FINALIZE` | `finalize` | N/A (disregarded in OpenMP backend) | in OpenMP backends, `ACC_CLAUSE_DELETE(...)` with `ACC_CLAUSE_FINALIZE` also only decrements the reference count |
  | `ACC_CLAUSE_NUM_GANGS(n)` | `num_gangs(n)` | `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | converted to `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` since v2.0.0 |
  | `ACC_CLAUSE_NUM_WORKERS(n)` | `num_workers(n)` | N/A (disregarded in OpenMP backend) | disregarded in the OpenMP target backend since v2.0.0 |
  | `ACC_CLAUSE_VECTOR_LENGTH(n)` | `vector_length(n)` | `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | |
  | `ACC_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_REDUCTION(__VA_ARGS__)` | |
  | `ACC_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_PRIVATE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPY(...)` | `copy(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)` | the `readonly:` (copyin) and `zero:` (copyout, create) modifiers are for OpenACC only; in the OpenMP target backend they remain in the `map` clause and the compilers reject it |
  | `ACC_CLAUSE_COPYOUT(...)` | `copyout(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)` | the `zero:` modifier is for OpenACC only (see `ACC_CLAUSE_COPYIN(...)`) |
  | `ACC_CLAUSE_CREATE(...)` | `create(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | the `zero:` modifier is for OpenACC only (see `ACC_CLAUSE_COPYIN(...)`) |
  | `ACC_CLAUSE_NO_CREATE(...)` | `no_create(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_DELETE(...)` | `delete(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | decrements the reference count as OpenACC `delete` does (v2.0.0 or later); `ACC_CLAUSE_FINALIZE` is disregarded in OpenMP backends |
  | `ACC_CLAUSE_PRESENT(...)` | `present(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_DEVICEPTR(...)` | `deviceptr(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(__VA_ARGS__)` | |
  | `ACC_CLAUSE_ATTACH(...)` | `attach(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_DETACH(...)` | `detach(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_USE_DEVICE(...)` | `use_device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(__VA_ARGS__)` | |
  | `ACC_CLAUSE_IF_PRESENT` | `if_present` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_COLLAPSE(n)` | `collapse(n)` | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | |
  | `ACC_CLAUSE_SEQ` | `seq` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_AUTO` | `auto` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_INDEPENDENT` | `independent` | `OMP_TARGET_CLAUSE_SIMD` | |
  | `ACC_CLAUSE_TILE(...)` | `tile(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_GANG` | `gang` | N/A (disregarded in OpenMP backend) | notation as `gang(n)` is N/A (`ACC_CLAUSE_NUM_GANGS(n)` is available) |
  | `ACC_CLAUSE_WORKER` | `worker` | N/A (disregarded in OpenMP backend) | notation as `worker(n)` is N/A (`ACC_CLAUSE_NUM_WORKERS(n)` is available) |
  | `ACC_CLAUSE_VECTOR` | `vector` | N/A (disregarded in OpenMP backend) | notation as `vector(n)` is N/A (`ACC_CLAUSE_VECTOR_LENGTH(n)` is available) |
  | `ACC_CLAUSE_READ` | `read` | `OMP_TARGET_CLAUSE_READ` | |
  | `ACC_CLAUSE_WRITE` | `write` | `OMP_TARGET_CLAUSE_WRITE` | |
  | `ACC_CLAUSE_UPDATE` | `update` | `OMP_TARGET_CLAUSE_UPDATE` | |
  | `ACC_CLAUSE_CAPTURE` | `capture` | `OMP_TARGET_CLAUSE_CAPTURE` | |
  | `ACC_CLAUSE_HOST(...)` | `host(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)` | |
  | `ACC_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_TO(__VA_ARGS__)` | |
  | `ACC_PASS_LIST(...)` | `(__VA_ARGS__)` | `OMP_TARGET_PASS_LIST(__VA_ARGS__)` | |
  | `ACC_CLAUSE_BIND(arg)` | `bind(arg)` | N/A (disregarded in OpenMP backend) | GCC 13 rejects `bind` on `routine` (NVIDIA HPC SDK 26.9 and Clang 23 accept it) |
  | `ACC_CLAUSE_NOHOST` | `nohost` | N/A (disregarded in OpenMP backends) | the procedure is also compiled for the host in OpenMP target (v2.0.0 or later) |
  | `ACC_CLAUSE_DEVICE_RESIDENT(...)` | `device_resident(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |
  | `ACC_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A (disregarded in OpenMP backend) | |

  </details>

* <details><summary>OpenMP target clauses</summary>

  | input | output | counterpart in OpenACC backend | counterpart in fallback mode (CPU execution without offloading) | note |
  | ---- | ---- | ---- | ---- | ---- |
  | `OMP_TARGET_CLAUSE_ALIGNED(...)` | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SIMDLEN(length)` | `OMP_CLAUSE_SIMDLEN(length)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SIMDLEN(length)` | |
  | `OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` | `device_type(nohost)` | `ACC_CLAUSE_NOHOST` for `nohost`; disregarded for `host` and `any` (v2.0.0 or later) | N/A (disregarded in fallback mode) | the argument is `host`, `nohost`, or `any`; another word stops the build in the OpenACC backend with `Solomon: the argument of OMP_TARGET_CLAUSE_DEVICE_TYPE must be host / nohost / any: <word>` (Fortran: `SOLOMON_ERROR_UNKNOWN_DEVICE_TYPE <word>` in the directive) |
  | `OMP_TARGET_CLAUSE_ENTER(...)` | `enter(__VA_ARGS__)` | N/A (dropped in OpenACC backend; the directive becomes a bare `acc routine`, which binds to the next function or fails to compile: use `SOLOMON_DECLARE_ON_DEVICE(...)` / `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` for variables and `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(...))` for functions) | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_INDIRECT(...)` | `indirect` without arguments; with arguments it follows in parentheses, `(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) | GCC 13 and NVIDIA HPC SDK 26.9 do not accept `indirect` |
  | `OMP_TARGET_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A (dropped in OpenACC backend; the directive becomes a bare `acc routine`, which binds to the next function or fails to compile: use `SOLOMON_DECLARE_ON_DEVICE(...)` / `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` for variables and `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(...))` for functions) | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_COPYIN(...)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | N/A (dropped in OpenACC backend: OpenACC has no `threadprivate`, and its `copyin` is a transfer to the device) | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | dropped by the `PRAGMA_OMP_TARGET_*` combined directives (OpenMP 5.2, section 17.4); kept in fallback mode. For a transfer from the host to the device, write `SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)` (or `ACC_CLAUSE_COPYIN(...)` / `OMP_TARGET_CLAUSE_MAP_TO(...)`) |
  | `OMP_TARGET_CLAUSE_NUM_THREADS(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `ACC_CLAUSE_VECTOR_LENGTH(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | |
  | `OMP_TARGET_CLAUSE_PROC_BIND(attr)` | `OMP_CLAUSE_PROC_BIND(attr)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_PROC_BIND(attr)` | |
  | `OMP_TARGET_CLAUSE_NUM_TEAMS(...)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | `ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | the conversion target was changed from `ACC_CLAUSE_NUM_WORKERS(__VA_ARGS__)` to `ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)` in v2.0.0; in the OpenACC backend, give only the upper bound (`num_gangs` has no lower bound) |
  | `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | `ACC_CLAUSE_VECTOR_LENGTH(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | in fallback mode, only `PRAGMA_OMP_TARGET_TEAMS(...)` takes it (the other directives become `parallel` forms and drop it); `SOLOMON_CLAUSE_NUM_THREADS(n)` and `ACC_CLAUSE_VECTOR_LENGTH(n)` become `num_threads(n)` for the `parallel` forms and `thread_limit(n)` for `PRAGMA_OMP_TARGET_TEAMS(...)` in fallback mode <br> In the OpenACC backend, the `PRAGMA_OMP_TARGET_*` directives drop it when `OMP_TARGET_CLAUSE_NUM_THREADS(...)` is given to the same directive (both become `vector_length`); `SOLOMON_OFFLOAD(...)`, `PRAGMA_ACC_KERNELS_LOOP(...)`, and `PRAGMA_ACC_PARALLEL_LOOP(...)` keep both and emit `vector_length` twice, so give only one of them there |
  | `OMP_TARGET_CLAUSE_NONTEMPORAL(...)` | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SAFELEN(length)` | `OMP_CLAUSE_SAFELEN(length)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SAFELEN(length)` | |
  | `OMP_TARGET_CLAUSE_ORDERED(...)` | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | give it without an argument to a loop that holds `PRAGMA_OMP_ORDERED()` blocks (clang 22 and Intel oneAPI 2026 reject `ordered(1)` there) |
  | `OMP_TARGET_CLAUSE_SCHEDULE(...)` | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DIST_SCHEDULE(...)` | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_BIND(binding)` | `OMP_CLAUSE_BIND(binding)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_BIND(binding)` | |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...)` | `use_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...)` | `use_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP(...)` | `defaultmap(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | `OMP_TARGET_CLAUSE_DEFAULTMAP(none)` | `ACC_CLAUSE_DEFAULT_NONE` | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | `OMP_TARGET_CLAUSE_DEFAULTMAP(present)` | `ACC_CLAUSE_DEFAULT_PRESENT` | N/A (disregarded in fallback mode) | GCC 13 and nvc++ 26.9 (`-mp=gpu`, `not supported yet`) reject `defaultmap(present)` (Clang 23 and Intel 2026.1 accept it); with nvc++, write `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_MAP_MODIFIER_PRESENT, ...)` instead (GCC 13 rejects that too) |
  | `OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...)` | `has_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | `is_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_USES_ALLOCATORS(...)` | `uses_allocators(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | N/A (disregarded in fallback mode) | |
  | `OMP_TARGET_CLAUSE_FROM(...)` | `from(__VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A (disregarded in fallback mode) | write the `present` modifier with `OMP_TARGET_CLAUSE_FROM_PRESENT(...)`; raw modifiers are not supported |
  | `OMP_TARGET_CLAUSE_TO(...)` | `to(__VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) | write the `present` modifier with `OMP_TARGET_CLAUSE_TO_PRESENT(...)`; raw modifiers are not supported |
  | `OMP_TARGET_CLAUSE_FROM_PRESENT(...)` | `from(present : __VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A (disregarded in fallback mode) | for `update`; nested in `OMP_TARGET_CLAUSE_MAP(...)`, it becomes `map(present, from : ...)` (OpenACC: `present(...)`); NVIDIA HPC SDK 26.9 rejects `present` in `to` / `from` of `update` in OpenMP target; GCC 13 rejects `map(present, ...)` (NVIDIA HPC SDK 26.9, Clang 23, and Intel 2026.1 accept it) |
  | `OMP_TARGET_CLAUSE_TO_PRESENT(...)` | `to(present : __VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A (disregarded in fallback mode) | for `update`; nested in `OMP_TARGET_CLAUSE_MAP(...)`, it becomes `map(present, to : ...)` (OpenACC: `present(...)`); NVIDIA HPC SDK 26.9 rejects `present` in `to` / `from` of `update` in OpenMP target; GCC 13 rejects `map(present, ...)` (NVIDIA HPC SDK 26.9, Clang 23, and Intel 2026.1 accept it) |
  | `OMP_TARGET_MAP_MODIFIER_ALWAYS` | map modifier always, only inside OMP_TARGET_CLAUSE_MAP(...) (see the note of OMP_TARGET_CLAUSE_MAP) | stops (no OpenACC counterpart) | not emitted (the map is not emitted) | |
  | `OMP_TARGET_MAP_MODIFIER_CLOSE` | map modifier close, only inside OMP_TARGET_CLAUSE_MAP(...) (see the note of OMP_TARGET_CLAUSE_MAP) | dropped (copyin / copyout as without the modifier) | not emitted (the map is not emitted) | |
  | `OMP_TARGET_MAP_MODIFIER_PRESENT` | map modifier present, only inside OMP_TARGET_CLAUSE_MAP(...) (see the note of OMP_TARGET_CLAUSE_MAP) | present(list) on data and compute constructs; stops on enter data / exit data | not emitted (the map is not emitted) | GCC 13 rejects `map(present, ...)` (NVIDIA HPC SDK 26.9, Clang 23, and Intel 2026.1 accept it) |
  | `OMP_TARGET_MAP_MODIFIER_MAPPER(id)` | map modifier mapper(id), only inside OMP_TARGET_CLAUSE_MAP(...) (see the note of OMP_TARGET_CLAUSE_MAP) | stops (no OpenACC counterpart) | not emitted (the map is not emitted) | |
  | `OMP_TARGET_MAP_MODIFIER_ITERATOR(spec)` | map modifier iterator(spec), only inside OMP_TARGET_CLAUSE_MAP(...) (see the note of OMP_TARGET_CLAUSE_MAP) | stops (no OpenACC counterpart) | not emitted (the map is not emitted) | |
  | `OMP_TARGET_PASS_LIST(...)` | `OMP_PASS_LIST(__VA_ARGS__)` | `ACC_PASS_LIST(__VA_ARGS__)` | `OMP_PASS_LIST(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SEQ_CST` | `OMP_CLAUSE_SEQ_CST` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SEQ_CST` | |
  | `OMP_TARGET_CLAUSE_ACQ_REL` | `OMP_CLAUSE_ACQ_REL` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ACQ_REL` | |
  | `OMP_TARGET_CLAUSE_RELEASE` | `OMP_CLAUSE_RELEASE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_RELEASE` | |
  | `OMP_TARGET_CLAUSE_ACQUIRE` | `OMP_CLAUSE_ACQUIRE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ACQUIRE` | |
  | `OMP_TARGET_CLAUSE_RELAXED` | `OMP_CLAUSE_RELAXED` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_RELAXED` | |
  | `OMP_TARGET_CLAUSE_READ` | `OMP_CLAUSE_READ` | `ACC_CLAUSE_READ` | `OMP_CLAUSE_READ` | |
  | `OMP_TARGET_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` | `ACC_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` | |
  | `OMP_TARGET_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` | `ACC_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` | |
  | `OMP_TARGET_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` | `ACC_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` | |
  | `OMP_TARGET_CLAUSE_COMPARE` | `OMP_CLAUSE_COMPARE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_COMPARE` | |
  | `OMP_TARGET_CLAUSE_FAIL(...)` | `OMP_CLAUSE_FAIL(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_FAIL(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_WEAK` | `OMP_CLAUSE_WEAK` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_WEAK` | |
  | `OMP_TARGET_CLAUSE_HINT(expression)` | `OMP_CLAUSE_HINT(expression)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_HINT(expression)` | |
  | `OMP_TARGET_CLAUSE_SIMD` | `OMP_CLAUSE_SIMD` | `ACC_CLAUSE_INDEPENDENT` | `OMP_CLAUSE_SIMD` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_SHARED` | `OMP_CLAUSE_DEFAULT_SHARED` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_SHARED` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_FIRSTPRIVATE` | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_PRIVATE` | `OMP_CLAUSE_DEFAULT_PRIVATE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_PRIVATE` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_NONE` | `OMP_CLAUSE_DEFAULT_NONE` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEFAULT_NONE` | |
  | `OMP_TARGET_CLAUSE_SHARED(...)` | `OMP_CLAUSE_SHARED(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_SHARED(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_PRIVATE(...)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_PRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_LASTPRIVATE(...)` | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_LINEAR(...)` | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_ALLOCATE(...)` | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | `ACC_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | |
  | `OMP_TARGET_CLAUSE_DEPEND(...)` | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | `wait` (the arguments are dropped; waits for all async queues) | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `device(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | |
  | `OMP_TARGET_CLAUSE_IF_TARGET(condition)` | `OMP_CLAUSE_IF(target : condition)` | `ACC_CLAUSE_IF(condition)` | N/A (disregarded in fallback mode) | applies to the `target` construct; use `OMP_TARGET_CLAUSE_IF(condition)` for `PRAGMA_OMP_TARGET_DATA(...)`, `..._ENTER_DATA(...)`, `..._EXIT_DATA(...)`, and `..._UPDATE(...)` |
  | `OMP_TARGET_CLAUSE_MAP(...)` | `OMP_CLAUSE_MAP(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) unless nested clauses are given (see note) | `OMP_CLAUSE_MAP(__VA_ARGS__)` | nested `OMP_TARGET_CLAUSE_TO(...)` / `OMP_TARGET_CLAUSE_FROM(...)` (and `OMP_TARGET_CLAUSE_TO_PRESENT(...)` / `OMP_TARGET_CLAUSE_FROM_PRESENT(...)`) become one `map(to : ...)` / `map(from : ...)` each in OpenMP and `copyin(...)` / `copyout(...)` in OpenACC; the modifiers `OMP_TARGET_MAP_MODIFIER_*` written first apply to every nested clause of the map (`map(present, to : ...)`); in OpenACC, `present` becomes `present(...)` on data and compute constructs and stops on enter data / exit data, `close` is dropped, and `always` / `mapper` / `iterator` stop; a raw list after a nested clause stops; without nested clauses, the map is disregarded in OpenACC (OpenMP data directives stop); if the third or a later element of a non-nested list is the name of a function-like macro that cannot be invoked with one argument (e.g., `#define fm2(x, y) ...` and `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`), preprocessing fails, and if it is the name of one whose expansion with one argument has a comma at the top level (e.g., `#define fc1(x) x, x`), the build stops with a Solomon error that names internal macros: place that element first or second (e.g., `OMP_TARGET_CLAUSE_MAP(fm2, a, b)`), or write it as an array section (`fm2[0:n]`); for `update`, do not wrap `OMP_TARGET_CLAUSE_TO(...)` / `OMP_TARGET_CLAUSE_FROM(...)` in this macro, since the update directive has no `map` clause and the wrapped clause is dropped (prefer `SOLOMON_MEMCPY_H2D(list)` / `SOLOMON_MEMCPY_D2H(list)`) |
  | `OMP_TARGET_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | `ACC_CLAUSE_CREATE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | `ACC_CLAUSE_COPYOUT(__VA_ARGS__)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | `ACC_CLAUSE_COPY(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` and one `finalize` on the exit data directive (v2.0.0 or later) | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_ORDER(...)` | `OMP_CLAUSE_ORDER(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_ORDER(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_NOWAIT` | `OMP_CLAUSE_NOWAIT` | `ACC_CLAUSE_ASYNC()` | `OMP_CLAUSE_NOWAIT` | |
  | `OMP_TARGET_CLAUSE_REDUCTION(...)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` | `ACC_CLAUSE_REDUCTION(__VA_ARGS__)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_IN_REDUCTION(...)` | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` | N/A (disregarded in OpenACC backend) | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` | |

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
  | `OMP_CLAUSE_PARTIAL(...)` | `partial` without arguments; with arguments it follows in parentheses, `(__VA_ARGS__)` |
  | `OMP_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` |
  | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `num_threads(nthreads)` |
  | `OMP_CLAUSE_PROC_BIND(attr)` | `proc_bind(attr)` |
  | `OMP_CLAUSE_NUM_TEAMS(...)` | `num_teams(__VA_ARGS__)` |
  | `OMP_CLAUSE_THREAD_LIMIT(num)` | `thread_limit(num)` |
  | `OMP_CLAUSE_NONTEMPORAL(...)` | `nontemporal(__VA_ARGS__)` |
  | `OMP_CLAUSE_SAFELEN(length)` | `safelen(length)` |
  | `OMP_CLAUSE_FILTER(thread_num)` | `filter(thread_num)` |
  | `OMP_CLAUSE_COPYPRIVATE(...)` | `copyprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_ORDERED(...)` | `ordered` without arguments; with arguments it follows in parentheses, `(__VA_ARGS__)` |
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
  | `OMP_CLAUSE_DESTROY(...)` | `destroy` without arguments; with arguments it follows in parentheses, `(__VA_ARGS__)` |
  | `OMP_PASS_LIST(...)` | `(__VA_ARGS__)` |
  | `OMP_CLAUSE_SEQ_CST` | `seq_cst` |
  | `OMP_CLAUSE_ACQ_REL` | `acq_rel` |
  | `OMP_CLAUSE_RELEASE` | `release` |
  | `OMP_CLAUSE_ACQUIRE` | `acquire` |
  | `OMP_CLAUSE_RELAXED` | `relaxed` |
  | `OMP_CLAUSE_READ` | `read` |
  | `OMP_CLAUSE_WRITE` | `write` |
  | `OMP_CLAUSE_UPDATE` | `update` |
  | `OMP_CLAUSE_DEPOBJ_UPDATE(...)` | `update(__VA_ARGS__)`: `PRAGMA_OMP_DEPOBJ(...)` takes it as its `update` clause (v2.0.0 or later) |
  | `OMP_CLAUSE_CAPTURE` | `capture` |
  | `OMP_CLAUSE_COMPARE` | `compare` |
  | `OMP_CLAUSE_FAIL(...)` | `fail(__VA_ARGS__)` (the argument is `OMP_CLAUSE_SEQ_CST`, `OMP_CLAUSE_ACQUIRE`, or `OMP_CLAUSE_RELAXED`: OpenMP 5.2, Section 15.8.3) |
  | `OMP_CLAUSE_WEAK` | `weak` |
  | `OMP_CLAUSE_HINT(expression)` | `hint(expression)` |
  | `OMP_CLAUSE_THREADS` | `threads` |
  | `OMP_CLAUSE_SIMD` | `simd` <br> joins the name of a loop directive without `simd` (`PRAGMA_OMP_FOR(OMP_CLAUSE_SIMD)` → `for simd`); dropped by the directives whose names have `simd` (v2.0.0 or later; NVIDIA HPC SDK: see `SOLOMON_CLAUSE_INDEPENDENT`) |
  | `OMP_CLAUSE_DOACROSS(...)` | `doacross(__VA_ARGS__)` <br> Clang 23 and Intel 2026.1 (C/C++) reject `doacross`; with them, `PRAGMA_OMP_ORDERED(...)` emits `OMP_CLAUSE_DEPEND(source)` / `OMP_CLAUSE_DEPEND(sink: ...)`: write them instead (deprecated in OpenMP 5.2, removed in 6.0; v2.0.0 or later). NVIDIA HPC SDK 26.9 rejects `doacross`, and nvc++ 26.9 ignores the clauses of `ordered` with a warning (`clauses not yet supported on ordered construct`) and runs the loop without the dependences (wrong results); nvfortran 26.9 rejects `ordered` with `depend` |
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
  | `OMP_CLAUSE_IF(...)` | `if(__VA_ARGS__)` <br> `teams` takes `if` (`PRAGMA_OMP_TEAMS(...)`, `..._TEAMS_DISTRIBUTE(...)`, `..._TEAMS_LOOP(...)`) from OpenMP 6.0, and the clause is emitted there only when `_OPENMP >= 202411`: it is a valid form, but compilers before OpenMP 6.0 may reject it (GCC 13 and nvfortran 26.9 do; in Fortran, Solomon emits it as written in every version, and Intel ifx 2026.1 accepts it and runs it correctly). Before 6.0, Clang-based compilers (Clang, Intel oneAPI) and gcc (C) drop it with the warning `Solomon: the if clause on teams needs OpenMP 6.0 ...`, so that the teams region runs with every team even when the condition is false, and g++ stops with `Solomon: teams of OpenMP 5.2 or earlier takes no if clause ...`: remove the clause or use a compiler of OpenMP 6.0. NVIDIA HPC SDK (nvc / nvc++) drops it without a warning (it rejects a host `teams` itself). In fallback mode, `PRAGMA_OMP_TARGET_TEAMS(...)` drops it before 6.0 (with the warning where it is emitted) |
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

### Available runtime functions

* Functions to choose the device and to get the stream of the offloading backend (they call the runtime routines of the backend)
* In C++17 or later they are `static constexpr` lambdas that make the same calls; otherwise they are function-like macros
* `get_offloaded_stream_id*` return a CUDA stream and are meant only for the NVIDIA HPC SDK:
  * OpenACC: they call `acc_get_cuda_stream(...)`; the compiler must provide it
  * OpenMP target: they are defined only when the code is compiled by nvc / nvc++ (`__NVCOMPILER`)
  * They are not defined in fallback mode and serial mode (there is no device stream)

  ```c++
  int n = get_num_devices_for_offloading();
  set_default_device_for_offloading(n - 1);
  int id = get_default_device_for_offloading();
  void *stream = get_offloaded_stream_id_async(); // NVIDIA HPC SDK only
  ```

  | input | output | backend |
  | ---- | ---- | ---- |
  | `get_num_devices_for_offloading()` | `acc_get_num_devices(acc_device_default)` <br> `omp_get_num_devices()` <br> `omp_get_num_devices()` <br> `0` | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) <br> serial mode |
  | `set_default_device_for_offloading(id)` | `acc_set_device_num(id, acc_device_default)` <br> `omp_set_default_device(id)` <br> `omp_set_default_device(id)` <br> nothing (`id` is evaluated and discarded) | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) <br> serial mode |
  | `get_default_device_for_offloading()` | `acc_get_device_num(acc_device_default)` <br> `omp_get_default_device()` <br> `omp_get_default_device()` <br> `0` | OpenACC <br> OpenMP target <br> OpenMP (fallback mode) <br> serial mode |
  | `get_offloaded_stream_id(async)` | `acc_get_cuda_stream(async)` <br> `ompx_get_cuda_stream(get_default_device_for_offloading(), async)` | OpenACC <br> OpenMP target (nvc / nvc++ only; the argument is a truth value (0: synchronous, nonzero: the `nowait` stream), so queue numbers are not distinguished) |
  | `get_offloaded_stream_id_async()` | `get_offloaded_stream_id(acc_async_noval)` <br> `get_offloaded_stream_id(1)` | OpenACC <br> OpenMP target (nvc / nvc++ only) |
  | `get_offloaded_stream_id_sync()` | `get_offloaded_stream_id(acc_async_sync)` <br> `get_offloaded_stream_id(0)` | OpenACC <br> OpenMP target (nvc / nvc++ only) |

## Acknowledgment

This work was partially supported by JSPS KAKENHI Grant Number JP23K11123, MEXT as "Feasibility Study on the future HPCI", MEXT as "Project for Establishment of a Center for Advanced HPC-AI Development Support", and MEXT Next-Generation Computational Science Global Reach & Innovation Program (Establishment of a Next-Generation Computational Framework for Multiphysics and Multiscale Astrophysics) Japan Grant Number JPMXP1920260203.
