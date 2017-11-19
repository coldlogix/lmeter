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
 *    This module builds the equations for points on line eqsource.
 *      For each point first build the pattern by build_pattern(),
 *      then it is used to build the equations by point_eqs().
 *      Should be initialized by build_eqs_init(),
 *      finished by build_eqs_finish().
 */

#include"lmeter.h"
#include"head_lm.h"

#ifdef PTHREADS
#include<pthread.h>
#endif

#ifndef NUMTHREADS
#define NUMTHREADS 1
#endif

struct patlink
{
    enum dtag       d_nd;	/* 1 if diagonal link, 0 if is not.     */
    short           l;
};

/* where should I move to find neighbour in sector with direction ?     */
signed char     dx[2][8] =
{
    {+1, 0, 0, -1, -1, 0, 0, +1},	/* for nd               */
    {+1, +1, -1, -1, -1, -1, +1, +1}	/* for d                */
};
signed char     dy[2][8] =
{
    {0, -1, -1, 0, 0, +1, +1, 0},	/* for nd               */
    {-1, -1, -1, -1, +1, +1, +1, +1}	/* for d                */
};

/* in what sector am I for my neighbour in sector,with direction ?      */
byte            nbrsectors[2][8] =
{
    {3, 6, 5, 0, 7, 2, 1, 4},	/* for nd               */
    {5, 4, 7, 6, 1, 0, 3, 2}	/* for d                */
};

int             usedline = 0;

#ifdef PTHREADS
pthread_mutex_t A_resize_mutex;

#endif

build_eqs_init ()
{
    int             ithr;

#ifdef PTHREADS
    if (pthread_mutex_init (&A_resize_mutex, NULL) != 0)
	error ("pthread_mutex_init");
#endif

    for (ithr = 0; ithr < NUMTHREADS; ithr++)
	allocate_EQ_matrixes (ithr);
}

build_eqs_finish ()
{
    int             ithr;

#ifdef PTHREADS
    if (pthread_mutex_destroy (&A_resize_mutex) != 0)
	error ("pthread_mutex_destroy");
#endif
    for (ithr = 0; ithr < NUMTHREADS; ithr++)
	free_EQ_matrixes (ithr);
}

struct task
{
    coord           x0;
    coord           xmax;
    int             ithr;
#ifdef PTHREADS
    pthread_t       tid;
#endif
}
tasks[NUMTHREADS];

void           *
build_line_eqs (void *mytask)
{
    coord           x0 = ((struct task *) mytask)->x0;
    coord           xmax = ((struct task *) mytask)->xmax;
    int             ithr = ((struct task *) mytask)->ithr;
    coord           x;

    for (x = x0; x < xmax; x++)
	if (eqsource[0][x].isused)
	{
	    struct patlink  pattern[8];

	    build_pattern (x, pattern);
#ifdef PSGRAPH
	    show_pattern (x, pattern, eqsource[0][x].isused - 1);
#endif
	    point_eqs (x, pattern, ithr);
	}
}

build_eqs ()
{
    int             ithr;

    usedline++;

    for (ithr = 0; ithr < NUMTHREADS; ithr++)
    {
	tasks[ithr].x0 = +(ithr * xmax) / NUMTHREADS;
	tasks[ithr].xmax = +((ithr + 1) * xmax) / NUMTHREADS;
	tasks[ithr].ithr = ithr;

#ifdef PTHREADS
	if (pthread_create (&(tasks[ithr].tid),
			    NULL,
			    build_line_eqs,
			    (void *) &(tasks[ithr])) != 0)
	{
	    error ("pthread_create");
	};
#else
	build_line_eqs ((void *) &(tasks[ithr]));
#endif /* PTHREADS */
    }
#ifdef PTHREADS
    for (ithr = 0; ithr < NUMTHREADS; ithr++)
	pthread_join (tasks[ithr].tid, NULL);
#endif

}				/*    end of build_eqs */

