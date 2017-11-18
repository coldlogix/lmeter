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

#include"csspdms.h"
#include"css_par.h"
/*
 *    Function:       LLTSolve
 *      Definition:     Solve system LLtX=B by solving LY=B, LtX=Y.
 *      on enter in Sol the RHS is stored, on exit - the solution.
 */
LLTSolve (SpMatrixP L, FloatType * Sol)
{
    XEQType         i, N = L->NCols;
    SpVectorP       LCols = L->Cols;
    FloatType      *Diag = L->Diag;

    /*      Solve system LY=B.      */
    for (i = 0; i < N; i++)
    {
	FloatType       xi = (Sol[i] /= Diag[i]);
	XEQType         XNZ, NNZ = LCols[i].NNZ;
	FloatType      *C = LCols[i].Coeffs;
	XEQType        *R = LCols[i].Rows;

	for (XNZ = 0; XNZ < NNZ; XNZ++)
	    Sol[*(R++)] -= *(C++) * xi;
    }				/*      then    */
    /*      Solve system LtX=Y.     */
    for (i = N; i > 0; i--)
    {
	XEQType         ii = i - 1;	/*      Working with unsigned i. */
	FloatType       Sum = Sol[ii];
	XEQType         XNZ, NNZ = LCols[ii].NNZ;
	FloatType      *C = LCols[ii].Coeffs;
	XEQType        *R = LCols[ii].Rows;

	for (XNZ = 0; XNZ < NNZ; XNZ++)
	    Sum -= *(C++) * Sol[*(R++)];
	Sol[ii] = Sum / Diag[ii];
    }				/*      for     */
    return 0;
}				/*      LLTSolve        */
/*
 *    Function:       VPlusEqAB
 *      Definition:     V+=AB, A - symmetric. Sinse unpermuted matrix A
 *      is used, we have to permute its row numbers using InvP.
 */
VPlusEqAB (FloatType * V, SpMatrixP A, FloatType * B, XEQType * InvP)
{
    XEQType         i, N = A->NCols;
    SpVectorP       ACols = A->Cols;
    FloatType      *ADiag = A->Diag;

    for (i = 0; i < N; i++)
    {
	FloatType       Sum = 0.0;
	XNZType         XNZ, NNZ = ACols[i].NNZ;
	FloatType      *C = ACols[i].Coeffs;
	XEQType        *R = ACols[i].Rows;

	for (XNZ = 0; XNZ < NNZ; XNZ++)
	    Sum += *(C++) * B[InvP[*(R++)]];
	Sum += ADiag[i] * B[InvP[i]];
	V[InvP[i]] += Sum;
    }				/*      for     */
    return 0;
}				/*      VPlusEqAB       */
/*
 *    Function:       VEqApB
 *      Definition:     V=AB, Colomns of A assumed to be
 *      permuted before. More effective in iterative solver.
 */
VEqApB (FloatType * V, SpMatrixP A, FloatType * B, XEQType * InvP)
{
    XEQType         i, N = A->NCols;
    SpVectorP       ACols = A->Cols;
    FloatType      *ADiag = A->Diag;

    for (i = 0; i < N; i++)
    {
	FloatType       Sum = 0.0;
	XNZType         XNZ, NNZ = ACols[i].NNZ;
	FloatType      *C = ACols[i].Coeffs;
	XEQType        *R = ACols[i].Rows;

	for (XNZ = 0; XNZ < NNZ; XNZ++)
	    Sum += *(C++) * B[*(R++)];
	Sum += ADiag[i] * B[InvP[i]];
	V[InvP[i]] = Sum;
    }				/*      for     */
    return 0;
}				/*      VEqApB  */
/*
 *    Function:       AToAp
 *      Definition:     Permute row numbers of A for VEqApB.
 */
AToAp (SpMatrixP A, XEQType * InvP)
{
    XEQType         i, N = A->NCols;
    SpVectorP       ACols = A->Cols;

    for (i = 0; i < N; i++)
    {
	XNZType         XNZ, NNZ = ACols[i].NNZ;
	XEQType        *R = ACols[i].Rows;

	for (XNZ = 0; XNZ < NNZ; XNZ++)
	{
	    *R = InvP[*R];
	    R++;
	}			/*      for     */
    }				/*      for     */
    return 0;
}				/*      AToAp   */
/*
 *    Function:       ApToA
 *      Definition:     Permute row numbers of A back after VEqApB.
 */
ApToA (SpMatrixP A, XEQType * Perm)
{
    XEQType         i, N = A->NCols;
    SpVectorP       ACols = A->Cols;

    for (i = 0; i < N; i++)
    {
	XNZType         XNZ, NNZ = ACols[i].NNZ;
	XEQType        *R = ACols[i].Rows;

	for (XNZ = 0; XNZ < NNZ; XNZ++)
	{
	    *R = Perm[*R];
	    R++;
	}			/*      for     */
    }				/*      for     */
    return 0;
}				/*      ApToA   */
/*
 *    Function:       ScalarProduct
 *      Definition:     Return A.B.
 */
FloatType 
ScalarProduct (XEQType N, FloatType * A, FloatType * B)
{
    XEQType         i;
    FloatType       Sum = 0;

    for (i = 0; i < N; i++)
	Sum += A[i] * B[i];
    return Sum;
}				/*      ScalarProduct   */
/*
 *    Function:       Norm
 *      Definition:     Return  R.R
 */
FloatType 
Norm (XEQType N, FloatType * R)
{
    XEQType         i;
    FloatType       Sum = 0;

    for (i = 0; i < N; i++)
	Sum += R[i] * R[i];
    return Sum;
}				/*      Norm    */
