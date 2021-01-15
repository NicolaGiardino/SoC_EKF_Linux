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
* FILE NAME: SOC_EKF.c                                                                              *
*                                                                                                   *
* PURPOSE: This library gives all the tools to implement a Matrix Object                            *
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
* Source: <SOC_EKF.h>                                                                               *
*                                                                                                   *
* Name          Type    IO Description                                                              *
* ------------- ------- -- -----------------------------                                            *
*   k           Kalman      Kalman object															*
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
*   08-07-2020    N.di Gruttola                    0.1         Project created	 			        *
*                  Giardino																		    *
*   28-12-2020    N.di Gruttola                    1          V1 Created					        *
*                  Giardino																		    *
*                                                                                                   *
*                                                                                                   *
*                                                                                                   *
****************************************************************************************************/

/* Include Global Parameters */

#include "../include/SOC_EKF.h"

/* Declare Static Variables */


static float           h;
static float           i_prev[U_SIZE];
static int             i_sign[U_SIZE];
static float           Q_bump;
static Matrix*         int_Gku;                         /* Used to compute G*u */                   
static float           Parameters[PARAM_SIZE - 1];     /* Parameters at time t */
static Matrix*         app;                         /* Used as a support variable */
static Matrix*         app2;                        /* Used as a support variable */
static Matrix*         app3;                        /* Used as a support variable */
static Matrix*         app4;                        /* Used as a support variable */
static Matrix*         t1;                          /* Used as a support variable */
static Matrix*         t2;                          /* Used as a support variable */

/* Declare Prototypes */

static void  getParam(float*, const float, const Matrix*);
static float SOCfromOCV(const float, const float, const Matrix*);
static float OCVfromSOC(const float, const float, const Matrix*);
static float dOCVfromSOC(const float, const float, const Matrix*);

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vSetup                                                         *
*                                                                               *
* PURPOSE: This function sets the parameters up,                                *
*           creating the matrices and                                           *
*          setting up all the used variables                                    *
*           within the library.                                                 *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* k         Kalman*      IO     Kalman structure, contains                      *
*                                all matrices needed for the execution of       *
*                                the algorithm                                  *
* T         const float  I      Temperature of the cell                         *
* v_0       float        I      Voltage of the cell at T-0                      *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/

void vSetup(Kalman* k, const float T, const float* v_0)
{
    
    /*
     *  The variables names need to be changed in those commented after the var creation. *
     *  They're the official Kalman variables and so they must be used                    *
     */

    k->x        = pxCreate(X_SIZE, 1);
    k->Pk       = pxCreate(X_SIZE, X_SIZE);     
    k->Qk       = pxCreate(U_SIZE, U_SIZE);     
    k->Rk       = pxCreate(Y_SIZE, Y_SIZE);      
    k->Fk       = pxCreate(X_SIZE, X_SIZE);     
    k->Gk       = pxCreate(X_SIZE, U_SIZE);     
    k->Hk       = pxCreate(Y_SIZE, X_SIZE);     
    k->D        = pxCreate(Y_SIZE, Y_SIZE); 

    /* Let's suppose 1st elem is a series module formed by PAR cells in parallel */
    for (size_t i = 0; i < Y_SIZE; i++)
    {
        i_prev[i] = 0;
        i_sign[i] = 0;

        k->x->matrix[I_IND + i][0]          = i_prev[i];
        k->x->matrix[H_IND + i][0]          = 0;

        k->Pk->matrix[I_IND + i][I_IND + i] = 100;
        k->Pk->matrix[H_IND + i][H_IND + i] = 0.01;
        k->Pk->matrix[Z_IND + i][Z_IND + i] = 0.001;

        k->Rk->matrix[i][i] = 0.3;
    }
    
    for (size_t i = 0; i < U_SIZE; i++)
        k->Qk->matrix[i][i] = 4;

    for (size_t i = 0; i < SER; i++)
    {

        k->x->matrix[Z_IND + i * PAR][0] = SOCfromOCV(v_0[i] , T, k->OvS);

        for (int j = 1; j < PAR; j++)
            k->x->matrix[Z_IND + i * PAR + j][0] = k->x->matrix[Z_IND + i * PAR][0];

    }


    k->Kk    = pxCreate(X_SIZE, Y_SIZE);
    k->y_p   = pxCreate(Y_SIZE, 1);
    k->Sk    = pxCreate(Y_SIZE, Y_SIZE);

    int_Gku  = pxCreate(X_SIZE, 1);
    
}

