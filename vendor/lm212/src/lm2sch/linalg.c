/*
 *   LMeter: Multilayer superconductive inductance extractor
 *   
 *   Copyright (C) 1992-1999 Paul Bunyk
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * File:         linalg.c
 * Description:  Implementation of a tiny linear algebra/matrix 
 *               manipulation library
 * Author:       Paul Bunyk
 * Language:     C
 * Package:      lm2sch
 * Status:       Experimental
 *
 * (C) Copyright 1998, Paul Bunyk, all rights reserved 
 * except under the provisions of Gnu Public License
 *
 */

#include "linalg.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Constructor/Destructor pair. 
 * Constructed matrices have all 0.0 elements
 */
LA_Matrix LA_create(int rows, int cols)
{
	LA_Matrix R;
	int i,j;

	if ((R = malloc(sizeof (struct LA__Matrix))) == NULL
	    ||(R->vals = malloc(sizeof(double)*rows*cols)) == NULL)
		LA_error("LA_create: Not enough memory");

	R->rows = rows;
	R->cols = cols;

	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
			LA_VAL(R,i,j) = 0.0;
	return R;	
}

void LA_delete(LA_Matrix A)
{
	free(A->vals);
	A->vals = NULL; /* will give SIGSEGV if we try to use it again */
	free(A);
}

LA_Matrix LA_unit(int size)
{
	LA_Matrix R;
	int i;
	
	R = LA_create(size, size);

	for (i=0; i<size; i++)
		LA_VAL(R,i,i) = 1.0;

	return R;
}

LA_Matrix LA_copy(LA_Matrix A)
{
	LA_Matrix R;
	int rows = A->rows, cols = A->cols;
	int i,j;

	R = LA_create(rows, cols);

	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
			LA_VAL(R,i,j) = LA_VAL(A,i,j);
	return R;	
}

LA_Matrix LA_stack(LA_Matrix A1, LA_Matrix A2)
{
	LA_Matrix R;
	int rows = A1->rows+A2->rows, cols = A1->cols;
	int i,j;

	if (cols != A2->cols) 
		LA_error("LA_stack: different column number");

	R = LA_create(rows, cols);

	for (i=0; i<A1->rows; i++)
		for (j=0; j<cols; j++)
			LA_VAL(R,i,j) = LA_VAL(A1,i,j);
	for (i=0; i<A2->rows; i++)
		for (j=0; j<cols; j++)
			LA_VAL(R,i+A1->rows,j) = LA_VAL(A2,i,j);
	return R;
}

/* Internal function: reuse R if not NULL or create a new one
 */
static LA_Matrix LA_use_or_create(LA_Matrix R, int rows, int cols)
{
	int i, j;

	if (R==NULL) 
		R = LA_create(rows, cols);
	else
	{
		if (R->rows != rows || R->cols != cols)
			LA_error("LA_use_or_create: incorrect shape, can't reuse");
		for (i=0; i<rows; i++)
			for (j=0; j<cols; j++)
				LA_VAL(R, i,j) = 0.0;
	}
	return R;
}

LA_Matrix LA_transpose(LA_Matrix R, LA_Matrix A)
{
	int rows = A->cols, cols = A->rows;
	int i,j;

	R = LA_use_or_create(R, rows, cols);

	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
			LA_VAL(R,i,j) = LA_VAL(A,j,i);
	return R;
}

LA_Matrix LA_dot(LA_Matrix R, LA_Matrix A1, LA_Matrix A2)
{
	int rows = A1->rows, cols = A2->cols, inner = A1->cols;
	int i,j,k;

	if (inner != A2->rows) 
		LA_error("LA_dot: incompatible shape");
	
	R = LA_use_or_create(R, rows, cols);

	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
			for (k=0; k<inner; k++)
				LA_VAL(R,i,j) += LA_VAL(A1,i,k)*LA_VAL(A2,k,j);
	return R;
}

/* Returns NULL if A is singular
 */
