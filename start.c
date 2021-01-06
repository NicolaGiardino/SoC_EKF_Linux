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
* FILE NAME: start.c                                                                                                                     *
*                                                                                                                                       *
* PURPOSE: This file is used to start the test, it forks the Stub and the main program                                                  *
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
* Source:                                                                                                                               *
*                                                                                                                                       *
* Name          Type    IO Description                                                                                                  *
* ------------- ------- -- -----------------------------                                                                                *
*                                                                                                                                       *
* STATIC VARIABLES:                                                                                                                     *
*                                                                                                                                       *
*   Name     Type                I/O      Description                                                                                   *
*   ----     ----                ---      -----------                                                                                   *
*                                                                                                                                       *
* EXTERNAL REFERENCES:                                                                                                                  *
*                                                                                                                                       *
* Source:                                                                                                                               *
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
*   06-01-2021    N.di Gruttola                    1          V1 Created					                                            *
*                  Giardino																		                                        *
*                                                                                                                                       *
* ALGORITHM (PDL)                                                                                                                       *
*                                                                                                                                       *
****************************************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>



int main()
{
    /* LOCAL VARIABLES:
    * Variable      Type                        Description
    * ------------- -------                     -------------------------------------
    * status        int                         Error checking variable
    * pid           pid_t                       Pid type variable
    */
    int status;
    pid_t pid;

    pid = fork();
    if (pid == 0)
    {
        execl("./Stub/main", NULL);
        perror("Error\n");
        kill(getppid(), SIGKILL);
        exit(0);
    }

    pid = fork();
    if (pid == 0)
    {
        execv("./main", "./csv/");
        perror("Error\n");
        kill(getppid(), SIGKILL);
        exit(0);
    }

    for (size_t i = 0; i < 2; i++)
    {

        pid = wait(&status);
        if (pid < 0)
        {
            perror("Error while killing\n ");
            exit(1);
        }

    }

    return 0;
}