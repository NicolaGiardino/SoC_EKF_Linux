#include "../include/procedure_kernel.h"

/* Declare global variables */

static int SlaveNo = 0;

static float current[PAR * SER];
static float voltage[SER];
static int Temp[PAR * SER];

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

void vKalmanLoop(Kalman *k, int s)
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

    filter.can_id = SLAVE | DATA_MSG | CTS | SlaveNo; /* 0b001100SlaveNo */
    filter.can_mask = SLAVE_MASK;
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    /* Send a RTS */
    frame.can_id = MASTER | DATA_MSG | RTS | SlaveNo;
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

    EKF_Step1(k, current, Temp[0]);
    EKF_Step2(k, voltage, Temp[0]);

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
    frame64.can_id = MASTER | DATA_MSG | SlaveNo;
    frame64.dlc = CAN_MAX_DLEN_16;
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

int iKalmanThread(void *ktof)
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
    struct timespec passed_ms;

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

    filter.can_id = SLAVE | DATA_MSG | CTS | SlaveNo; /* 0b001100SlaveNo */
    filter.can_mask = SLAVE_MASK;
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    /* Send a RTS */
    frame.can_id = MASTER | DATA_MSG | RTS | SlaveNo;
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

    /* Setup KF TBD */
    printf("Setting up EKF\n");
    Setup(&kf->k, Temp[0], voltage);

    /* Store variables */
    val[0] = kf->k.x->matrix[Z_IND][0];
    val[1] = kf->k.Pk->matrix[Z_IND][Z_IND];
    vInitLogger(val);

    /* Wait period */
    passed_ms.tv_nsec = lRt_gettime() - passed_ms.tv_nsec;
    vWait_rest_of_period(&pinfo, &passed_ms);

    printf("Starting Kalman Loop\n");

    while (!iGetExit())
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
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iEndThread                                                     *
*                                                                               *
* PURPOSE: This thread is used to receive the end of test message from the Stub *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* args      void*        IO     Input to the thread                             *
*                                                                               *
* RETURN VALUE: int                                                             *
*                                                                               *
********************************************************************************/

int iEndThread(void *args)
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
    filter.can_id = SLAVE | ERR_MSG; /* 0b010000xxxxx */
    filter.can_mask = SLAVE_MASK;

    vBind_can(s, filter, 1);

    while (!iGetExit())
    {
        vEndLoop(s);
    }
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
int iInitLCD()
{

    wiringPiSetup();
    lcdIndex = lcdInit(2, 16, 8, LCD_RS, LCD_E, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
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

void vPrintLCD(float val[2])
{
    lcdClear(lcd);
    lcdPosition(lcd, 0, 0);
    lcdPrintf(lcd, "SOC: %f.2%%", val[0] * 100);
    lcdPosition(lcd, 0, 1);
    lcdPrintf(lcd, "Temp: %d°C", val[1]);
}

#endif