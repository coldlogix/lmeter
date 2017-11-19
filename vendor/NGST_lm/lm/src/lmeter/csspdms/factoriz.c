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

FloatType       Threshold = THRESHOLD_DEFAULT;
FloatType       InitThreshold = INITTHRESHOLD_DEFAULT;
long unsigned   L_NNZ_Avail;
long unsigned   L_NNZ;

/*
 *    Function:       Factorize
 *      Definition:     Do incomplete numerical factorization of A giving
 *      the LOWER triangular matrix L.
 *      Values Lij: fabs(Lij)<Threshold*Ljj are dropped.
 *      Diagonal values are modified to ensure positive-definiteness.
 */
SpMatrixP 
Factorize (SpMatrixP A, XEQType * Perm, XEQType * InvP)
{
    XEQType         N = A->NCols, i, j;
    SpMatrixP       L;
    FloatType      *ADiag = A->Diag, *LDiag;
    SpVectorP       ACols = A->Cols, LCols;
    XNZType        *First;	/*      Current XNZ for active colomns. */
    XEQType        *Link;	/*      List of active colomns.         */

    /*  Link[j],Link[Link[j]] ... are colomns with nonzero      */
    /*  value in j'th row. The last active colomn is with       */
    /*  Link[i]==EOList;                                        */
#define	EOList	(XEQType)-1
    XNZType        *RowToXNZ;	/*      Table to encode Row No. */

    /*  to the XNZ index in jRows, jCoeffs (or EOList).         */
    XEQType        *R, *jRows;
    FloatType      *C, *jCoeffs;
    XNZType         XNZ, NNZ, jNNZ;
    FloatType       Ljj, Lij;
    XNZType         Fi;
    long unsigned   L_DROPPED = 0, L_INSERTED = 0;
    unsigned        CheckPoint = CHECK_TIME;

    /* 
     * Calculate how many NZ elements can we afford. Start from LNNZMEM,
     * then subtract A matrix and L matrix overhead, tmp. storage
     * and divide by one NZ element size. Then subtract the number of 
     * non-zeros stored in A.
     */
    L_NNZ_Avail = LNNZMEM * 1024 * 1024;
    L_NNZ_Avail -= 2 * N * (sizeof (FloatType) +
			    sizeof (XEQType) +
			    sizeof (SpVector) + 16);
    L_NNZ_Avail -= N * (sizeof (*Link) +
			sizeof (*First) +
			sizeof (*RowToXNZ) +
			sizeof (*jRows) +
			sizeof (*jCoeffs));
    L_NNZ_Avail /= (sizeof (FloatType) + sizeof (XEQType));

    for (j = 0; j < N; j++)
	L_NNZ_Avail -= ACols[j].NNZ;

    L_NNZ = 0;
#ifdef	STAT_COUNT
    L_NACTCOL = 0;
#endif

    L = SpMatrixCreate (N);
    LDiag = L->Diag;
    LCols = L->Cols;

    allocateN (Link, N);
    allocateN (First, N);
    allocateN (RowToXNZ, N);
    allocateN (jRows, N);
    allocateN (jCoeffs, N);

    for (j = 0; j < N; j++)
	RowToXNZ[j] = Link[j] = EOList;

    for (j = 0; j < N; j++)
    {
	/*      Compute j'th colomn.            */
	{
	    XEQType         Permj = Perm[j];

	    Ljj = ADiag[Permj];
	    R = ACols[Permj].Rows;
	    C = ACols[Permj].Coeffs;
	    NNZ = ACols[Permj].NNZ;
	}
	jNNZ = 0;
	for (XNZ = 0; XNZ < NNZ; XNZ++)
	{
	    XEQType         Node = InvP[*R];

	    if (Node > j)
	    {
		jRows[jNNZ] = Node;
		jCoeffs[jNNZ] = *C;
		assert (RowToXNZ[Node] == EOList);
		RowToXNZ[Node] = jNNZ++;
	    }			/*      then    */
	    R++;
	    C++;
	}
	/*      For all active colomns i.       */
	i = Link[j];
	while (i != EOList)
	{
	    /*  Modify j'th colomn by i'th.     */
#ifdef	STAT_COUNT
	    L_NACTCOL++;
#endif
	    assert (i < j);
	    Fi = First[i];
	    R = LCols[i].Rows + Fi;
	    assert (*R == j);
	    C = LCols[i].Coeffs + Fi;
	    Lij = *C;
	    Ljj -= Lij * Lij;
	    NNZ = LCols[i].NNZ;
	    if (++Fi < NNZ)
	    {
		XEQType         iNext = Link[i];

		++R;
		++C;
		/*      Insert i'th colomn to list of   */
		/*      colomns with nonzero row *R.    */
		Link[i] = Link[*R];
		Link[*R] = i;
		/*      Modify First.   */
		First[i] = Fi;
		/*      And use coeffs. */
		for (XNZ = Fi; XNZ < NNZ; XNZ++)
		{
		    XNZType         jXNZ = RowToXNZ[*R];

		    if (jXNZ == EOList)
		    {
			/*      Insert new row. */
			jRows[jNNZ] = *R;
			jCoeffs[jNNZ] = -(*C) * Lij;
			RowToXNZ[*R] = jNNZ++;
		    }		/*      then    */
		    else
		    {
			jCoeffs[jXNZ] -= (*C) * Lij;
			assert (jRows[jXNZ] == *R);
		    }		/*      else    */
		    R++;
		    C++;
		}		/*      for     */
		i = iNext;
	    }			/*      then    */
	    else
		/*        Colomn is no longer active.     */
	    {
		LCols[i].Flags |= SWAPPABLE;
		i = Link[i];
	    }			/*      then    */
	}			/*      while   */
	if (Ljj < EPS)
	{

	    /* message("Matrix is not positive-definite."); */
	    /*  Modify ADiag to make Ljj==1.    */
	    Ljj -= ADiag[Perm[j]];
	    ADiag[Perm[j]] += 1.0 - Ljj;
	    Ljj = 1.0;
	}			/*      then    */
	Ljj = sqrt (Ljj);
	LDiag[j] = Ljj;
	if (jNNZ)
	{
	    XEQType         Shrink = 0;
	    FloatType       ThrLjj = Threshold * Ljj;

	    for (XNZ = 0; XNZ < jNNZ; XNZ++)
	    {
		FloatType       Lij = jCoeffs[XNZ] / Ljj;

		assert (RowToXNZ[jRows[XNZ]] == XNZ);
		RowToXNZ[jRows[XNZ]] = EOList;
		if (fabs (Lij) < ThrLjj)
		    Shrink++;
		else
		    /*        if ( Shrink )   */
		{
		    jRows[XNZ - Shrink] = jRows[XNZ];
		    jCoeffs[XNZ - Shrink] = Lij;
		}		/*      else    */
	    }			/*      for     */
	    jNNZ -= Shrink;
	    L_DROPPED += Shrink;
	    if (jNNZ)
	    {
		SpVectorBuild (LCols + j, jRows, jCoeffs, jNNZ);
		/*      Insert this colomn to the list. */
		Link[j] = Link[(LCols + j)->Rows[0]];
		Link[(LCols + j)->Rows[0]] = j;
		L_INSERTED += jNNZ;
	    }			/*      then    */
	    /*  Compare L_INSERTED/L_NNZ_Avail and j/N  */
	    if (--CheckPoint == 0)
	    {
		double          factpercent = 100 * ((double) j) / N;
		double          mempercent = 100 * ((double) L_INSERTED) / L_NNZ_Avail;

		sprintf (strbuff,
			 "%3.0lf%% factorized, %3.0lf%% L memory used",
			 factpercent, mempercent);
		message (strbuff);
		if (mempercent > 1.1 * factpercent)
		{
		    static          firstinc = 1;

		    if (firstinc)
		    {
			Threshold = InitThreshold;
			firstinc = 0;
			sprintf (strbuff,
				 "At colomn %lu Threshold increased",
				 (long unsigned) j);
			message (strbuff);
		    }		/*      then    */
		    else
		    {
			Threshold *= DELTA_THR;
			sprintf (strbuff,
				 "At colomn %lu Threshold increased again",
				 (long unsigned) j);
			message (strbuff);
		    }		/*      else    */
		    if (Threshold >= 1.0)
			warning ("Threshold was increased to 1.0");
		}		/*      then    */
		CheckPoint = CHECK_TIME;
	    }
	}			/*      then    */
    }				/*      for     */
    free (First);
    free (Link);
    free (RowToXNZ);
    free (jRows);
    free (jCoeffs);
    sprintf (strbuff, "Final Threshold is %18.16lf",
	     (double) Threshold);
    message (strbuff);
    sprintf (strbuff, "%lu NZs were dropped, %lu inserted",
	     L_DROPPED, L_INSERTED);
    message (strbuff);
#ifdef	STAT_COUNT
    /*  It's an average number, divide it by N.         */
    L_NACTCOL /= N;
    L_NNZ = L_INSERTED;
    sprintf (strbuff, "%lu columns active on average",
	     L_NACTCOL);
    message (strbuff);
#endif
    return L;
}				/*      Factorize       */
