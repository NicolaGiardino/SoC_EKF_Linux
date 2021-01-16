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
* FILE NAME: procedure.c                                                                            *
*                                                                                                   *
* PURPOSE: This library is used to implement all the threads and the relative funcions              *
*               to be used in the main of the Master                                                *
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
* Source: <procedure.h>                                                                             *
*                                                                                                   *
* Name          Type    IO Description                                                              *
* ------------- ------- -- -----------------------------                                            *
*                                                                                                   *
*                                                                                                   *
* STATIC VARIABLES:                                                                                 *
*                                                                                                   *
*   Name     Type       I/O      Description                                                        *
*   ----     ----       ---      -----------                                                        *
*   current  float[][]           Current of cells                                                   *
*   voltage  float[][]           Voltage of cells                                                   *
*   Temp     int                 Temperature of cells                                               *
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

#include "../include/procedure.h"

/* Declare global variables */

static float current [PAR * SER];
static float voltage [SER];
static int Temp      [PAR * SER];

static int iSearch_Min(float[], int);

/* Sets state of exit_threads to 1, closing threads and shutting down the program */
void vKill_handler() { exit_threads = 1; }
/* Returns state of exit_threads variable, connected to SIGINT */
int iGetExit() { return exit_threads; }

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vKalmanLoop                                                    *
*                                                                               *
* PURPOSE: Function that executes the Kalman Filter                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type                        IO     Description                      *
* --------- --------                    --     -------------------------------- *
* kft       struct Kalman*              IO     Contains Kalman Structure        *
* s         int                         I      Socket index                     *
*                                                                               *
* RETURN VALUE: NULL                                                            *
*                                                                               *
********************************************************************************/

