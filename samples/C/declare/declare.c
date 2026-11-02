///
/// @file declare.c
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief usage of the declare directives: functions called in offloaded regions and variables resident on the device
///
/// @copyright Copyright (c) 2026 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
/// The sum of N ones is computed on the device, then squared by a function called inside an offloaded region
/// through a device-resident variable; the program prints the result and returns 0 when it equals N * N.
/// When offloading, it also checks that the host copy of the device-resident variable kept the value set on the host
/// before the region: the device copy is a separate variable and is not copied back. Define SHARED_HOST_DEVICE_MEMORY
/// to skip this check where the host and the device share the variable (e.g., unified memory or host fallback).
///

#include <stdio.h>

#include <solomon.hpp>

#define N (1000)
#define SENTINEL (-1.0)

// named form: declare a function that is defined later (OpenACC: routine (helper) seq, OpenMP target: declare target
// (helper)); the function must be declared before the directive
void helper(double* a);
SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(helper), SOLOMON_CLAUSE_SEQUENTIAL)

// pair form: the function between SOLOMON_DECLARE_OFFLOADED and SOLOMON_DECLARE_OFFLOADED_END can be called on the
// device (OpenACC: routine seq for the definition that follows, OpenMP target: declare target ... end declare target);
// keep one function per pair, since the OpenACC routine directive applies to the next function only
void square(double* a);
SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_SEQUENTIAL) void square(double* a) { *a = (*a) * (*a); }
SOLOMON_DECLARE_OFFLOADED_END

void helper(double* a) { square(a); }

// a global variable that also lives on the device (OpenACC: declare create(resident), OpenMP target: declare target
// (resident))
double resident[4];
SOLOMON_DECLARE_ON_DEVICE(resident)

int main(void) {
  double x[N];
  double s = 0.0;
  for (int i = 0; i < N; i++) {
    x[i] = 1.0;
  }

  SOLOMON_MALLOC_ON_DEVICE(x)
  SOLOMON_MEMCPY_H2D(x)

  // sum on the device
  SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_REDUCTION(+ : s))
  for (int i = 0; i < N; i++) {
    s += x[i];
  }
  SOLOMON_END_OFFLOAD

  // set the host copy only: the device copy of resident is not updated by this
  for (int i = 0; i < 4; i++) {
    resident[i] = SENTINEL;
  }

  // call the declared functions on the device through the device-resident variable
  SOLOMON_OFFLOAD_SERIAL(SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(s))
  {
    resident[0] = s;
    helper(&resident[0]);
    s = resident[0];
  }
  SOLOMON_END_OFFLOAD_SERIAL
  SOLOMON_SYNCHRONIZE()

  SOLOMON_FREE_FROM_DEVICE(x)

  const double expected = (double)N * (double)N;
  printf("%.1f\n", s);
  if (s != expected) {
    fprintf(stderr, "FAILED: %.1f (expected %.1f)\n", s, expected);
    return 1;
  }
  // OFFLOAD_BY_OPENACC / OFFLOAD_BY_OPENMP_TARGET remain defined after solomon.hpp only for the backend in use,
  // whether -DSOLOMON_OFFLOAD_BY_* or -DOFFLOAD_BY_* was given
#if (defined(OFFLOAD_BY_OPENACC) || defined(OFFLOAD_BY_OPENMP_TARGET)) && !defined(SHARED_HOST_DEVICE_MEMORY)
  if (resident[0] != SENTINEL) {
    fprintf(stderr, "FAILED: the host copy of resident changed to %.1f (expected %.1f)\n", resident[0], SENTINEL);
    fprintf(stderr, "        the device copy is not separate: SOLOMON_DECLARE_ON_DEVICE(resident) is missing, or the program\n");
    fprintf(stderr, "        ran on the host (host fallback) or with unified memory; define SHARED_HOST_DEVICE_MEMORY for these\n");
    return 1;
  }
#endif
  return 0;
}
