/*
 * Copyright (C) 2014 Hann Woei Ho
 *               2015 Freek van Tienen <freek.v.tienen@gmail.com>
 *
 * This file is part of Paparazzi.
 *
 * Paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file modules/guidance_opticflow/guidance_opticflow_hover.c
 * @brief Optical-flow based control for Linux based systems
 *
 * Control loops for optic flow based hovering.
 * Computes setpoint for the lower level attitude stabilization to control horizontal velocity.
 */

// Own Header
#include "guidance_opticflow_hover.h"

#include "subsystems/abi.h"

// Stabilization
#include "firmwares/rotorcraft/stabilization/stabilization_attitude.h"
#include "firmwares/rotorcraft/guidance/guidance_v.h"
#include "autopilot.h"
#include "subsystems/datalink/downlink.h"

/** Default sender to accect VELOCITY_ESTIMATE messages from */
#ifndef VISION_VELOCITY_ESTIMATE_ID 
#define VISION_VELOCITY_ESTIMATE_ID 1  // OPTICFLOW_SEND_ABI_ID  // ABI_BROADCAST  //Yingfu Testing
#endif
PRINT_CONFIG_VAR(VISION_VELOCITY_ESTIMATE_ID)

#define CMD_OF_SAT  1500 // 40 deg = 2859.1851

#ifndef VISION_PHI_PGAIN
#define VISION_PHI_PGAIN 400
#endif
PRINT_CONFIG_VAR(VISION_PHI_PGAIN)

#ifndef VISION_PHI_IGAIN
#define VISION_PHI_IGAIN 20
#endif
PRINT_CONFIG_VAR(VISION_PHI_IGAIN)

#ifndef VISION_THETA_PGAIN
#define VISION_THETA_PGAIN 400
#endif
PRINT_CONFIG_VAR(VISION_THETA_PGAIN)

#ifndef VISION_THETA_IGAIN
#define VISION_THETA_IGAIN 20
#endif
PRINT_CONFIG_VAR(VISION_THETA_IGAIN)

#ifndef VISION_DESIRED_VX
#define VISION_DESIRED_VX 0
#endif
PRINT_CONFIG_VAR(VISION_DESIRED_VX)

#ifndef VISION_DESIRED_VY
#define VISION_DESIRED_VY 0
#endif
PRINT_CONFIG_VAR(VISION_DESIRED_VY)

/* Check the control gains */
#if (VISION_PHI_PGAIN < 0)      ||  \
  (VISION_PHI_IGAIN < 0)        ||  \
  (VISION_THETA_PGAIN < 0)      ||  \
  (VISION_THETA_IGAIN < 0)
#error "ALL control gains have to be positive!!!"
#endif

static abi_event velocity_est_ev;

/* Initialize the default gains and settings */
struct opticflow_stab_t opticflow_stab = {
  .phi_pgain = VISION_PHI_PGAIN,
  .phi_igain = VISION_PHI_IGAIN,
  .theta_pgain = VISION_THETA_PGAIN,
  .theta_igain = VISION_THETA_IGAIN,
  .desired_vx = VISION_DESIRED_VX,
  .desired_vy = VISION_DESIRED_VY
};


static void stabilization_opticflow_vel_cb(uint8_t sender_id __attribute__((unused)),
    uint32_t stamp, float vel_x, float vel_y, float vel_z, float noise);

/* Yingfu Practice */
static void velocity_following_opticflow_vel_cb(uint8_t sender_id __attribute__((unused)),
    uint32_t stamp, float vel_x, float vel_y, float vel_z, float noise);

/**
 * Initialization of horizontal guidance module.
 */
void guidance_h_module_init(void)
{
  // Subscribe to the VELOCITY_ESTIMATE ABI message  // to define the "velocity_est_ev", which is a "abi_event", by ID (VISION_VELOCITY_ESTIMATE_ID) and CallBack function (stabilization_opticflow_vel_cb).
  AbiBindMsgVELOCITY_ESTIMATE(VISION_VELOCITY_ESTIMATE_ID, &velocity_est_ev, stabilization_opticflow_vel_cb);  //define a callback function, stabilization_opticflow_vel_cb, when there is velocity_est_ev signal come in, run the callback function.
  // Yingfu Testing: Run AbiBindMsgVELOCITY_ESTIMATE Only Once
  //AbiBindMsgVELOCITY_ESTIMATE(VISION_VELOCITY_ESTIMATE_ID, &velocity_est_ev, velocity_following_opticflow_vel_cb);
}

/**
 * Horizontal guidance mode enter resets the errors
 * and starts the controller.
 */
