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
 *    Function:       NumericalFactorize
 *      Definition:     Do numerical factorization of A giving the LOWER
 *      triangular matrix L. A is assumed to be symbolically factorized.
 *      L is stored by COLOMNS and returned on the place of A.
 */
NumericalFactorize (SpMatrix * A)
{
    XEQType         N = A->NCols, i, j;
    FloatType      *Diag = A->Diag;
    SpVector       *Cols = A->Cols;
    XNZType        *First;	/*      Current XNZ for active colomns. */
    XEQType        *Link;	/*      List of active colomns.         */

    /*      Link[j],Link[Link[j]] ... are colomns with nonzero      */
    /*      value in j'th row. The last active colomn is with       */
    /*      Link[i]==EOList;                                        */
#define	EOList	(XEQType)-1
    FloatType      *Temp;	/*      Temporary vector to store Lij.  */
    XEQType        *R;
    FloatType      *C;
    FloatType       Ljj, Lij;
    XNZType         Fi;
    XNZType         NNZ, XNZ;

#ifdef	STAT_COUNT
    L_NACTCOL = 0;
#endif

    allocateN (Link, N + 1);
    allocateN (First, N);
    allocateN (Temp, N);
    for (j = 0; j < N; j++)
	Link[j] = EOList;

    for (j = 0; j < N; j++)
    {				/*      Compute j'th colomn.            */
	Ljj = Diag[j];
	/*      Init Temp.                      */
	R = Cols[j].Rows;
	C = Cols[j].Coeffs;
	NNZ = Cols[j].NNZ;
	for (XNZ = 0; XNZ < NNZ; XNZ++)
	    Temp[*(R++)] = *(C++);
	/*      For all active colomns i.       */
	i = Link[j];
	while (i != EOList)
	{			/*      Modify j'th colomn by i'th.     */
#ifdef	STAT_COUNT
	    L_NACTCOL++;
#endif
	    Fi = First[i];
	    R = Cols[i].Rows + Fi;
	    assert (*R == j);
	    C = Cols[i].Coeffs + Fi;
	    Lij = *C;
	    Ljj -= Lij * Lij;
	    NNZ = Cols[i].NNZ;
	    if (++Fi < NNZ)
	    {
		XEQType         iNext = Link[i];

		++R;
		++C;
		/*      Insert i'th colomn to list of   */
		/*      colomns with nonzero value in *R */
		Link[i] = Link[*R];
		Link[*R] = i;
		/*      Modify First.   */
		First[i] = Fi;
		/*      And use coeffs. */
		for (XNZ = Fi; XNZ < NNZ; XNZ++)
		    Temp[*(R++)] -= *(C++) * Lij;
		i = iNext;
	    }			/*      then    */
	    else
		/*        Colomn is no longer active.     */
	    {
		Cols[i].Flags |= SWAPPABLE;
		i = Link[i];
	    }			/*      then    */
	}			/*      while   */
	if (Ljj < EPS)
	    error ("Matrix is not positive-definite.");
	Ljj = sqrt (Ljj);
	Diag[j] = Ljj;
	NNZ = Cols[j].NNZ;
	if (NNZ)
	{
	    R = Cols[j].Rows;
	    C = Cols[j].Coeffs;
	    /*      Insert this colomn to the list. */
	    Link[j] = Link[*R];
	    Link[*R] = j;
	    /*      Save the coeffs.                */
	    for (XNZ = 0; XNZ < NNZ; XNZ++)
		*(C++) = Temp[*(R++)] / Ljj;
	}			/*      then    */
    }				/*      for     */
    free (Temp);
    free (First);
    free (Link);
#ifdef	STAT_COUNT
    /*      It's an average number, divide it by N.         */
    L_NACTCOL /= N;
#endif
    return 0;
}				/*      NumericalFactorize      */
