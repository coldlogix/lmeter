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

#define	DLNZ	8

/*
 *    Function:       SpNVectorsCreate
 *      Definition:     Create array of N SpVectors.
 */
SpVectorP 
SpNVectorsCreate (XEQType N)
{
    SpVectorP       New;

    allocateN (New, N);
    return New;
}				/*      SpVectorsNCreate        */
/*
 *    Function:       SpNVectorsResize
 *      Definition:     Make array of SpVectors of length NOld be NNew length,
 *      return pointer to this realloced array.
 */
SpVectorP 
SpNVectorsResize (SpVectorP SpV, XEQType NOld, XEQType NNew)
{
    XEQType         i;

    reallocateN (SpV, NNew);
    for (i = NOld; i < NNew; i++)
    {
	SpV[i].NNZ = SpV[i].LNZ = 0;
	SpV[i].Rows = NULL;
	SpV[i].Coeffs = NULL;
	SpV[i].Left = SpV[i].Right = NULL;
	SpV[i].Flags = 0;
    }				/*      for     */
    return SpV;
}				/*      SpNVectorsResize        */
/*
 *    Function:       SpNVectorsFree
 *      Definition:     Free all allocated fields in N items of type SpV.
 */
SpNVectorsFree (SpVectorP SpV, XEQType N)
{
    XEQType         i;

    for (i = 0; i < N; i++)
    {
	Free (SpV[i].Rows);
	Free (SpV[i].Coeffs);
	if (SpV[i].Flags & INSERTABLE)
	{
	    Free (SpV[i].Left);
	    Free (SpV[i].Right);
	}			/*      then    */
    }				/*      for     */
    Free (SpV);
    return 0;
}				/*      SpNVectorsFree  */
/*
 *      Function:     SpVectorStartInsertion
 *      Definition:     Initialize SpVector structures for fast search
 *      and insertion of new elements.
 *      Restrictions:   Assume the vector is empty before StartInsertion.
 */
SpVectorStartInsertion (SpVectorP SpV)
{
    XNZType         LNZ;

    if (SpV->NNZ)
	error ("Insertion to non-empty SpV unsupported");
    assert (!(SpV->Flags & INSERTABLE));
    SpV->Flags |= INSERTABLE;
    SpV->LNZ = LNZ = DLNZ;
    allocateN (SpV->Rows, LNZ);
    allocateN (SpV->Coeffs, LNZ);
    allocateN (SpV->Left, LNZ);
    allocateN (SpV->Right, LNZ);
    return 0;
}				/*      SpVectorStartInsertion  */
/*
 *    Function:       SpVectorEndInsertion
 *      Definition:     Restore the original state of SpVector.
 */
SpVectorEndInsertion (SpVectorP SpV)
{
    XNZType         XNZ, Cur, LNZ = SpV->LNZ, NNZ = SpV->NNZ;
    XEQType        *Rows = SpV->Rows, *NewRows;
    FloatType      *Coeffs = SpV->Coeffs, *NewCoeffs;
    XNZType        *Left = SpV->Left, *Right = SpV->Right;
    XNZType        *Stack, SP = 0;
    int             GoLeft;

    assert (SpV->Flags & INSERTABLE);
    SpV->Flags &= ~INSERTABLE;
    if (NNZ == 0)
    {
	free (Rows);
	free (Coeffs);
	free (Left);
	free (Right);
	SpV->Rows = NULL;
	SpV->Coeffs = NULL;
	SpV->Left = SpV->Right = NULL;
	SpV->NNZ = SpV->LNZ = 0;
	return 0;
    }				/*      then    */
    allocateN (Stack, NNZ);
    allocateN (NewRows, NNZ);
    allocateN (NewCoeffs, NNZ);
    XNZ = 0;
    GoLeft = 1;
    Cur = 0;
    do
    {
	if (GoLeft && Left[Cur])
	{			/*      Push Cur        */
	    Stack[SP++] = Cur;
	    Cur = Left[Cur];
	}			/*      then    */
	else
	{			/*      Write out Cur   */
	    NewRows[XNZ] = Rows[Cur];
	    NewCoeffs[XNZ] = Coeffs[Cur];
	    XNZ++;
	    /*      Move to next    */
	    if (Right[Cur])
	    {
		Cur = Right[Cur];
		GoLeft = 1;
	    }			/*      then    */
	    else if (SP)
	    {
		Cur = Stack[--SP];
		GoLeft = 0;
	    }			/*      then    */
	    else
		break;
	}			/*      else    */
    }
    while (1);
    assert (SP == 0);
    free (Stack);
    free (Rows);
    free (Coeffs);
    free (Left);
    free (Right);
    SpV->Left = SpV->Right = NULL;
    SpV->NNZ = SpV->LNZ = NNZ;
    SpV->Rows = NewRows;
    SpV->Coeffs = NewCoeffs;
    return 0;
}				/*      SpVectorEndInsertion    */
/*
 *    Function:       SpVectorAdd
 *      Definition:     SpV[Row]+=Coeff
 */