/************************************************************************************************************************************
*                                                                                                                                   *
* FUNCTION NAME: vEKF_Step1                                                                                                         *
*                                                                                                                                   *
* PURPOSE:                                                                                                                          *
*   This function is used for the Innovation step of the EKF (Kalman Filter Step 1).                                                *
*                                                                                                                                   *
* ARGUMENT LIST:                                                                                                                    *
*                                                                                                                                   *
* Argument  Type         IO     Description                                                                                         *
* --------- --------     --     ---------------------------------                                                                   *
* k         Kalman*      IO     Kalman structure, contains                                                                          *
*                                all matrices needed for the execution of                                                           *
*                                the algorithm                                                                                      *
* T         const float  I      Temperature of the cell                                                                             *
* u         float        I      Current of the cell at time t                                                                       *
*                                                                                                                                   *
* RETURN VALUE: void                                                                                                                *
*                                                                                                                                   *
************************************************************************************************************************************/

void vEKF_Step1(Kalman* k, float* u, const float T)
{
 /* LOCAL VARIABLES:
  * Variable      Type           Description
  * ------------- -------        ---------------
  * i             size_t         Loop counter
  * j             size_t         Loop counter
  * Parameters    float[8]       Cell Parameters for a given T
  */

#if DEBUG3
    printf("Kalman Filter Step 1 begin\n");
#endif

    size_t i;
    size_t j;

    /* EKF Step1 Setup */
    getParam(Parameters, T, k->Param);

    Parameters[RC] = exp(-DeltaT / fabs(Parameters[RC]));
#if DEBUG_PRINT
    for (size_t i = 0; i < PARAM_SIZE - 1; i++)
    {
        printf("%f\n", Parameters[i]);
    }
#endif

    for (size_t i = 0; i < PAR * SER; i++)
    {

        if (u[i] < 0)
            u[i] *= Parameters[eta];

        if (fabs(u[i]) > (Parameters[Q] / 100))
            i_sign[i] = signum(u[i]);
            
    }

#if DEBUG_PRINT
    printf("i_prev/sign: %f %d\n", i_prev[0], sign[0]);
#endif

    /* Setting up the derivative matrices */
    k->Fk->matrix[I_IND][I_IND] = Parameters[RC];
    k->Fk->matrix[H_IND][H_IND] = exp(-fabs(i_prev[0] * Parameters[G] / (3600 * Parameters[Q])));
    k->Fk->matrix[Z_IND][Z_IND] = 1;

#if DEBUG_PRINT
    printf("Fk\n");
    vPrint(k->Fk);
#endif

    for (int i = 1; i < PAR * SER; i++)
    {
        k->Fk->matrix[I_IND + i][I_IND + i] = Parameters[RC];
        k->Fk->matrix[H_IND + i][H_IND + i] = exp(-fabs(i_prev[i] * Parameters[G] / (3600 * Parameters[Q])));
        k->Fk->matrix[Z_IND + i][Z_IND + i] = 1;
    }

    k->Gk->matrix[I_IND][0]   = 1 - Parameters[RC];
    k->Gk->matrix[Z_IND][0]   = -DeltaT / (3600 * Parameters[Q]);
    int_Gku->matrix[Z_IND][0] = k->Gk->matrix[Z_IND][0] * i_prev[0];
    int_Gku->matrix[I_IND][0] = k->Gk->matrix[I_IND][0] * i_prev[0];
    int_Gku->matrix[H_IND][0] = (expf(-fabs(i_prev[0] * Parameters[G] * DeltaT / (3600 * Parameters[Q]))) - 1) * signum(i_prev[0]);
    k->Gk->matrix[H_IND][0]   = -fabs(Parameters[G] * DeltaT / (3600 * Parameters[Q])) * expf(-fabs(i_prev[0] * Parameters[G] / (3600 * Parameters[Q]))) * (1 + signum(i_prev[0]) * k->x->matrix[H_IND][0]);

#if DEBUG_PRINT
    printf("Gk\n");
    vPrint(k->Gk);
    printf("int_Gku\n");
    vPrint(int_Gku);
#endif

    for (int i = 1; i < PAR * SER; i++)
    {

        k->Gk->matrix[I_IND + i][i]             = 1 - Parameters[RC];
        k->Gk->matrix[Z_IND + i][i]             = k->Gk->matrix[Z_IND][0];
        int_Gku->matrix[Z_IND + i][0]           = k->Gk->matrix[Z_IND][0] * i_prev[i];
        int_Gku->matrix[I_IND + i][0]           = k->Gk->matrix[I_IND][0] * i_prev[i];
        int_Gku->matrix[H_IND + i][0]           = (expf(-fabs(i_prev[0] * Parameters[G] / (3600 * Parameters[Q]))) - 1) * signum(i_prev[i]);
        k->Gk->matrix[H_IND + i][i + PAR * SER] = -fabs(Parameters[G] * DeltaT / (3600 * Parameters[Q])) * expf(-fabs(i_prev[i] * Parameters[G] / (3600 * Parameters[Q]))) * (1 + signum(i_prev[i]) * k->x->matrix[H_IND + i][0]);
    }


    /* EKF Step 1a */
    app = pxMultiply(k->Fk, k->x);

    SAFE_FUNC(iSum(k->x, app, int_Gku));
#if DEBUG_PRINT
    printf("x_k\n");
    vPrint(k->x);
#endif

    /* EKF Step 1b */
    app  = pxMultiply(k->Fk, k->Pk);
    app2 = pxMultiply(k->Gk, k->Qk);
    t1   = pxTranspose(k->Fk);
    t2   = pxTranspose(k->Gk);
    app3 = pxMultiply(app, t1); 
    app4 = pxMultiply(app2, t2);

#if DEBUG_PRINT
    printf("Pk\n");
    vPrint(k->Pk);
#endif

    SAFE_FUNC(iSum(k->Pk, app3, app4));

#if DEBUG_PRINT
    printf("Qk\n");
    vPrint(k->Qk);
    printf("Pk\n");
    vPrint(k->Pk);
#endif

    vDestroy(app);
    vDestroy(app2);
    vDestroy(app3);
    vDestroy(app4);
    vDestroy(t1);
    vDestroy(t2);
    

    /* EKF Step 1c */

    

    for (size_t i = 0; i < PAR * SER; i++)
    {

        k->y_p->matrix[i][0] = OCVfromSOC(k->x->matrix[Z_IND + i][0], T, k->OvS);
        k->y_p->matrix[i][0] = k->y_p->matrix[i][0] + (Parameters[M0] * i_sign[i] + Parameters[M] * k->x->matrix[H_IND + i][0] - Parameters[R] * k->x->matrix[I_IND + i][0] - Parameters[R0] * u[i]);
    }

    for (size_t i = 0; i < SER; i++)
    {
        i_prev[i] = u[i];
        for (size_t j = 0; j < PAR; j++)
            i_prev[i + j] = i_prev[i];

    }
    
    #if DEBUG3
        printf("Kalman Filter Step 1 End\n");
    #endif

}

