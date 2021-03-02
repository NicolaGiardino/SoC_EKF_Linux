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
/****************************************************************************************************
* FILE NAME: libthreads.c                                                                           *
*                                                                                                   *
* PURPOSE: This library gives all the tools to implement threads		                            *
*                                                                                                   *
* FILE REFERENCES:                                                                                  *
*                                                                                                   *
*   Name    I/O     Description                                                                     *
*   ----    ---     -----------                                                                     *
*   none                                                                                            *
*                                                                                                   *
*                                                                                                   *
* EXTERNAL VARIABLES:                                                                               *
*                                                                                                   *
* Source: <libthreads.h>                                                                            *
*                                                                                                   *
* Name          Type    IO Description                                                              *
* ------------- ------- -- -----------------------------                                            *
*                                                                                                   *
*                                                                                                   *
* STATIC VARIABLES:                                                                                 *
*                                                                                                   *
*   Name     Type       I/O      Description                                                        *
*   ----     ----       ---      -----------                                                        *
*   none                                                                                            *
*                                                                                                   *
* EXTERNAL REFERENCES:                                                                              *
*                                                                                                   *
*  Name                       Description                                                           *
*  -------------              -----------                                                           *
*  none                                                                                             *
*                                                                                                   *
* ABNORMAL TERMINATION CONDITIONS, ERROR AND WARNING MESSAGES:                                      *
*    none, compliant with the standard ISO9899:1999                                                 *
*                                                                                                   *
* ASSUMPTIONS, CONSTRAINTS, RESTRICTIONS: tbd                                                       *
*                                                                                                   *
* NOTES: see documentations                                                                         *
*                                                                                                   *
* REQUIREMENTS/FUNCTIONAL SPECIFICATIONS REFERENCES:                                                *
*                                                                                                   *
* DEVELOPMENT HISTORY:                                                                              *
*                                                                                                   *
*   Date          Author            Change Id     Release     Description Of Change                 *
*   ----          ------            ---------     ------      ----------------------                *
*   28-12-2020    N.di Gruttola                    1          V1 Created					        *
*                  Giardino																		    *
*                                                                                                   *
*                                                                                                   *
*                                                                                                   *
****************************************************************************************************/

/* Include Global Parameters */

#include "../include/libthreads.h"

/********************************************************************************
*                                                                               *
* FUNCTION NAME: lRt_gettime                                                    *
*                                                                               *
* PURPOSE: Return the current clock time using CLOCK_MONOTONIC				    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
*                                                                               *
* RETURN VALUE: long                                                            *
*                                                                               *
********************************************************************************/

