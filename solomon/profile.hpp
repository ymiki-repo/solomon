///
/// @file solomon/profile.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief vendor-neutral profiler tags (v2.0.0 or later)
///
/// @details user switches (specify on the compilation line):
///          - SOLOMON_TAGGED_PROFILE: enable the manual tagging macros (SOLOMON_PROFILE_RANGE_BEGIN/END, SOLOMON_PROFILE_MARK)
///          - SOLOMON_TAGGED_PROFILE_AUTO: additionally tag the Solomon directive macros automatically (implies SOLOMON_TAGGED_PROFILE)
///          the profiler backend (NVTX, rocTX, or ITT) is selected automatically from the compiler in use;
///          to force a specific backend, define one of SOLOMON_TAGGED_PROFILE_WITH_NVTX, SOLOMON_TAGGED_PROFILE_WITH_ROCTX, or SOLOMON_TAGGED_PROFILE_WITH_ITT
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_PROFILE_HPP)
#define SOLOMON_PROFILE_HPP

#if defined(SOLOMON_TAGGED_PROFILE_AUTO) && !defined(SOLOMON_TAGGED_PROFILE)
#define SOLOMON_TAGGED_PROFILE
#endif  // defined(SOLOMON_TAGGED_PROFILE_AUTO) && !defined(SOLOMON_TAGGED_PROFILE)

#if defined(SOLOMON_TAGGED_PROFILE) && !defined(SOLOMON_FORTRAN)

// the profiler APIs are host-side; in compilers which run a separate device-side pass over the
// same translation unit (e.g., clang-based OpenMP offloading), that pass must not activate any backend
#if defined(__AMDGCN__) || defined(__NVPTX__) || defined(__SPIR__) || defined(__SPIRV__)
#define SOLOMON_INTERNAL_PROFILE_DEVICE_PASS
#endif  // defined(__AMDGCN__) || defined(__NVPTX__) || defined(__SPIR__) || defined(__SPIRV__)

// select the profiler backend automatically unless one is forced
// note: AMD's OpenMP compiler defines no vendor macro in the host-side pass (verified on ROCm),
//       so rocTX cannot be auto-selected; define SOLOMON_TAGGED_PROFILE_WITH_ROCTX explicitly
#if !defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX) && !defined(SOLOMON_TAGGED_PROFILE_WITH_ROCTX) && !defined(SOLOMON_TAGGED_PROFILE_WITH_ITT)
#if defined(__NVCOMPILER) || defined(__NVCOMPILER_MAJOR__) || defined(__CUDACC__)
#define SOLOMON_TAGGED_PROFILE_WITH_NVTX
#elif defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_COMPILER)
#define SOLOMON_TAGGED_PROFILE_WITH_ITT
#endif  // defined(__NVCOMPILER) || defined(__NVCOMPILER_MAJOR__) || defined(__CUDACC__)
#endif  // no backend forced

#if defined(SOLOMON_INTERNAL_PROFILE_DEVICE_PASS)
// device-side pass: keep every tagging macro empty (the host-side pass emits the calls)
#undef SOLOMON_TAGGED_PROFILE_WITH_NVTX
#undef SOLOMON_TAGGED_PROFILE_WITH_ROCTX
#undef SOLOMON_TAGGED_PROFILE_WITH_ITT
#endif  // defined(SOLOMON_INTERNAL_PROFILE_DEVICE_PASS)

// the header of the selected backend: when it cannot be found (checked with __has_include where available), stop with
// one error that tells how to make it available, instead of a missing-header error followed by undeclared names
#if defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX)
#define SOLOMON_INTERNAL_PROFILE_HEADER <nvtx3/nvToolsExt.h>
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_ROCTX)
#define SOLOMON_INTERNAL_PROFILE_HEADER <roctracer/roctx.h>
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_ITT)
#define SOLOMON_INTERNAL_PROFILE_HEADER <ittnotify.h>
#endif  // defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX)
#if defined(SOLOMON_INTERNAL_PROFILE_HEADER)
#if defined(__has_include)
#if !__has_include(SOLOMON_INTERNAL_PROFILE_HEADER)
#define SOLOMON_INTERNAL_PROFILE_HEADER_MISSING
#endif  // !__has_include(SOLOMON_INTERNAL_PROFILE_HEADER)
#endif  // defined(__has_include)
#endif  // defined(SOLOMON_INTERNAL_PROFILE_HEADER)