/************************************************************************************************************************************
*                                                                                                                                   *
* FUNCTION NAME: vEKF_Step2                                                                                                         *
*                                                                                                                                   *
* PURPOSE:                                                                                                                          *
*   This function is used for the Update step of the EKF. (Kalman Filter Step 2)                                                    *
*                                                                                                                                   *
* ARGUMENT LIST:                                                                                                                    *
*                                                                                                                                   *
* Argument  Type         IO     Description                                                                                         *
* --------- --------     --     ---------------------------------                                                                   *
* k         Kalman*      IO     Kalman structure, contains                                                                          *
*                                all matrices needed for the execution of                                                           *
*                                the algorithm                                                                                      *
* y         float*       I      Voltage of the cell at time t                                                                       *
*                                                                                                                                   *
* T         const float  I      Temperature at time t                                                                               *
*                                                                                                                                   *
* RETURN VALUE: void                                                                                                                *
*                                                                                                                                   *
************************************************************************************************************************************/

void vEKF_Step2(Kalman* k, float* y, const float T) //y is of size SER
{

    /* LOCAL VARIABLES:
  * Variable      Type           Description
  * ------------- -------        ---------------
  * i             size_t         Loop counter
  * j             size_t         Loop counter
  * z             size_t         Loop counter
  * Parameters    float[8]       Cell Parameters for a given T
  */

    size_t i;
    size_t j;
    size_t z;

#if DEBUG3
        printf("Kalman Filter Step 2 Begin\n");
#endif

    for (i = 0; i < PAR * SER; i++)
    {
        k->Hk->matrix[i][Z_IND + i] = dOCVfromSOC(k->x->matrix[Z_IND + i][0], T, k->OvS);
        k->Hk->matrix[i][H_IND + i] = Parameters[M];
    	k->Hk->matrix[i][I_IND + i] = -Parameters[R];
        k->D->matrix[i][i]          = 1;
    }

#if DEBUG_PRINT
    printf("Hk\n");
    vPrint(k->Hk);
    printf("Rk\n");
    vPrint(k->Rk);
#endif

    app  = pxMultiply(k->Hk, k->Pk);
    app2 = pxMultiply(k->D, k->Rk);
    t1   = pxTranspose(k->Hk);
    t2   = pxTranspose(k->D);
    app3 = pxMultiply(app, t1);
    app4 = pxMultiply(app2, t2);

    SAFE_FUNC(iSum(k->Sk, app3, app4));

#if DEBUG_PRINT 
    printf("Sk\n");
    vPrint(k->Sk);
#endif

    vDestroy(app);
    vDestroy(app2);
    vDestroy(app3);
    vDestroy(app4);
    vDestroy(t1);
    vDestroy(t2);

    t1  = pxTranspose(k->Hk);
    app = pxMultiply(k->Pk, t1);

#if DEBUG_PRINT
    printf("P_k\n");
    vPrint(k->Pk);
    printf("Hk'\n");
    vPrint(t1);
#endif

    vDestroy(t1);

    if (k->Sk->r == 1)
    {
        float f;
        f = 1 / k->Sk->matrix[0][0];
        SAFE_FUNC(iSc_Multiply(k->Kk, app, f));
    }

    else
    {
        t2 = pxInverse(k->Sk);

        if (iMultiply(k->Kk, app, t2))
            perror("Error Multiply");

        vDestroy(t2);
    }

    vDestroy(app);

#if DEBUG_PRINT
    printf("Kk\n");
    vPrint(k->Kk);
#endif

    /*
     * Step 2b - State estimate update
     * Firstly I check for errors, so if the quadratic of residual between the input and yhat is
     * greater than 100 times the covariance of y, then the first column of Kalman Gain is 0
     */

    for (i = 0; i < SER; i++)
    {
        for (j = 0; j < PAR; j++)
        {
            if (((y[i] - k->y_p->matrix[i + j][0]) * (y[i] - k->y_p->matrix[i + j][0])) > 100 * k->Sk->matrix[i + j][i + j])
            {
                for (z = i + j; z < 3 + i + j; z++)
                    k->Kk->matrix[z][i + j] = 0;
            }
        }
    }

#if DEBUG_PRINT
    printf("Kk\n");
    vPrint(Kk);
#endif

    for (i = 0; i < SER; i++)
    {
        k->y_p->matrix[i * PAR][0] = y[i] - k->y_p->matrix[i * PAR][0];

        for (j = 1; j < PAR; j++)
            k->y_p->matrix[i * PAR + j][0] = k->y_p->matrix[i * PAR][0];

    }
#if DEBUG_PRINT
    printf("r_k\n");
    vPrint(y_p);
#endif

    t1   = pxCopy(k->x);
    app = pxMultiply(k->Kk, k->y_p);

    SAFE_FUNC(iSum(k->x, t1, app));

#if DEBUG_PRINT
    printf("x_k\n");
    vPrint(k->x);
#endif
    vDestroy(app);
    vDestroy(t1);

    /* Check if values are between ranges */
    for (i = 0; i < PAR * SER; i++)
    {
        if (k->x->matrix[Z_IND + i][0] > 1) k->x->matrix[Z_IND + i][0] = 1;
        if (k->x->matrix[Z_IND + i][0] < 0) k->x->matrix[Z_IND + i][0] = 0;
        if (k->x->matrix[H_IND + i][0] > 1) k->x->matrix[H_IND + i][0] = 1;
        if (k->x->matrix[H_IND + i][0] < -1) k->x->matrix[H_IND + i][0] = -1;
    }

    /* Step 2c - Error covariance measurement update */

    t1   = pxTranspose(k->Kk);
    app  = pxMultiply(k->Kk, k->Sk);
    app2 = pxMultiply(app, t1);
    app3 = pxCopy(k->Pk);

    SAFE_FUNC(iSubtract(k->Pk, app3, app2));

#if DEBUG_PRINT
    printf("Pk\n");
    vPrint(k->Pk);
#endif
    vDestroy(app);
    vDestroy(app2);
    vDestroy(app3);
    vDestroy(t1);

    /* If I have a bad voltage estimate I'm going to bump up SigmaX by mupltiplying it with Q param 
    for (i = 0; i < SER; i++)
    {
        for (j = 0; j < PAR; j++)
        {
            if (((y[i] - y_p->matrix[i * PAR + j][0]) * (y[i] - y_p->matrix[i * PAR + j][0])) > 4 * Sk->matrix[i * PAR + j][i * PAR + j])
                k->Pk->matrix[Z_IND + i * PAR + j][Z_IND + i * PAR + j] *= Q_bump;
        }
    }*/
    
    #if DEBUG3
        printf("Kalman Filter Step 2 End\n");
    #endif
}


