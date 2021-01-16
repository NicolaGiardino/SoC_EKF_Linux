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
* FILE NAME: matrix.c                                                                               *
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
* Source: <matrix.h>                                                                                *
*                                                                                                   *
* Name          Type    IO Description                                                              *
* ------------- ------- -- -----------------------------                                            *
*   m           Matrix      Matrix object, contains                                                 *
*                             a float**, being the matrix, and 2 int, being rows and columns        *
*                                                                                                   *
*   v           Vector      Vector object, contains                                                 *
*                             a float*, being the vector, and 1 int, being rows                     *
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
*   09-06-2020    N.di Gruttola                     1         Added comments, code satisfies        *
*                  Giardino                                    iso9899:1999, as requested per       *
*                                                              MISRA-C:2004                         *
*                                                                                                   *
*   23-11-2020   N.di Gruttola                      2         Modified whole library, modified      *
*                   Giardino &                                  nomenclature                        *
*                G. Di Cecio                                                                        *
*                                                                                                   *
*                                                                                                   *
****************************************************************************************************/

/* Include Global Parameters */

#include "../include/matrix.h"

/* Declare Prototypes */

static float  vec_mult             (float *, float *, unsigned int);
static int    row_scalar_multiply  (Matrix *, unsigned int , float);

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxCreate                                                       *
*                                                                               *
* PURPOSE: Creates the object Matrix, and then fills it with zeros              *
*           returning the pointer to the created matrix                         *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *  
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* r         int          I      Number of rows                                  *
* c         int          I      Number of columns                               *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix* pxCreate(unsigned int r, unsigned int c)
{
	size_t i;

    Matrix *m = (Matrix*) malloc(sizeof(Matrix));
	  heap_usage += sizeof(Matrix);
    m->matrix=(float**)malloc(r*sizeof(float*));
	  heap_usage += r*sizeof(float*);
    for (i=0; i < r; i++)
    {
        m->matrix[i] =(float *)malloc(c * sizeof(float));
        heap_usage += c*sizeof(float);
    }

    m->c = c;
    m->r = r;

    iZeroMat(m);
    return (Matrix*) m;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iResize                                                        *
*                                                                               *
* PURPOSE: Resizes the object Matrix                                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I/O    Matrix to resize                                *
* r         int          I      Number of rows                                  *
* c         int          I      Number of columns                               *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/

int iResize(Matrix* m, unsigned int r, unsigned int c)
{

    if (r < m->r || c < m->c)
    {
        perror("Invalid values");
        return -1;
    }

    heap_usage -= (m->r + m->c);

    size_t i;

    m->matrix=(float**)realloc(m->matrix, r*sizeof(float*));
	  heap_usage += r*sizeof(float*);

    for (i=0; i < r; i++)
    {
        m->matrix[i] =(float *)realloc(m->matrix[i], c * sizeof(float));
        heap_usage += c*sizeof(float);
    }
    m->c = c;
    m->r = r;

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxVectorCreate                                                 *
*                                                                               *
* PURPOSE: Creates the object Matrix, and then fills it with zeros              *
*           returning the pointer to the created matrix                         *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* n         int          I      Number of rows                                  *
*                                                                               *
* RETURN VALUE: Vector*                                                         *
********************************************************************************/

Vector* pxVectorCreate(unsigned int n)
{

    Vector* v = (Vector*) malloc(sizeof(Vector));
    heap_usage += sizeof(Vector);
    v->n = n;
    v->vector = (float*) malloc(n*sizeof(float));
    heap_usage += n*sizeof(float);

    return v;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vVectorDestroy                                                 *
*                                                                               *
* PURPOSE: Destroys the Object                                                  *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* v         Vector*      I      Vector to free                                  *
*                                                                               *
* RETURN VALUE: int [true or false]                                             *
********************************************************************************/

void vVectorDestroy(Vector* v)
{

    if(v != NULL)
    {
        free(v->vector);
        heap_usage -= (v->n)*sizeof(float);
        free(v);
        heap_usage -= sizeof(Vector);
    }

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vDestroy                                                       *
*                                                                               *
* PURPOSE: Destroys the Object                                                  *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Matrix to free                                  *
*                                                                               *
* RETURN VALUE: int [true or false]                                             *
********************************************************************************/

void vDestroy(Matrix* m)
{
    if(m!=NULL)
    {
        Matrix* p = m;
        size_t i;
        for (i=0; i < p->r; i++){
	        heap_usage -= (p->c)*sizeof(float);
	        free(p->matrix[i]);
        }
	    heap_usage -= (p->r)*sizeof(float*);
        free(p->matrix);
	    heap_usage -= sizeof(Matrix);
        free(m);
    }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iInverse                                                       *
*                                                                               *
* PURPOSE: Creates the inverse of the matrix given as input,                    *
*            returning -1 if failed, , 0 if successful	                        *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the object to invert                 *
* invert    Matrix*      O      Pointer to the object to invert                 *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iInverse(Matrix *invert, Matrix *m)
{
    size_t i;
    size_t j;
    size_t l;

    float factor;

    if (m == NULL)
    {
        return NULL;
    }
    if ((m)->r != (m)->c)
    {
        return NULL;
    }
    /* reduce each of the rows to get a lower triangle */
    for (i = 0; i < (m)->r; i++)
    {
        for (j = i + 1; j < (m)->c; j++)
        {
            if ((m)->matrix[i][i] == 0)
            {
                for (l = i + 1; l < m->c; l++)
                {
                    if (m->matrix[l][l] != 0)
                    {
                        iRowSwap(m, i, l);
                        break;
                    }
                }
                continue;
            }
            factor = (m)->matrix[i][j] / ((m)->matrix[i][i]);
            iReduce(invert, i, j, factor);
            iReduce((m), i, j, factor);
        }
    }

    /* now finish the upper triangle  */
    for (i = (m)->r - 1; i > 0; i--)
    {
        for (j = i - 1; j >= 0; j--)
        {
            if ((m)->matrix[i][i] == 0)
                continue;
            if (j == -1)
                break;
            factor = (m)->matrix[i][j] / ((m)->matrix[i][i]);
            iReduce(invert, i, j, factor);
            iReduce((m), i, j, factor);
        }
    }

    /* scale everything to 1 */
    for (i = 0; i < (m)->r; i++)
    {
        if ((m)->matrix[i][i] == 0)
            continue;
        factor = 1 / ((m)->matrix[i][i]);
        row_scalar_multiply(invert, i, factor);
        row_scalar_multiply((m), i, factor);
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxInverse                                                      *
*                                                                               *
* PURPOSE: Creates the inverse of the matrix given as input,                    *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object to invert                 *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix *pxInverse(Matrix *m)
{
    Matrix *inv = pxIdentity((m)->c);
    int check = iInverse(inv, m);
    
    if (check < 0)
    {
        vDestroy(inv);
        perror("Error Inverse");
        return NULL;
    }
    else
        return inv;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iZeroMat                                                        *
*                                                                               *
* PURPOSE: Fills the given matrix with zeros                                    *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      IO     Pointer to the object to fill                   *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/

int iZeroMat(Matrix* m)
{
    size_t i;
    size_t j;
    for (i=0; i<m->r; i++)
    {
        for(j=0;j<m->c;j++)
            m->matrix[i][j]=0;
    }

    return 0;
}


/********************************************************************************
*                                                                               *
* FUNCTION NAME: iSum                                                           *
*                                                                               *
* PURPOSE: Computes the sum of the matrices                                     *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
* s         Matrix*      O      Pointer to the sum object                       *
*                                                                               *
* RETURN VALUE: int                                                        		*
********************************************************************************/
int iSum(Matrix* s, Matrix* m1, Matrix* m2)
{
    if (m1 == NULL || m2 == NULL)
    {
        printf("Nullptr\n");
        return -1;
    }

    if(m1->r != m2->r || m1->c != m2->c)
    {
        printf("Different\n");
        return -1;
    }

    size_t i;
    size_t j;
    for (i=0; i<m1->r; i++)
    {
        for (j=0; j<m1->c; j++)
            s->matrix[i][j]=m1->matrix[i][j]+m2->matrix[i][j];
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxSum                                                          *
*                                                                               *
* PURPOSE: Computes the sum of the matrices                                     *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
*                                                                               *
* RETURN VALUE: Matrix*                                                    		*
********************************************************************************/

Matrix* pxSum (Matrix* m1, Matrix* m2)
{
    Matrix* m3 = pxCreate(m1->c, m1->r);
    int check = iSum(m3,m1,m2);
    if(check<0)
    {
        vDestroy(m3);
        return NULL;
    }
    else
        return m3;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iSubtract                                                      *
*                                                                               *
* PURPOSE: Computes the subtraction of the matrices                             *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
* s         Matrix*      O      Pointer to the subtract object                  *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iSubtract(Matrix *s,Matrix* m1,Matrix* m2)
{
    if(m1==NULL || m2==NULL)
    {
        return -1;
    }
    if(m1->r != m2->r || m1->c != m2->c)
    {
        return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m1->r; i++)
    {
        for (j=0; j<m1->c; j++)
        {
            s->matrix[i][j]=m1->matrix[i][j]-m2->matrix[i][j];
        }
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxSubtract                                                     *
*                                                                               *
* PURPOSE: Computes the subtraction of the matrices                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix* pxSubtract(Matrix* m1, Matrix* m2)
{
    Matrix* m3 = pxCreate(m1->r, m2->c);
    int check = iSubtract(m3,m1,m2);
    if(check<0)
    {
        vDestroy(m3);
        return NULL;
    }
    else
        return m3;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iSc_multiply                                                   *
*                                                                               *
* PURPOSE: Multiplies the matrix to a scalar                                    *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* f         float        I      Multiplier                                      *
* s         Matrix*      O      Pointer to the result object     	            *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iSc_Multiply(Matrix *s, Matrix* m1, float f)
{
    if(m1==NULL)
    {
        return -1;
    }
	if((s->c != m1->c) || (s->r != m1->r))
    {
		return -1;
    }
    size_t i;
    size_t j;
    for (i = 0; i < m1->r; i++)
    {
        for (j = 0; j < m1->c; j++)
        {
            s->matrix[i][j] = m1->matrix[i][j] * f;
        }
    }

    return 0;
}
/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxSc_Multiply                                                  *
*                                                                               *
* PURPOSE: Multiplies the matrix to a scalar                                    *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* f         float        I      Multiplier                                      *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/


Matrix* pxSc_Multiply(Matrix* m, float f)
{
    Matrix* m3 = pxCreate(m->r,m->c);
    int check = iSc_Multiply(m3,m,f);
    if(check<0)
    {
        vDestroy(m3);
        return NULL;
    }
    else
        return m3;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iEquals                                                        *
*                                                                               *
* PURPOSE: Compares the 2 matrices                                              *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iEquals(Matrix* m1, Matrix* m2)
{
    if(m1==NULL || m2==NULL)
    {
        return -1;
    }
    if(m1->r != m2->r || m1->c != m2->c)
    {
        return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m1->r; i++)
    {
        for (j=0; j<m1->c; j++)
        {
            if(m1->matrix[i][j]!=m2->matrix[i][j])
                return 0;
        }
    }

    return 1;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iMultiply                                                      *
*                                                                               *
* PURPOSE: Multiplies the 2 matrices                                            *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object to multiply           *
* m2        Matrix*      I      Pointer to the 2nd object to multiply           *
* product   Matrix*      O      Pointer to the product object					*
*					                                                            *
* RETURN VALUE: int 	                                                        *
********************************************************************************/
int iMultiply(Matrix* product,Matrix *m1, Matrix *m2)
{

    size_t i;
    size_t j;
    size_t k;
    if (m1 == NULL || m2 == NULL)
    {
        return -1;
    }
    if(m1->c != m2->r)
    {
        return -1;
    }
    if((m1->r!=product->r) || (m2->c!=product->c))
    {
    	return -1;
    }

    for (i = 0; i < m1->r; ++i)
    {
        for (j = 0; j < m2->c; ++j)
        {
            for (k = 0; k < m1->c; ++k)
            {
                product->matrix[i][j] += m1->matrix[i][k] * m2->matrix[k][j];
            }
        }
    }
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxMultiply                                                     *
*                                                                               *
* PURPOSE: Multiplies the 2 matrices                                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object to multiply           *
* m2        Matrix*      I      Pointer to the 2nd object to multiply           *
*					                                                            *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix*  pxMultiply (Matrix* m1, Matrix* m2)
{

    Matrix* m3 = pxCreate(m1->r, m2->c);
    int check = iMultiply(m3,m1,m2);
    if(check<0)
    {
        vDestroy(m3);
        return NULL;
    }
    else
        return m3;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iTranspose                                                     *
*                                                                               *
* PURPOSE: Transposes the matrix                                                *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
* t         Matrix*      O      Pointer to the result object					*
*                                                                               *
* RETURN VALUE: int                                    		                    *
********************************************************************************/
int iTranspose(Matrix *t, Matrix* m)
{
    if(m==NULL)
    {
        return -1;
    }
    if((m->c != t->r) || (m->r != t->c))
	{
    	return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<t->r; i++)
    {
        for (j=0; j<t->c; j++)
            t->matrix[i][j]=m->matrix[j][i];
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxTranspose                                                    *
*                                                                               *
* PURPOSE: Transposes the matrix                                                *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
*                                                                               *
* RETURN VALUE: int                                    		                    *
********************************************************************************/

Matrix*  pxTranspose (Matrix* m)
{
    Matrix* t = pxCreate(m->c, m->r);
    int check = iTranspose(t, m);
    if(check<0)
    {
        vDestroy(t);
        return NULL;
    } 
    else
        return t;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iIdentity                                                      *
*                                                                               *
* PURPOSE: Fills the given matrix with ones on main diagonal                    *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      IO     Pointer to the object to fill                   *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iIdentity(Matrix* m)
{
    if(m->c != m->r)	//if it's not square matrix
    {
        return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m->r; i++)
    {
        for (j=0; j<m->c; j++)
            m->matrix[i][j]=(i==j);
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxIdentity                                                     *
*                                                                               *
* PURPOSE: Fills the given matrix with ones on main diagonal                    *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* n         int           I     Number of r/c of the matrix                     *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix* pxIdentity (unsigned int n)
{
    Matrix* m = pxCreate(n,n);
    int check = iIdentity(m);
    if(check<0)
    {
        vDestroy(m);
        return NULL;
    } 
    else
        return m;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: fDeterminant                                                    *
*                                                                               *
* PURPOSE: Computes the determinant using LU decomposition                      *
*            returns -1 if failed                                               *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
*                                                                               *
* RETURN VALUE: float                                                           *
********************************************************************************/
float fDeterminant(Matrix* m)
{
    if(m==NULL || (m->c!=m->r))
    {
        return -1;
    }
    Matrix* L;
    L=pxCreate(m->r,m->c);
    Matrix* U;
    U=pxCreate(m->r,m->c);


    float detL=0;
    float detU=0;

    size_t i;
    size_t j;

    for (i=0; i<m->r; i++)
    {
        float sum1=0;
        float sum2=0;
        for (j=0; j<m->r-i; j++)
        {
            sum1*=L->matrix[i][j];
            sum2*=U->matrix[i][j];
        }
        detL+=sum1;
        detU+=sum2;
    }
	vDestroy(L);
	vDestroy(U);
    return detL*detU;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iLU                                                            *
*                                                                               *
* PURPOSE: Compares the LU transformation                                       *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
* L         Matrix*      O      Pointer to the L decomposed object              *
* U         Matrix*      O      Pointer to the U decomposed object              *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iLU(Matrix* m, Matrix* L, Matrix* U)
{
	if((m->c != m->r) || m==NULL) //if it's not square matrix
    {
		return -1;
    }
    iZeroMat(L);
    iZeroMat(U);
    size_t i;
    size_t j;
    size_t k;
    for (i = 0; i < m->c; i++)
    {
        int sum=0;

        for(k=i;k<m->r;k++)
        {

            //sum of Lij*Ujk
            for (j=0; j<i; j++)
                sum+=(L->matrix[i][j]*U->matrix[j][k]);

            U->matrix[i][k]=m->matrix[i][k]-sum;
        }
        for(k=i;k<m->r;k++)
        {
            if(i==k)
                L->matrix[i][i]=1;

            else
            {
                int sum=0;
                for (j=0;j<i;j++)
                    sum+=(L->matrix[k][i]*U->matrix[j][i]);

                L->matrix[k][i]=(m->matrix[k][i]-sum/U->matrix[i][i]);
            }
        }
    }
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iEigenvalues                                                   *
*                                                                               *
* PURPOSE: Computes the eigenvalues of the matrix                               *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the 1st object                       *
* values    float*       O      Pointer to the eigenvalues object               *
*                                                                               *
* RETURN VALUE: int		                                                        *
********************************************************************************/
/*
int iEigenvalues(float* values, Matrix *m)
{
    float factor;
    Matrix *r = pxCreate(m->r, m->c);
    unsigned int i, j, l;
    if(m == NULL)
        return -1;
    if(m->r != m->c)
        return -1;
    iCopy(r,m);
    
    for(i = 0; i < r->r; i++)
    {
        for(j = i + 1; j < r->c; j++)
        {
            if(r->matrix[i][i] == 0)
            {
                for(l = i+1; l < r->c; l++)
                {
                    if(r->matrix[l][l] != 0)
                    {
                        iRowSwap(r, i, l);
                        break;
                    }
                }
                continue;
            }
            factor = r->matrix[i][j]/(r->matrix[i][i]);
            iReduce(r, i, j, factor);
        }
    }
    for(i = 0; i < r->r; i++)
        values[i] = r->matrix[i][i];
	iDestroy(r);
    return 0;
}*/

int iEigenvalues(Vector* values, Matrix *m)
{
    float factor;
    Matrix *r = pxCreate(m->r, m->c);
    size_t i;
    size_t j;
    size_t l;
    if (m == NULL)
    {
        return -1;
    }
    if(m->r != m->c)
    {
        return -1;
    }
    iCopy(r,m);
    /* reduce each of the rows to get a lower triangle */
    for(i = 0; i < r->r; i++)
    {
        for(j = i + 1; j < r->c; j++)
        {
            if(r->matrix[i][i] == 0)
            {
                for(l = i+1; l < r->c; l++)
                {
                    if(r->matrix[l][l] != 0)
                    {
                        iRowSwap(r, i, l);
                        break;
                    }
                }
                continue;
            }
            factor = r->matrix[i][j]/(r->matrix[i][i]);
            iReduce(r, i, j, factor);
        }
    }
    for(i = 0; i < r->r; i++)
    {
        values->vector[i] = r->matrix[i][i];
    }
    vDestroy(r);
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vPrint	                                                        *
*                                                                               *
* PURPOSE: Prints the matrix                                                    *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
*                                                                               *
* RETURN VALUE: void                                                            *
********************************************************************************/
void vPrint(Matrix* m)
{
  if(m!=NULL){
    size_t i;
    size_t j;
    for (i = 0; i < m->r; i++)
    {
        for (j = 0; j < m->c; j++)
        {
            printf("%f\t", m->matrix[i][j]);
        }
        printf("\n");
    }
  }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vPrintVector                                                   *
*                                                                               *
* PURPOSE: Prints the vector                                                    *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* v         Vector*      I      Pointer to the object                           *
*                                                                               *
* RETURN VALUE: void                                                            *
********************************************************************************/

void vPrintVector(Vector* v)
{

    size_t i;

    if (v != NULL)
    {
        for (i = 0; i < v->n; i++)
            printf("%f\t", v->vector[i]);
        printf("\n");
    }

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vec_mult                                                       *
*                                                                               *
* PURPOSE: Multiplies two vectors, declared as static,                          *
*           to be used in this header only                                      *
*           returning -1 if failed,                                             *
*           the pointer to the scalar if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* v1        float*       I      Pointer to the 1st object                       *
* v2        float*       I      Pointer to the 2nd object                       *
* lenght    int          I      Lenght of the vectors
*                                                                               *
* RETURN VALUE: float                                                           *
********************************************************************************/
static float vec_mult(float* v1,float* v2 ,unsigned int lenght)
{
    if(v1==NULL || v2==NULL || lenght<=0)
    {
        return -1;
    }
    float v=0;
    size_t i;
    for (i = 0; i < lenght; i++)
    {
        v += (v1[i] * v2[i]);
    }
    return v;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iCopy                                                           *
*                                                                               *
* PURPOSE: Copies the matrix into first parameter                               *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* c         Matrix*      O      Pointer to the object   						*
* m         Matrix*      I      Pointer to the object						    *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iCopy(Matrix *c, Matrix* m)
{
    if(m==NULL)
    {
        return -1;
    }
	if(c->r != m->r)
	{
    	return -1;
    }
    if(c->c != m->c)
	{
    	return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m->r; i++)
    {
        for (j=0; j<m->c; j++)
            c->matrix[i][j] = m->matrix[i][j];
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxCopy                                                         *
*                                                                               *
* PURPOSE: Copies the matrix into first parameter                               *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object						    *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix*  pxCopy (Matrix* m)
{
    Matrix* c = pxCreate(m->r,m->c);
    int check = iCopy(c,m);
    if(check<0) 
    {
        vDestroy(c);
        return NULL;
    } 
    else
        return c;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iRrowSwap                                                      *
*                                                                               *
* PURPOSE: Swaps two rows of the matrix                                         *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      IO     Pointer to the object                           *
* a         int          I      1st row to swap                                 *
* b         int          I      2nd row to swap                                 *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iRowSwap(Matrix* m, unsigned int a, unsigned int b)
{
    float temp;
    size_t i;

    if(m == NULL)
    {
        return -1;
    }
    if(m->c <= a || m->c <= b)
    {
        return -1;
    }
    for(i = 0; i < m->r; i++)
    {
        temp = m->matrix[i][a];
        m->matrix[i][a] = m->matrix[i][b];
        m->matrix[i][b] = temp;
    }
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iReduce                                                        *
*                                                                               *
* PURPOSE: reduces a piece of the matrix by a factor f                          *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
* a         int          I      First column                                    *
* b         int          I      Second column                                   *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iReduce(Matrix* m, unsigned int a, unsigned int b,float f)
{
    size_t i;
    if(m == NULL)
    {
        return -1;
    }
    if(m->c < a || m->c < b)
    {
        return -1;
    }
    for(i = 0; i < m->r; i++)
    {
        m->matrix[i][b]  -= m->matrix[i][a]*f;
    }
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iChol                                                          *
*                                                                               *
* PURPOSE: Computes the Cholesky factorization of the matrix                    *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
* L			Matrix*		 O		Pointer to the result object					*
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
int iChol(Matrix* L, Matrix* m)
{
    if (L == NULL || m == NULL)
    {
        return -1;
    }
    if((L->r != m->r) || (L->c != m->c))
	{
    	    return -1;
    }
    size_t i;
    size_t j;
    size_t k;
    for (i = 0; i < L->c; i++)
    {
        for (j = 0; j < (i + 1); j++)
        {
            float s = 0;
            for (k = 0; k < j; k++)
                s += L->matrix[i][k] * L->matrix[j][k];
            L->matrix[i][j] = (i == j) ?
                sqrt((m->matrix[i][i]) - s) :
                (1.0 / L->matrix[j][j] * (m->matrix[i][j] - s));
        }
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxChol                                                         *
*                                                                               *
* PURPOSE: Computes the Cholesky factorization of the matrix                    *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix*  pxChol (Matrix* m)
{
    Matrix* c = pxCreate(m->r,m->c);

    int check = iChol(c,m);
    if(check<0) 
    {
        vDestroy(c);
        return NULL;
    } 
    else
        return c;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iSqrtm                                                         *
*                                                                               *
* PURPOSE: Computes the square root of the matrix's elements                    *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
* a			Matrix*		 O		Pointer to the result object					*
*                                                                               *
* RETURN VALUE: int                                                        		*
********************************************************************************/
int iSqrtm(Matrix *a, Matrix* m)
{
    if(m==NULL)
    {
        return -1;
    }
    if((a->r != m->r) || (a->c != m->c))
	{
    	return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m->r; i++)
    {
        for (j=0; j<m->c; j++)
            a->matrix[i][j]=sqrt(m->matrix[i][j]);
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: piSqrtm                                                        *
*                                                                               *
* PURPOSE: Computes the square root of the matrix's elements                    *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the object                           *
*                                                                               *
* RETURN VALUE: Matrix*                                                   		*
********************************************************************************/

Matrix*  pxSqrtm (Matrix* m)
{
    Matrix* c = pxCreate(m->r,m->c);
  
    int check = iSqrtm(c,m);
    if(check<0) 
    {
        vDestroy(c);
        return NULL;
    } 
    else
    {
        return c;
    }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iExpm                                                          *
*                                                                               *
* PURPOSE: Computes the exponential of the matrix's elements                    *
*           returning -1 if failed, 0 if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* f         float        I      Exponential                                     *
* a			Matrix*      O      Pointer to the result object					*
*                                                                               *
* RETURN VALUE: int		                                                        *
********************************************************************************/
int iExpm(Matrix* a,Matrix* m,float e)
{
    if(m==NULL)
    {
        return -1;
    }
    if((a->r != m->r) || (a->c != m->c))
	{
    	return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m->r; i++)
    {
        for (j=0; j<m->c; j++)
        {
            a->matrix[i][j]=pow(m->matrix[i][j],e);
        }
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxExpm                                                         *
*                                                                               *
* PURPOSE: Computes the exponential of the matrix's elements                    *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* f         float        I      Exponential                                     *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix*  pxExpm (Matrix *m, float f)
{
    Matrix* c = pxCreate(m->r,m->c);
    
    int check = iExpm(c,m,f);
    if(check<0) 
    {
        vDestroy(c);
        return NULL;
    } 
    else
    {
        return c;
    }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: row_scalar_multiply                                            *
*                                                                               *
* PURPOSE: multiplies a row by a factor                                         *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m         Matrix*      I      Pointer to the 1st object                       *
* row       float        I      No. of row                                      *
* factor    float        I      factor to multiply                              *
*                                                                               *
* RETURN VALUE: int                                                             *
********************************************************************************/
static int row_scalar_multiply(Matrix *m, unsigned int row, float factor)
{
    size_t i;
    if(m == NULL)
    {
        return -1;
    }
    if(m->c <= row)
    {
        return -1;
    }
    for(i = 0; i < m->r; i++)
    {
        m->matrix[i][row] *= factor;
    }
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iBlkdiag                                                       *
*                                                                               *
* PURPOSE: Computes the block diagonal of the matrices                          *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
* m2        Matrix*      I      Pointer to the 3rd object                       *
* m         Matrix*		 O		Pointer to the result object					*
*                                                                               *
* RETURN VALUE: int		                                                        *
********************************************************************************/
int iBlkdiag(Matrix*m, Matrix* m1,Matrix* m2,Matrix* m3)
{
    if(m1==NULL || m2==NULL || m3==NULL)
    {
        return -1;
    }
	if((m->r != (m1->r + m2->r + m3->r)) || (m->c != (m1->c + m2->c + m3->c)))
	{
    	return -1;
    }
    size_t i;
    size_t j;
    for (i=0; i<m->r; i++)
    {
        for (j=0; j<m->c; j++)
        {
            if(j<m1->c && i<m1->r)
            {
                m->matrix[i][j]=m1->matrix[i][j];
            }
            else if(j<(m1->c+m2->c) && i<(m1->r+m2->r) && j>=m1->c && i>=m1->r)
            {
                m->matrix[i][j]=m2->matrix[i-(m1->r)][j-(m1->c)];
            }
            else if(j<(m1->c+m2->c+m3->c) && i<(m1->r+m2->r+m3->r) && j>=(m1->c+m2->c) && i>=(m1->r+m2->r))
            {
                m->matrix[i][j]=m3->matrix[i-(m1->r+m2->r)][j-(m1->c+m2->c)];
            }
        }
    }

    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxBlkdiag                                                      *
*                                                                               *
* PURPOSE: Computes the block diagonal of the matrices                          *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* m2        Matrix*      I      Pointer to the 2nd object                       *
* m2        Matrix*      I      Pointer to the 3rd object                       *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix*  pxBlkdiag (Matrix* m1, Matrix* m2, Matrix* m3)
{
    Matrix* c = pxCreate((m1->r+m2->r+m3->r),(m1->c+m2->c+m3->c));
    
    int check = iBlkdiag(c,m1,m2,m3);
    if(check<0)
    {
        vDestroy(c);
        return NULL;
    }
    else
    {
        return c;
    }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iDiag                                                          *
*                                                                               *
* PURPOSE: Computes the diagonal of the matrix                                  *
*            returns -1 if failed, 0 if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
* d  		Matrix*		 O		Pointer to the result object					*
*                                                                               *
* RETURN VALUE: int		                                                        *
********************************************************************************/
int iDiag(Matrix*d, Matrix* m)
{
    if(m==NULL)
    {
        return -1;
    }
	if((m->r!=d->r) || (d->c != m->r) )
    {
        return -1;
    }
    size_t i;
    for (i=0; i<m->r; i++)
    {
        d->matrix[i][i]=m->matrix[i][0];
    }
    return 0;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pxDiag                                                         *
*                                                                               *
* PURPOSE: Computes the diagonal of the matrix                                  *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* m1        Matrix*      I      Pointer to the 1st object                       *
*                                                                               *
* RETURN VALUE: Matrix*                                                         *
********************************************************************************/

Matrix*  pxDiag (Matrix* m)
{
    Matrix* c = pxCreate(m->r,m->c);
    
    int check = iDiag(c,m);
    if(check<0)
    {
        vDestroy(c);
        return NULL;
    } 
    else
    {
        return c;
    }
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: iInterp1                                                       *
*                                                                               *
* PURPOSE: Computes the interpolation of a set of points                        *
*           returning -1 if failed,                                             *
*           the point if successfull                                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* x_new     float        I      Point to interpolate                            *
* x         float*       I      Vector of x points                              *
* y         float*       I      Vector of y points                              *
*                                                                               *
* RETURN VALUE: float                                                           *
********************************************************************************/
float iInterp1(float x_new, float* x, float* y)
{

    size_t i;

    for (i = 0; i < sizeof(x) && x_new <= x[i]; i++);

    return (y[i] + ((x_new - x[i]) * (y[i + 1] - y[i]) / (x[i + 1] - x[i])));

}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: pCsvToFloat                                                    *
*                                                                               *
* PURPOSE: Reads from a CSV file, then passes the floats into a file            *
*           returning NULL if failed,                                           *
*           the pointer to the values if successfull                            *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* path      const char*  I      Path of the csv file                            *
* dim       int          O      Dimension of vector                             *
*                                                                               *
* RETURN VALUE: float*                                                          *
********************************************************************************/
float* pxCsvToFloat(const char* path, int* dim)
{
    FILE* myFile;
    int n = 0, i = 0;
    float* val;
    float values;

    myFile = fopen(path, "r");
    if (myFile == NULL)
    {
        perror("failed to open file");
        dim = 0;
        return NULL;
    }

    while (fscanf(myFile, "%f", &values) == 1)
    {
        n++;
        fscanf(myFile, ",");
    }
	 *dim = n+1;
    val = malloc(*dim*sizeof(*dim));
    heap_usage += *dim*sizeof(*dim);
    while (fscanf(myFile, "%f", &val[i++]) == 1)
    {
        fscanf(myFile, ",");
    }

    fclose(myFile);

    return val;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: vSeed	                                                        *
*                                                                               *
* PURPOSE: Sets the seed for a random values generator                          *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* s         const float  I      Seed                                            *
*                                                                               *
* RETURN VALUE: void                                                            *
********************************************************************************/

static float rand_n;
void vSeed(const float s)
{
    rand_n = s;
}

/********************************************************************************
*                                                                               *
* FUNCTION NAME: fRandn                                                          *
*                                                                               *
* PURPOSE: Normally distributed random numbers generator that uses a            *
*           congruent linear algorithm, with c=0, a=16807 and m=2147483647      *
*           returning -1 if failed,                                             *
*           the pointer to the value if successfull                             *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* none                                                                          *
*                                                                               *
* RETURN VALUE: float                                                           *
********************************************************************************/
float fRandn()
{
    if (rand_n > 0)
    {
        rand_n = rand_n * 16807;
        rand_n = fmodf(rand_n, 2147483647);
        return rand_n;
    }
    else
    {
        perror("Not a valid seed");
        return -1;
    }
}
/********************************************************************************
*                                                                               *
* FUNCTION NAME: uGetHeapUsage                                                  *
*                                                                               *
* PURPOSE: Returns the allocated bytes in heap memory                           *
*                                                                               *
*                                                                               *
* ARGUMENT LIST:                                                                *
*                                                                               *
* Argument  Type         IO     Description                                     *
* --------- --------     --     ---------------------------------               *
* none                                                                          *
*                                                                               *
* RETURN VALUE: __u32                                                           *
********************************************************************************/
int uGetHeapUsage()
{

    return heap_usage;

}
