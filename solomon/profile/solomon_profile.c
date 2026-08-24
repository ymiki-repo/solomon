///
/// @file solomon/profile/solomon_profile.c
/// @author Yohei MIKI (The University of Tokyo)
/// @brief Fortran-callable wrappers for the vendor-neutral profiler tags (v2.0.0 or later)
///
/// @details compile this file with the same compiler and offloading flags as your Fortran objects,
///          add -DSOLOMON_TAGGED_PROFILE (the profiler backend is then selected automatically), and link the object:
///            nvc -c -mp=gpu -DSOLOMON_TAGGED_PROFILE solomon/profile/solomon_profile.c
///          without -DSOLOMON_TAGGED_PROFILE, the wrappers become no-ops (linking always succeeds)
/// @note the wrappers use the common Fortran calling convention (lower-case name with a trailing
///       underscore; hidden string lengths appended after the arguments)
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#include "../profile.hpp"

#define SOLOMON_PROFILE_TAG_CAPACITY 512

#if defined(SOLOMON_INTERNAL_PROFILE_ENABLED)

static void solomon_profile_copy(char *dst, int *pos, const char *src, int len) {
  int ii;
  for (ii = 0; ii < len && *pos < SOLOMON_PROFILE_TAG_CAPACITY - 1; ii++) {
    dst[(*pos)++] = src[ii];
  }
  dst[*pos] = '\0';
}

void solomon_profile_range_begin_(const char *name, int len) {
  char tag[SOLOMON_PROFILE_TAG_CAPACITY];
  int pos = 0;
  solomon_profile_copy(tag, &pos, name, len);
  solomon_internal_profile_push(tag);
}

void solomon_profile_range_end_(void) { solomon_internal_profile_pop(); }

void solomon_profile_mark_(const char *name, int len) {
  char tag[SOLOMON_PROFILE_TAG_CAPACITY];
  int pos = 0;
  solomon_profile_copy(tag, &pos, name, len);
  solomon_internal_profile_mark(tag);
}

// build "FILE:LINE MACRO" from the components passed by the Fortran macros
static void solomon_profile_build_tag(char *tag, const char *file, const int *line, const char *name, int flen, int nlen) {
  char digits[16];
  int pos = 0, ndigits = 0, value = (*line > 0) ? *line : 0;
  solomon_profile_copy(tag, &pos, file, flen);
  solomon_profile_copy(tag, &pos, ":", 1);
  do {
    digits[ndigits++] = (char)('0' + (value % 10));
    value /= 10;
  } while (value > 0 && ndigits < (int)sizeof(digits));
  while (ndigits > 0) {
    ndigits--;
    solomon_profile_copy(tag, &pos, &digits[ndigits], 1);
  }
  solomon_profile_copy(tag, &pos, " ", 1);
  solomon_profile_copy(tag, &pos, name, nlen);
}

void solomon_profile_range_begin_at_(const char *file, const int *line, const char *name, int flen, int nlen) {
  char tag[SOLOMON_PROFILE_TAG_CAPACITY];
  solomon_profile_build_tag(tag, file, line, name, flen, nlen);
  solomon_internal_profile_push(tag);
}

void solomon_profile_mark_at_(const char *file, const int *line, const char *name, int flen, int nlen) {
  char tag[SOLOMON_PROFILE_TAG_CAPACITY];
  solomon_profile_build_tag(tag, file, line, name, flen, nlen);
  solomon_internal_profile_mark(tag);
}

#else  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)

// no-op wrappers (profiling disabled or no backend available)
void solomon_profile_range_begin_(const char *name, int len) { (void)name; (void)len; }
void solomon_profile_range_end_(void) {}
void solomon_profile_mark_(const char *name, int len) { (void)name; (void)len; }
void solomon_profile_range_begin_at_(const char *file, const int *line, const char *name, int flen, int nlen) { (void)file; (void)line; (void)name; (void)flen; (void)nlen; }
void solomon_profile_mark_at_(const char *file, const int *line, const char *name, int flen, int nlen) { (void)file; (void)line; (void)name; (void)flen; (void)nlen; }

#endif  // defined(SOLOMON_INTERNAL_PROFILE_ENABLED)