/********************************************************************************
*                                                                               *
* FUNCTION NAME: getParam                                                       *
*                                                                               *
* PURPOSE: This function returns the Parameters based on Temperature            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type          IO     Description                                    *
* --------- --------      --     ---------------------------------              *
* Params    float*        O      Parameters at Temp T                           *
* T         const float   I      Temperature of the cell                        *
* P         const Matrix* I      Matrix of the cell's parameters                *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/
static void getParam(float* Params, const float T, const Matrix* P)
{
    /* LOCAL VARIABLES:
     * Variable      Type    Description
     * ------------- ------- -------------------------------------
     * i             size_t  Loop counter
     * j             size_t  Loop counter
     */

    size_t i;
    size_t j;

    if (T <= P->matrix[TEMP][0])
    {
        for (i = 0; i < PARAM_SIZE; i++)
            Params[i] = P->matrix[i][0];
    }
    else if (T >= P->matrix[TEMP][P->c-1]) 
    {
        for (size_t i = 0; i < PARAM_SIZE; i++)
            Params[i] = P->matrix[i][P->c];
    }
    else {

        for (j = 0; j < P->c; j++)
        {
            if (P->matrix[TEMP][j] <= T && P->matrix[TEMP][j + 1] > T)
                break;
        }

        for (size_t i = 0; i < PARAM_SIZE - 1; i++)
            Params[i] = P->matrix[i][j];

    }

}



