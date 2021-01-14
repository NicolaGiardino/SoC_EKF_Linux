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
* FILE NAME: procedure_stub.c                                                                       *
*                                                                                                   *
* PURPOSE: This library is used to implement all the threads and the relative funcions              *
*               to be used in the main of the Stub                                                  *
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
* Source: <procedure_stub                                                                           *
*                                                                                                   *
* Name          Type    IO Description                                                              *
* ------------- ------- -- -----------------------------                                            *
*                                                                                                   *
*                                                                                                   *
* STATIC VARIABLES:                                                                                 *
*                                                                                                   *
*   Name     Type       I/O      Description                                                        *
*   ----     ----       ---      -----------                                                        *
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

#include "../include/procedure_stub.h"

void quit_handler() { quit = 1; }
int16_t getQuit() { return quit; }

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vStubSendData                                                  *
*                                                                               *
* PURPOSE: This function serves as a stub for the Kalman thread                 *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* input     Matrix*      I      Input to the thread                             *
* i			size_t		 I      Index of next data								*
* s			int			 I		Socket index									*
*                                                                               *
* RETURN VALUE: void                                                            *
*              TMP                                                                 *
********************************************************************************/

void vStubSendData(int s, size_t i, Matrix* input)
{
	/* LOCAL VARIABLES:
    * Variable      Type           	    Description
    * ------------- -------        	    ---------------
    * frame         struct can_frame16	Struct of the CAN message
    * f64		    struct can_frame64	Struct of the CAN message
    * filter		struct can_filter 	Struct of the CAN mask 
    * j             size_t              Loop counter
    * data          b32data             Union for float-to-u16 conversion
    * dataint       b32int              Union for int-to-u16 conversion
    */
	struct can_frame16 frame;
	struct can_frame64 f64;
	struct can_filter  filter;
	b32data data;
	b32int  dataint;
	size_t j;

	filter.can_id = MASTER | DATA_MSG | RTS | SLAVEADDR;
	filter.can_mask = CAN_MASK;
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

#if DEBUG_PRINT
	printf("[STUB] ");
	printf("Waiting for RTS\n");
#endif

	vRcv_can16(s, &frame);

	frame.can_id = SLAVE | DATA_MSG | CTS | SLAVEADDR;
	frame.dlc	 = 0;

#if DEBUG_PRINT
	printf("[STUB] ");
	printf("Sending CTS\n");
#endif
	vSnd_can16(s, &frame);

#if DEBUG_PRINT
	printf("[STUB] ");
	printf("Sending data\n");
#endif

	for (j = 0; j < PAR * SER; j++)
	{
#if DEBUG_PRINT
        printf("[STUB] ");
		printf("Sending data\n");
#endif
		/* Sending Current and Temp data */
		frame.dlc 		  = CAN_MAX_DLEN;
		dataint.n		  = j;
		frame.data[CELL1] = dataint.u16[0];
		frame.data[CELL2] = dataint.u16[1];
		data.f			  = input->matrix[CURRENT][i];
		frame.data[CURR1] = data.u16[0];
		frame.data[CURR2] = data.u16[1];
		vSnd_can16(s, &frame);

		/* Sending Voltage data */
		frame.dlc 		  = CAN_MAX_DLEN;
		data.f 			  = input->matrix[VOLTAGE][i];
		frame.data[VOLT1] = data.u16[0];
		frame.data[VOLT2] = data.u16[1];
		dataint.n 		  = Temp;
		frame.data[TMP1]  = dataint.u16[0];
		frame.data[TMP2]  = dataint.u16[1];
		vSnd_can16(s, &frame);
	}

	if(i > 0)
	{

		filter.can_id = MASTER | DATA_MSG | addr;
		filter.can_mask = CAN_MASK;
		setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));
		vRcv_can64(s, &f64);
#if DEBUG_PRINT
		printf("[STUB] ");
		printf("Message Received from %#08x: %#08x\n", frame.can_id, frame.data);
#endif
		/* Here the Slave should activate the discharge on the right cells */

	}

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vStubEnd	                                                    *
*                                                                               *
* PURPOSE: This function serves to signal the finish of the test to the master  *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* s			int			 I		Socket index									*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vStubEnd(int s)
{
	/* LOCAL VARIABLES:
    * Variable      Type           	    Description
    * ------------- -------        	    ---------------
    * frame         struct can_frame16	Struct of the CAN message
	*/
	struct can_frame16 frame;
	frame.can_id = SLAVE | END_MSG | SLAVEADDR;
	frame.dlc = 2;
	frame.data[0] = 0x400;
	vSnd_can16(s, &frame);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pvStubThread                                                   *
*                                                                               *
* PURPOSE: This thread serves as stub                                           *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* args      void*        IO     Input to the thread                             *
*                                                                               *
* RETURN VALUE: NULL                                                            *
*                                                                               *
********************************************************************************/

void *pvStubThread(void *args)
{
	/* LOCAL VARIABLES:
    * Variable      Type           	    Description
    * ------------- -------        	    ---------------
    * filter		struct can_filter 	Struct of the CAN mask 
    * i             size_t              Loop counter
	* s				int					CAN socket address
	* time			struct timespec		Test random error sending
	* input			Matrix*				Matrix containing the input from sensors
	*/

	Matrix *input = (Matrix *)args;
	struct timespec time;
	size_t i;
	int s;
	struct can_filter filter;

	printf("[STUB] ");
	printf("Stub Init\n");
	printf("%d\n", input->c);
	s = iInit_can();

	filter.can_id = MASTER | REG_MSG;
	filter.can_mask = CAN_MASK;
	vBind_can(s, filter, 1);

	printf("[STUB] ");
	printf("Sending data\n");

	vStubSendData(s, 0, input);

	for (i = 0; i < 1500; i++)
	{
#if DEBUG3
		printf("[STUB] ");
		printf("Sending data %d\n", i);
#endif
		vStubSendData(s, i, input);
		if(i==0)
			vInitLoggerStub(input->matrix[SOC][i]);
		else
			vStoreDataStub(input->matrix[SOC][i], i);

	}

	printf("[STUB] ");
	printf("Sending end of test\n");
	vStubEnd(s);
	quit_handler();

	vDestroy_can(s);
	pthread_exit(NULL);

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vInitLoggerStub                                                *
*                                                                               *
* PURPOSE: Initialize log structure                                             *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* store		float					I	   Log value 							*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vInitLoggerStub(float store)
{

	val[0] = store;

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vStoreDataStub                                                 *
*                                                                               *
* PURPOSE: Log data in structure                                                *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         			IO     Description                          *
* --------- --------     			--     ---------------------------------    *
* store		float					I	   Log value							*
* index		size_t					I	   Index of passed value				*
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vStoreDataStub(float store, size_t index)
{

	val[index] = store;

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vFinalStoreStub                                                *
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

void vFinalStoreStub()
{

	size_t i;
	FILE *file;

#if DEBUG4
	for (size_t j = 0; j < 10; j++)
	{
		printf("[STUB] ");
		printf("%f\n", val[j]);
	}
#endif

	file = fopen("../SOC_True.txt", "w");
	for (i = 0; i < 1500; i++)
		fprintf(file, "%f\t%d\n", val[i], i);
	fclose(file);

}