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
#include<string.h>

#ifdef PTHREADS
#include<pthread.h>
#endif

FloatType       Epsilon = EPS;	/*      The desired accuracy.   */
unsigned        MaxIterNo = MAXITER_DEFAULT;

#undef	WRITE_RESIDUAL

struct task
{
    SpMatrixP       A;
    SpMatrixP       L;
    SpMatrixP       B;
    XEQType        *Perm;
    XEQType        *InvP;
    XEQType         irhs0;
    XEQType         irhs1;
    FloatType       Eps;
#ifdef PTHREADS
    pthread_t       tid;
#endif	/* ; */
};

/*
 * A single solver thread: Solve for one given RHS. 
 * Used in IterativeSolve below.
 */
void           *
IterativeSolveRHS (void *mytask)
{
    struct task    *task = (struct task *) mytask;
    SpMatrixP       A = task->A;
    SpMatrixP       L = task->L;
    SpMatrixP       B = task->B;
    XEQType        *Perm = task->Perm;
    XEQType        *InvP = task->InvP;
    XEQType         irhs0 = task->irhs0;
    XEQType         irhs1 = task->irhs1;
    XEQType         irhs;
    FloatType       Eps = task->Eps;
    FloatType      *b, *x, *r, *v, *d, *z;	/*       Work vectors.   */
    XEQType         N = A->NCols, i;
    FloatType       rzNext;

    for (irhs = irhs0; irhs < irhs1; irhs++)
    {
	int             iter = 0;

	allocateN (v, N);
	allocateN (d, N);
	allocateN (z, N);
	allocateN (r, N);
	allocateN (x, N);
	allocateN (b, N);

	/*      Clear x & b.    */
	for (i = 0; i < N; i++)
	    x[i] = b[i] = 0.0;
	/*      Permute B[r] giving dense b, x=b        */
	{
	    XNZType         XNZ, NNZ = B->Cols[irhs].NNZ;
	    FloatType      *Coeffs = B->Cols[irhs].Coeffs;
	    XEQType        *Rows = B->Cols[irhs].Rows;

	    for (XNZ = 0; XNZ < NNZ; XNZ++)
	    {
		XEQType         Node = InvP[Rows[XNZ]];

		x[Node] = b[Node] = Coeffs[XNZ];
	    }			/*      for     */
	    free (Coeffs);
	    free (Rows);
	    B->Cols[irhs].Rows = NULL;
	    B->Cols[irhs].Flags |= DENSE;
	    B->Cols[irhs].NNZ = N;
	    /* B->Cols[r].Coeffs will be assigned later.        */
	}
	/*      x: LLtx=b       */
	LLTSolve (L, x);
	/*      r=Ax-b  */
	VEqApB (r, A, x, InvP);
	for (i = 0; i < N; i++)
	    z[i] = (r[i] -= b[i]);
	/*      z: LLtz=r       */
	LLTSolve (L, z);
	/*      d=z     */
	memcpy (d, z, N * sizeof (*d));
	/*      The main iteration cycle:       */
	rzNext = ScalarProduct (N, r, z);
	while (rzNext > Eps && iter < MaxIterNo)
	{
	    FloatType       rz, dv, Ro, Beta;

	    iter++;
#ifdef	WRITE_RESIDUAL
	    sprintf (strbuff, "Residual:%20lf.", rzNext);
	    message (strbuff);
	    fprintf (fresidual, "%5d\t%20lf\n", iter, rzNext);
#endif
	    /*  v=Ad    */
	    VEqApB (v, A, d, InvP);
	    /*  Ro=<r,z>/<d,v>  */
	    rz = rzNext;
	    dv = ScalarProduct (N, d, v);
	    Ro = rz / dv;
	    /*  x = x - Ro d; r = r - Ro v      */
	    for (i = 0; i < N; i++)
	    {
		x[i] -= Ro * d[i];
		z[i] = (r[i] -= Ro * v[i]);
	    }			/*      for     */
	    /*  z: l.Lt.z=r     */
	    LLTSolve (L, z);
	    /*  Beta = <r,z>New/<r,z>Old        */
	    rzNext = ScalarProduct (N, r, z);
	    Beta = rzNext / rz;
	    /*  d = z + Beta d  */
	    for (i = 0; i < N; i++)
		d[i] = z[i] + Beta * d[i];
	}			/*      while   */
	if (iter == MaxIterNo)
	    warning ("Unable to achieve the desired accuracy.");
	sprintf (strbuff, "Sqrt[zAz/N] %18.16lf %3d iterations.",
		 (double) sqrt (rzNext / N), iter);
	message (strbuff);
	/*      Permute the solution.   */
	{
	    FloatType      *Sol;

	    allocateN (Sol, N);
	    for (i = 0; i < N; i++)
		Sol[Perm[i]] = x[i];
	    B->Cols[irhs].Coeffs = Sol;
	}
	free (v);
	free (d);
	free (z);
	free (r);
	free (x);
	free (b);
    }
    return NULL;
}

/*
 *    Function:       IterativeSolve
 *      Definition:     Solve system AX=B using preconditioned conjugate
 *      gradient technique where preconditioning is done by incomplete
 *      factorization. L - incomplete factor. The solution is permuted
 *      using Perm, InvP.
 */
IterativeSolve (SpMatrixP A, SpMatrixP L, SpMatrixP B,
		XEQType * Perm, XEQType * InvP)
{
    XEQType         NRHS = B->NCols, N = A->NCols;
    int             ithr;
    FloatType       Eps;
    struct task    *tasks;

#ifdef	WRITE_RESIDUAL
    FILE           *fresidual;

    fileopen (fresidual, "residual.txt", "w");
#endif

    Eps = Epsilon * Epsilon * N;
    sprintf (strbuff, "Desired Epsilon is %18.16lf", (double) Epsilon);
    message (strbuff);
    sprintf (strbuff, "N*[Epsilon]^2 is %18.16lg", (double) Eps);
    message (strbuff);

    /*      Permute A row numbers for effective A.B.        */
    AToAp (A, InvP);

    allocateN (tasks, NUMTHREADS);

    for (ithr = 0; ithr < NUMTHREADS; ithr++)
    {
	tasks[ithr].A = A;
	tasks[ithr].L = L;
	tasks[ithr].B = B;
	tasks[ithr].Perm = Perm;
	tasks[ithr].InvP = InvP;
	tasks[ithr].Eps = Eps;

	/* + here specifies that * should be performed before / */
	tasks[ithr].irhs0 = +(ithr * NRHS) / NUMTHREADS;
	tasks[ithr].irhs1 = +((ithr + 1) * NRHS) / NUMTHREADS;

#ifdef PTHREADS
	if (pthread_create (&(tasks[ithr].tid),
			    NULL,
			    IterativeSolveRHS,
			    (void *) &(tasks[ithr])) != 0)
	{
	    error ("pthread_create");
	}
	else
	{
	    sprintf (strbuff, "Forked out thread %d",
		     (int) tasks[ithr].tid);
	    message (strbuff);
	}
#else
	IterativeSolveRHS ((void *) &(tasks[ithr]));
#endif /* PTHREADS */
    }				/*      for     */

#ifdef PTHREADS
    for (ithr = 0; ithr < NUMTHREADS; ithr++)
	pthread_join (tasks[ithr].tid, NULL);
#endif

    /*      Permute A row numbers back.     */
    ApToA (A, Perm);

#ifdef	WRITE_RESIDUAL
    fclose (fresidual);
#endif

    return 0;
}				/*      IterativeSolve  */