#if defined(SOLOMON_INTERNAL_PROFILE_HEADER_MISSING) && defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX)
#error "nvtx3/nvToolsExt.h not found: with the NVIDIA HPC SDK, compile with -acc=gpu, -mp=gpu, -cuda, or -cudalib=nvtx3; otherwise add the include directory of the CUDA toolkit (e.g., -I${CUDA_HOME}/include)"
#elif defined(SOLOMON_INTERNAL_PROFILE_HEADER_MISSING) && defined(SOLOMON_TAGGED_PROFILE_WITH_ROCTX)
#error "roctracer/roctx.h not found: add -I${ROCM_PATH}/include to the compilation flags (and -L${ROCM_PATH}/lib -lroctx64 when linking)"
#elif defined(SOLOMON_INTERNAL_PROFILE_HEADER_MISSING) && defined(SOLOMON_TAGGED_PROFILE_WITH_ITT)
#error "ittnotify.h not found: add -I${VTUNE_PROFILER_DIR}/sdk/include to the compilation flags (and -L${VTUNE_PROFILER_DIR}/sdk/lib64 -littnotify when linking)"
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX)
// NVTX (NVIDIA Tools Extension); headers are found when the OpenACC/OpenMP offloading flags of the NVIDIA HPC SDK are enabled
#include <nvtx3/nvToolsExt.h>
static inline void solomon_internal_profile_push(const char *name) { (void)nvtxRangePushA(name); }
static inline void solomon_internal_profile_pop(void) { (void)nvtxRangePop(); }
static inline void solomon_internal_profile_mark(const char *name) { nvtxMarkA(name); }
#define SOLOMON_INTERNAL_PROFILE_ENABLED
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_ROCTX)
// rocTX (ROCm); add -I${ROCM_PATH}/include to the compilation flags and
// -L${ROCM_PATH}/lib -lroctx64 to the link flags
#include <roctracer/roctx.h>
static inline void solomon_internal_profile_push(const char *name) { (void)roctxRangePushA(name); }
static inline void solomon_internal_profile_pop(void) { (void)roctxRangePop(); }
static inline void solomon_internal_profile_mark(const char *name) { roctxMarkA(name); }
#define SOLOMON_INTERNAL_PROFILE_ENABLED
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_ITT)
// ITT (Intel Instrumentation and Tracing Technology); the header and library ship with VTune:
// add -I${VTUNE_PROFILER_DIR}/sdk/include to the compilation flags and
// -L${VTUNE_PROFILER_DIR}/sdk/lib64 -littnotify to the link flags
#include <ittnotify.h>
static inline __itt_domain *solomon_internal_profile_domain(void) {
  static __itt_domain *domain = 0;
  if (!domain) {
    domain = __itt_domain_create("solomon");
  }
  return domain;
}
static inline void solomon_internal_profile_push(const char *name) { __itt_task_begin(solomon_internal_profile_domain(), __itt_null, __itt_null, __itt_string_handle_create(name)); }
static inline void solomon_internal_profile_pop(void) { __itt_task_end(solomon_internal_profile_domain()); }
static inline void solomon_internal_profile_mark(const char *name) { __itt_marker(solomon_internal_profile_domain(), __itt_null, __itt_string_handle_create(name), __itt_scope_task); }
#define SOLOMON_INTERNAL_PROFILE_ENABLED
#else  // no backend
#if !defined(SOLOMON_INTERNAL_PROFILE_DEVICE_PASS)
#warning "SOLOMON_TAGGED_PROFILE is enabled, but no profiler backend was detected for this compiler; define SOLOMON_TAGGED_PROFILE_WITH_NVTX, SOLOMON_TAGGED_PROFILE_WITH_ROCTX (required for AMD compilers), or SOLOMON_TAGGED_PROFILE_WITH_ITT explicitly"
#endif  // !defined(SOLOMON_INTERNAL_PROFILE_DEVICE_PASS)
#endif  // defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX)

