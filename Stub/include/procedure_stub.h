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
#ifndef PROCEDURE_STUB_H
#define PROCEDURE_STUB_H


/* Include Global Parameters */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../../include/SOC_EKF.h"


/* Declare Global Variables */

static int       Temp = 25;
static __u16	 quit = 0;
static float     val[1500];

/* Declare Prototypes */

void quit_handler();
int16_t getQuit();

void  vStubSendData (int s, size_t i, Matrix* input);
void  vStubEnd  	(int s);

void* pvStubThread	(void* args);

void vInitLoggerStub(float store);
void vStoreDataStub(float store, size_t index);
void vFinalStoreStub();

#endif