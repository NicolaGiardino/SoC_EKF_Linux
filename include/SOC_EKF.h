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

/***************************************************************************************************
*   FILENAME:  SOC_EKF.h                                                                           *
*                                                                                                  *
*                                                                                                  *
*   PURPOSE:   Library that defines the object Kalman and all its functions                        *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
*   GLOBAL VARIABLES:                                                                              *
*                                                                                                  *
*                                                                                                  *
*   Variable        Type        Description                                                        *
*   --------        ----        -------------------                                                *
*   m               Kalman      Kalman object		                                               *
*                                                                                                  *
*   DEVELOPMENT HISTORY :                                                                          *
*                                                                                                  *
*                                                                                                  *
*   Date          Author            Change Id     Release     Description Of Change                *
*   ----          ------            -------- -    ------      ----------------------               *
*   08-07-2020    N.di Gruttola                     1         Project created				       *
*                  Giardino																		   *
*                                                                                                  *
***************************************************************************************************/

#ifndef SOC_EKF_h
#define SOC_EKF_h

/* Include Global Parameters */

#include "matrix.h"
#include "libthreads.h"

/* Definition of Macros */

/*Let's suppose we have n modules in series each formed by m series in parallel, for a total of n*m cells:
	by defining this we're going to modify our system variables according to this.
	*/

#define PAR 1
#define SER 1

#define Z_IND     	2 * PAR * SER   /*        Index of SOC       */
#define H_IND     	1 * PAR * SER   /*      Hysteresis index     */
#define I_IND     	0 * PAR * SER   /*       Current index       */
#define X_SIZE 	  	3 * PAR * SER   /*    Size of state vector   */
#define Y_SIZE 	  	PAR * SER       /* Size of the output vector */
#define U_SIZE 	  	PAR * SER       /*  Size of the input vector */
#define PARAM_SIZE 	9              /*    Number of parameters   */

/* Indexes of Dynamic Cell  */
#define TIME 		0
#define CURRENT 	1
#define VOLTAGE 	2
#define SOC 		3

/* Indexes of Cell Model Parameters */
#define Q 			0
#define G 			1
#define M 			2
#define M0 			3
#define RC 			4
#define R 			5
#define R0 			6
#define eta 		7
#define TEMP 		8

/* Frequency */
#define DeltaT 		1

/* Indexes of Cell SOC_OCv relationship */
#define OCV 		0
#define OCV0 		1
#define OCVrel 		2
#define SOC 		3
#define SOC0 		4
#define SOCrel 		5
#define dOCV0 		6
#define dOCVrel 	7
#define OvSLenght 	8

/* Paths to Cell Dynamic Data */
#define PATH_T 		"./csv/CellDataTime.csv"
#define PATH_C 		"./csv/CellDataCurrent.csv"
#define PATH_O 		"./csv/CellDataSOC.csv"
#define PATH_V 		"./csv/CellDataVoltage.csv"

/* Debug macros */
#define DEBUG3 		0
#define DEBUG_PRINT 0

/* Kalman variables structure */
typedef struct Kalman
{

	Matrix* x;
	Matrix* Pk;		
	Matrix* Fk;		
	Matrix* Gk;		
	Matrix* Hk;		
	Matrix* Qk;		
	Matrix* Rk;		
	Matrix* OvS;
	Matrix *Param;
	Matrix *D;
	Matrix *Kk;
	Matrix *Sk;
	Matrix *y_p;

} Kalman;


/* Declare Prototypes */

void vSetup		(Kalman *, const float, const float*);
void vEKF_Step1	(Kalman *, float *, const float);
void vEKF_Step2	(Kalman *, float *, const float);
void vDelete	(Kalman *);


#endif /* SOC_EKF_h */
