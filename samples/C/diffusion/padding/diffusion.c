

#include <math.h>
#include <stdio.h>

#include <solomon.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338328  // pi
#endif                                        // M_PI

#define INDEX(nx, ny, nz, ii, jj, kk) ((kk) + (nz) * ((jj) + (ny) * (ii)))
#define PADDED_ID(nx, ny, nz, ii, jj, kk, pad) INDEX(nx, ny, nz + pad, ii, jj, kk)

#define IMIN(a, b) (((a) < (b)) ? (a) : (b))
#define IMAX(a, b) (((a) > (b)) ? (a) : (b))

double diffusion3d(int nx, int ny, int nz, float dx, float dy, float dz, float dt, float kappa,
                   const float *restrict f, float *restrict fn, const int pad) {
  const float ce = kappa * dt / (dx * dx);
  const float cw = ce;
  const float cn = kappa * dt / (dy * dy);
  const float cs = cn;
  const float ct = kappa * dt / (dz * dz);
  const float cb = ct;

  const float cc = 1.0 - (ce + cw + cn + cs + ct + cb);

  OFFLOAD(AS_INDEPENDENT, COLLAPSE(3), ACC_CLAUSE_PRESENT(f, fn))
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      for (int k = 0; k < nz; k++) {
        const int ix = PADDED_ID(nx, ny, nz, i, j, k, pad);
        const int ip = PADDED_ID(nx, ny, nz, IMIN(i + 1, nx - 1), j, k, pad);
        const int im = PADDED_ID(nx, ny, nz, IMAX(i - 1, 0), j, k, pad);
        const int jp = PADDED_ID(nx, ny, nz, i, IMIN(j + 1, ny - 1), k, pad);
        const int jm = PADDED_ID(nx, ny, nz, i, IMAX(j - 1, 0), k, pad);
        const int kp = PADDED_ID(nx, ny, nz, i, j, IMIN(k + 1, nz - 1), pad);
        const int km = PADDED_ID(nx, ny, nz, i, j, IMAX(k - 1, 0), pad);

        fn[ix] = cc * f[ix] + ce * f[ip] + cw * f[im] + cn * f[jp] + cs * f[jm] + ct * f[kp] + cb * f[km];
      }
    }
  }

  return (double)(nx * ny * nz) * 13.0;
}

void init(int nx, int ny, int nz, float dx, float dy, float dz, float *f, const int pad) {
  const float kx = 2.0F * (float)M_PI;
  const float ky = kx;
  const float kz = kx;

  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      for (int k = 0; k < nz; k++) {
        const int ix = PADDED_ID(nx, ny, nz, i, j, k, pad);
        const float x = dx * ((float)i + 0.5F);
        const float y = dy * ((float)j + 0.5F);
        const float z = dz * ((float)k + 0.5F);

        f[ix] = 0.125F * (1.0F - cosf(kx * x)) * (1.0F - cosf(ky * y)) * (1.0F - cosf(kz * z));
      }
    }
  }
}

double accuracy(double time, int nx, int ny, int nz, float dx, float dy, float dz, float kappa, const float *f, const int pad) {
  const float kx = 2.0F * (float)M_PI;
  const float ky = kx;
  const float kz = kx;

  const float ax = expf(-kappa * (float)time * (kx * kx));
  const float ay = expf(-kappa * (float)time * (ky * ky));
  const float az = expf(-kappa * (float)time * (kz * kz));

  double ferr = 0.0;

  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      for (int k = 0; k < nz; k++) {
        const int ix = PADDED_ID(nx, ny, nz, i, j, k, pad);
        const float x = dx * ((float)i + 0.5F);
        const float y = dy * ((float)j + 0.5F);
        const float z = dz * ((float)k + 0.5F);

        const float f0 = 0.125F * (1.0F - ax * cosf(kx * x)) * (1.0F - ay * cosf(ky * y)) * (1.0F - az * cosf(kz * z));

        ferr += (f[ix] - f0) * (f[ix] - f0);
      }
    }
  }

  return sqrt(ferr / (double)(nx * ny * nz));
}
