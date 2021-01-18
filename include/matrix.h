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
*   FILENAME:  matrix.h                                                                            *
*                                                                                                  *
*                                                                                                  *
*   PURPOSE:   Library that defines the object Matrix and all its functions                        *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
*   GLOBAL VARIABLES:                                                                              *
*                                                                                                  *
*                                                                                                  *
*   Variable        Type        Description                                                        *
*   --------        ----        -------------------                                                *
*   m               Matrix      Matrix object, contains                                            *
*                                a float**, being the matrix, and 2 int, being rows and columns    *
*                                                                                                  *
*   v           Vector      Vector object, contains                                                *
*                             a float*, being the vector, and 1 int, being rows                    *
*                                                                                                  *
* STATIC VARIABLES:                                                                                *
*                                                                                                  *
*   Name            Type         I/O      Description                                              *
*   ----            ----         ---      -----------                                              *
*   heap_usage      usigned int           Variables that counts the  bytes used in heap memory     *
*                                                                                                  *
*   DEVELOPMENT HISTORY :                                                                          *
*                                                                                                  *
*                                                                                                  *
*   Date          Author            Change Id     Release     Description Of Change                *
*   ----          ------            -------- -    ------      ----------------------               *
*   09-06-2020    N.di Gruttola                     1         Added comments, code satisfies       *
*                  Giardino                                    iso9899:1999, as requested per      *
*                                                              MISRA-C:2004                        *
*                                                                                                  *
*   23-11-2020   N.di Gruttola                      2         Modified whole library, modified     *
*                   Giardino &                                  nomenclature                       *
*                G. Di Cecio                                                                       *
*                                                                                                  *
***************************************************************************************************/

#ifndef MATRIX_h
#define MATRIX_h

/* Include Global Parameters */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* Definition of Macros */

#define signum(op)                             \
    ({                                         \
        float retf = (op);                     \
        (retf == 0 ? 0 : (retf < 0 ? -1 : 1)); \
    })

/*
* Matrix Object:
*       float** being the pointer to the matrix
*       int c and r are no. of columns and no. of rows
*/

/* Declare Static Variables */
static int heap_usage = 0;

/* Declare Global Variables */

/*
* Matrix Object:
*       float** being the pointer to the matrix
*       int c and r are no. of columns and no. of rows
*/

typedef struct Matrix 
{
    float** matrix;
    unsigned int     c;
    unsigned int     r;
}Matrix;

typedef struct Vector
{
    float* vector;
    unsigned int    n;
}Vector;

/* Declare Prototypes */
int      iZeroMat        (Matrix *);                      
Matrix*  pxCreate        (unsigned int , unsigned int);
int      iResize         (Matrix*, unsigned int , unsigned int);
Vector*  pxVectorCreate  (unsigned int);                           
void     vDestroy        (Matrix *);                      
void     vVectorDestroy  (Vector *);                      
int      iSum            (Matrix*, Matrix *, Matrix *);   
Matrix*  pxSum           (Matrix*, Matrix*);              
int      iMultiply       (Matrix*, Matrix *, Matrix *);   
Matrix*  pxMultiply      (Matrix*, Matrix*);              
int      iSubtract       (Matrix*, Matrix *, Matrix *);   
Matrix*  pxSubtract      (Matrix*, Matrix*);              
int      iSc_Multiply    (Matrix*, Matrix *, float);      
Matrix*  pxSc_Multiply   (Matrix*, float);                
int		 iInverse        (Matrix *,Matrix *);             
Matrix*  pxInverse       (Matrix*);                                
int      iIdentity       (Matrix *);                      
Matrix*  pxIdentity      (unsigned int);                           
int      iTranspose      (Matrix *, Matrix *);            
Matrix*  pxTranspose     (Matrix*);                       
int      iEquals         (Matrix *, Matrix *);            
float    fDeterminant    (Matrix *);                      
void     vPrint          (Matrix *);                      
void     vPrintVector    (Vector* );                      
//int      iEigenvalues    (float*, Matrix *);            
int      iEigenvalues    (Vector*, Matrix *);             
int      iCopy           (Matrix *, Matrix *);            
Matrix*  pxCopy          (Matrix*);                       
int      iRowSwap        (Matrix *, unsigned int, unsigned int);            
int      iReduce         (Matrix *, unsigned int , unsigned int , float);   
int      iChol           (Matrix*, Matrix *);             
Matrix*  pxChol          (Matrix*);                       
int      iLU             (Matrix *, Matrix *, Matrix *);  
int      iSqrtm          (Matrix*, Matrix *);             
Matrix*  pxSqrtm         (Matrix*);                       
int      iExpm           (Matrix*, Matrix *, float);      
Matrix*  pxExpm          (Matrix*, float);                
int		 iBlkdiag        (Matrix*,Matrix *, Matrix *, Matrix *); 
Matrix*  pxBlkdiag       (Matrix*, Matrix*, Matrix*);     
int 	 iDiag           (Matrix *, Matrix *);            
Matrix*  pxDiag          (Matrix*);                       
float    fInterp1        (float , float *, float *);      
float*   pxCsvToFloat    (const char *, int*);            
float    fRandn          ();                              
void     vSeed           (const float);                   
int    uGetHeapUsage   ();                              

#endif /* matrix_h */