/*
 *    Here ase some arrays to store "element matrixes".
 *      BVars, NBVars stores (in sorted order) different BasVar numbers of
 *      the point vicinity (up to 9 of them: neighbours in 8 sectors + 1
 *      point's own).
 *      PTEs[b][i][j] contains the coefficient in the equation for variable
 *      (BVars[0]+i-1) by the variable (BVars[b]+j-1), where b<NBVars,
 *      i & j are electrode numbers. Those 1s are subtracted because
 *      electrode numbers are counted from 1.
 *      Terminal currents (PTAs) are stored in the same way.
 *      RHSides are stored in simple rectangular tables (neld x nout and
 *      ntea x nout).
 *      Temporary arrays are used when constructing an equation,
 *      Max number of terms in one equation is 9*neld<9*NLAYMAX 
 *      Max number of equations in one point is neld<NLAYMAX. 
 *
 *      All those array are large enough not to be put on stack, 
 *      they are allocated once (per thread, if necessary) and then reused.
 */

struct eq_arrays
{
    unsigned        BVars[9];
    unsigned        NBVars;
    float        ***PTEs;	/*      [9][NLAYMAX][NLAYMAX];  */
    float        ***PTAs;	/*      [9][NTRMMAX][NLAYMAX];  */
    float         **PTERHSs;	/*      [NLAYMAX][NTRMMAX];     */
    float         **PTARHSs;	/*      [NTRMMAX][NTRMMAX];     */
    FloatType     **TmpCoeffs;	/*      [NLAYMAX][9*NLAYMAX];   */
    XEQType       **TmpRows;	/*      [NLAYMAX][9*NLAYMAX];   */
    XNZType        *TmpNNZs;	/*      [NLAYMAX];              */
}
eq_arrays[NUMTHREADS];

/* They were combined in one structure just recently to facilitate 
 * multithreading. I do not want to change actual code, thus those #defines. 
 */

#define BVars           eq_arrays[ithr].BVars
#define NBVars          eq_arrays[ithr].NBVars
#define PTEs            eq_arrays[ithr].PTEs
#define PTAs            eq_arrays[ithr].PTAs
#define PTERHSs         eq_arrays[ithr].PTERHSs
#define PTARHSs         eq_arrays[ithr].PTARHSs
#define TmpCoeffs       eq_arrays[ithr].TmpCoeffs
#define TmpRows         eq_arrays[ithr].TmpRows
#define TmpNNZs         eq_arrays[ithr].TmpNNZs

/*
 *    Function:       allocate_EQ_matrixes
 *      Definition:     Allocate work vectors in eq_arrays
 */
allocate_EQ_matrixes (int ithr)
{
    unsigned        i, j;

    allocateN (PTEs, 9);
    allocateN (PTAs, 9);
    for (i = 0; i < 9; i++)
    {
	allocateN (PTEs[i], NLAYMAX);
	allocateN (PTAs[i], NTRMMAX);
	for (j = 0; j < NLAYMAX; j++)
	{
	    allocateN (PTEs[i][j], NLAYMAX);
	}			/*      for     */
	for (j = 0; j < NTRMMAX; j++)
	{
	    allocateN (PTAs[i][j], NLAYMAX);
	}			/*      for     */
    }				/*      for     */

    allocateN (PTERHSs, NLAYMAX);
    allocateN (PTARHSs, NTRMMAX);
    for (i = 0; i < NLAYMAX; i++)
    {
	allocateN (PTERHSs[i], NTRMMAX);
    }				/*      for     */
    for (i = 0; i < NTRMMAX; i++)
    {
	allocateN (PTARHSs[i], NTRMMAX);
    }				/*      for     */

    allocateN (TmpCoeffs, NLAYMAX);
    allocateN (TmpRows, NLAYMAX);
    allocateN (TmpNNZs, NLAYMAX);
    for (i = 0; i < NLAYMAX; i++)
    {
	allocateN (TmpCoeffs[i], NLAYMAX * 9);
	allocateN (TmpRows[i], NLAYMAX * 9);
    }				/*      for     */
    return 0;

}				/*      allocate_EQ_matrixes    */

/*
 *    Function:       free_EQ_matrixes
 *      Definition:     Free work vectors in eq_arrays
 */
