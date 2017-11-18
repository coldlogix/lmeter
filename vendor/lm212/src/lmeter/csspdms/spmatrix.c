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
 *    Function:       SpMatrixCreate
 *      Definition:     Create a new sparse matrix with width NCols.
 */
SpMatrixP 
SpMatrixCreate (XEQType NCols)
{
    SpMatrixP       New;

    allocateN (New, 1);
    New->NCols = NCols;
    New->Cols = SpNVectorsCreate (NCols);
    allocateN (New->Diag, NCols);
    return New;
}				/*      SpMatrixCreate  */
/*
 *    Function:       SpMatrixFree
 *      Definition:     Free sparse matrix.
 */
int 
SpMatrixFree (SpMatrixP SpM)
{
    free (SpM->Diag);
    SpNVectorsFree (SpM->Cols, SpM->NCols);
    free (SpM);
    return 0;
}				/*      SpMatrixFree    */
/*
 *    Function:       SpMatrixResize
 *      Definition:     Resize SpM to become NCols in width.
 */
SpMatrixResize (SpMatrixP SpM, XEQType NCols)
{
    XEQType         C;

    SpM->Cols = SpNVectorsResize (SpM->Cols, SpM->NCols, NCols);
    reallocateN (SpM->Diag, NCols);
    /*      realloc don't clear memory, do it manually.     */
    for (C = SpM->NCols; C < NCols; C++)
	SpM->Diag[C] = 0.0;
    SpM->NCols = NCols;
    return 0;
}				/*      SpMatrixResize  */
/*
 *    Function:       SpMatrixMultiply
 *      Definition:     Return the new DENSE matrix Transpose[A].B .
 *      Restriction:    Now only for non-DENSE A and DENSE B.
 */
SpMatrixP 
SpMatrixMultiply (SpMatrixP A, SpMatrixP B)
{
    XEQType         i, j, Bw = B->NCols, Aw = A->NCols;
    SpMatrixP       C;

    C = SpMatrixCreate (Bw);
    /*      Make C DENSE    */
    free (C->Diag);
    C->Diag = NULL;
    for (j = 0; j < Bw; j++)
    {
	allocateN (C->Cols[j].Coeffs, Aw);
	C->Cols[j].Flags |= DENSE;
    }				/*      for     */
    for (i = 0; i < Aw; i++)
    {
	XNZType         AXNZ, ANNZ = A->Cols[i].NNZ;
	XEQType        *ARows = A->Cols[i].Rows;
	FloatType      *ACoeffs = A->Cols[i].Coeffs;

	if (A->Cols[i].Flags & DENSE)
	    error ("Unsupported matrix types for multiplication");
	for (j = 0; j < Bw; j++)
	{			/*      XNZType         BXNZ,BNNZ=B->Cols[j].NNZ;
				   XEQType              *BRows=B->Cols[j].Rows; */
	    FloatType      *BCoeffs = B->Cols[j].Coeffs;
	    FloatType       Sum = 0.0;

	    if (!(B->Cols[j].Flags & DENSE))
		error ("Unsupported matrix types for multiplication");
	    for (AXNZ = 0; AXNZ < ANNZ; AXNZ++)
		Sum += ACoeffs[AXNZ] * BCoeffs[ARows[AXNZ]];
	    C->Cols[j].Coeffs[i] = Sum;
	}			/*      for     */
    }				/*      for     */
    return C;
}				/*      SpMatrixMultiply        */
/*
 *    Function:       SpMatrixDecrement
 *      Definition:     A-=B.
 *      Restriction:    Now only for DENSE A and non-DENSE B.
 */
