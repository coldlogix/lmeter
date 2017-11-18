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
 *    Function:       SymbolicFactorize
 *      Definition:     Factorize matrix A symbolically, i.e. create in A
 *      place for the elements of lower triangular matrix L. Coeffs for new
 *      elements are initialized by 0.0.
 *      Effective way to do it is to add to each j'th colomn of A elements
 *      from k'th colomn, but only if number m=(min. row number in colomn k)
 *      is equal to j.
 *      During this process elements are permuted as defined by Perm,InvP
 *      arrays. Perm[NewNumber]==OldNumber, InvP[OldNumber]==NewNumber.
 *      The initial sequence of variables is restored after the solution.
 */
SymbolicFactorize (SpMatrix * A, XEQType * Perm, XEQType * InvP)
{
    XEQType         N = A->NCols;
    SpVector       *ACols = A->Cols, *LCols;

    /*      Colomns of the resulting matrix L are returned on the   */
    /*      place of A colomns.                                     */
    FloatType      *ADiag = A->Diag, *LDiag;

    /*      Original and permuted diagonals.                        */
    XEQType        *MergeLink;

    /*      Vector MergeLink of length N is used for representing   */
    /*      lists of colomns with equal number m.                   */
    /*      Values MergeLink[j],MergeLink[MergeLink[j]] ... contain */
    /*      the (incremented by 1) colomn numbers of colomns with   */
    /*      m==j.                                                   */
    XEQType         XMerge;	/*      IndeX in this list.     */
    XEQType        *ReachLink;

    /*      Vector ReachLink of length N is used for representing   */
    /*      lists of rows.                                          */
    /*      Values ReachLink[j],ReachLink[ReachLink[j]] ... contain */
    /*      at step j the row numbers of nonzero rows in colomn j.  */
    /*      Last element of this list contains 0.                   */
    XNZType         ReachLen;	/*      Length of this list.    */
    XEQType         XReach;	/*      IndeX in this list.     */
    XEQType         j, Permj, i, m;	/*      Colomn numbers.         */
    XEQType        *Rows;	/*      Rows of colomn to work. */
    XNZType         XNZ, NNZ;

#ifdef	STAT_COUNT
    L_NNZ = 0;
#endif

    allocateN (LCols, N);
    allocateN (MergeLink, N);
    allocateN (ReachLink, N);

    for (j = 0; j < N; j++)
    {				/*      Initialize ReachLink by rows of A[j].           */
	Permj = Perm[j];
	NNZ = ACols[Permj].NNZ;
	Rows = ACols[Permj].Rows;
	ReachLink[j] = 0;
	ReachLen = 0;
	for (XNZ = 0; XNZ < NNZ; XNZ++)
	{
	    XEQType         Row = InvP[Rows[XNZ]];

	    if (Row > j)	/*      LOWER   */
	    {			/*      Insert Row to ReachLink.        */
		XReach = j;
		while (ReachLink[XReach] && ReachLink[XReach] < Row)
		    XReach = ReachLink[XReach];
		assert (ReachLink[XReach] != Row);
		ReachLink[Row] = ReachLink[XReach];
		ReachLink[XReach] = Row;
		ReachLen++;
	    }			/*      then    */
	}			/*      for     */
	/*      For all colomns in MergeList.                   */
	XMerge = j;
	while (i = MergeLink[XMerge])
	{			/*      To obtain colomn number, subtract one.  */
	    i--;
	    /*      Add rows of i'th colomn to ReachLink.   */
	    NNZ = LCols[i].NNZ;
	    Rows = LCols[i].Rows;
	    /*      Insert the remaining Rows.              */
	    XReach = j;
	    for (XNZ = 1; XNZ < NNZ; XNZ++)
	    {
		XEQType         Row = Rows[XNZ];

		/*      Insert Row to ReachLink.        */
		while (ReachLink[XReach] && ReachLink[XReach] < Row)
		    XReach = ReachLink[XReach];
		if (ReachLink[XReach] != Row)
		{
		    ReachLink[Row] = ReachLink[XReach];
		    ReachLink[XReach] = Row;
		    ReachLen++;
		}		/*      then    */
	    }			/*      for     */
	    XMerge = i;
	}			/*      while   */
	if (ReachLen)
	{			/*      Save j'th colomn.               */
	    XEQType        *NewRows;
	    FloatType      *NewCoeffs;
	    XNZType         NewXNZ;

	    allocateN (NewRows, ReachLen);
	    allocateN (NewCoeffs, ReachLen);
	    /*      Fill NewRows from ReachLink.    */
	    NewXNZ = 0;
	    XReach = j;
	    while (XReach = ReachLink[XReach])
		NewRows[NewXNZ++] = XReach;
	    assert (NewXNZ == ReachLen);
	    if (NNZ = ACols[Permj].NNZ)
	    {			/*      Fill NewCoeffs from Coeffs.     */
		FloatType      *Coeffs = ACols[Permj].Coeffs;

		Rows = ACols[Permj].Rows;
		for (XNZ = 0; XNZ < NNZ; XNZ++)
		{
		    XEQType         Row = InvP[Rows[XNZ]];

		    if (Row > j)	/*      LOWER   */
		    {
			NewXNZ = 0;
			while (Row > NewRows[NewXNZ])
			    NewXNZ++;
			assert (Row == NewRows[NewXNZ]);
			assert (NewXNZ < ReachLen);
			NewCoeffs[NewXNZ] = Coeffs[XNZ];
		    }		/*      then    */
		}		/*      for     */
		/*      Free A colomn.  */
		free (Rows);
		free (Coeffs);
	    }			/*      then    */
	    LCols[j].Rows = NewRows;
	    LCols[j].Coeffs = NewCoeffs;
	    LCols[j].NNZ = ReachLen;
	    /*      Add j'th colomn to MergeLink.   */
	    m = ReachLink[j];
	    MergeLink[j] = MergeLink[m];
	    MergeLink[m] = j + 1;
#ifdef	STAT_COUNT
	    L_NNZ += ReachLen;
#endif
	}			/*      then    */
    }				/*      for     */
    free (MergeLink);
    free (ReachLink);
    allocateN (LDiag, N);
    for (j = 0; j < N; j++)
	LDiag[j] = ADiag[Perm[j]];
    free (ADiag);
    free (ACols);
    A->Cols = LCols;
    A->Diag = LDiag;
    return 0;
}				/*      SymbolicFactorize       */