free_EQ_matrixes (int ithr)
{
    unsigned        i, j;

    for (i = 0; i < 9; i++)
    {
	for (j = 0; j < NLAYMAX; j++)
	    free (PTEs[i][j]);
	for (j = 0; j < NTRMMAX; j++)
	    free (PTAs[i][j]);
	allocateN (PTEs[i], NLAYMAX);
	allocateN (PTAs[i], NTRMMAX);
    }				/*      for     */
    for (i = 0; i < NLAYMAX; i++)
	free (PTERHSs[i]);
    for (i = 0; i < NTRMMAX; i++)
	free (PTARHSs[i]);
    free (PTEs);
    free (PTAs);
    free (PTERHSs);
    free (PTARHSs);

    for (i = 0; i < NLAYMAX; i++)
    {
	free (TmpCoeffs[i]);
	free (TmpRows[i]);
    }
    free (TmpCoeffs);
    free (TmpRows);
    free (TmpNNZs);

    return 0;
}				/*      free_EQ_matrixes        */

/*
 *    Function:       point_eqs
 *      Definition:     The heart of LM program - write out the equations,
 *      RHSs and terminal currents for one point.
 */
point_eqs (coord x, struct patlink * pattern, int ithr)
{
    roundcnf _HUGE *ownrcf, _HUGE * nbrrcfs[8], _HUGE * nbrrcf;
    unsigned        ownbasvar, nbrbasvars[8];
    byte            nbrXBVs[8], ownXBV, XBV;	/* Indexes in BVars     */
    byte            nbrsector[8];
    byte            XBVToNeld[9];
    cnftype         notwritten = ~0;	/* which elds have no PTE yet   */
    byte            l, s, i, mylayup, mylaydown, nblayup, nblaydown, nl,
                    ns;
    byte            ield, ownneld, term, imetl, t, LocalVar;
    double          coeff;
    layelinfo      *layels;
    byte            uplay, dnlay;
    int             st1, st2;
    float        ***EQs, *EQ;
    float          *RHS;
    unsigned        NNZ;

    assert (x < xmax);
    assert (eqsource[0][x].incnf < cnfarrlen);

    ownrcf = configs + eqsource[0][x].incnf;
    ownbasvar = eqsource[0][x].varnum;
    ownneld = ownrcf->neld;
    /* To find all neighbours use information in pattern.           */
    /* Initialize BVars with nbrbasvars & ownbasvar in sorted order. */
    NBVars = 1;
    BVars[0] = ownbasvar;
    for (s = 0; s < 8; s++)
    {
	coord           deltax, deltay;
	enum dtag       d_nd;
	short           l;

	if ((l = pattern[s].l) == 0)	/* bounds to space */
	    nbrrcfs[s] = NULL;
	else
	{
	    d_nd = pattern[s].d_nd;
	    deltay = dy[d_nd][s] * l;
	    deltax = dx[d_nd][s] * l;
	    assert (abs (deltax) <= POW2 (kmax));
	    assert (abs (deltay) <= POW2 (kmax));
	    nbrrcfs[s] = configs + eqsource[deltay][x + deltax].incnf;
	    nbrbasvars[s] = eqsource[deltay][x + deltax].varnum;
	    nbrsector[s] = nbrsectors[d_nd][s];
	    for (XBV = 0; XBV < NBVars; XBV++)
		if (BVars[XBV] >= nbrbasvars[s])
		    break;
	    if (XBV == NBVars)
		BVars[NBVars++] = nbrbasvars[s];
	    else if (BVars[XBV] > nbrbasvars[s])
	    {
		unsigned        XBV1 = NBVars++;

		while (XBV1 > XBV)
		{
		    BVars[XBV1] = BVars[XBV1 - 1];
		    XBV1--;
		}		/*      while   */
		BVars[XBV] = nbrbasvars[s];
	    }			/*      else if */
	    assert (BVars[XBV] == nbrbasvars[s]);
	}			/*      else    */
    }				/*      for     */
    /*      Initialize nbrXBVs, ownXBV & XBVToNeld. */
    for (XBV = 0; XBV < NBVars; XBV++)
	XBVToNeld[XBV] = 0;
    for (s = 0; s < 8; s++)
	if (nbrrcfs[s] != NULL)
	    for (XBV = 0; XBV < NBVars; XBV++)
		if (BVars[XBV] == nbrbasvars[s])
		{
		    nbrXBVs[s] = XBV;
		    if (XBVToNeld[XBV] < nbrrcfs[s]->neld)
			XBVToNeld[XBV] = nbrrcfs[s]->neld;
		    break;
		}		/*      then    */
    for (XBV = 0; XBV < NBVars; XBV++)
	if (BVars[XBV] == ownbasvar)
	{
	    ownXBV = XBV;
	    if (XBVToNeld[XBV] < ownneld)
		XBVToNeld[XBV] = ownneld;
	    break;
	}			/*      then    */
    for (l = 1; l <= nlay; l++)
    {
	if ((ield = ownrcf->layels[l].ield) == 0)
	    continue;
	assert (ield <= ownneld);
	if (notwritten & POW2 (ield))	/* need PTE     */
	{
	    notwritten &= ~POW2 (ield);
	    EQs = PTEs;
	    LocalVar = ield;
	    RHS = PTERHSs[LocalVar];
	}
	else if ((term = ownrcf->layels[l].term) > 0 && term != ISOHOLE)
	{
	    /* generate PTA for terminal */
	    assert (term <= ntea);
	    EQs = PTAs;
	    LocalVar = term;
	    RHS = PTARHSs[LocalVar];
	}
	else
	    continue;

	/* count currs from all sectors                 */
	for (s = 0; s < 8; s++)
	{
	    nbrrcf = nbrrcfs[s];
	    /* is it near Free space ?                      */
	    if (nbrrcf == NULL)
		continue;

	    /* find own up/down layers                      */
	    imetl = ownrcf->layels[l].imetl;
	    assert (imetl <= ownrcf->nmetl);
	    mylayup = ownrcf->gaps[s][imetl].uplay;
	    nl = mylayup;
	    mylaydown = ownrcf->gaps[s][imetl].downlay;

	    /* is there upper eld ?                         */
	    if (mylayup == 0)
		continue;

	    assert (mylayup > mylaydown);
	    coeff = ownrcf->gaps[s][imetl].coeff;

	    /* find nbr up/down layers                      */
	    ns = nbrsector[s];
	    imetl = nbrrcf->layels[nl].imetl;
	    assert (imetl <= nbrrcf->nmetl);
	    nblayup = nbrrcf->gaps[ns][imetl].uplay;
	    nblaydown = nbrrcf->gaps[ns][imetl].downlay;

	    assert (nblayup == mylayup && nblaydown == mylaydown);
	    /*
	     * assert(fabs(coeff-nbrrcf->gaps[ns][imetl].coeff) 
	     *      < 0.00001); 
	     *
	     * This was very usefull assertion
	     * while debugging LMeter, but when we introduced
	     * edge admittance, it is no longer true for 45
	     * degree edges. Now we hope, that all difference
	     * between those two coeffs are due to edge
	     * admittance and we USE THE LARGEST ONE --- to
	     * take this admittance into account.
	     */
	    coeff = MAX (coeff, nbrrcf->gaps[ns][imetl].coeff);
	    /*
	     * Add to current equation four sums: 
	     *      psi var from current point              
	     *      then psi var from nbr. point.           
	     *      Each psi var consists of parts from     
	     *      up/down layers of the gap.              
	     */
	    for (st1 = 0,
	    /*      First pass - for own.   */
		 layels = ownrcf->layels,
		 uplay = mylayup,
		 dnlay = mylaydown,
		 EQ = EQs[ownXBV][LocalVar];
		 st1 < 2;
		 st1++,
	    /*      Second pass - for nbr.  */
		 layels = nbrrcf->layels,
		 uplay = nblayup,
		 dnlay = nblaydown,
		 EQ = EQs[nbrXBVs[s]][LocalVar],
		 coeff = -coeff
		)
	    {
		unsigned        var;
		signed          term;
		byte            lay;

		assert (layels[uplay].ield);
		/* both are metal       */
		assert (dnlay == 0 || layels[dnlay].ield);
		for (st2 = 0,
		     lay = uplay;
		     st2 < 2;
		     st2++,
		     lay = dnlay,
		     coeff = -coeff
		    )
		    while (lay > 0)
		    {
			if ((var = layels[lay].ield) == 0)
			    /* not metal,skip */
			    lay--;
			else if (term = layels[lay].term)
			    /* add term phi */
			    if (term == ISOHOLE)
				/* skip */
				lay--;
			    else
			    {
				assert (term <= ntea);
				if (term <= nout)
				    RHS[term] += coeff;
				lay = layels[lay].nxtlay;
				assert (lay > 0 && lay <= nlay);
			    }
			else
			    /* add phi var    */
			{
			    EQ[var] += coeff;
			    lay--;
			}
		    }		/*      while   */
	    }			/*      for     */
	}			/*      for     */
    }				/*      for     */
    /*      Add PTA to PTE if necessary.                    */
    for (i = 1; i <= ownneld; i++)
	if (term = ownrcf->addpta[i])
	{
	    for (XBV = 0; XBV < NBVars; XBV++)
		for (ield = 1; ield <= XBVToNeld[XBV]; ield++)
		    PTEs[XBV][i][ield] -= PTAs[XBV][term][ield];
	    for (t = 1; t <= nout; t++)
		PTERHSs[i][t] -= PTARHSs[term][t];
	}			/*      then    */
    /*      Assemble partial matrixes to the global one.    */
    /*      Assemble PTEs.  */
#ifdef PTHREADS
    if (pthread_mutex_lock (&A_resize_mutex) != 0)
	error ("pthread_mutex_lock");
#endif
    if (A->NCols < ownbasvar + ownneld)
    {
	XEQType         Newsize = max (ownbasvar + ownneld, 2 * A->NCols);

	sprintf (strbuff, "LM: Matrix size increased %u -> %u",
		 A->NCols, Newsize);
	message (strbuff);
	SpMatrixResize (A, Newsize);
    }				/*      then    */
#ifdef PTHREADS
    if (pthread_mutex_unlock (&A_resize_mutex) != 0)
	error ("pthread_mutex_unlock");
#endif
    for (i = 1; i <= ownneld; i++)
	TmpNNZs[i] = 0;
    for (XBV = 0; XBV < NBVars; XBV++)
    {
	for (i = 1; i <= ownneld; i++)
	{
	    unsigned        var = i + ownbasvar - 1;

	    NNZ = TmpNNZs[i];
	    for (ield = 1; ield <= XBVToNeld[XBV]; ield++)
	    {
		if (var == ield + BVars[XBV] - 1)
		    A->Diag[var] = PTEs[XBV][i][ield];
		else if (fabs (PTEs[XBV][i][ield]) > 1e-6)
		{
		    TmpCoeffs[i][NNZ] = PTEs[XBV][i][ield];
		    TmpRows[i][NNZ] = ield + BVars[XBV] - 1;
		    NNZ++;
		}		/*      else if */
		PTEs[XBV][i][ield] = 0.0;
	    }			/*      for     */
	    TmpNNZs[i] = NNZ;
	    assert (NNZ < 9 * NLAYMAX);
	}			/*      for     */
    }				/*      for     */
    for (i = 1; i <= ownneld; i++)
    {
	unsigned        var = i + ownbasvar - 1;
	XNZType         NNZ = TmpNNZs[i];

#ifndef	NDEBUG
	unsigned        XNZ;

	for (XNZ = 1; XNZ < NNZ; XNZ++)
	{
	    assert (TmpRows[i][XNZ] > TmpRows[i][XNZ - 1]);
	    assert (TmpRows[i][XNZ] < lastvar);
	}			/*      for     */
#endif
	if (NNZ)
	{
	    allocateN (A->Cols[var].Rows, NNZ);
	    allocateN (A->Cols[var].Coeffs, NNZ);
	    A->Cols[var].NNZ = NNZ;
	    memcpy (A->Cols[var].Rows,
		    TmpRows[i],
		    sizeof (XEQType) * NNZ);
	    memcpy (A->Cols[var].Coeffs,
		    TmpCoeffs[i],
		    sizeof (FloatType) * NNZ);
	}			/*      then    */
    }				/*      for     */
    /*      Assemble PTERHSs.       */
    for (i = 1; i <= ownneld; i++)
	for (t = 1; t <= nout; t++)
	{
	    if (fabs (PTERHSs[i][t]) > 1e-6)
		SpVectorAdd (B->Cols + t - 1,
			     i + ownbasvar - 1,
			     -PTERHSs[i][t] / currunit);
	    PTERHSs[i][t] = 0.0;
	}			/*      for     */
    /*      Assemble PTAs.  */
    for (XBV = 0; XBV < NBVars; XBV++)
    {
	for (t = 1; t <= ntea; t++)
	{
	    for (ield = 1; ield <= XBVToNeld[XBV]; ield++)
	    {
		if (fabs (PTAs[XBV][t][ield]) > 1e-6)
		    SpVectorAdd (C->Cols + t - 1,
				 ield + BVars[XBV] - 1,
				 PTAs[XBV][t][ield]);
		PTAs[XBV][t][ield] = 0.0;
	    }			/*      for     */
	}			/*      for     */
    }				/*      for     */
    /*      Assemble PTARHSs.       */
    for (t = 1; t <= ntea; t++)
    {
	for (term = 1; term <= nout; term++)
	{
	    if (fabs (PTARHSs[t][term]) > 1e-6)
		SpVectorAdd (D->Cols + term - 1,
			     t - 1,
			     -PTARHSs[t][term] / currunit);
	    PTARHSs[t][term] = 0.0;
	}
    }				/*      for     */
#ifdef SHOWFLOW
    for (i = 1; i <= ownneld; i++)
      {
	int imetl;
	for (l=1; l <= nlay; l++)
	  if (i == ownrcf->layels[l].ield)
	    {
	      imetl = ownrcf->layels[l].imetl;
	      break;
	    }
	SpVectorAdd (XYZ->Cols + 0, i + ownbasvar - 1, (FloatType) x);
	SpVectorAdd (XYZ->Cols + 1, i + ownbasvar - 1, (FloatType) y);
	SpVectorAdd (XYZ->Cols + 2, i + ownbasvar - 1, (FloatType) imetl-1);
      }
#endif

    return 0;
}				/*  end of point_eqs */

