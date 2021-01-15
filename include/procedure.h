/****************************************************************************************
* This file is part of The SoC_EKF_Linux Project.                                       *
*                                                                                       *
* Copyright � 2020-2021 By Nicola di Gruttola Giardino. All rights reserved.           * 
* @mail: nicoladgg@protonmail.com                                                       *
*                                                                                       *
* SoC_EKF_Linux is free software: you can redistribute it and/or modify                 *
* it under the terms of the GNU General Public License as published by                  *
* the Free Software Foundation, either version 3 of the License, or                     *
* (at your option) any later version.                                                   *
*                                                                                       *
* SoC_EKF_Linux is distributed in the hope that it will be useful,                      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                         *
* GNU General Public License for more details.                                          *
*                                                                                       *
* You should have received a copy of the GNU General Public License                     *
* along with The SoC_EKF_Linux Project.  If not, see <https://www.gnu.org/licenses/>.   *
*                                                                                       *
* In case of use of this project, I ask you to mention me, to whom it may concern.      *
*****************************************************************************************/
#ifndef PROCEDURE_H
#define PROCEDURE_H


/* Include Global Parameters */
#include "SOC_EKF.h"

/* Definition of Macros */

#define SOC_RANGE           0.05f   /* Maximum difference in SoC between two cells */

#define NTHREADS			2       /* Number of threads to be created */
#define KALMAN              0           /* Kalman thread index */
#define END                 1            /* End thread index */

#define RASPI_SOC           1

#if RASPI_SOC

#include <wiringPi.h>
#include <lcd.h>

/* USE WIRINGPI PIN NUMBERS */
#define LCD_RS              25          /* Register select pin */
#define LCD_E               24             /* Enable Pin */
#define LCD_D4              23             /* Data pin D4 */
#define LCD_D5              22             /* Data pin D5 */
#define LCD_D6              21             /* Data pin D6 */
#define LCD_D7              14             /* Data pin D7 */

#endif

/* Declare Global Variables */

struct KalmanForThread                  /* Structure containing the argument to be passed to the threads */
{

    Kalman  k;                          /* Kalman structure definition */
    struct threads thread[NTHREADS];    /* Thread strucutre definition */

};

/* Declare Static Variables */

static int exit_threads = 0;
static int lcdIndex;

/* Declare Prototypes */

void vKill_handler();
int iGetExit();

/* Threads Prototypes */
void* pvKalmanThread(void* arg);
void* pvEndThread(void* arg);

/* Periodic and Aperiodic Functions*/
void vKalmanLoop(Kalman* k, int s);
void vEndLoop(int s);

#ifdef RASPI_SOC
/* Define lcd 16x2 display functions */
void vInitLCD();
void vPrintLCD(float val[2]);
#endif

#endif