SpMatrixP 
SpMatrixDecrement (SpMatrixP A, SpMatrixP B)
{
    XEQType         i, j, Aw = A->NCols;

    if (Aw != B->NCols)
	error ("Bad matrix sizes");
    for (i = 0; i < Aw; i++)
    {
	XNZType         BXNZ, BNNZ = B->Cols[i].NNZ;
	XEQType        *BRows = B->Cols[i].Rows;
	FloatType      *BCoeffs = B->Cols[i].Coeffs;
	FloatType      *ACoeffs = A->Cols[i].Coeffs;

	if (B->Cols[i].Flags & DENSE || !(A->Cols[i].Flags & DENSE))
	    error ("Unsupported matrix types for subtraction");
	for (BXNZ = 0; BXNZ < BNNZ; BXNZ++)
	    ACoeffs[BRows[BXNZ]] -= BCoeffs[BXNZ];
    }				/*      for     */
    return A;
}				/*      SpMatrixDecrement       */

/*
 *    Function:       SpMatrixCheckSymm
 *      Definition:     Very rude (only for debugging) procedure to check
 *      whether matrix is symmetric.
 */
#define	SYMMTOL	1e-6
SpMatrixCheckSymm (SpMatrixP A)
{
    XEQType         i, N = A->NCols;
    SpVectorP       Cols = A->Cols;
    FloatType      *Diag = A->Diag;

    for (i = 0; i < N; i++)
    {
	XNZType         XNZ = 0, NNZ = Cols[i].NNZ;
	XEQType        *Rows = Cols[i].Rows;
	FloatType      *Coeffs = Cols[i].Coeffs;

	while (XNZ < NNZ && Rows[XNZ] <= i)
	    XNZ++;
	while (XNZ < NNZ)
	{
	    XEQType         Row = Rows[XNZ];
	    XNZType         SXNZ, SNNZ = Cols[Row].NNZ;
	    XEQType        *SRows = Cols[Row].Rows;
	    FloatType      *SCoeffs = Cols[Row].Coeffs;

	    for (SXNZ = 0; SXNZ < SNNZ; SXNZ++)
		if (SRows[SXNZ] == i)
		    break;
	    if (SXNZ == SNNZ && fabs (Coeffs[XNZ]) / Diag[i] > SYMMTOL)
	    {
		sprintf (strbuff, "Matrix is structurally non-symmetric: %f - 0.0",
			 Coeffs[XNZ]);
		error (strbuff);
	    }			/*      then    */
	    if (fabs (SCoeffs[SXNZ] - Coeffs[XNZ]) / Diag[i] > SYMMTOL)
	    {
		sprintf (strbuff, "Matrix is numerically non-symmetric: %f - %f",
			 SCoeffs[SXNZ], Coeffs[XNZ]);
		error (strbuff);
	    }			/*      then    */
	    XNZ++;
	}			/*      while   */
    }				/*      for     */
    message ("Matrix is symmetric");
    return 0;
}				/*      SpMatrixCheckSymm       */
/*
 *    Function:       SpMatrixPrint
 *      Definition:     Prints the SMALL SpMatrix to the file named.
 */
SpMatrixPrint (char *fname, SpMatrixP A)
{
    FILE           *f;
    XEQType         i, j, N = A->NCols;
    SpVectorP       Cols = A->Cols;
    FloatType      *Diag = A->Diag;

    fileopen (f, fname, "w");
    for (i = 0; i < N; i++)
    {
	XNZType         XNZ = 0, NNZ = Cols[i].NNZ;
	XEQType        *Rows = Cols[i].Rows;
	FloatType      *Coeffs = Cols[i].Coeffs;

	for (j = 0; j < N; j++)
	{
	    FloatType       Val;

	    if (XNZ < NNZ && j == Rows[XNZ])
		Val = Coeffs[XNZ++];
	    else if (XNZ < NNZ && j == i)
		Val = Diag[i];
	    else
		Val = 0.0;
	    fprintf (f, "%+6.2f", (double) Val);
	}			/*      for     */
	fputc ('\n', f);
    }				/*      for     */
    fclose (f);
    return 0;
}				/*      SpMatrixPrint   */
