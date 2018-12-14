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
 * @file "modules/yf_practicemodule/yf_practicemodule.c"
 * @author Yingfu Xu
 * For YF's first attempt to create a new module. Its main function is to guide the Bebop to fly pre-planned trajectory's in the CyberZoo using Optitrack or optical flow.
 */

#include "modules/yf_practicemodule/yf_practicemodule.h"
#include <stdio.h>

void time_printer_init ()
{
    sys_time_init();
    

    if(get_sys_time_float() > 5)
    {
        printf("Time printer initialized! \n");
        // autopilot_set_motors_on(true);  // start engine 
        // waypoint_set_here_2d(STDBY); //take off?
    }
    timeCounterLast = 0;
}

void clock_time_printer ()
{
    systemTime = get_sys_time_float();
    timeCounter = systemTime / 5;
    printf("System Time: %d\n",systemTime);
    printf("5 Second Counter = %d\n",timeCounter);
    if(timeCounter > timeCounterLast)
    {
        // autopilot_set_motors_on(true);
        autopilot_guided_goto_body_relative(0.0, 0.0, -0.2, 0.0/180.0*3.1415926); // go up
        // autopilot_guided_move_ned(0.5, 0.5, 0.0, 0.0/180.0*3.1415926);
        printf("Moving ... \n");
        timeCounterLast = timeCounter;
    }
    printf("PositionNed X = %f\n",stateGetPositionNed_f()->x);
    printf("PositionNed Y = %f\n",stateGetPositionNed_f()->y);
    printf("PositionNed Z = %f\n",stateGetPositionNed_f()->z);



}
void timeout_ending () 
{
    // if(systemTime >= 10)
    {
        // printf("10 seconds! \n \n");
    }
}


