#include "Utilities.h"
#include "Solver.h"
#include <float.h>

extern struct Sim_Struct sim_struct;

size_t TIMESTEP = 0;
size_t x;
size_t y;
size_t left;
size_t right;
double res_sum;

void check_residual() {
  res_sum = 0.;
  for (int i=0; i<sim_struct.grid_size_x*sim_struct.grid_size_y; i++) {
    res_sum += fabs(sim_struct.u[i] - sim_struct.residual[i]);
    sim_struct.residual[i] = sim_struct.u[i];
  }

  std::cout << "Residual at time " << TIMESTEP << " " << res_sum << std::endl;
  
  if (res_sum < sim_struct.tolerance) {
    save_speed_to_file();
    std::cout << "Convergence found. Exiting.\n";
    exit(0);
  }

}

void run_sim_timestep() {

  sim_struct.u_max = -DBL_MAX;
  sim_struct.u_min = DBL_MAX;

  // Calculating E and F stencil quantities
  // backwards differences used here because forward is used in stencil
  for (x=0; x<sim_struct.grid_size_x; x++) {
    for (y=0; y<sim_struct.grid_size_y; y++) {

      // hack for periodic boundary conditions in x-direction
      left = x-1;
      right = (x+1) % sim_struct.grid_size_x;
      if (x == 0) {
        left = sim_struct.grid_size_x-1;
      }

      sim_struct.E2[s_i(x,y)] = sim_struct.rho[s_i(x,y)] * pow(sim_struct.u[s_i(x,y)], 2.) + sim_struct.rho[s_i(x,y)] * pow(sim_struct.c, 2.) - (2./3.) * sim_struct.mu * (2. * (sim_struct.u[s_i(x,y)] - sim_struct.u[s_i(left,y)]) / sim_struct.dx - (sim_struct.v[s_i(x,y+1)] - sim_struct.v[s_i(x,y-1)]) / (2.*sim_struct.dy));
      sim_struct.E3[s_i(x,y)] = sim_struct.rho[s_i(x,y)] * sim_struct.u[s_i(x,y)] * sim_struct.v[s_i(x,y)] - sim_struct.mu * ((sim_struct.v[s_i(x,y)] - sim_struct.v[s_i(left,y)]) / sim_struct.dx + (sim_struct.u[s_i(x,y+1)] - sim_struct.u[s_i(x,y-1)]) / (2.*sim_struct.dy));

      sim_struct.F2[s_i(x,y)] = sim_struct.rho[s_i(x,y)] * sim_struct.u[s_i(x,y)] * sim_struct.v[s_i(x,y)] - sim_struct.mu * ((sim_struct.u[s_i(x,y)] - sim_struct.u[s_i(x,y-1)]) / sim_struct.dy + (sim_struct.v[s_i(right,y)] - sim_struct.v[s_i(left,y)]) / (2.*sim_struct.dx));
      sim_struct.F3[s_i(x,y)] = sim_struct.rho[s_i(x,y)] * pow(sim_struct.v[s_i(x,y)], 2.) + sim_struct.rho[s_i(x,y)] * pow(sim_struct.c, 2.) - (2./3.) * sim_struct.mu * (2. * (sim_struct.v[s_i(x,y)] - sim_struct.v[s_i(x,y-1)]) / sim_struct.dy - (sim_struct.u[s_i(right,y)] - sim_struct.u[s_i(left,y)]) / (2.*sim_struct.dx));

    }
  }

  // Calculating predictor step and temporary velocities
  for (x=0; x<sim_struct.grid_size_x; x++) {
    for (y=0; y<sim_struct.grid_size_y; y++) {

      right = (x+1) % sim_struct.grid_size_x;

      // boundary conditions at walls
      if (y == 0) {
        sim_struct.rho_s[s_i(x,y)] = sim_struct.rho[s_i(x,y)] - (sim_struct.dt/(2.*sim_struct.dx)) * (-sim_struct.rho_v[s_i(x,y+2)] + 4.*sim_struct.rho_v[s_i(x,y+1)] - 3.*sim_struct.rho_v[s_i(x,y)]);
      } else if (y == sim_struct.grid_size_y-1) {
        sim_struct.rho_s[s_i(x,y)] = sim_struct.rho[s_i(x,y)] - (sim_struct.dt/(2.*sim_struct.dx)) * -(-sim_struct.rho_v[s_i(x,y-2)] + 4.*sim_struct.rho_v[s_i(x,y-1)] - 3.*sim_struct.rho_v[s_i(x,y)]);
      } else {
        sim_struct.rho_s[s_i(x,y)] = sim_struct.rho[s_i(x,y)] - (sim_struct.dt/sim_struct.dx) * (sim_struct.rho_u[s_i(right,y)] - sim_struct.rho_u[s_i(x,y)]) - (sim_struct.dt/sim_struct.dy) * (sim_struct.rho_v[s_i(x,y+1)] - sim_struct.rho_v[s_i(x,y)]);
        sim_struct.rho_u_s[s_i(x,y)] = sim_struct.rho_u[s_i(x,y)] - (sim_struct.dt/sim_struct.dx) * (sim_struct.E2[s_i(right,y)] - sim_struct.E2[s_i(x,y)]) - (sim_struct.dt/sim_struct.dy) * (sim_struct.F2[s_i(x,y+1)] - sim_struct.F2[s_i(x,y)]) + sim_struct.force*sim_struct.dt;
        sim_struct.rho_v_s[s_i(x,y)] = sim_struct.rho_v[s_i(x,y)] - (sim_struct.dt/sim_struct.dx) * (sim_struct.E3[s_i(right,y)] - sim_struct.E3[s_i(x,y)]) - (sim_struct.dt/sim_struct.dy) * (sim_struct.F3[s_i(x,y+1)] - sim_struct.F3[s_i(x,y)]);
      }

      sim_struct.u_s[s_i(x,y)] = sim_struct.rho_u_s[s_i(x,y)] / sim_struct.rho_s[s_i(x,y)];
      sim_struct.v_s[s_i(x,y)] = sim_struct.rho_v_s[s_i(x,y)] / sim_struct.rho_s[s_i(x,y)];

    }
  }

  // Calculating E and F star quantities
  // forward differences here because backward used in stencil
  for (x=0; x<sim_struct.grid_size_x; x++) {
    for (y=0; y<sim_struct.grid_size_y; y++) {

      // hack for periodic boundary conditions in x-direction
      left = x-1;
      right = (x+1) % sim_struct.grid_size_x;
      if (x == 0) {
        left = sim_struct.grid_size_x-1;
      }

      sim_struct.E2_s[s_i(x,y)] = sim_struct.rho_s[s_i(x,y)] * pow(sim_struct.u_s[s_i(x,y)], 2.) + sim_struct.rho_s[s_i(x,y)] * pow(sim_struct.c, 2.) - (2./3.) * sim_struct.mu * (2. * (sim_struct.u_s[s_i(right,y)] - sim_struct.u_s[s_i(x,y)]) / sim_struct.dx - (sim_struct.v_s[s_i(x,y+1)] - sim_struct.v_s[s_i(x,y-1)]) / (2.*sim_struct.dy));
      sim_struct.E3_s[s_i(x,y)] = sim_struct.rho_s[s_i(x,y)] * sim_struct.u_s[s_i(x,y)] * sim_struct.v_s[s_i(x,y)] - sim_struct.mu * ((sim_struct.v_s[s_i(right,y)] - sim_struct.v_s[s_i(x,y)]) / sim_struct.dx + (sim_struct.u_s[s_i(x,y+1)] - sim_struct.u_s[s_i(x,y-1)]) / (2.*sim_struct.dy));

      sim_struct.F2_s[s_i(x,y)] = sim_struct.rho_s[s_i(x,y)] * sim_struct.u_s[s_i(x,y)] * sim_struct.v_s[s_i(x,y)] - sim_struct.mu * ((sim_struct.u_s[s_i(x,y+1)] - sim_struct.u_s[s_i(x,y)]) / sim_struct.dy + (sim_struct.v_s[s_i(right,y)] - sim_struct.v_s[s_i(left,y)]) / (2.*sim_struct.dx));
      sim_struct.F3_s[s_i(x,y)] = sim_struct.rho_s[s_i(x,y)] * pow(sim_struct.v_s[s_i(x,y)], 2.) + sim_struct.rho_s[s_i(x,y)] * pow(sim_struct.c, 2.) - (2./3.) * sim_struct.mu * (2. * (sim_struct.v_s[s_i(x,y+1)] - sim_struct.v_s[s_i(x,y)]) / sim_struct.dy - (sim_struct.u_s[s_i(right,y)] - sim_struct.u_s[s_i(left,y)]) / (2.*sim_struct.dx));

    }
  }

  // Final corrector step
  for (x=0; x<sim_struct.grid_size_x; x++) {
    for (y=0; y<sim_struct.grid_size_y; y++) {

      left = x-1;
      if (x == 0) {
        left = sim_struct.grid_size_x-1;
      }

      // boundary conditions at walls
      if (y == 0) {
        sim_struct.rho_u[s_i(x,y)] = 0.;
        sim_struct.rho_v[s_i(x,y)] = 0.;
        sim_struct.rho[s_i(x,y)] = 0.5 * (sim_struct.rho[s_i(x,y)] + sim_struct.rho_s[s_i(x,y)] - (sim_struct.dt/(2.*sim_struct.dx)) * (-sim_struct.rho_v_s[s_i(x,y+2)] + 4.*sim_struct.rho_v_s[s_i(x,y+1)] - 3.*sim_struct.rho_v_s[s_i(x,y)]));
      } else if (y == sim_struct.grid_size_y-1) {
        sim_struct.rho_u[s_i(x,y)] = 0.;
        sim_struct.rho_v[s_i(x,y)] = 0.;
        sim_struct.rho[s_i(x,y)] = 0.5 * (sim_struct.rho[s_i(x,y)] + sim_struct.rho_s[s_i(x,y)] - (sim_struct.dt/(2.*sim_struct.dx)) * -(-sim_struct.rho_v_s[s_i(x,y-2)] + 4.*sim_struct.rho_v_s[s_i(x,y-1)] - 3.*sim_struct.rho_v_s[s_i(x,y)]));
      } else {
        sim_struct.rho[s_i(x,y)] = 0.5 * (sim_struct.rho[s_i(x,y)] + sim_struct.rho_s[s_i(x,y)] - (sim_struct.dt/sim_struct.dx) * (sim_struct.rho_u_s[s_i(x,y)] - sim_struct.rho_u_s[s_i(left,y)]) - (sim_struct.dt/sim_struct.dy) * (sim_struct.rho_v_s[s_i(x,y)] - sim_struct.rho_v_s[s_i(x,y-1)]));
        sim_struct.rho_u[s_i(x,y)] = 0.5 * (sim_struct.rho_u[s_i(x,y)] + sim_struct.rho_u_s[s_i(x,y)] - (sim_struct.dt/sim_struct.dx) * (sim_struct.E2_s[s_i(x,y)] - sim_struct.E2_s[s_i(left,y)]) - (sim_struct.dt/sim_struct.dy) * (sim_struct.F2_s[s_i(x,y)] - sim_struct.F2_s[s_i(x,y-1)]) + sim_struct.force*sim_struct.dt);
        sim_struct.rho_v[s_i(x,y)] = 0.5 * (sim_struct.rho_v[s_i(x,y)] + sim_struct.rho_v_s[s_i(x,y)] - (sim_struct.dt/sim_struct.dx) * (sim_struct.E3_s[s_i(x,y)] - sim_struct.E3_s[s_i(left,y)]) - (sim_struct.dt/sim_struct.dy) * (sim_struct.F3_s[s_i(x,y)] - sim_struct.F3_s[s_i(x,y-1)]));
      }

      sim_struct.u[s_i(x,y)] = sim_struct.rho_u[s_i(x,y)] / sim_struct.rho[s_i(x,y)];
      sim_struct.v[s_i(x,y)] = sim_struct.rho_v[s_i(x,y)] / sim_struct.rho[s_i(x,y)];

      record_speed(x,y);

    }
  }

  if ((TIMESTEP+1) % 100 == 0) {
    check_residual();

  }

  TIMESTEP += 1;
}
