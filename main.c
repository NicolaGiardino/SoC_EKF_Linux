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

/****************************************************************************************************************************************
* FILE NAME: main.c                                                                                                                     *
*                                                                                                                                       *
* PURPOSE: In the main program we're going to acquire the param data from the cell model, through the Kalman structure's data.          *
*           The main will then acquire data from sensors, using CAN sockets, from dynamic data of the cell, to be used in the loop      *
*           as input for the two steps of the Kalman Filter.                                                                            *
*                                                                                                                                       *
* FILE REFERENCES:                                                                                                                      *
*                                                                                                                                       *
*   Name    I/O     Description                                                                                                         *
*   ----    ---     -----------                                                                                                         *
*   none                                                                                                                                *
*                                                                                                                                       *
*                                                                                                                                       *
* EXTERNAL VARIABLES:                                                                                                                   *
*                                                                                                                                       *
* Source: <SOC_SPKF.h>                                                                                                                  *
*                                                                                                                                       *
* Name          Type    IO Description                                                                                                  *
* ------------- ------- -- -----------------------------                                                                                *
* k             Kalman  I  Kalman structure, contains                                                                                   *
*                                all matrices needed for the execution of                                                               *
*                                the algorithm                                                                                          *
* m             Matrix* I  Matrix object                                                                                                *
*                                                                                                                                       *
* STATIC VARIABLES:                                                                                                                     *
*                                                                                                                                       *
*   Name     Type                I/O      Description                                                                                   *
*   ----     ----                ---      -----------                                                                                   *
*   kf       KalmanForThread     I        Struct that contains the values for the thread                                                *
*                                                                                                                                       *
* EXTERNAL REFERENCES:                                                                                                                  *
*                                                                                                                                       *
* Source: <procedure.h>                                                                                                                 *
*                                                                                                                                       *
*  Name                       Description                                                                                               *
*  -------------              -----------                                                                                               *
*                                                                                                                                       *
* ABNORMAL TERMINATION CONDITIONS, ERROR AND WARNING MESSAGES:                                                                          *
*    none, compliant with the standard ISO9899:1999                                                                                     *
*                                                                                                                                       *
* ASSUMPTIONS, CONSTRAINTS, RESTRICTIONS: tbd                                                                                           *
*                                                                                                                                       *
* NOTES: see documentations                                                                                                             *
*                                                                                                                                       *
* REQUIREMENTS/FUNCTIONAL SPECIFICATIONS REFERENCES:                                                                                    *
*                                                                                                                                       *
* DEVELOPMENT HISTORY:                                                                                                                  *
*                                                                                                                                       *
*   Date          Author            Change Id     Release     Description Of Change                                                     *
*   ----          ------            -------- -    ------      ----------------------                                                    *
*   08-12-2020    N.di Gruttola                    1          Added pthreads, added comments, code satisfies                            *
*                  Giardino                                    iso9899:1999, as requested per                                           *
*                                                              MISRA-C:2004                                                             *
*   28-12-2020    N.di Gruttola                    1          V1 Created					                                            *
*                  Giardino																		                                        *
*                                                                                                                                       *
* ALGORITHM (PDL)                                                                                                                       *
*                                                                                                                                       *
****************************************************************************************************************************************/

#include "include/procedure.h"

