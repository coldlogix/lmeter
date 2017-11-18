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

#include"lmeter.h"
#include"head_lm.h"

/*
 *    Function:       out_trmcur
 *      Definition:     Write out terminal current in the suitable format.
 */
static FILE    *curstxt = NULL;
out_trmcur (int iout, int itea, double trmcur)
{
/*      if      (curstxt==NULL) fileopen(curstxt,"curs.txt","w");
 *    fprintf (curstxt,"%4d\t%4d\t%16.8lf\n",iteaitrm[iout+1],iteaitrm[itea+1],trmcur);
 *      fflush  (curstxt);
 */ fprintf (stdout, "%4d\t%4d\t%16.8lf\n", iteaitrm[iout + 1], iteaitrm[itea + 1], trmcur);
    return 0;
}				/*      out_trmcur      */
/*
 *    Table to encode N to initial Threshold.
 */
struct
{
    XEQType         N;
    FloatType       Threshold;
}
Thrs[] =
{
    {
	25000, 0.0000000
    }
    ,
    {
	50000, 0.0000001
    }
    ,
    {
	100000, 0.0000005
    }
    ,
    {
	200000, 0.000001
    }
    ,
    {
	400000, 0.00001
    }
};
/*
 *    Function:       solve
 *      Definition:     Solve AX=B, compute CX-D, print it out.
 */
solve (void)
{
    XEQType         i, N = A->NCols;
    int             iout, itea;
    XEQType        *Perm, *InvP;
    SpMatrixP       L, currents;
    XEQType         TermNNZ = 0;

    allocateN (Perm, N);
    allocateN (InvP, N);
    NestedDissections (A, Perm, InvP);
    message ("LM: Variables were permuted");
    for (i = 0; i < sizeof (Thrs) / sizeof (Thrs[0]); i++)
    {
	Threshold = Thrs[i].Threshold;
	if (N <= Thrs[i].N)
	    break;
    }				/*      for     */
    InitThreshold = 2 * Threshold;
    sprintf (strbuff, "N=%lu, current Threshold is %lf", (long unsigned) N, (double) Threshold);
    message (strbuff);
    L = Factorize (A, Perm, InvP);
    message ("LM: Matrix was numerically factorized");
#ifdef	STAT_COUNT
    fprintf (stderr, "L_NNZ=%lu\tL_NACTCOL=%lu\n", L_NNZ, L_NACTCOL);
#endif
    Epsilon = 1e-9;
    for (itea = 0; itea < ntea; itea++)
	if (C->Cols[itea].NNZ > TermNNZ)
	    TermNNZ = C->Cols[itea].NNZ;
    Epsilon /= (double) TermNNZ;
    IterativeSolve (A, L, B, Perm, InvP);
    SpMatrixFree (A);
    SpMatrixFree (L);
    free (Perm);
    free (InvP);
    message ("LM: Systems were solved");
    currents = SpMatrixDecrement (SpMatrixMultiply (C, B), D);
    for (iout = 0; iout < nout; iout++)
	for (itea = 0; itea < ntea; itea++)
	    out_trmcur (iout, itea, currents->Cols[iout].Coeffs[itea]);
    message ("LM: Solution complete");
    return 0;
}				/*      solve   */
