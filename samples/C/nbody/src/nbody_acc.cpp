///
/// @file nbody_acc.cpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief benchmark of direct N-body simulation (w/o orbit integration) (implementation in OpenACC style)
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#include <stdio.h>
#include <stdlib.h>
#include <time.h>  // clock_gettime
#include <unistd.h>

#include <cmath>
#include <solomon.hpp>
#include <type_traits>

#include "allocate.hpp"
#include "benchmark.hpp"
#include "common.hpp"
#include "gen_ic.hpp"
#include "gravity.hpp"
#include "type.hpp"

static const float newton = 1.0F;  // gravitational constant

#ifndef BENCHMARK_MODE
void kick(const int num, velocity *vel, acceleration *acc, const float dt) {
  PRAGMA_ACC_PARALLEL_LOOP(ACC_CLAUSE_INDEPENDENT)
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    // initialization
    auto vi = vel[ii];
    const auto ai = acc[ii];

    vi.x = std::fma(dt, ai.x, vi.x);
    vi.y = std::fma(dt, ai.y, vi.y);
    vi.z = std::fma(dt, ai.z, vi.z);

    vel[ii] = vi;
  }
}

void drift(const int num, position *pos, velocity *vel, const float dt) {
  PRAGMA_ACC_KERNELS_LOOP(ACC_CLAUSE_INDEPENDENT)
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    // initialization
    auto pi = pos[ii];
    const auto vi = vel[ii];

    pi.x = std::fma(dt, vi.x, pi.x);
    pi.y = std::fma(dt, vi.y, pi.y);
    pi.z = std::fma(dt, vi.z, pi.z);

    pos[ii] = pi;
  }
}

#ifdef CALCULATE_POTENTIAL
double
#else   // CALCULATE_POTENTIAL
void
#endif  // CALCULATE_POTENTIAL
write_snapshot(const int num, position *pos, velocity *vel, velocity *vel_tmp, acceleration *acc, char *file, const int snp_id, const float time, const float dt) {
  // memcpy from device to host
  PRAGMA_ACC_UPDATE_HOST(pos [0:num], vel [0:num], acc [0:num])

  // backward integration for velocity
  const auto dt_2 = std::ldexp(dt, -1);
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    const acceleration ai = acc[ii];
    velocity vi = vel[ii];
    vi.x -= dt_2 * ai.x;
    vi.y -= dt_2 * ai.y;
    vi.z -= dt_2 * ai.z;
    vel_tmp[ii] = vi;
  }

  // write snapshot
  char filename[64];
  sprintf(filename, "dat/%s_snp%03d.dat", file, snp_id);
  FILE *fp;
  fp = fopen(filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "ERROR: failed to open \"%s\"\n", filename);
    exit(1);
  }
  fprintf(fp, "#x y z vx vy vz ax ay ax");
#ifdef CALCULATE_POTENTIAL
  fprintf(fp, " pot");
#endif  // CALCULATE_POTENTIAL
  fprintf(fp, "\n");
  for (int ii = 0; ii < num; ii++) {
    fprintf(fp, "%e %e %e %e %e %e %e %e %e",
            pos[ii].x, pos[ii].y, pos[ii].z,
            vel_tmp[ii].x, vel_tmp[ii].y, vel_tmp[ii].z,
            acc[ii].x, acc[ii].y, acc[ii].z);
#ifdef CALCULATE_POTENTIAL
    fprintf(fp, "%e", acc[ii].w);
#endif  // CALCULATE_POTENTIAL
    fprintf(fp, "\n");
  }
  fclose(fp);

