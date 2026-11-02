

#define _XOPEN_SOURCE 700  // lockf, fileno

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <solomon.hpp>

#include "diffusion.h"
#include "misc.h"

static void usage(const char *exe) {
  fprintf(stderr, "Usage is: %s N\n", exe);
  fprintf(stderr, "\tN: number of grid points <int>, 3 <= N and N^3 <= %d (N <= 1290)\n", INT_MAX);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "ERROR: %d input parameters are given (%d is required)\n", argc - 1, 1);
    usage(argv[0]);
    return 1;
  }
  int nx = 0;
  // N^3 <= INT_MAX, checked without overflow: N^2 <= INT_MAX / N (N^2 fits in long long)
  if ((read_int(argv[1], 3, INT_MAX, &nx) != 0) || ((long long)nx * (long long)nx > (long long)(INT_MAX / nx))) {
    fprintf(stderr, "ERROR: invalid input parameter: N = \"%s\"\n", argv[1]);
    usage(argv[0]);
    return 1;
  }

  const int ny = nx;
  const int nz = ny;
  const int n = nx * ny * nz;

  const float lx = 1.0F;
  const float ly = 1.0F;
  const float lz = 1.0F;

  const float dx = lx / (float)nx;
  const float dy = ly / (float)ny;
  const float dz = lz / (float)nz;

  const float kappa = 0.1F;
  const float dt = 0.1F * fminf(fminf(dx * dx, dy * dy), dz * dz) / kappa;

  const int nt = 100000;
  double time = 0.0;
  int icnt = 0;
  double flop = 0.0;
  double elapsed_time = 0.0;
  const double byte_per_flop = sizeof(float) * 8.0 / 13.0;

  float *f = (float *)malloc(sizeof(float) * n);
  float *fn = (float *)malloc(sizeof(float) * n);
  if ((f == NULL) || (fn == NULL)) {
    fprintf(stderr, "ERROR: failed to allocate 2 arrays of %d floats\n", n);
    return 1;
  }

#if defined(APPLY_FIRST_TOUCH)
  // first touch
  SOLOMON_OFFLOAD()
  for (int ii = 0; ii < n; ii++) {
    f[ii] = 0.0F;
    fn[ii] = 0.0F;
  }
#endif  // defined(APPLY_FIRST_TOUCH)

  init(nx, ny, nz, dx, dy, dz, f);

  PRAGMA_ACC_DATA(ACC_CLAUSE_COPY(f [0:n]), ACC_CLAUSE_CREATE(fn [0:n])) {
    start_timer();

    for (; icnt < nt && time + 0.5 * dt < 0.1; icnt++) {
#if !defined(BENCHMARK_MODE)
      if (icnt % 100 == 0) fprintf(stdout, "time(%4d) = %7.5f\n", icnt, time);
#endif  //! defined(BENCHMARK_MODE)

      flop += diffusion3d(nx, ny, nz, dx, dy, dz, dt, kappa, f, fn);

      swap(&f, &fn);

      time += dt;
    }

    elapsed_time = get_elapsed_time();

    // the pointers are swapped inside the data region: copy back the array that f now points to
    SOLOMON_MEMCPY_D2H(f [0:n])
  }

#if !defined(BENCHMARK_MODE)
  fprintf(stdout, "Time = %8.3f [sec]\n", elapsed_time);
  fprintf(stdout, "Performance = %7.2f [GFlop/s]\n", flop / elapsed_time * 1.0e-09);
  fprintf(stdout, "Bandwidth   = %7.2f [GB/s]\n", byte_per_flop * flop / elapsed_time * 1.0e-09);
#endif  //! defined(BENCHMARK_MODE)

  const double ferr = accuracy(time, nx, ny, nz, dx, dy, dz, kappa, f);
#if !defined(BENCHMARK_MODE)
  fprintf(stdout, "Error[%d][%d][%d] = %10.6e\n", nx, ny, nz, ferr);
#endif  //! defined(BENCHMARK_MODE)

  char filename[256];
  const int len = snprintf(filename, sizeof(filename), "diffusion_benchmark_%s%d.csv", COMPILER, MODEL_ID);
  if ((len < 0) || ((size_t)len >= sizeof(filename))) {
    fprintf(stderr, "ERROR: the CSV file name for COMPILER \"%s\" is too long (at most %zu bytes)\n", COMPILER, sizeof(filename) - 1);
    return 1;
  }
  // every run appends one row (also runs started at the same time in this directory); the header goes to an empty file
  FILE *fp = fopen(filename, "a");
  if (fp == NULL) {
    fprintf(stderr, "ERROR: failed to open \"%s\"\n", filename);
    return 1;
  }
  if (lockf(fileno(fp), F_LOCK, 0) != 0) {
    // without the lock (a file system without locks), concurrent runs may write the header twice
  }
  fseek(fp, 0, SEEK_END);
  if (ftell(fp) == 0) {
    fprintf(fp, "Model_ID,Optimization_level");
    fprintf(fp, ",nx,ny,nz");
    fprintf(fp, ",time[s],Flops,Flop/s,Bytes,B/s,error\n");
  }
  fprintf(fp, "%d,%s", MODEL_ID, OPT_LEVEL);
  fprintf(fp, ",%d", nx);
  fprintf(fp, ",%d", ny);
  fprintf(fp, ",%d", nz);
  fprintf(fp, ",%e,%e,%e,%e,%e,%e\n", elapsed_time, flop, flop / elapsed_time, byte_per_flop * flop, byte_per_flop * flop / elapsed_time, ferr);
  if (fclose(fp) != 0) {
    fprintf(stderr, "ERROR: failed to write \"%s\"\n", filename);
    return 1;
  }

  free(f);
  f = NULL;
  free(fn);
  fn = NULL;

  return 0;
}
