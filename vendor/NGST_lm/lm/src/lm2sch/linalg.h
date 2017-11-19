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
 * File:         linalg.h
 * Description:  Interface to tiny linear algebra/matrix 
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

/* Matrix type, vectors are also matrices but have just one column. 
 */
typedef struct LA__Matrix
{
	int rows, cols;
	double *vals;
} *LA_Matrix;

/* Access macro, can be used as LVal as well.
 * row,col start from 0.
 */
#define LA_VAL(A, row, col) ((A)->vals[(row)*((A)->cols)+(col)])

/* Constructor/Destructor pair. 
 * Constructed matrices have all 0.0 elements
 * LA_unit returns a new square unit matrix of a given size.
 */
LA_Matrix LA_create(int rows, int cols); 
void LA_delete(LA_Matrix A); 
LA_Matrix LA_unit(int size); 

/* "Special" constructors: build matrices out of matrices 
 */
LA_Matrix LA_copy(LA_Matrix A);
LA_Matrix LA_stack(LA_Matrix A1, LA_Matrix A2); /* A1 on top of A2 */

/* Matrix operations, first argument is where to put the result, if it
 * is NULL, a new matrix is created (which should be deleted later by
 * the user), if not NULL, it should have the appropriate shape.
 */
LA_Matrix LA_transpose(LA_Matrix R, LA_Matrix A);           /* A^T   */
LA_Matrix LA_dot(LA_Matrix R, LA_Matrix A1, LA_Matrix A2);  /* A1.A2 */

/* In-place matrix operations: inversion,
 * accumulating matrices. They return A itself.
 */
LA_Matrix LA_inverse(LA_Matrix A);                          /* A=A^-1  */
LA_Matrix LA_add(LA_Matrix A, double w, LA_Matrix B);       /* A += w*B */

/* A wierd one: take A, scale rows by V1 components, 
 * scale columns by V2 components, return a new matrix.
 * It behaves as if we create diagonal matrices with V1 and V2 
 * along the diagonal and compute A1.A.A2
 * If a vector is NULL it is not used.
 */
LA_Matrix LA_bracket(LA_Matrix V1, LA_Matrix A, LA_Matrix V2);

/* miscellaneous */
void LA_print(LA_Matrix A); 
void LA_error(char *message);

/* -------- This is all I need, feel free to add more ----------- */



