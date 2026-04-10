

#ifndef DIFFUSION_H
#define DIFFUSION_H

double diffusion3d(int nx, int ny, int nz, float dx, float dy, float dz, float dt, float kappa,
                   const float *f, float *fn, const int pad);
void init(int nx, int ny, int nz, float dx, float dy, float dz, float *f, const int pad);
double accuracy(double time, int nx, int ny, int nz, float dx, float dy, float dz, float kappa, const float *f, const int pad);

#endif /* DIFFUSION_H */
