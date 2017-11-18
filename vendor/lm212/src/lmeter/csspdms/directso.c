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
 *    Function:       DirectSolve
 *      Definition:     Solve system LLtX=B by solving LY=B, LtX=Y.
 *      Vectors Y[i] and X[i] are stored on the place of vector B[i].
 *      After this the solution is permuted using Perm.
 */
DirectSolve (SpMatrixP L, SpMatrixP B, XEQType * Perm, XEQType * InvP)
{
    XEQType         i, NCols = L->NCols, r, NRHS = B->NCols;
    FloatType      *Sol, *PSol;

    for (r = 0; r < NRHS; r++)
    {				/*      Solve system LY[r]=B[r].        */
	/*      Permute B[r] giving dense Sol.  */
	{
	    XNZType         XNZ, NNZ = B->Cols[r].NNZ;
	    FloatType      *Coeffs = B->Cols[r].Coeffs;
	    XEQType        *Rows = B->Cols[r].Rows;

	    allocateN (Sol, NCols);
	    for (XNZ = 0; XNZ < NNZ; XNZ++)
		Sol[InvP[Rows[XNZ]]] = Coeffs[XNZ];
	    free (Coeffs);
	    free (Rows);
	    B->Cols[r].Rows = NULL;
	    B->Cols[r].Flags |= DENSE;
	    B->Cols[r].NNZ = NCols;
	    /* B->Cols[r].Coeffs will be assigned later.    */
	}
	LLTSolve (L, Sol);
	/*      Permute the solution.   */
	allocateN (PSol, NCols);
	for (i = 0; i < NCols; i++)
	    PSol[Perm[i]] = Sol[i];
	free (Sol);
	B->Cols[r].Coeffs = PSol;
    }				/*      for     */
    return 0;
}				/*      DirectSolve     */