/********************************************************************************
*                                                                               *
* FUNCTION NAME: SOCfromOCV                                                     *
*                                                                               *
* PURPOSE: Computing State of Charge starting from Open Circuit Voltage         *
*  Computing state of charge by interpolation. Using the function:              *
*      y=(x-x1)(y1-y0)/(x1-x0)+y0                                               *
*  Where y is SOC, computed as:                                                 *
*      SOC(z,t)=SOC0(z)+T*SOCrel(z)                                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type          IO     Description                                    *
* --------- --------      --     ---------------------------------              *
* ocv       const float   I      OCV at time t                                  *
* T         const float   I      Temperature of the cell                        *
* OvS       const Matrix* I      Matrix of OCV vs SOC                           *
*                                                                               *
* RETURN VALUE: s2                                                              *
*             being cell's SOC                                                  *
*                                                                               *
********************************************************************************/
static float SOCfromOCV(const float ocv, const float T, const Matrix* OvS)
{
    /* LOCAL VARIABLES:
     * Variable      Type    Description
     * ------------- ------- ----------------
     * i             size_t  Loop counter
     * s1            float   Search help
     * s2            float   state-of-charge
     */

    size_t i;
    float s1;
    float s2;

    if (OvS->matrix[OCV][0] >= ocv) 
    {
        s1 = (OvS->matrix[SOC0][1] + T * OvS->matrix[SOCrel][1]) - (OvS->matrix[SOC0][0] + T * OvS->matrix[SOCrel][0]);
        s2 = (ocv - OvS->matrix[OCV][1]) * s1 / (OvS->matrix[OCV][1] - OvS->matrix[OCV][0]) + OvS->matrix[SOC0][1] + OvS->matrix[SOCrel][1] * T;
    }

    else
    {
        for (i = 0; i < OvS->c; i++)
        {
            if (OvS->matrix[OCV][i] <= ocv && OvS->matrix[OCV][i + 1] >= ocv)
                break;
        }
        if (i == OvS->c)
        {
            s1 = (OvS->matrix[SOC0][OvS->c] + T * OvS->matrix[SOCrel][OvS->c]) - (OvS->matrix[SOC0][OvS->c - 1] + T * OvS->matrix[SOCrel][OvS->c - 1]);
            s2 = (ocv - OvS->matrix[OCV][OvS->c]) * s1 / (OvS->matrix[OCV][OvS->c] - OvS->matrix[OCV][OvS->c - 1]) + OvS->matrix[SOC0][OvS->c] + OvS->matrix[SOCrel][OvS->c] * T;
        }
        else
        {
            s1 = (OvS->matrix[SOC0][i + 1] + T * OvS->matrix[SOCrel][i + 1]) - (OvS->matrix[SOC0][i] + T * OvS->matrix[SOCrel][i]);
            s2 = (ocv - OvS->matrix[OCV][i + 1]) * s1 / (OvS->matrix[OCV][i + 1] - OvS->matrix[OCV][i]) + OvS->matrix[SOC0][i + 1] + OvS->matrix[SOCrel][i + 1] * T;
        }
    }

    return s2;

}