LA_Matrix LA_inverse(LA_Matrix A)
{
	/* This is Gauss-Jordan Elimination algorithm written
	 * along the lines of "Numerical Recipes in C", 2.1, page 36.
	 */
	int n = A->rows;
	int *indxc, *indxr, *ipiv;
	int i,icol, irow,j,k,l,ll;

	if (A->rows != A->cols) 
		LA_error("LA_inverse: non-square matrix");

	indxc = malloc(sizeof(int)*n);
	indxr = malloc(sizeof(int)*n);
	ipiv  = malloc(sizeof(int)*n);
	/* BUG: we do not free these vectors 
	 * in the case of an error */
	
	if (indxc == NULL || indxr == NULL || ipiv == NULL)
		LA_error("LA_inverse: Not enough memory");
	for (j=0; j<n; j++)
		indxc[j] = indxr[j] = ipiv[j] = 0;
	
	for (i=0; i<n; i++)
	{
		double big = 0.0, pivinv;
		for (j=0; j<n; j++)
			if (ipiv[j] != 1)
				for (k=0; k<n; k++)
				{
					if (ipiv[k] == 0)
					{
						double val = fabs(LA_VAL(A,j,k));
						if (val>=big)
						{
							big = val;
							irow = j;
							icol = k;
						}
					}
					else
						if (ipiv[k] > 1)
							return NULL;
				}
		++(ipiv[icol]);
		if (irow != icol)
			for (l=0; l<n; l++)
			{
				/* swap A[irow][l] and A[icol][l] */
				double temp = LA_VAL(A,irow,l);
				LA_VAL(A,irow,l) = LA_VAL(A,icol,l);
				LA_VAL(A,icol,l) = temp;
			}
		indxr[i] = irow;
		indxc[i] = icol;
		if (fabs(LA_VAL(A,icol,icol)) < 1e-6) /* some epsilon */
			return NULL;
		pivinv = 1.0 / LA_VAL(A,icol,icol); 
		LA_VAL(A,icol,icol) = 1.0;
		for (l=0; l<n; l++)
			LA_VAL(A,icol,l) *= pivinv;
		for (ll=0; ll<n; ll++)
			if (ll != icol)
			{
				double val = LA_VAL(A, ll, icol);
				LA_VAL(A, ll, icol) = 0.0;
				for (l=0; l<n; l++)
					LA_VAL(A,ll,l) -= val * LA_VAL(A, icol,l);
			}
	}
	for (l=n-1; l>=0; l--)
		if (indxr[l] != indxc[l])
			for (k=0; k<n; k++)
			{
				/* Swap A[k][indxr[l]] and A[k][indxc[l]] */
				double temp = LA_VAL(A, k, indxr[l]);
				LA_VAL(A, k, indxr[l]) = LA_VAL(A, k, indxc[l]);
				LA_VAL(A, k, indxc[l]) = temp;
			}
	free (indxr);
	free (indxc);
	free (ipiv);

	return A;
}

/* row/col scaling by a vector */
LA_Matrix LA_bracket(LA_Matrix V1, LA_Matrix A, LA_Matrix V2)
{
	LA_Matrix R;
	int i,j, rows = A->rows, cols = A->cols;

	if (V1 && V1->cols > 1 || V2 && V2->cols > 1)
		LA_error("LA_bracket: Not a vector");
	if (V1 && A->rows != V1->rows || V2 && A->cols != V2->rows)
		LA_error("LA_bracket: Incompatible shape");

	R = LA_create(rows, cols);

	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
		{
			LA_VAL(R,i,j) = LA_VAL(A, i, j);
			if (V1) 
				LA_VAL(R,i,j) *= LA_VAL(V1, i,0);
			if (V2)
				LA_VAL(R,i,j) *= LA_VAL(V2, j,0);
		}

	return R;

	
}

LA_Matrix LA_add(LA_Matrix A, double w, LA_Matrix B) /* A += w*B */
{
	int i,j, rows = A->rows, cols = A->cols;
	if (rows != B->rows || cols != B->cols)
		LA_error("LA_add: Incompatible shape");
	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
			LA_VAL(A,i,j) += w * LA_VAL(B,i,j);
	return A;
}

/* miscellaneous */
void LA_print(LA_Matrix A)
{
	int rows = A->rows, cols = A->cols;
	int i,j;

	printf("%d %d\n", rows, cols);

	for (i=0; i<rows; i++)
	{
		for (j=0; j<cols; j++)
			printf("%8.3lf", LA_VAL(A,i,j));
		printf("\n");
	}
}

LA_Matrix LA_read()
{
	LA_Matrix R;
	int rows, cols;
	int i,j;

	scanf("%d %d", &rows, &cols);

	R = LA_create(rows, cols);

	for (i=0; i<rows; i++)
		for (j=0; j<cols; j++)
			scanf("%lf", &LA_VAL(R,i,j));
	return R;
}

void LA_error(char *message)
{
	fprintf(stderr, "Internal error: %s, exiting...\n", message);
	exit(1);
}

/*
main()
{
	LA_Matrix A,B,C, TC, TCC, Q, Q1;

	printf("A >>> ");
	A = LA_read();
	LA_print(A);

	printf("B >>> ");
	B = LA_read();
	LA_print(B);

	C = LA_stack(A,B);
	LA_print(C);

	TC = LA_transpose(C);
	LA_print(TC);

	TCC = LA_dot(TC,C);
	LA_print(TCC);

	printf("Q >>> ");
	Q = LA_read();
	Q1 = LA_copy(Q);
	LA_print(Q);
	LA_inverse(Q);
	LA_print(Q);	
	LA_print(LA_dot(Q1, Q));
}
*/