/********************************************************************************
*                                                                               *
* FUNCTION NAME: main                                                           *
*                                                                               *
* PURPOSE: This function sets the parameters up, reads from the files           *
                and creates the thread                                          *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* argc      int          I      counter of inputs                               *
* argv      char*        I      Takes as input the path of the files            *
*                                                                               *
* RETURN VALUE: int                                                             *
*                                                                               *
********************************************************************************/
int main(int argc, char* argv[]) 
{

    /* LOCAL VARIABLES:
    * Variable      Type                        Description
    * ------------- -------                     -------------------------------------
    * j             int                         Loop counter
    * n             int                         File lenght counter
    * values        float                       Support for file values
    * path          char[]                      Path of the files, input to fopen()
    * kf            struct KalmanForThread      Input to the thread
    * myFile        FILE*                       File inputs
    * thread        struct threads*             Thread structure
    */

    signal(SIGINT, vKill_handler);

    struct KalmanForThread *kf = (struct KalmanForThread *)malloc(sizeof(struct KalmanForThread));

    FILE* myFile;
    int n = 0, j = 0;
    float values;
    char path[100] = { 0 };

    struct sched_param param;

    /* Take Cell Model Parameters from files */
    for (size_t i = 0; i < PARAM_SIZE; i++)
    {
        path[0] = '\0';
        strcpy(path, argv[1]);
        switch (i)
        {
        case(Q):
            strcat(path, "CellModelQParam.csv");
            break;
        case(G):
            strcat(path, "CellModelGParam.csv");
            break;
        case(M):
            strcat(path, "CellModelMParam.csv");
            break;
        case(M0):
            strcat(path, "CellModelM0Param.csv");
            break;
        case(R):
            strcat(path, "CellModelRParam.csv");
            break;
        case(RC):
            strcat(path, "CellModelRCParam.csv");
            break;
        case(R0):
            strcat(path, "CellModelR0Param.csv");
            break;
        case(eta):
            strcat(path, "CellModeletaParam.csv");
            break;
        case(TEMP):
            strcat(path, "CellModeltemps.csv");
            break;
        default:
            break;
        }
        printf("%s\n", path);
        myFile = fopen(path, "r");
        if (myFile == NULL)
        {
            perror("failed to open file");
            return 1;
        }

        while (fscanf(myFile, "%f", &values) == 1)
        {
            n++;
            fscanf(myFile, ",");
        }
        rewind(myFile);


        if (!i) kf->k.Param = pxCreate(PARAM_SIZE, n);

        while (fscanf(myFile, "%f", &kf->k.Param->matrix[i][j++]) == 1)
        {
            fscanf(myFile, ",");
        }

        fclose(myFile);
        j = 0;
        printf("OK %ld\n", i);
    }

    j = 0;
    n = 0;
    /* Take Cell Model Parameters from files */
    for (size_t i = 0; i < OvSLenght; i++)
    {
        path[0] = '\0';
        strcpy(path, argv[1]);
        switch (i)
        {
        case(OCV):
            strcat(path, "CellModelOCV.csv");
            break;
        case(OCV0):
            strcat(path, "CellModelOCV0.csv");
            break;
        case(OCVrel):
            strcat(path, "CellModelOCVrel.csv");
            break;
        case(SOC):
            strcat(path, "CellModelSOC.csv");
            break;
        case(SOC0):
            strcat(path, "CellModelSOC0.csv");
            break;
        case(SOCrel):
            strcat(path, "CellModelSOCrel.csv");
            break;
        case(dOCV0):
            strcat(path, "CellModeldOCV0.csv");
            break;
        case(dOCVrel):
            strcat(path, "CellModeldOCVrel.csv");
            break;
        default:
            break;
        }
        printf("%s\n", path);
        myFile = fopen(path, "r");
        if (myFile == NULL)
        {
            perror("failed to open file");
            return 1;
        }

        if (!i) {
            while (fscanf(myFile, "%f", &values) == 1)
            {
                n++;
                fscanf(myFile, ",");
            }
            rewind(myFile);
            kf->k.OvS = pxCreate(OvSLenght, n);
        }

        while (fscanf(myFile, "%f", &kf->k.OvS->matrix[i][j++]) == 1)
        {
            fscanf(myFile, ",");
        }

        fclose(myFile);
        j = 0;
        printf("OK %ld\n", i);
        printf("\n");
    }


    /* Assign thread values */
    kf->thread[KALMAN].type         = 0;
    kf->thread[KALMAN].policy       = SCHED_FIFO;
    kf->thread[KALMAN].priority     = 75;
    kf->thread[KALMAN].func         = pvKalmanThread;
    kf->thread[KALMAN].args         = (void*)kf;

    kf->thread[END].type            = 0;
    kf->thread[END].policy          = SCHED_FIFO;
    kf->thread[END].priority        = 75;
    kf->thread[END].func            = pvEndThread;

    /* Lock memory */
    SAFE_PFUNC(mlockall(MCL_CURRENT | MCL_FUTURE));

    /* Create pthread */
    SAFE_PFUNC(iCreate_thread(&kft->thread[KALMAN]));
    SAFE_PFUNC(iCreate_thread(&kft->thread[END]));

    for (size_t i = 1; i < NTHREADS; i++)
    {
        SAFE_PFUNC(pthread_join(kft->thread[i].pthread, NULL));
    }

    /* Unlock memory */
    SAFE_PFUNC(munlockall());

    /* Final store */
    vFinalStore();

    /* Clearing memory */
    Delete(&kf->k);
    free(kf);

    return 0;
    
}