void guidance_h_module_enter(void)
{
  /* Reset the integrated errors */
  opticflow_stab.err_vx_int = 0;
  opticflow_stab.err_vy_int = 0;

  /* Set rool/pitch to 0 degrees and psi to current heading */
  opticflow_stab.cmd.phi = 0;
  opticflow_stab.cmd.theta = 0;
  opticflow_stab.cmd.psi = stateGetNedToBodyEulers_i()->psi;
}

/**
 * Read the RC commands
 */
void guidance_h_module_read_rc(void)
{
  // TODO: change the desired vx/vy
}

/**
 * Main guidance loop
 * @param[in] in_flight Whether we are in flight or not
 */
void guidance_h_module_run(bool in_flight)
{
  /* Update the setpoint */
  stabilization_attitude_set_rpy_setpoint_i(&opticflow_stab.cmd);

  /* Run the default attitude stabilization */
  stabilization_attitude_run(in_flight);
}

/**
 * Update the controls on a new VELOCITY_ESTIMATE ABI message.
 */
static void stabilization_opticflow_vel_cb(uint8_t sender_id __attribute__((unused)),
    uint32_t stamp, float vel_x, float vel_y, float vel_z, float noise)
{
  /* Check if we are in the correct AP_MODE before setting commands */
  if (autopilot_get_mode() != AP_MODE_MODULE) {
    return;
  }

  /* Calculate the error */
  float err_vx = opticflow_stab.desired_vx - vel_x;
  float err_vy = opticflow_stab.desired_vy - vel_y;

  /* Calculate the integrated errors (TODO: bound??) */
  opticflow_stab.err_vx_int += err_vx / 512;
  opticflow_stab.err_vy_int += err_vy / 512;

  /* Calculate the commands */
  opticflow_stab.cmd.phi   = opticflow_stab.phi_pgain * err_vy
                             + opticflow_stab.phi_igain * opticflow_stab.err_vy_int;
  opticflow_stab.cmd.theta = -(opticflow_stab.theta_pgain * err_vx
                               + opticflow_stab.theta_igain * opticflow_stab.err_vx_int);

  /* Bound the roll and pitch commands */
  BoundAbs(opticflow_stab.cmd.phi, CMD_OF_SAT);
  BoundAbs(opticflow_stab.cmd.theta, CMD_OF_SAT);

  // Yingfu
  // printf("%f  %f CMD!\n", opticflow_stab.cmd.theta, opticflow_stab.cmd.phi /* , err_vx, err_vy */);
  // printf("%f  %f\n", opticflow_stab.cmd.theta/2851.0*40.0, err_vx);
  // printf("%f  %f I!\n", opticflow_stab.theta_igain, opticflow_stab.phi_igain);

}

/**
 * Yingfu Practice: Fly in a time-varying command speed
 */
static void velocity_following_opticflow_vel_cb(uint8_t sender_id __attribute__((unused)),
    uint32_t stamp, float vel_x, float vel_y, float vel_z, float noise)
{
  /* Check if we are in the correct AP_MODE before setting commands */
  if (autopilot_get_mode() != AP_MODE_MODULE) {
    return;
  }

  /* Time varying velocity command */
  vel_x_t = 0.8 * sin(0.2 * 360.0 * get_sys_time_float() / 180.0 * 3.14159);  // 20 second / circle
  vel_y_t = 0; //1.8 * cos(0.1 * 360 * get_sys_time_usec() / 180 * 3.14159);

  /* Calculate the error */
  float err_vx = vel_x_t - vel_x;
  float err_vy = vel_y_t - vel_y;

  /* Calculate the integrated errors (TODO: bound??) */
  opticflow_stab.err_vx_int += err_vx / 512;
  opticflow_stab.err_vy_int += err_vy / 512;

  /* Calculate the commands */
  opticflow_stab.cmd.phi   = opticflow_stab.phi_pgain * err_vy
                             + opticflow_stab.phi_igain * opticflow_stab.err_vy_int;
  opticflow_stab.cmd.theta = -(opticflow_stab.theta_pgain * err_vx
                               + opticflow_stab.theta_igain * opticflow_stab.err_vx_int);

  /* Bound the roll and pitch commands */
  BoundAbs(opticflow_stab.cmd.phi, CMD_OF_SAT);
  BoundAbs(opticflow_stab.cmd.theta, CMD_OF_SAT);

  // Yingfu
  // printf("%f  %f CMD!\n", opticflow_stab.cmd.theta, opticflow_stab.cmd.phi /* , err_vx, err_vy */);
  printf("%f  %f \n", opticflow_stab.cmd.theta/2851.0*40.0, vel_x_t);
  // printf("%f  %f I!\n", opticflow_stab.theta_igain, opticflow_stab.phi_igain);

}