#endif  // defined(SOLOMON_TAGGED_PROFILE) && !defined(SOLOMON_FORTRAN)

#if defined(SOLOMON_TAGGED_PROFILE) && defined(SOLOMON_FORTRAN)
// Fortran: the macros expand to calls into the bundled helper (solomon/profile/solomon_profile.c);
// compile the helper with the same compiler and offloading flags (plus -DSOLOMON_TAGGED_PROFILE) and link its object
#define SOLOMON_INTERNAL_PROFILE_ENABLED_FORTRAN
#endif  // defined(SOLOMON_TAGGED_PROFILE) && defined(SOLOMON_FORTRAN)

///
/// @brief begin a named profiler range (manual tagging; no trailing semicolon is required)
///
#if defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
#define SOLOMON_PROFILE_RANGE_BEGIN(name) solomon_internal_profile_push(name);
#elif defined(SOLOMON_INTERNAL_PROFILE_ENABLED_FORTRAN)
#define SOLOMON_PROFILE_RANGE_BEGIN(name) call solomon_profile_range_begin(name)
#else  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
#define SOLOMON_PROFILE_RANGE_BEGIN(name)
#endif  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)

///
/// @brief end the innermost profiler range (manual tagging)
///
#if defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
#define SOLOMON_PROFILE_RANGE_END solomon_internal_profile_pop();
#elif defined(SOLOMON_INTERNAL_PROFILE_ENABLED_FORTRAN)
#define SOLOMON_PROFILE_RANGE_END call solomon_profile_range_end()
#else  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
#define SOLOMON_PROFILE_RANGE_END
#endif  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)

///
/// @brief record a named instantaneous event (manual tagging)
///
#if defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
#define SOLOMON_PROFILE_MARK(name) solomon_internal_profile_mark(name);
#elif defined(SOLOMON_INTERNAL_PROFILE_ENABLED_FORTRAN)
#define SOLOMON_PROFILE_MARK(name) call solomon_profile_mark(name)
#else  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
#define SOLOMON_PROFILE_MARK(name)
#endif  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)

// automatic tag string: "FILE:LINE MACRO"
#define SOLOMON_INTERNAL_PROFILE_STR_IMPL(x) #x
#define SOLOMON_INTERNAL_PROFILE_STR(x) SOLOMON_INTERNAL_PROFILE_STR_IMPL(x)
#define SOLOMON_INTERNAL_PROFILE_TAG(mname) __FILE__ ":" SOLOMON_INTERNAL_PROFILE_STR(__LINE__) " " mname