SpVectorAdd (SpVectorP SpV, XEQType Row, FloatType Coeff)
{
    XNZType         XNZ, LastXNZ, LNZ = SpV->LNZ, NNZ = SpV->NNZ;
    XEQType        *Rows = SpV->Rows;
    FloatType      *Coeffs = SpV->Coeffs;
    XNZType        *Left = SpV->Left, *Right = SpV->Right;

    assert (SpV->Flags & INSERTABLE);
    XNZ = 0;
    if (NNZ)
	do
	{
	    assert (XNZ < NNZ);
	    LastXNZ = XNZ;
	    if (Row < Rows[XNZ])
		XNZ = Left[XNZ];
	    else if (Row > Rows[XNZ])
		XNZ = Right[XNZ];
	    else
		/* Row==Rows[XNZ], add Coeff and return   */
	    {
		Coeffs[XNZ] += Coeff;
		return 0;
	    }
	}
	while (XNZ);
    else
	LastXNZ = 0;
    assert (LastXNZ < NNZ || NNZ == 0);
    assert (NNZ <= LNZ);
    /*      Insert new element and grow a branch from LastXNZ.      */
    if (NNZ == LNZ)		/*      Expand arrays   */
    {
	SpV->LNZ = (LNZ += DLNZ);
	reallocateN (Rows, LNZ);
	reallocateN (Coeffs, LNZ);
	reallocateN (Left, LNZ);
	reallocateN (Right, LNZ);
	SpV->Rows = Rows;
	SpV->Coeffs = Coeffs;
	SpV->Left = Left;
	SpV->Right = Right;
	for (XNZ = NNZ; XNZ < LNZ; XNZ++)
	    Left[XNZ] = Right[XNZ] = 0;
    }				/*      then    */
    Rows[NNZ] = Row;
    Coeffs[NNZ] = Coeff;
    assert (!Left[NNZ] && !Right[NNZ]);
    if (Row < Rows[LastXNZ])
	Left[LastXNZ] = NNZ;
    else			/*Row>Rows[LastXNZ] */
	Right[LastXNZ] = NNZ;
    SpV->NNZ = NNZ + 1;
    return 0;
}				/*      SpVectorAdd     */
/*
 *    Function:       SpVectorBuild
 *      Definition:     Build the SpVector from Rows & Coeffs by sorting
 *      them by Rows. Rows & Coeffs are assumed to be at least NNZ+1 length.
 */
SpVectorBuild (SpVectorP SpV, XEQType * Rows, FloatType * Coeffs, XNZType NNZ)
{
    XNZType         XNZ, Front;
    XEQType         Row;
    FloatType       Coeff;

    /*      Sort by Rows. Simple N^2 algorith is used.      */
    Rows[NNZ] = 0;		/*      Insert the sentinel.    */
    Front = 0;
    while (Front < NNZ)
    {
	XNZ = Front;
	while (Rows[XNZ] < Rows[XNZ + 1])
	    XNZ++;
	/*      XNZ+1'th element must be moved lower.   */
	Front = ++XNZ;
	if (Front < NNZ)
	{
	    Row = Rows[XNZ];
	    Coeff = Coeffs[XNZ];
	    while (XNZ && Rows[XNZ - 1] > Row)
	    {
		Rows[XNZ] = Rows[XNZ - 1];
		Coeffs[XNZ] = Coeffs[XNZ - 1];
		XNZ--;
	    }			/*      while   */
	    Rows[XNZ] = Row;
	    Coeffs[XNZ] = Coeff;
	}			/*      then    */
    }				/*      while   */
    allocateN (SpV->Rows, NNZ);
    allocateN (SpV->Coeffs, NNZ);
    SpV->NNZ = SpV->LNZ = NNZ;
    memcpy (SpV->Rows, Rows, NNZ * sizeof (*Rows));
    memcpy (SpV->Coeffs, Coeffs, NNZ * sizeof (*Coeffs));
    return 0;
}				/*      SpVectorBuild   */
/*
 *    Function:       SpVectorPlusEq
 *      Definition:     SpV1+=SpV2;
 */
SpVectorPlusEq (SpVectorP SpV1, SpVectorP SpV2)
{
    int             WasNotIns = 0;
    XNZType         XNZ, NNZ = SpV2->NNZ;
    XEQType        *R = SpV2->Rows;
    FloatType      *C = SpV2->Coeffs;

    if (!(SpV1->Flags & INSERTABLE))
    {
	WasNotIns = 1;
	SpVectorStartInsertion (SpV1);
    }				/*      then    */
    for (XNZ = 0; XNZ < NNZ; XNZ++)
	SpVectorAdd (SpV1, *(R++), *(C++));
    if (WasNotIns)
	SpVectorEndInsertion (SpV1);
    return 0;
}				/*      SpVectorPlusEq  */