#ifdef CALCULATE_POTENTIAL
  // calculate energy
  double Ekin = 0.0, Epot = 0.0;
  for (std::remove_const_t<decltype(num)> ii = 0; ii < num; ii++) {
    const double mi = pos[ii].w;
    const double pot = acc[ii].w;

    const double vx = vel_tmp[ii].x;
    const double vy = vel_tmp[ii].y;
    const double vz = vel_tmp[ii].z;

    Ekin += mi * (vx * vx + vy * vy + vz * vz);
    Epot += mi * pot;
  }
  Ekin = std::ldexp(Ekin, -1);
  Epot = std::ldexp(Epot, -1);
  const auto Etot = Ekin + Epot;
  const auto Virial = Ekin / (-Epot);

  // write time evolution of energy
  sprintf(filename, "dat/%s_energy.dat", file);
  fp = fopen(filename, (snp_id > 0) ? "a" : "w");
  if (fp == NULL) {
    fprintf(stderr, "ERROR: failed to open \"%s\"\n", filename);
    exit(1);
  }
  if (snp_id == 0) {
    fprintf(fp, "#time Etot Ekin Epot Virial\n");
  }
  fprintf(fp, "%e %le %le %le %le\n", time, Etot, Ekin, Epot, Virial);
  fclose(fp);

  return (Etot);
#endif  // CALCULATE_POTENTIAL
}
#endif  // BENCHMARK_MODE

int main(void) {
#ifndef BENCHMARK_MODE
  static char file[16];
  sprintf(file, "collapse");
  const auto num = 1031;        // number of N-body particles, 1031 is the minimum prime number greater than 1024
  const auto eps = 1.5625e-2F;  // Plummer softening
  const auto Mtot = 1.0F;       // total mass
  const auto rad = 1.0F;        // initial-radius of the sphere
  const auto virial = 0.2F;     // initial virial-ratio
  const auto epsinv = 1.0F / eps;

  // set dt etc. (snapshot_interval must be multiply of dt)
  const auto dt = 7.8125e-3F;             // fixed time-step in the simulation
  const auto ft = 10.0F;                  // final time of the simulation
  const auto snapshot_interval = 0.125F;  // interval between snapshots

  // memory allocation
  position *pos;
  velocity *vel, *vel_tmp;
  acceleration *acc;
  allocate_Nbody_particles(&pos, &vel, &vel_tmp, &acc, num);
  PRAGMA_ACC_ENTER_DATA_CREATE(pos [0:num], vel [0:num], acc [0:num])

  // initialize N-body simulation
  auto time = 0.0F;
  auto time_from_snapshot = 0.0F;
  auto step = 0;
  auto present = 0;
  auto previous = present;
  const auto snp_fin = static_cast<decltype(present)>(std::ceil(ft / snapshot_interval));

  // generate initial-condition
#ifdef CALCULATE_POTENTIAL
  double Etot0, Etot;
#endif  // CALCULATE_POTENTIAL
  if (step == 0) {
    set_uniform_sphere(num, pos, vel, Mtot, rad, virial, newton);
    // memcpy from host to device
    PRAGMA_ACC_UPDATE_DEVICE(pos [0:num], vel [0:num])
    calc_acc(num, pos, acc, num, pos, eps);
    trim_acc(num, acc, newton
#ifdef CALCULATE_POTENTIAL
             ,
             pos, epsinv
#endif  // CALCULATE_POTENTIAL
    );
    // write snapshot
#ifdef CALCULATE_POTENTIAL
    Etot0 =
#endif  // CALCULATE_POTENTIAL
        write_snapshot(num, pos, vel, vel_tmp, acc, file, present, time, dt);
    // half-step integration for velocity
    kick(num, vel, acc, std::ldexp(dt, -1));
  }

  while (present < snp_fin) {
    step++;
    time_from_snapshot += dt;
    if (time_from_snapshot >= snapshot_interval) {
      present++;
    }

    // orbit integration (2nd-order leapfrog scheme)
    drift(num, pos, vel, dt);
    calc_acc(num, pos, acc, num, pos, eps);
    trim_acc(num, acc, newton
#ifdef CALCULATE_POTENTIAL
             ,
             pos, epsinv
#endif  // CALCULATE_POTENTIAL
    );
    kick(num, vel, acc, dt);

    // write snapshot
    if (present > previous) {
      previous = present;
      time_from_snapshot = 0.0F;
      time += snapshot_interval;
#ifdef CALCULATE_POTENTIAL
      Etot =
#endif  // CALCULATE_POTENTIAL
          write_snapshot(num, pos, vel, vel_tmp, acc, file, present, time, dt);
    }
  }
#ifdef CALCULATE_POTENTIAL
  static char filename[64];
  sprintf(filename, "log/%s_error.dat", file);
  FILE *fp;
  fp = fopen(filename, "a");
  if (fp == NULL) {
    fprintf(stderr, "ERROR: failed to open \"%s\"\n", filename);
    exit(1);
  }
  fprintf(fp, "%e %e\n", dt, Etot / Etot0 - 1.0);
  fclose(fp);
#endif  // CALCULATE_POTENTIAL

  // memory deallocation
  PRAGMA_ACC_EXIT_DATA_DELETE(pos [0:num], vel [0:num], acc [0:num])
  release_Nbody_particles(pos, vel, vel_tmp, acc);
#else  // BENCHMARK_MODE
  const float Mtot = 1.0F;
  const float rad = 1.0F;
  const float virial = 0.2F;
  // measure performance
  static char filename[64];
  sprintf(filename, "log/nbody_acc_benchmark_%s%d.csv", COMPILER, MODEL_ID);
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
    fprintf(fp, "Model_ID");
    fprintf(fp, ",T_tot");
    fprintf(fp, ",N,time(s),N_pairs/s,TFlop/s\n");
  }