/*
 *    Function:       num_vars
 *      Definition:     Store variable numbers in varnum and modify lastvar.
 */
num_vars (void)
{
    coord           x;
    cnfkey          key;
    byte            neld;

    for (x = 0; x < xmax; x++)
	if (endinterf[0][x].isused)	/* is marked by dotter  */
	{
	    key = endinterf[0][x].incnf;
	    assert (key < cnfarrlen);
	    neld = configs[key].neld;
	    endinterf[0][x].varnum = lastvar;
	    lastvar += neld;
	}

}				/*      num_vars        */

/*
 *    That is the function to build pattern - array which describes
 *    the direction and length to move to the neighbour in all 8 sectors.
 */

build_pattern (coord x, struct patlink *pattern)
{
    enum dtag       move45;
    byte            klookat, kpoint, kfinal, nset;
    unsigned        step;
    byte            s, s1;	/* slots */
    int             deltax, deltay;

    assert (x >= 0 && x < xmax);

    for (s = 0; s < 8; s++)
	pattern[s].l = 0;

    assert (eqsource[0][x].isused);
    kpoint = eqsource[0][x].isused - 1;		/* 1 was added while storing    */
    assert (kpoint <= kmax);
    /*    search for neighbour in the following order 
     *    K-KPOINT:      -2 -2  -1 -1  0  0  1  1
     *    D_ND    :      ND  D  ND  D ND  D ND  D
     *
     * For higher-k nodes when 45 degree lines are present, sometimes we should
     * start at kpoint-2. Paul B., 12/16/02
     */
    klookat = (kpoint == 0) ? 0 : kpoint - 1;
    if (klookat > 0) klookat--;
    kfinal = (kpoint == kmax) ? kmax : kpoint + 1;
    move45 = ND;
    nset = 0;
    step = POW2 (klookat);
    while (nset < 8 && klookat <= kfinal)
    {
	for (s = move45; s < 8; s += 2)
	{
	    deltax = dx[move45][s] * step;
	    deltay = dy[move45][s] * step;
	    if (x + deltax >= xmax ||
		x + deltax < 0 ||
		deltay < to_back ||
		deltay > to_front)
		continue;
	    if (!eqsource[deltay])
		printf ("%d %d %d\n", (int) deltay, (int) to_front, (int) to_back);
	    assert (eqsource[deltay]);
	    if (eqsource[deltay][x + deltax].isused)
	    {
		s1 = s;
		if (pattern[s1].l == 0)
		{
		    pattern[s1].l = step;
		    pattern[s1].d_nd = move45;
		    nset++;
		}
		s1 = (s) ? s - 1 : 7;
		if (pattern[s1].l == 0)
		{
		    pattern[s1].l = step;
		    pattern[s1].d_nd = move45;
		    nset++;
		}
	    }
	}
	if (move45 == ND)
	    move45 = DG;	/* use the same step */
	else
	{
	    move45 = ND;
	    step = POW2 (++klookat);
	}
    }

}				/* end of build_pat */
