

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <solomon.hpp>

#include "diffusion.h"
#include "misc.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "ERROR: insufficient number of input parameters: %d (at least %d inputs are required)\n", argc, 2);
    fprintf(stderr, "Usage is: %s N\n", argv[0]);
    fprintf(stderr, "\tN: number of grid points <int>\n");
  }
  int padding = 0;
  if (argc >= 3) {
    const int tmp = atoi(argv[2]);
    if (tmp > 0) {
      padding = tmp;
    }
  }

  const int nx = atoi(argv[1]);
  const int ny = nx;
  const int nz = ny;
  const int n = nx * ny * (nz + padding);

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

#if defined(APPLY_FIRST_TOUCH)
  // first touch
  OFFLOAD()
  for (int ii = 0; ii < n; ii++) {
    f[ii] = 0.0F;
    fn[ii] = 0.0F;
  }
#endif  // defined(APPLY_FIRST_TOUCH)

  init(nx, ny, nz, dx, dy, dz, f, padding);

  PRAGMA_ACC_DATA(ACC_CLAUSE_COPY(f [0:n]), ACC_CLAUSE_CREATE(fn [0:n])) {
    start_timer();

    for (; icnt < nt && time + 0.5 * dt < 0.1; icnt++) {
#if !defined(BENCHMARK_MODE)
      if (icnt % 100 == 0) fprintf(stdout, "time(%4d) = %7.5f\n", icnt, time);
#endif  //! defined(BENCHMARK_MODE)

      flop += diffusion3d(nx, ny, nz, dx, dy, dz, dt, kappa, f, fn, padding);

      swap(&f, &fn);

      time += dt;
    }

    elapsed_time = get_elapsed_time();
  }

#if !defined(BENCHMARK_MODE)
  fprintf(stdout, "Time = %8.3f [sec]\n", elapsed_time);
  fprintf(stdout, "Performance = %7.2f [GFlop/s]\n", flop / elapsed_time * 1.0e-09);
  fprintf(stdout, "Bandwidth   = %7.2f [GB/s]\n", byte_per_flop * flop / elapsed_time * 1.0e-09);
#endif  //! defined(BENCHMARK_MODE)

  const double ferr = accuracy(time, nx, ny, nz, dx, dy, dz, kappa, f, padding);
#if !defined(BENCHMARK_MODE)
  fprintf(stdout, "Error[%d][%d][%d] = %10.6e\n", nx, ny, nz, ferr);
#endif  //! defined(BENCHMARK_MODE)

  char filename[64];
  sprintf(filename, "diffusion_benchmark_%s%d.csv", COMPILER, MODEL_ID);
  FILE *fp;
  if (access(filename, F_OK) == 0) {
    fp = fopen(filename, "a");
    if (fp == NULL) {
      fprintf(stderr, "ERROR: failed to open \"%s\"\n", filename);
      exit(1);
    }
  } else {
    fp = fopen(filename, "w");
    if (fp == NULL) {
      fprintf(stderr, "ERROR: failed to open \"%s\"\n", filename);
      exit(1);
    }
    fprintf(fp, "Model_ID,Optimization_level,padding");
    fprintf(fp, ",nx,ny,nz");
    fprintf(fp, ",time[s],Flops,Flop/s,Bytes,B/s,error\n");
  }
  fprintf(fp, "%d,%s,%d", MODEL_ID, OPT_LEVEL, padding);
  fprintf(fp, ",%d", nx);
  fprintf(fp, ",%d", ny);
  fprintf(fp, ",%d", nz);
  fprintf(fp, ",%e,%e,%e,%e,%e,%e\n", elapsed_time, flop, flop / elapsed_time, byte_per_flop * flop, byte_per_flop * flop / elapsed_time, ferr);

  free(f);
  f = NULL;
  free(fn);
  fn = NULL;

  return 0;
}