void vKalmanLoop(Kalman* k, int s)
{
    /* LOCAL VARIABLES:
    * Variable      Type           	    Description
    * ------------- -------        	    ---------------
    * frame         struct can_frame16	Struct of the CAN message
    * frame64       struct can_frame64	Struct of the CAN message
    * filter		struct can_filter 	Struct of the CAN mask
    * NoOfCell      int                 Number of cells in the 
    * i             size_t              Loop counter
    * data          b32data             Union for float-to-u16 conversion
    * dataint       b32int              Union for int-to-u16 conversion
    * soc           __u64               Data to send, bits set to 1 are cells to be discharged
    * index         int                 Index of Seach_Min
    */
    struct can_frame16 frame;
    struct can_filter filter;

    int NoOfCell;
    int index;

    b32data data;
    b32int dataint;
    size_t i;

    struct can_frame64 frame64;

    __u64 soc = 0;

    filter.can_id = SLAVE | DATA_MSG | CTS | SLAVEADDR; /* 0b001100SlaveNo */
    filter.can_mask = SLAVE_MASK;
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    /* Send a RTS */
    frame.can_id = MASTER | DATA_MSG | RTS | SLAVEADDR;
    frame.dlc = 0;

#if DEBUG
    printf("Sending Ready to Send\n");
#endif

    vSnd_can16(s, &frame);

#if DEBUG
    printf("Waiting Clear to Send\n");
#endif

    /* Receive CTS from CAN */
    vRcv_can16(s, &frame);

    
#if DEBUG
        printf("Receiving data\n");
#endif

    if(frame.dlc == 0 && frame.can_id == filter.can_id)
    {
        for (i = 0; i < PAR * SER; i++)
        {
            //Receive Current and Temp data
            vRcv_can16(s, &frame);
            c_assert(frame.dlc == (CAN_MAX_DLEN_16 * 2));
            dataint.u16[0] = frame.data[CELL1];
            dataint.u16[1] = frame.data[CELL2];
            NoOfCell = dataint.n;
            data.u16[0] = frame.data[CURR1];
            data.u16[1] = frame.data[CURR2];
            current[NoOfCell] = data.f;

            //Receive Voltage data
            vRcv_can16(s, &frame);
            c_assert(frame.dlc == CAN_MAX_DLEN_16 * 2);
            if (NoOfCell % PAR == 0)
            {
                data.u16[0] = frame.data[VOLT1];
                data.u16[1] = frame.data[VOLT2];
                voltage[NoOfCell % PAR] = data.f;
            }

            dataint.u16[0] = frame.data[TMP1];
            dataint.u16[1] = frame.data[TMP2];
            Temp[NoOfCell] = dataint.n;
        }
    }

#if DEBUG2
    printf("Frame data: %f %f %d\n", current[0], voltage[0], Temp[0]);
#endif


#if DEBUG
    printf("EKF\n");
#endif

    vEKF_Step1(k, current, Temp[0]);
    vEKF_Step2(k, voltage, Temp[0]);
    
#if DEBUG_PRINTSOC
    printf("The soc is: %f.2%%\n", k->x->matrix[Z_IND][0] * 100);
#endif

#if RASPI_SOC
    /* Calculating mean SoC and mean Temperature*/
    float val[2] = {0, 0};

    for (i = 0; i < PAR * SER; i++)
        val[0] += k->x->matrix[Z_IND + i][0];
    val[0] /= (PAR * SER);
    for (i = 0; i < PAR * SER; i++)
        val[1] += (float)Temp[i];
    val[1] /= (PAR * SER);
    vPrintLCD(val);
#endif

    index = iSearch_Min(k->x->matrix[Z_IND], k->x->c);

    for (i = 0; i < k->x->c; i++)
    {
        if (k->x->matrix[Z_IND][i] >= (k->x->matrix[Z_IND][index] + SOC_RANGE))
            soc = soc | (2 ^ i);
    }

#if DEBUG
    printf("Message sent to activate discharge: 0x%x\n", soc);
#endif

    /* Send via CAN */
    frame64.can_id = MASTER | DATA_MSG | SLAVEADDR;
    frame64.dlc  = CAN_MAX_DLEN_16;
    frame64.data = soc;
    
    vSnd_can64(s, &frame64);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vErrorLoop                                                     *
*                                                                               *
* PURPOSE: This function solves the errors sent ffrom the slave                 *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* s         int          I      Socket index                                    *
*                                                                               *
* RETURN VALUE: NULL                                                            *
*                                                                               *
********************************************************************************/

void vEndLoop(int s)
{ 
    /* LOCAL VARIABLES:
    * Variable      Type           	    Description
    * ------------- -------        	    ---------------
    * frame         struct can_frame16	Struct of the CAN message
    */
    struct can_frame16 frame;

    vRcv_can16(s, &frame);
    printf("End message Received!\n");
    printf("Message no. %ld\n", frame.data[0]);
    printf("Sending SIGKILL\n");

    vKill_handler();
}


/********************************************************************************
*                                                                               *
* FUNCTION NAME: pvKalmanThread                                                 *
*                                                                               *
* PURPOSE: This thread serves as the performer                                  *
*               of the Kalman Filter, it works by taking the input values       *
*               and then executing the filter                                   *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* ktof      void*        IO     Input to the thread                             *
*                                                                               *
* RETURN VALUE: NULL                                                            *
*                                                                               *
********************************************************************************/

void* pvKalmanThread(void* ktof)
{
    /* LOCAL VARIABLES:
    * Variable      Type           	        Description
    * ------------- -------        	        ---------------
    * kf            struct KalmanForThread	Struct of the Kalman structure + threads
    * s             int                     Can socket's address 
    * frame         struct can_frame16	    Struct of the CAN message
    * filter		struct can_filter      	Struct of the CAN mask
    * NoOfCell      int                     Number of cells in the 
    * i             size_t                  Loop counter
    * data          b32data                 Union for float-to-u16 conversion
    * dataint       b32int                  Union for int-to-u16 conversion
    * val           int[2]                  Data to be stored
    * index         int                     Index of value to be stored
    * pinfo         struct period_info      Struct containing periodic thread informations on time
    * passed_ms     struct timespec         Struct containing the ms passed between init of the Kalman step and finish
    */

    struct KalmanForThread *kf = (struct KalmanForThread *)ktof;

    struct period_info pinfo;
    struct timespec    passed_ms;

    struct can_filter filter;
    struct can_frame16 frame;

    int NoOfCell;
    int s;
    b32data data;
    b32int dataint;
    size_t i;
    float val[2];
    size_t index = 1;

#if RASPI_SOC
    /* Setting a 1s period */
    pinfo.period_ns = 1 * NS_PER_SEC;
#else
    /* Setting a 15ms period */
    pinfo.period_ns = 15 * NS_PER_MS;
#endif

    vPeriodic_task_init(&pinfo);

    passed_ms.tv_nsec = lRt_gettime();

    s = iInit_can();

#if RASPI_SOC
    vInitLCD();
#endif

    filter.can_id = SLAVE | DATA_MSG | CTS; /* 0b001100xxxxx */
    filter.can_mask = SLAVE_MASK;
    vBind_can(s, filter, 1);


    filter.can_id = SLAVE | DATA_MSG | CTS | SLAVEADDR; /* 0b001100SlaveNo */
    filter.can_mask = SLAVE_MASK;
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    /* Send a RTS */
    frame.can_id = MASTER | DATA_MSG | RTS | SLAVEADDR;
    frame.dlc = 0;

#if DEBUG
    printf("Sending Ready to Send\n");
#endif
    vSnd_can16(s, &frame);

#if DEBUG
    printf("Waiting Clear to Send\n");
#endif

    /* Receive CTS from CAN */
    vRcv_can16(s, &frame);

#if DEBUG
    printf("Receiving data\n");
#endif

    if (frame.dlc == 0 && frame.can_id == filter.can_id)
    {
        for (i = 0; i < PAR * SER; i++)
        {
            //Receive Current and Temp data
            vRcv_can16(s, &frame);
            c_assert(frame.dlc == (CAN_MAX_DLEN_16 * 2));
            dataint.u16[0] = frame.data[CELL1];
            dataint.u16[1] = frame.data[CELL2];
            NoOfCell       = dataint.n;
            data.u16[0]    = frame.data[CURR1];
            data.u16[1]    = frame.data[CURR2];
            current[NoOfCell] = data.f;

            //Receive Voltage data
            vRcv_can16(s, &frame);
            c_assert(frame.dlc == CAN_MAX_DLEN_16 * 2);
            if (NoOfCell % PAR == 0)
            {
                data.u16[0] = frame.data[VOLT1];
                data.u16[1] = frame.data[VOLT2];
                voltage[NoOfCell % PAR] = data.f;
            }

            dataint.u16[0] = frame.data[TMP1];
            dataint.u16[1] = frame.data[TMP2];
            Temp[NoOfCell] = dataint.n;
        }
    }

    /* Setup KF TBD */
    printf("Setting up EKF\n");
    vSetup(&kf->k, Temp[0], voltage);

    /* Store variables */
    val[0] = kf->k.x->matrix[Z_IND][0];
    val[1] = kf->k.Pk->matrix[Z_IND][Z_IND];
    vInitLogger(val);

    /* Wait period */
    passed_ms.tv_nsec = lRt_gettime() - passed_ms.tv_nsec;
    vWait_rest_of_period(&pinfo, &passed_ms);

    printf("Starting Kalman Loop\n");

    while(!iGetExit())
    {

        passed_ms.tv_nsec = lRt_gettime();

        /* Compute Kalman Loop */
        vKalmanLoop(&kf->k, s);

        /* Store variables */
        val[0] = kf->k.x->matrix[Z_IND][0];
        val[1] = kf->k.Pk->matrix[Z_IND][Z_IND];
        vStoreData(val, index);
        index++;

            /* Wait period */
            /*
         * Modify this to make wait for the rest of the real period                 *
         * (ex it took 1ms to compute, the delta_t is 15ms, so final wait is 14ms)  *
         */
            passed_ms.tv_nsec = lRt_gettime() - passed_ms.tv_nsec;
        vWait_rest_of_period(&pinfo, &passed_ms);

    }

    pthread_exit(NULL);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pvEndThread                                                    *
*                                                                               *
* PURPOSE: This thread is used to receive the end of test message from the Stub *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* args      void*        IO     Input to the thread                             *
*                                                                               *
* RETURN VALUE: NULL                                                            *
*                                                                               *
********************************************************************************/

void *pvEndThread(void *args)
{
    /* LOCAL VARIABLES:
    * Variable      Type           	        Description
    * ------------- -------        	        ---------------
    * s             int                     Can socket's address 
    * filter		struct can_filter      	Struct of the CAN mask
    */

    struct can_filter filter;
    int s;

    s = iInit_can();
    filter.can_id = SLAVE | END_MSG; /* 0b010000xxxxx */
    filter.can_mask = SLAVE_MASK;

    vBind_can(s, filter, 1);

    while(!iGetExit())
    {
        vEndLoop(s);
    }

    pthread_exit(NULL);

}


/********************************************************************************
*                                                                               *
* FUNCTION NAME: iSearch_Min                                                    *
*                                                                               *
* PURPOSE: Search the min in an array                                           *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* arr       int[]        I       Array of ints                                  *
* n         int          I       No. of numbers                                 *
*                                                                               *
* RETURN VALUE: int                                                             *
*                                                                               *
********************************************************************************/

static int iSearch_Min(float arr[], int n)
{ 
    /* LOCAL VARIABLES:
    * Variable      Type      Description
    * ------------- -------   ---------------
    * i             int       Loop counter 
    * min		    float     Minimum number
    */

    float min;
    int i;

    if (n == 1)
    {
        min = arr[0];
        return 0;
    }

    if (arr[0] > arr[1])
    {
        min = arr[1];
    }
    else
    {
        min = arr[0];
    }

    for (i = 2; i < n; i++)
    {
        if (arr[i] < min)
            min = arr[i];
    }

    return i;

}

#if RASPI_SOC

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vInit_LCD                                                      *
*                                                                               *
* PURPOSE: Initializes the lcd 16x2 display                                     *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/
void vInitLCD()
{

    wiringPiSetup();
    lcdIndex = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vPrintLCD                                                      *
*                                                                               *
* PURPOSE: Prints data on 16x2 lcd display                                      *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* val       int[]        I       Array of ints                                  *
* lcd       int          I       Address of lcd display                         *
*                                                                               *
* RETURN VALUE: int                                                             *
*                                                                               *
********************************************************************************/
int mn = 0;
int sc = 0;
void vPrintLCD(float val[2])
{
    sc++;
    if(sc==60)
    {
        sc = 0;
        mn++;
    }
    lcdClear(lcdIndex);
    lcdPosition(lcdIndex, 0, 0);
    lcdPrintf(lcdIndex, "SoC: %.2f%% ", val[0] * 100);
    lcdPosition(lcdIndex, 0, 1);
    lcdPrintf(lcdIndex, "%.2fC  T+%d:%d", val[1], mn, sc);
}

#endif