long lRt_gettime()
{
 /* LOCAL VARIABLES:
  * Variable      Type           	Description
  * ------------- -------        	---------------
  * rc            int	         	Control variable 
  * ts			  struct timespec 	struct that contains the actual time
  */
	struct timespec ts;
	int rc;

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (rc != 0) 
	{
		perror("clock_gettime() failed");
		return 0;
	}

	return ts.tv_nsec;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vInc_period                                                    *
*                                                                               *
* PURPOSE: Increment the period until total time to wait is calculated          *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* pinfo     struct period_info*     IO     Standard period to wait              *
* passed_ns	struct timespec*		I	   Time passed since start of thread    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vInc_period(struct period_info* pinfo, struct timespec* passed_ns)
{
    pinfo->next_period.tv_nsec += pinfo->period_ns - passed_ns->tv_nsec;

    while (pinfo->next_period.tv_nsec >= 1000000000)
    {
        /* timespec nsec overflow */
        pinfo->next_period.tv_sec++;
        pinfo->next_period.tv_nsec -= 1000000000;
    }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vPeriodic_task_init                                            *
*                                                                               *
* PURPOSE: Init the wait period and get actual time                             *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* pinfo     struct period_info*     IO     Standard period to wait              *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vPeriodic_task_init(struct period_info* pinfo)
{

	/* for simplicity, if no period is set, hardcoding a 15ms period */
	if(pinfo->period_ns == 0)
		pinfo->period_ns = 15*NS_PER_MS;

	clock_gettime(CLOCK_MONOTONIC, &(pinfo->next_period));
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vWait_rest_of_period                                           *
*                                                                               *
* PURPOSE: Used as a realtime sleep function                                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* pinfo     struct period_info*     IO     Standard period to wait              *
* passed_ns	struct timespec*		I	   Time passed since start of thread    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vWait_rest_of_period(struct period_info* pinfo, struct timespec* passed_ns)
{
    vInc_period(pinfo, passed_ns);

    /* for simplicity, ignoring possibilities of signal wakes */
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &pinfo->next_period, NULL);
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vTs_minus                                                      *
*                                                                               *
* PURPOSE: Subtract time                                                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* ts_end	struct timespec*		I	   Time structure					    *
* ts_start	struct timespec*		I	   Time structure					    *
* ts_delta	struct timespec*		I	   Time structure					    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vTs_minus(struct timespec *ts_end, struct timespec *ts_start, struct timespec *ts_delta)
{
	if (ts_end == NULL || ts_start == NULL || ts_delta == NULL) {
		perror("ERROR");
		return;
	}

	ts_delta->tv_sec = ts_end->tv_sec - ts_start->tv_sec;
	ts_delta->tv_nsec = ts_end->tv_nsec - ts_start->tv_nsec;
	vTs_normalize(ts_delta);
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vTs_plus                                                       *
*                                                                               *
* PURPOSE: Sum time 	                                                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* ts_a   	struct timespec*		I	   Time structure					    *
* ts_b		struct timespec*		I	   Time structure					    *
* ts_sum	struct timespec*		I	   Time structure					    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vTs_plus(struct timespec *ts_a, struct timespec *ts_b, struct timespec *ts_sum)
{
	if (ts_a == NULL || ts_b == NULL || ts_sum == NULL) 
	{
		perror("ERROR");
		return;
	}

	ts_sum->tv_sec = ts_a->tv_sec + ts_b->tv_sec;
	ts_sum->tv_nsec = ts_a->tv_nsec + ts_b->tv_nsec;
	vTs_normalize(ts_sum);
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vTs_normalie                                                   *
*                                                                               *
* PURPOSE: 				                                                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* ts		struct timespec*		I	   Time structure					    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vTs_normalize(struct timespec *ts)
{
	if (ts == NULL) {
		/* FIXME: write a real error logging system */
		perror("ERROR");
		return;
	}

	/* get the abs(nsec) < NS_PER_SEC */
	while (ts->tv_nsec > NS_PER_SEC) {
		ts->tv_sec++;
		ts->tv_nsec -= NS_PER_SEC;
	}
	while (ts->tv_nsec < -NS_PER_SEC) {
		ts->tv_sec--;
		ts->tv_nsec += NS_PER_SEC;
	}

	/* get the values to the same polarity */
	if (ts->tv_sec > 0 && ts->tv_nsec < 0) {
		ts->tv_sec--;
		ts->tv_nsec += NS_PER_SEC;
	}
	if (ts->tv_sec < 0 && ts->tv_nsec > 0) {
		ts->tv_sec++;
		ts->tv_nsec -= NS_PER_SEC;
	}
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vInitLogger                                                    *
*                                                                               *
* PURPOSE: Initialize log structure                                             *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* val		float[]					I	   Log values							*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vInitLogger(float val[])
{

	data.values[0][0] = val[0];
	data.values[1][0] = val[1];

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vStoreData                                                     *
*                                                                               *
* PURPOSE: Log data in structure                                                *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* val		float[]					I	   Log values							*
* index		size_t					I	   Index of passed value				*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vStoreData(float val[], size_t index)
{

	data.values[0][index] = val[0];
	data.values[1][index] = val[1];

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vFinalStore                                                    *
*                                                                               *
* PURPOSE: Store data in txt file                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vFinalStore()
{
/* LOCAL VARIABLES:
  * Variable      Type           Description
  * ------------- -------        ---------------
  * i             size_t         Loop counter
  * file	      FILE*          Pointer to the file where infos are stored
  */

	size_t i;
	FILE *file;

#if DEBUG4
		for (size_t j = 0; j < 10; j++)
			printf("%f\n", data.values[1][j]);
#endif

	file = fopen("SOC.txt", "w");
	for (i = 0; i < 1500; i++)
		fprintf(file, "%f\t%d\n", data.values[0][i], i);
	fclose(file);

	file = fopen("Pk.txt", "w");
	for (i = 0; i < 1500; i++)
		fprintf(file, "%f\t%d\n", data.values[1][i], i);
	fclose(file);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vLock_memory                                                   *
*                                                                               *
* PURPOSE: Calls mlockall, locks memory                                         *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vLock_memory()
{
    if (mlockall(MCL_CURRENT | MCL_FUTURE)) 
	{
			perror("failed to lock memory\n");
			exit(1);
	}
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iCreate_thread                                                 *
*                                                                               *
* PURPOSE: Creates real-time thread			                                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* thread	struct threads*			I	   Thread to initialize				    *
*                                                                               *
* RETURN VALUE: int                                                             *
*                                                                               *
********************************************************************************/

int iCreate_thread(struct threads* thread)
{
   /* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * ret           int            		Control variable
    * param			struct sched_param	Thread's parameters structure
    */

	int ret;
	struct sched_param param;
	param.sched_priority = thread->priority;

    pthread_attr_init(&thread->attr);
    pthread_attr_setdetachstate(&thread->attr,PTHREAD_CREATE_JOINABLE);
    pthread_attr_setinheritsched(&thread->attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thread->attr, thread->policy);
    pthread_attr_setschedparam(&thread->attr, &param);

    if ((ret = pthread_create(&thread->pthread, &thread->attr, thread->func, (void*) thread->args))) 
	{
      printf("pthread_create failed: %d (%s)\n", ret, strerror(ret));
      pthread_attr_destroy(&thread->attr);
      return -1;
    }

    /* Sporadic server compatibility to be added
	if (thread->policy == SCHED_SPORADIC)
        ret = iCreate_thread_ss(thread);
	*/

    pthread_attr_destroy(&thread->attr);

    return  ret;

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vJoin_Threads                                                  *
*                                                                               *
* PURPOSE: Joins threads			                                            *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument    Type         			IO     Description                          *
* ---------   --------     			--     ---------------------------------    *
* all_threads struct threads*		I	   Threads to join					    *
* n 		  int					I	   Number of threads to join			*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vJoin_Threads(struct threads *all_threads, __u32 n)
{
  for (size_t i = 0; i < n; i++)
  {
    SAFE_PFUNC(pthread_join(all_threads[i].pthread,NULL));
  }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vInit_mutex                                                    *
*                                                                               *
* PURPOSE: Initializes mutexes                                                  *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* thread	struct threads*			I	   Structure containing mutex 			*
*													to initialize  			    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vInit_mutex(struct threads* thread)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * attr			pthread_mutexattr_t	Mutex attribute
    */
	pthread_mutexattr_t attr;
    //int protocol;

    SAFE_PFUNC(pthread_mutexattr_init(&attr));

    if(thread->type == MUT_PI)
	    SAFE_PFUNC(pthread_mutexattr_setprotocol(&attr,PTHREAD_PRIO_INHERIT));

	SAFE_PFUNC(pthread_mutex_init(&thread->mutex, &attr));

    pthread_mutexattr_destroy(&attr);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vInit_mutex_cond                                               *
*                                                                               *
* PURPOSE: Initializes condition variables                                      *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* thread	struct threads*			I	   Structure containing condition    	*
*												variables to initialize  		*
* ncond     int						I	   										*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vInit_mutex_cond(struct threads* thread)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * attr			pthread_mutexattr_t	Mutex attribute
    */
	pthread_mutexattr_t attr;
    //int protocol;

    SAFE_PFUNC(pthread_mutexattr_init(&attr));

    if(thread->type == MUT_COND_PI){
        SAFE_PFUNC(pthread_mutexattr_setprotocol(&attr,PTHREAD_PRIO_INHERIT));
    }
    SAFE_PFUNC(pthread_mutex_init(&thread->mutex, &attr));

    pthread_cond_init(&thread->cond, NULL);

    pthread_mutexattr_destroy(&attr);
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vSet_pthread_priority                                          *
*                                                                               *
* PURPOSE: Changes priority and/or policy of thread		                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* thread	struct threads*			I	   Structure containing threads			*
* priority  const int				I	   New priority  						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vSet_pthread_priority(struct threads* thread, const int priority)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * sp			struct sched_param	Thread's parameters structure
    */

	struct sched_param sp;
    sp.sched_priority = priority;
    SAFE_PFUNC(sched_setscheduler(0, thread->policy, &sp));
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vDestroy_all                                 			        *
*                                                                               *
* PURPOSE: Destroys mutexe and condition variable		                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* thread	struct threads*			I	   Structure containing mutex 			*
*													and cond var  			    *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vDestroy_all(struct threads* thread)
{

    pthread_cond_destroy(&thread->cond);
    pthread_mutex_destroy(&thread->mutex);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iInit_can			                                            *
*                                                                               *
* PURPOSE: Initializes can socket						                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
*                                                                               *
* RETURN VALUE: int                                                             *
*                                                                               *
********************************************************************************/

int iInit_can()
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * s				int					Socket's address
    */
	int s;
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	return s;

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vBind_can				                                        *
*                                                                               *
* PURPOSE: Binds can socket								                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* rfilter	struct can_filter		I	   Mask of can socket					*
* hasFilter int						I	   if has filter						*
* s			int						I	   Address to socket					*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vBind_can(int s, struct can_filter rfilter, int hasFilter)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * ifr			struct ifreq		Interface structure
    */
	struct ifreq ifr;
	strcpy(ifr.ifr_name, CAN_IFACE );
	ioctl(s, SIOCGIFINDEX, &ifr);

	struct sockaddr_can addr;
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	SAFE_FUNC(bind(s, (struct sockaddr *)&addr, sizeof(addr)));

	if(hasFilter)
		setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vRcv_can				                                        *
*                                                                               *
* PURPOSE: Receive function for can messages			                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
* frame		struct can_frame*		O	   Message received						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vRcv_can(int s, struct can_frame *frame)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * nbytes		int					Number of bytes received
    */
	int nbytes;
	nbytes = read(s, frame, sizeof(struct can_frame));
	if (nbytes < 0)
		perror("Error can read");

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vRcv_can16				                                        *
*                                                                               *
* PURPOSE: Receive function for can messages, 16 bits values                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
* frame		struct can_frame16*		O	   Message received						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vRcv_can16(int s, struct can_frame16* frame)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * nbytes		int					Number of bytes received
    */
	int nbytes;
	nbytes = read(s, frame, sizeof(struct can_frame16));
	if(nbytes < 0)
		perror("Error can read");

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vRcv_can64				                                        *
*                                                                               *
* PURPOSE: Receive function for can messages, 64 bits values                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
* frame		struct can_frame64*		O	   Message received						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vRcv_can64(int s, struct can_frame64* frame)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * nbytes		int					Number of bytes received
    */
	int nbytes;
	nbytes = read(s, frame, sizeof(struct can_frame64));
	if (nbytes < 0)
		perror("Error can read");

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vSnd_can				                                        *
*                                                                               *
* PURPOSE: Receive function for can messages			                        *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
* frame		struct can_frame*		I	   Message to send						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vSnd_can(int s, const struct can_frame *frame)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * nbytes		int					Number of bytes sent
    */
	int nbytes;
	nbytes = write(s, frame, sizeof(struct can_frame16));
	if (nbytes < 0)
		perror("Error can write");

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vSnd_can16				                                        *
*                                                                               *
* PURPOSE: Receive function for can messages, 16 bits values                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
* frame		struct can_frame16*		I	   Message to send						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vSnd_can16(int s, const struct can_frame16* frame)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * nbytes		int					Number of bytes sent
    */
	int nbytes;
	nbytes = write(s, frame, sizeof(struct can_frame16));
	if (nbytes < 0)
		perror("Error can write");

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vSnd_can64				                                        *
*                                                                               *
* PURPOSE: Receive function for can messages, 64 bits values                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
* frame		struct can_frame64*		I	   Message to send						*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vSnd_can64(int s, const struct can_frame64* frame)
{
	/* LOCAL VARIABLES:
    * Variable      Type           		Description
    * ------------- -------        		---------------
    * nbytes		int					Number of bytes sent
    */
	int nbytes;
	nbytes = write(s, frame, sizeof(struct can_frame64));
	if (nbytes < 0)
		perror("Error can write");

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vDestroy_can			                                        *
*                                                                               *
* PURPOSE: Closes can socket 								                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* s			int						I	   Address to socket					*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vDestroy_can(int s)
{

	SAFE_PFUNC(close(s));

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vDestroy_can			                                        *
*                                                                               *
* PURPOSE: Closes can socket 								                    *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument 		 Type         			IO     Description                      *
* ---------		 --------     			--     ------------------------------   *
* bytes_temp	 __u8[4]				IO	   Bytes converted from float		*
* float_varible  float					I	   Float to be converted			*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void float2Bytes(__u8 bytes_temp[4], float float_variable)
{
	memcpy(bytes_temp, (unsigned char *)(&float_variable), 4);
}
