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
 *    Create permutations Perm & InvP of variables of A.
 *      Perm[NewNumber]==OldNumber, InvP[OldNumber]==NewNumber.
 */
/*      The following procedures creates the new reordering in Perm.    */
/*      Sometimes we have to mark used/unused variables -it's done      */
/*      by the constants below.                                         */
/*      InvP is returned with the inverse permutation, but is is often  */
/*      internally used to mark those used/unused variables.            */
#define	UnUsed	0
#define	Used	1
#define	Marked	2
/*
 *    Function:       Trivial
 *      Definition:     No permutation at all.
 */
Trivial (SpMatrixP A, XEQType * Perm, XEQType * InvP)
{
    XEQType         N = A->NCols;
    XEQType         i;

    for (i = 0; i < N; i++)
	Perm[i] = InvP[i] = i;
    return 0;
}				/*      Trivial */
/*
 *    Function:       NestedDissections
 *      Definition:     Implements the nested Dissection algorithm to
 *      decrease filling.
 */
NestedDissections (SpMatrixP A, XEQType * Perm, XEQType * InvP)
{
    XEQType         N = A->NCols;
    XEQType         i;		/*      Perm[i] was just numbered.      */
    XEQType         j;		/*      j is the first free NewNumber.  */
    XEQType         k;		/*      To search non-numbered variable. */
    XEQType        *Sep, NSep;	/*      Separator       */
    XEQType        *LS, *XLS, NLvl;	/*      Level structure */
    XEQType        *Mask = InvP;

    for (i = 0; i < N; i++)
	Mask[i] = UnUsed;
    allocateN (XLS, N);
    allocateN (LS, N);
    j = 0;
    for (k = 0; j < N && k < N; k++)
	while (Mask[k] == UnUsed)
	{
	    XEQType         Root;

	    /*      Find and numerate Separator.    */
	    Sep = Perm + j;
	    Root = k;
	    FindRoot (A, &Root, Mask, &NLvl, XLS, LS);
	    if (NLvl < 3)
	    {			/*      The whole comp. is Separator    */
		NSep = XLS[NLvl];
		for (i = 0; i < NSep; i++)
		{
		    Sep[i] = LS[i];
		    Mask[LS[i]] = Used;
		}		/*      for     */
	    }			/*      then    */
	    else
	    {
		XEQType         MidLvl = NLvl / 2;
		XEQType         MidBeg = XLS[MidLvl];
		XEQType         MidEnd = XLS[MidLvl + 1];
		XEQType         NextBeg = MidEnd;
		XEQType         NextEnd = XLS[MidLvl + 2];

		for (i = NextBeg; i < NextEnd; i++)
		{
		    assert (Mask[LS[i]] == UnUsed);
		    Mask[LS[i]] = Marked;
		}		/*      for     */
		NSep = 0;
		for (i = MidBeg; i < MidEnd; i++)
		{
		    XEQType         Node = LS[i];
		    XEQType         XNZ, NNZ = A->Cols[Node].NNZ;
		    XEQType        *Rows = A->Cols[Node].Rows;

		    for (XNZ = 0; XNZ < NNZ; XNZ++)
			if (Mask[Rows[XNZ]] == Marked)
			{	/* Node has nbr. at the next level. */
			    /* Add it to Separator. */
			    Sep[NSep++] = Node;
			    Mask[Node] = Used;
			    break;
			}	/*      then    */
		}		/*      for     */
		for (i = NextBeg; i < NextEnd; i++)
		{
		    assert (Mask[LS[i]] == Marked);
		    Mask[LS[i]] = UnUsed;
		}		/*      for     */
	    }			/*      else    */
	    j += NSep;
	}			/*      while   */
    /*      assert(j==N);   */
    free (XLS);
    free (LS);
    /*      Reverse this permutation.       */
    for (i = 0, j = N - 1; i < N / 2; i++, j--)
    {
	XEQType         Temp = Perm[i];

	Perm[i] = Perm[j];
	Perm[j] = Temp;
    }				/*      for     */
    for (i = 0; i < N; i++)
	InvP[Perm[i]] = i;
    return 0;
}				/*      NestedDissections       */
/*
 *    Function:       RootLS
 *      Definition:     Find Rooted Level Structure (NLvl,XLS,LS) of
 *      the component (Root,Mask) of graph A.
 */
RootLS (SpMatrixP A, XEQType Root, XEQType * Mask,
	XEQType * PNLvl, XEQType * XLS, XEQType * LS)
{
    XEQType         NLvl, CCSize;
    XEQType         i, LBeg = 0, LEnd = 1;

    Mask[Root] = Used;
    LS[0] = Root;
    XLS[0] = 0;
    NLvl = 0;
    do
    {
	CCSize = LEnd;
	/*      For all nodes in current level. */
	for (i = LBeg; i < LEnd; i++)
	{
	    XEQType         Node = LS[i];
	    XNZType         XNZ, NNZ = A->Cols[Node].NNZ;
	    XEQType        *Rows = A->Cols[Node].Rows;

	    /*      For all neighbours of the Node. */
	    for (XNZ = 0; XNZ < NNZ; XNZ++)
	    {
		XEQType         Nbr = Rows[XNZ];

		if (Mask[Nbr] == UnUsed)
		{
		    LS[CCSize++] = Nbr;
		    Mask[Nbr] = Used;
		}		/*      then    */
	    }
	}			/*      for     */
	XLS[++NLvl] = LBeg = LEnd;
	LEnd = CCSize;
    }
    while (LEnd > LBeg);
    /*      Restore Mask.   */
    for (i = 0; i < CCSize; i++)
	Mask[LS[i]] = UnUsed;
    *PNLvl = NLvl;
    return 0;
}				/*      RootLS  */
/*
 *    Function:       FindRoot
 *      Definition:     Find the pseudo-peripherical node in the component
 *      (Root,Mask) of graph A, return it in Root.
 *      Used algorithm belongs to Gibbs et al.
 */
