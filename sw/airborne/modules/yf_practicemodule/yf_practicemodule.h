/*
 * Copyright (C) Yingfu Xu
 *
 * This file is part of paparazzi
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file "modules/yf_practicemodule/yf_practicemodule.h"
 * @author Yingfu Xu
 * For YF's first attempt to create a new module. Its main function is to guide the Bebop to fly pre-planned trajectory's in the CyberZoo using Optitrack or optical flow.
 */

// For timer
#include "mcu_periph/sys_time.h"
#include "mcu.h"  

// For Guided
#include "firmwares/rotorcraft/autopilot_guided.h"
#include "autopilot.h"
#include "firmwares/rotorcraft/guidance.h"
#include "state.h"

#include "navigation.h"

#ifndef YF_PRACTICEMODULE_H
#define YF_PRACTICEMODULE_H

float systemTime;
int timeCounter;
int timeCounterLast;

extern void time_printer_init ();
extern void clock_time_printer();
extern void timeout_ending ();

#endif

