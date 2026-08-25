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

// select the profiler backend automatically unless one is forced
#if !defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX) && !defined(SOLOMON_TAGGED_PROFILE_WITH_ROCTX) && !defined(SOLOMON_TAGGED_PROFILE_WITH_ITT)
#if defined(__NVCOMPILER) || defined(__NVCOMPILER_MAJOR__) || defined(__CUDACC__)
#define SOLOMON_TAGGED_PROFILE_WITH_NVTX
#elif defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_COMPILER)
#define SOLOMON_TAGGED_PROFILE_WITH_ITT
#elif defined(__HIP_PLATFORM_AMD__) || defined(__AMDGCN__) || defined(__AMDGPU__)
#define SOLOMON_TAGGED_PROFILE_WITH_ROCTX
#endif  // defined(__NVCOMPILER) || defined(__NVCOMPILER_MAJOR__) || defined(__CUDACC__)
#endif  // no backend forced

#if defined(SOLOMON_TAGGED_PROFILE_WITH_NVTX)
// NVTX (NVIDIA Tools Extension); headers are found when the OpenACC/OpenMP offloading flags of the NVIDIA HPC SDK are enabled
#include <nvtx3/nvToolsExt.h>
static inline void solomon_internal_profile_push(const char *name) { (void)nvtxRangePushA(name); }
static inline void solomon_internal_profile_pop(void) { (void)nvtxRangePop(); }
static inline void solomon_internal_profile_mark(const char *name) { nvtxMarkA(name); }
#define SOLOMON_INTERNAL_PROFILE_ENABLED
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_ROCTX)
// rocTX (ROCm); add -lroctx64 to the link line
#include <roctracer/roctx.h>
static inline void solomon_internal_profile_push(const char *name) { (void)roctxRangePushA(name); }
static inline void solomon_internal_profile_pop(void) { (void)roctxRangePop(); }
static inline void solomon_internal_profile_mark(const char *name) { roctxMarkA(name); }
#define SOLOMON_INTERNAL_PROFILE_ENABLED
#elif defined(SOLOMON_TAGGED_PROFILE_WITH_ITT)
// ITT (Intel Instrumentation and Tracing Technology); the header and library ship with VTune:
// add -I${VTUNE_PROFILER_DIR}/sdk/include to the compilation flags and
// -L${VTUNE_PROFILER_DIR}/sdk/lib64 -littnotify to the link flags
#if defined(__has_include)
#if !__has_include(<ittnotify.h>)
#error "ittnotify.h not found: add -I${VTUNE_PROFILER_DIR}/sdk/include to the compilation flags (and -L${VTUNE_PROFILER_DIR}/sdk/lib64 -littnotify when linking)"
#endif
#endif  // defined(__has_include)
#include <ittnotify.h>
static inline __itt_domain *solomon_internal_profile_domain(void) {
  static __itt_domain *domain = (void *)0;
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
#warning "SOLOMON_TAGGED_PROFILE is enabled, but no profiler backend was detected for this compiler; define SOLOMON_TAGGED_PROFILE_WITH_NVTX, SOLOMON_TAGGED_PROFILE_WITH_ROCTX, or SOLOMON_TAGGED_PROFILE_WITH_ITT explicitly"
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
#define SOLOMON_INTERNAL_TAG_MARK(mname) SOLOMON_PROFILE_MARK(SOLOMON_INTERNAL_PROFILE_TAG(mname))
#define SOLOMON_INTERNAL_TAG_PUSH(mname) SOLOMON_PROFILE_RANGE_BEGIN(SOLOMON_INTERNAL_PROFILE_TAG(mname))
#define SOLOMON_INTERNAL_TAG_POP SOLOMON_PROFILE_RANGE_END
#elif defined(SOLOMON_INTERNAL_PROFILE_ENABLED_FORTRAN) && defined(SOLOMON_TAGGED_PROFILE_AUTO)
// Fortran cannot concatenate string literals in the preprocessor; pass file, line, and macro name separately
#define SOLOMON_INTERNAL_TAG_RANGE(mname, directive) call solomon_profile_range_begin_at(__FILE__, __LINE__, mname) directive call solomon_profile_range_end()
#define SOLOMON_INTERNAL_TAG_MARK(mname) call solomon_profile_mark_at(__FILE__, __LINE__, mname)
#define SOLOMON_INTERNAL_TAG_PUSH(mname) call solomon_profile_range_begin_at(__FILE__, __LINE__, mname)
#define SOLOMON_INTERNAL_TAG_POP call solomon_profile_range_end()
#else  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED) && defined(SOLOMON_TAGGED_PROFILE_AUTO)
#define SOLOMON_INTERNAL_TAG_RANGE(mname, directive) directive
#define SOLOMON_INTERNAL_TAG_MARK(mname)
#define SOLOMON_INTERNAL_TAG_PUSH(mname)
#define SOLOMON_INTERNAL_TAG_POP
#endif  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED) && defined(SOLOMON_TAGGED_PROFILE_AUTO)

#endif  // !defined(SOLOMON_PROFILE_HPP)
