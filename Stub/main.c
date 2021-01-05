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
* PURPOSE: In the Stub we're going to acquire the sensor data read from files. This program is obviously going to act as a Stub to test *
*           the main program.                                                                                                           *
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

#include "./include/procedure_stub.h"

int main()
{
    signal(SIGINT, quit_handler);

    Matrix* input;

    FILE* myFile;
    int j = 0;
    int n = 0;
    float values;

    /* Take Cell Model Inputs from files */

    for (size_t i = 0; i < 4; i++)
    {

        j = 0;
        switch (i)
        {
        case(TIME):
            myFile = fopen("../csv/CellDataTime.csv", "r");
            printf("TIME\n");
            break;
        case(VOLTAGE):
            myFile = fopen("../csv/CellDataVoltage.csv", "r");
            printf("VOLTAGE\n");
            break;
        case(CURRENT):
            myFile = fopen("../csv/CellDataCurrent.csv", "r");
            printf("CURRENT\n");
            break;
        case(SOC):
            myFile = fopen("../csv/CellDataSOC.csv", "r");
            printf("SOC\n");
            break;
        default:
            break;
        }
        if (myFile == NULL) {
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

            input = pxCreate(4, n);
        }

        while (fscanf(myFile, "%f", &input->matrix[i][j++]) == 1)
        {
            fscanf(myFile, ",");
        }

        fclose(myFile);
    }


    /*
     * Create Stub thread. It must:
     *   +Receive the registration message and answer properly
     *   +Send CAN messages simulating Slave Data Messages until the error is sent
     *
     * A second thread must:
     *   +Send an error message after a random no of seconds to test the error thread.
     *   +It must call for a SIGINT after having sent the error message (call kill_handler())
    */

    struct threads thread;
    thread.type     = 0;
    thread.policy   = SCHED_FIFO;
    thread.priority = 70;
    thread.func     = pvStubThread;
    thread.args     = (void*)input;

    /* Lock memory */
    SAFE_PFUNC(mlockall(MCL_CURRENT | MCL_FUTURE));

    iCreate_thread(&thread);

    pthread_join(thread.pthread, NULL);

    vFinalStoreStub();

    vDestroy(input);

    return 0;

}