/********************************************************************************
*                                                                               *
* FUNCTION NAME: SOCfromOCV                                                     *
*                                                                               *
* PURPOSE: Computing Open Circuit Voltage starting from State of Charge         *
*  Computing state of charge by interpolation. Using the function:              *
*      y=(x-x1)(y1-y0)/(x1-x0)+y0                                               *
*  Where y is SOC, computed as:                                                 *
*      OCV(z,t)=OCV0(z)+T*OCVrel(z)                                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type          IO     Description                                    *
* --------- --------      --     ---------------------------------              *
* soc       const float   I      SOC at time t                                  *
* T         const float   I      Temperature of the cell                        *
* OvS       const Matrix* I      Matrix of OCV vs SOC                           *
*                                                                               *
* RETURN VALUE: ocv                                                             *
*             being cell's OCV                                                  *
*                                                                               *
********************************************************************************/
static float OCVfromSOC(const float soc, const float T, const Matrix* OvS)
{
    /* LOCAL VARIABLES:
     * Variable      Type    Description
     * ------------- ------- ----------------
     * i             size_t  Loop counter
     * s1            float   Search help
     * ocv           float   open circuit voltage
     */

    size_t i = 0;
    float  s1 = 0;
    float  ocv = 0;

    if (OvS->matrix[SOC][0] >= soc) 
    {
        s1 = (OvS->matrix[OCV0][1] + T * OvS->matrix[OCVrel][1]) - (OvS->matrix[OCV0][0] + T * OvS->matrix[OCVrel][0]);
        ocv = (soc - OvS->matrix[SOC][1]) * s1 / (OvS->matrix[SOC][1] - OvS->matrix[SOC][0]) + OvS->matrix[OCV0][1] + OvS->matrix[OCVrel][1] * T;
    }
    else
    {
        for (i = 0; i < OvS->c; i++)
        {
            if (OvS->matrix[SOC][i] <= soc && OvS->matrix[SOC][i + 1] >= soc)
                break;
        }
        if (i == OvS->c)
        {
            s1 = (OvS->matrix[OCV0][OvS->c] + T * OvS->matrix[OCVrel][OvS->c]) - (OvS->matrix[OCV0][OvS->c - 1] + T * OvS->matrix[OCVrel][OvS->c - 1]);
            ocv = (soc - OvS->matrix[SOC][OvS->c]) * s1 / (OvS->matrix[SOC][OvS->c] - OvS->matrix[SOC][OvS->c - 1]) + OvS->matrix[OCV0][OvS->c] + OvS->matrix[OCVrel][OvS->c] * T;
        }
        else
        {
            s1 = (OvS->matrix[OCV0][i + 1] + T * OvS->matrix[OCVrel][i + 1]) - (OvS->matrix[OCV0][i] + T * OvS->matrix[OCVrel][i]);
            ocv = (soc - OvS->matrix[SOC][i + 1]) * s1 / (OvS->matrix[SOC][i + 1] - OvS->matrix[SOC][i]) + OvS->matrix[OCV0][i + 1] + OvS->matrix[OCVrel][i + 1] * T;
        }
    }

    return ocv;

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: dOCVfromSOC                                                    *
*                                                                               *
* PURPOSE: Computing derivative of Open Circuit Voltage                         *
*           starting from State of Charge                                       *
*  Computing state of charge by interpolation. Using the function:              *
*      y=(x-x1)(y1-y0)/(x1-x0)+y0                                               *
*  Where y is SOC, computed as:                                                 *
*      dOCV(z,t)=dOCV0(z)+T*dOCVrel(z)                                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type          IO     Description                                    *
* --------- --------      --     ---------------------------------              *
* soc       const float   I      SOC at time t                                  *
* T         const float   I      Temperature of the cell                        *
* OvS       const Matrix* I      Matrix of dOCV vs SOC                           *
*                                                                               *
* RETURN VALUE: docv                                                             *
*             being cell's dOCV                                                  *
*                                                                               *
********************************************************************************/
static float dOCVfromSOC(const float soc, const float T, const Matrix* OvS)
{
    /* LOCAL VARIABLES:
     * Variable      Type    Description
     * ------------- ------- ----------------
     * i             size_t  Loop counter
     * s1            float   Search help
     * ocv           float   open circuit voltage
     */

    size_t i = 0;
    float  s1 = 0;
    float  docv = 0;

    if (OvS->matrix[SOC][0] >= soc) 
    {
        s1 = (OvS->matrix[dOCV0][1] + T * OvS->matrix[dOCVrel][1]) - (OvS->matrix[dOCV0][0] + T * OvS->matrix[dOCVrel][0]);
        docv = (soc - OvS->matrix[SOC][1]) * s1 / (OvS->matrix[SOC][1] - OvS->matrix[SOC][0]) + OvS->matrix[dOCV0][1] + OvS->matrix[dOCVrel][1] * T;
    }
    else
    {
        for (i = 0; i < OvS->c; i++)
        {
            if (OvS->matrix[SOC][i] <= soc && OvS->matrix[SOC][i + 1] >= soc)
                break;
        }
        if (i == OvS->c)
        {
            s1 = (OvS->matrix[dOCV0][OvS->c] + T * OvS->matrix[dOCVrel][OvS->c]) - (OvS->matrix[dOCV0][OvS->c - 1] + T * OvS->matrix[dOCVrel][OvS->c - 1]);
            docv = (soc - OvS->matrix[SOC][OvS->c]) * s1 / (OvS->matrix[SOC][OvS->c] - OvS->matrix[SOC][OvS->c - 1]) + OvS->matrix[dOCV0][OvS->c] + OvS->matrix[dOCVrel][OvS->c] * T;
        }
        else
        {
            s1 = (OvS->matrix[dOCV0][i + 1] + T * OvS->matrix[dOCVrel][i + 1]) - (OvS->matrix[dOCV0][i] + T * OvS->matrix[dOCVrel][i]);
            docv = (soc - OvS->matrix[SOC][i + 1]) * s1 / (OvS->matrix[SOC][i + 1] - OvS->matrix[SOC][i]) + OvS->matrix[dOCV0][i + 1] + OvS->matrix[dOCVrel][i + 1] * T;
        }
    }

    return docv;

}


/********************************************************************************
*                                                                               *
* FUNCTION NAME: vDelete                                                        *
*                                                                               *
* PURPOSE: This function destroys the object Kalman                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* k         Kalman*      IO     Kalman structure, contains                      *
*                                all matrices needed for the execution of       *
*                                the algorithm                                  *
*                                                                               *
* RETURN VALUE: void                                                            *
*                                                                               *
********************************************************************************/
void vDelete(Kalman* k) 
{
    vDestroy(k->Qk);
    vDestroy(k->Rk);
    vDestroy(k->x);
    vDestroy(k->Pk);
    vDestroy(k->OvS);
    vDestroy(k->Param);
    vDestroy(k->Fk);
    vDestroy(k->Gk);
    vDestroy(k->Hk);
    vDestroy(k->D);
    vDestroy(k->Kk);
    vDestroy(k->y_p);
    vDestroy(k->Sk);
    vDestroy(int_Gku);
}