FindRoot (SpMatrixP A, XEQType * RootP, XEQType * Mask,
	  XEQType * NLvlP, XEQType * XLS, XEQType * LS)
{
    XEQType         Root = *RootP;
    XEQType         CCSize, MinDeg, j, NLvl, NewNLvl;

    RootLS (A, Root, Mask, NLvlP, XLS, LS);
    NLvl = (*NLvlP);
    CCSize = XLS[NLvl];
    if (NLvl == 1 || NLvl == CCSize)
	return 0;
    do
    {				/*      Find in the last level node with min. degree.   */
	j = XLS[NLvl - 1];
	MinDeg = CCSize;
	Root = LS[j];
	for (; j < CCSize; j++)
	{
	    XEQType         Node = LS[j], Degree = 0;
	    XNZType         XNZ, NNZ = A->Cols[Node].NNZ;
	    XEQType        *Rows = A->Cols[Node].Rows;

	    for (XNZ = 0; XNZ < NNZ; XNZ++)
		if (Mask[Rows[XNZ]] == Used)
		    Degree++;
	    if (Degree < MinDeg)
	    {
		Root = Node;
		MinDeg = Degree;
	    }			/*      then    */
	}			/*      for     */
	RootLS (A, Root, Mask, &NewNLvl, XLS, LS);
	if (NewNLvl <= NLvl)
	{
	    *RootP = Root;
	    *NLvlP = NLvl;
	    return 0;
	}			/*      then    */
	NLvl = NewNLvl;
    }
    while (NLvl < CCSize);
    *NLvlP = NLvl;
    *RootP = Root;
    return 0;
}				/*      FindRoot        */

/*
 *    Function:       ReverseCuthillMcKee
 *      Definition:     The implementation of RCM algorithm for
 *      matrix profile minimization.
 */
ReverseCuthillMcKee (SpMatrixP A, XEQType * Perm, XEQType * InvP)
{
    XEQType         N = A->NCols;
    XEQType         i;		/*      Perm[i] was just numbered.      */
    XEQType         j;		/*      j is the first free NewNumber.  */
    XEQType         k;		/*      to search non-numbered variable. */
    XNZType        *Degree;	/*      Degree[i]==A->Cols[i].NNZ       */
    XEQType        *Mask = InvP;
    XEQType        *LS, *XLS, NLvl;	/*      Level structure */

    allocateN (XLS, N);
    allocateN (LS, N);
    allocateN (Degree, N);
    for (i = 0; i < N; i++)
    {
	Mask[i] = UnUsed;
	Degree[i] = A->Cols[i].NNZ;
    }				/*      for     */
    i = j = k = 0;
    while (j < N)
    {
	XEQType         Root;

	/*      Find first not numbered variable ...    */
	while (Mask[k] != UnUsed)
	    k++;
	assert (k < N);
	Root = k;
	FindRoot (A, &Root, Mask, &NLvl, XLS, LS);
	Perm[j] = k;
	Mask[k] = Used;
	j++;
	/*      ... and permute the others.             */
	while (i < j)
	{
	    XEQType         Node = Perm[i];
	    XEQType         j0 = j;
	    XNZType         XNZ, NNZ = Degree[Node];
	    XEQType        *Rows = A->Cols[Node].Rows;

	    /*      For all neighbours of Node ...  */
	    for (XNZ = 0; XNZ < NNZ; XNZ++)
		if (Mask[Rows[XNZ]] == UnUsed)
		{
		    Perm[j] = Rows[XNZ];
		    Mask[Rows[XNZ]] = j;
		    j++;
		}		/*      then    */
	    /*      Sort Perm[j0]..Perm[j-1] by Degrees.    */
	    {
		XEQType         p, q;

		for (p = j0; p < j - 1; p++)
		{
		    XNZType         mindeg = Degree[Perm[p]];
		    XEQType         minq = p;

		    for (q = p + 1; q <= j - 1; q++)
			if (Degree[Perm[q]] < mindeg)
			{
			    mindeg = Degree[Perm[q]];
			    minq = q;
			}	/*      then    */
		    if (minq != p)
		    {		/*      Swap Perm[minq] & Perm[p]       */
			XEQType         Temp = Perm[minq];

			Perm[minq] = Perm[p];
			Perm[p] = Temp;
		    }		/*      then    */
		}		/*      for     */
	    }
	    i++;
	}			/*      while   */
    }				/*      while   */
    free (Degree);
    free (XLS);
    free (LS);
    /*      Reverse this permutation.       */
    for (i = 0, j = N - 1; i < N / 2; i++, j--)
    {
	XEQType         Temp = Perm[i];

	Perm[i] = Perm[j];
	Perm[j] = Temp;
    }				/*      for     */
    for (i = 0; i < N; i++)
	InvP[Perm[i]] = i;
    return 0;
}				/*      ReverseCuthillMcKee     */