// hooks used by the directive macros in solomon.hpp; they receive the bare macro name
// and build the "FILE:LINE MACRO" tag per language (pass-through unless SOLOMON_TAGGED_PROFILE_AUTO is enabled)
#if defined(SOLOMON_INTERNAL_PROFILE_ENABLED) && defined(SOLOMON_TAGGED_PROFILE_AUTO)
#define SOLOMON_INTERNAL_TAG_RANGE(mname, directive) { SOLOMON_PROFILE_RANGE_BEGIN(SOLOMON_INTERNAL_PROFILE_TAG(mname)) directive SOLOMON_PROFILE_RANGE_END }
// the tag of a directive that is followed by a statement (a loop or a structured block) must not be a statement of
// its own, so that the directive macro stays one statement (e.g., as the body of an if without braces): the call is
// made in the initialization of a loop that runs its body (the directive and its statement) exactly once (only for
// the range of SOLOMON_OFFLOAD_SERIAL, whose end must follow the statement: a continue / break inside that block
// would end at this loop; marks use the if / else form below); the name
// of the loop variable carries the line number, so that directive macros nested on different lines do not shadow it
// (two SOLOMON_OFFLOAD_SERIAL nested on the same line give one -Wshadow warning; the behaviour is still correct)
#define SOLOMON_INTERNAL_PROFILE_CAT_IMPL(a, b) a##b
#define SOLOMON_INTERNAL_PROFILE_CAT(a, b) SOLOMON_INTERNAL_PROFILE_CAT_IMPL(a, b)
#define SOLOMON_INTERNAL_PROFILE_ONCE_IMPL(v, start, end) for (int v = (start, 0); !v; end, v = 1)
#define SOLOMON_INTERNAL_PROFILE_ONCE(start, end) SOLOMON_INTERNAL_PROFILE_ONCE_IMPL(SOLOMON_INTERNAL_PROFILE_CAT(solomon_internal_profile_once_, __LINE__), start, end)
// a mark goes in the condition of an if whose else is the directive and its statement: unlike a loop, it does not
// catch the user's continue / break (Q-044, D-091); gcc / clang may warn -Wdangling-else on a braceless if around it
#define SOLOMON_INTERNAL_TAG_MARK(mname) if ((solomon_internal_profile_mark(SOLOMON_INTERNAL_PROFILE_TAG(mname)), 0)) {} else
// the range of SOLOMON_OFFLOAD_SERIAL ends right after its structured block (SOLOMON_INTERNAL_TAG_POP is empty)
#define SOLOMON_INTERNAL_TAG_PUSH(mname) SOLOMON_INTERNAL_PROFILE_ONCE(solomon_internal_profile_push(SOLOMON_INTERNAL_PROFILE_TAG(mname)), solomon_internal_profile_pop())
#define SOLOMON_INTERNAL_TAG_POP
#elif defined(SOLOMON_INTERNAL_PROFILE_ENABLED_FORTRAN) && defined(SOLOMON_TAGGED_PROFILE_AUTO)
// Fortran cannot concatenate string literals in the preprocessor; pass file, line, and macro name separately.
// the generated call statements are emitted through the _Pragma mechanism ("#pragma solomon_fprof call ...",
// restored to plain call statements by the sed stage of spp.sh / fortran.mk) so that each statement is
// guaranteed to land on its own line regardless of the cpp version in use; the tags have their own pragma macro,
// since the pragma macro of the directives expands to nothing in serial execution
#if defined(PRINT_GENERATED_PRAGMA)
#define SOLOMON_INTERNAL_FPROF_SHOW(...) _Pragma(SOLOMON_INTERNAL_PROFILE_STR(message("#pragma " SOLOMON_INTERNAL_PROFILE_STR(__VA_ARGS__))))
#else  // defined(PRINT_GENERATED_PRAGMA)
#define SOLOMON_INTERNAL_FPROF_SHOW(...)
#endif  // defined(PRINT_GENERATED_PRAGMA)
#define SOLOMON_INTERNAL_FPROF_PRAGMA(...) _Pragma(#__VA_ARGS__) SOLOMON_INTERNAL_FPROF_SHOW(__VA_ARGS__)
#define SOLOMON_INTERNAL_FPROF_CALL(...) SOLOMON_INTERNAL_FPROF_PRAGMA(solomon_fprof __VA_ARGS__)
#define SOLOMON_INTERNAL_TAG_RANGE(mname, directive) SOLOMON_INTERNAL_FPROF_CALL(call solomon_profile_range_begin_at(__FILE__, __LINE__, mname)) directive SOLOMON_INTERNAL_FPROF_CALL(call solomon_profile_range_end())
#define SOLOMON_INTERNAL_TAG_MARK(mname) SOLOMON_INTERNAL_FPROF_CALL(call solomon_profile_mark_at(__FILE__, __LINE__, mname))
#define SOLOMON_INTERNAL_TAG_PUSH(mname) SOLOMON_INTERNAL_FPROF_CALL(call solomon_profile_range_begin_at(__FILE__, __LINE__, mname))
#define SOLOMON_INTERNAL_TAG_POP SOLOMON_INTERNAL_FPROF_CALL(call solomon_profile_range_end())
#else  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED) && defined(SOLOMON_TAGGED_PROFILE_AUTO)
#define SOLOMON_INTERNAL_TAG_RANGE(mname, directive) directive
#define SOLOMON_INTERNAL_TAG_MARK(mname)
#define SOLOMON_INTERNAL_TAG_PUSH(mname)
#define SOLOMON_INTERNAL_TAG_POP
#endif  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED) && defined(SOLOMON_TAGGED_PROFILE_AUTO)

#endif  // !defined(SOLOMON_PROFILE_HPP)
