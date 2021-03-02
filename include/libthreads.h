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
#ifndef LIBTHREADS_H
#define LIBTHREADS_H

/***************************************************************************************************
*   FILENAME:  libthreads.h                                                                        *
*                                                                                                  *
*                                                                                                  *
*   PURPOSE:   Library that defines the function for thread creation and usage, also CAN 		   *
				interface usage, timing and datalogging						                       *
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
/* Include Global Parameters */

#include <sys/syscall.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/* Definition of Macros */

#define MUT		    		0b010
#define MUT_COND    		0b011
#define MUT_PI      		0b110
#define MUT_COND_PI 		0b111

#define NS_PER_MS  			1000000
#define NS_PER_US  			1000
#define NS_PER_SEC 			1000000000
#define US_PER_MS  			1000
#define US_PER_SEC 			1000000
#define MS_PER_SEC 			1000

#define CAN_IFACE 			"vcan0"

#ifndef CAN_MAX_DLEN
#define CAN_MAX_DLEN 		8
#endif

#define CAN_MAX_DLEN_16		4
#define CAN_MAX_DLEN_64		1

/* Definition of CAN ID Masks */

#define MASTER      		0b10000000000
#define SLAVE       		0b00000000000

#define DATA_MSG			0b00100000000
#define END_MSG				0b01000000000

#define RTS 				0b00000100000
#define CTS  				0b00001000000

#define SLAVEADDR			0b00000000001

#define SLAVE_MASK  		0b11111100000
#define CAN_MASK			0b11111111111 /* 0x7FF */

/* Definition of CAN data indexes */
#define CELL1			  	0
#define CELL2				1
#define CURR1				2
#define CURR2				3
#define VOLT1				0
#define VOLT2 				1
#define TMP1 				2
#define TMP2				3

/* For debugging purposes */
#define DEBUG				0
#define DEBUG2				0
#define DEBUG4 				0

/* Error returned from functions */
#define SAFE_PFUNC(op)                                                   \
	do                                                                   \
	{                                                                    \
		int ret = (op);                                                  \
		if (ret != 0)                                                    \
		{                                                                \
			printf("Test %s unresolved: got %i (%s) on line %i\n  %s\n", \
				   __FILE__, ret, strerror(ret), __LINE__, #op);         \
			fflush(stdout);                                              \
			exit(-1);                                                    \
		}                                                                \
	} while (0)

#define SAFE_FUNC(op)                                                    \
	({                                                                   \
		int ret = (op);                                                  \
		if (ret == -1)                                                   \
		{                                                                \
			printf("Test %s unresolved: got %i (%s) on line %i\n  %s\n", \
				   __FILE__, ret, strerror(errno), __LINE__, #op);       \
			fflush(stdout);                                              \
			exit(-1);                                                    \
		}                                                                \
		ret;                                                             \
	})
	
/*
 *  Assertions shall be used to perform basic sanity checks throughout the code.
 *	All functions of more than 10 lines should have at least one assertion
 *		if (!c_assert(p >= 0) == true)
 *			return ERROR;
 *  __FILE__ and __LINE__ are predefined by the macro preprocessor to produce
 *	the filename and line-number of the failing assertion. The syntax #e turns
 *	the assertion condition e into a string that is printed as part of the error message
 */

#define c_assert(e) ((e) ? (1) : \
	printf("%s,%d: assertion '%s' failed\n", \
	__FILE__, __LINE__, #e))

/* Declare Global Variables */

/* Structure containing all necessary variables for threads */
struct threads 
{

	int 			type;							/* Type of thread, see macros */
	pthread_t 		pthread;							/* pthread identifier */
	pthread_attr_t  attr;								/* pthread attribute */
	pthread_mutex_t mutex;								/* mutex identifier */
	pthread_cond_t  cond;						  /* condition variable identifier */
	int 		    priority;						 /* Priority of the thread */
	int 			policy;				/* Scheduling policy (SCHED_FIFO, SCHED_RR, SCHED_DEADLINE) */
	int 			flags;								/* Thread flags */
	int 			id;					 
	void*			args;					   /* Argument to be passed to the thread */
	void*			(*func) (void*);				   /* Thread function */

};

/* Structure containing variables for periodic functions */
struct period_info
{

	struct timespec next_period;
	long period_ns;

};

/* Can structure w/ 16bits data */
struct can_frame16
{

	canid_t     can_id; 						/* 32 bit CAN_ID + EFF/RTR/ERR flags */
	__u8 	    dlc;					/* frame payload length in byte (0 .. CAN_MAX_DLEN) */
	__u8 		__pad;										/* padding */
	__u8 		__res0;								  /* reserved / padding */
	__u8 		__res1;								  /* reserved / padding */
	__u16       data[CAN_MAX_DLEN_16] __attribute__((aligned(8)));

};

/* Can structure w/ 64bits data */
struct can_frame64
{

	canid_t 	can_id; 						/* 32 bit CAN_ID + EFF/RTR/ERR flags */
	__u8 	    dlc;					/* frame payload length in byte (0 .. CAN_MAX_DLEN) */
	__u8 		__pad;										/* padding */
	__u8 		__res0;								/* reserved / padding */
	__u8 		__res1;								/* reserved / padding */
	__u64   	data __attribute__((aligned(8)));
	
};

/* Union to convert floats to u32, u16 and u8 */
typedef union
{

	__u32  u32;
	__u16  u16[2];
	__u8   u8[4];
	float  f;

} b32data;

/* Union to convert ints to u32, u16 and u8 */
typedef union
{

	__u8  u8[4];
	__u16 u16[2];
	__u32 u32;
	int n;

} b32int;

/* Datalogging structure */
struct DataLogger
{

 	float values[2][1500];

};

/* Singleton for datalogging */
static struct DataLogger data;

/* Declare Prototypes */

void vPeriodic_task_init(struct period_info* pinfo);
void vInc_period(struct period_info* pinfo, struct timespec *passed_ns);
void vWait_rest_of_period(struct period_info *pinfo, struct timespec *passed_ns);
void vTs_minus(struct timespec *ts_end, struct timespec *ts_start, struct timespec *ts_delta);
void vTs_plus(struct timespec *ts_a, struct timespec *ts_b, struct timespec *ts_sum);
void vTs_normalize(struct timespec *ts);
long lRt_gettime();

void vInitLogger(float val[]);
void vStoreData(float val[], size_t index);
void vFinalStore();

void vLock_memory();
int  iCreate_thread(struct threads* thread);
int  iCreate_thread_ss(struct threads* thread);
void vSet_pthread_priority(struct threads *thread, const int priority);
void vJoin_Threads(struct threads *all_threads, __u32 n);
void vInit_mutex(struct threads* thread);
void vInit_mutex_cond(struct threads* thread);
void vDestroy_all(struct threads* thread);

int  iInit_can();
void vBind_can(int s, struct can_filter rfilter, int hasFilter);
void vRcv_can(int s, struct can_frame* frame);
void vRcv_can16(int s, struct can_frame16 *frame);
void vRcv_can64(int s, struct can_frame64* frame);
void vSnd_can(int s, const struct can_frame *frame);
void vSnd_can16(int s, const struct can_frame16 *frame);
void vSnd_can64(int s, const struct can_frame64 *frame);
void vDestroy_can(int s);

#endif