#ifndef BENCHMARK_SADDLE_REGION
  for (int num = NMIN; num <= NMAX; num <<= 1) {
#else   // BENCHMARK_SADDLE_REGION
  for (int num = NMAX_SADDLE; num >= NMIN_SADDLE; num -= NINC_SADDLE) {
#endif  // BENCHMARK_SADDLE_REGION
    // memory allocation
    position *pos;
    velocity *vel, *vel_tmp;
    acceleration *acc;
    allocate_Nbody_particles(&pos, &vel, &vel_tmp, &acc, num);
    PRAGMA_ACC_ENTER_DATA_CREATE(pos [0:num], vel [0:num], acc [0:num])

    // generate initial-condition
    set_uniform_sphere(num, pos, vel, Mtot, rad, virial, newton);
    // memcpy from host to device
    PRAGMA_ACC_UPDATE_DEVICE(pos [0:num], vel [0:num])
    struct timespec ini;
    struct timespec end;
    const int iter = NMAX_CRIT / ((num < NMAX_CRIT) ? num : NMAX_CRIT);

    // launch benchmark
    clock_gettime(CLOCK_MONOTONIC, &ini);

    // calculate gravitational acceleration
    for (int loop = 0; loop < iter; loop++) {
      calc_acc(num, pos, acc, num, pos, 4.0F * rad / static_cast<float>(num));
    }

    // finalize benchmark
    clock_gettime(CLOCK_MONOTONIC, &end);

    // record measured performance
    const auto elapsed = std::fma(1.0e-9, static_cast<double>(end.tv_nsec - ini.tv_nsec), end.tv_sec - ini.tv_sec) / static_cast<double>(iter);
    const auto Npairs = static_cast<decltype(elapsed)>(num) * static_cast<decltype(elapsed)>(num);
    fprintf(fp, "%d", MODEL_ID);
    fprintf(fp, ",%d", NTHREADS);
    fprintf(fp, ",%d,%e,%e,%e\n", num, elapsed, Npairs / elapsed, 1.0e-12 * FLOPS_COUNT * Npairs / elapsed);

    // memory deallocation
    PRAGMA_ACC_EXIT_DATA_DELETE(pos [0:num], vel [0:num], acc [0:num])
    release_Nbody_particles(pos, vel, vel_tmp, acc);
  }
  fclose(fp);
#endif  // BENCHMARK_MODE

  return (0);
}
