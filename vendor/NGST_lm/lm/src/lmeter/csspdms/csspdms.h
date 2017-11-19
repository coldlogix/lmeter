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
 * Header for C Sparse Symmetric Positive Definite Matrix Solver.
 * 
 * Subroutines provided by this library implement algorithms described in
 *      Alan George, Joseph W-H Liu, "Computer Solution of Large Sparse
 *      Positive Definite Systems", Prentice-Hall, 1981
 */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<math.h>

#ifndef	___MISC_H
#include"misc.h"
#endif

/*
 *    Some system-dependent additions.
 */

#ifdef	__GNUC__
#include<unistd.h>
#endif

#if     defined(__GNUC__) || defined (__NDPC__)
typedef unsigned XEQType;
typedef unsigned XNZType;

#else
typedef unsigned XEQType;
typedef unsigned XNZType;

#endif

typedef double  FloatType;

typedef struct
{
    XNZType         NNZ;	/*      Current Number of NZ.   */
    XEQType        *Rows;
    FloatType      *Coeffs;
    unsigned        Flags;
    /*
     *    The following - for internal use of SpVector only.
     */
    XNZType         LNZ;
    XNZType        *Left, *Right;
}
SpVector       , *SpVectorP;
typedef struct
{
    XEQType         NCols;
    SpVectorP       Cols;
    FloatType      *Diag;
}
SpMatrix       , *SpMatrixP;

/*
 *    Flags of SpVector Type:
 */
#define	SWAPPABLE		(1<<0)
#define	INSERTABLE		(1<<1)
#define	DENSE			(1<<2)

SpMatrixP       SpMatrixCreate (XEQType NCols);
int             SpMatrixResize (SpMatrixP SpM, XEQType NCols);
int             SpMatrixFree (SpMatrixP SpM);

#define	SpMatrixAdd	( SpM,Col,Row,Coeff )				  	\
	do								       	\
	{       if	((Col)>=(SpM)->NCols)	SpMatrixResize((SpM),(Col+1));	\
		if	((Col)==(Row))	(SpM)->Diag[Col]+=(Coeff);	       	\
		else	SpVectorAdd((SpM)->Cols+(Col),(Row),(Coeff));	       	\
	}	while	(0)

SpVectorP       SpNVectorsCreate (XEQType N);
SpVectorP       SpNVectorsResize (SpVectorP SpV, XEQType NOld, XEQType NNew);
int             SpNVectorsFree (SpVectorP SpV, XEQType N);

int             SpVectorStartInsertion (SpVectorP SpV);
int             SpVectorAdd (SpVectorP SpV, XEQType Row, FloatType Coeff);
int             SpVectorEndInsertion (SpVectorP SpV);
int             SpVectorBuild (SpVectorP SpV, XEQType * Rows, FloatType * Coeffs, XNZType NNZ);
int             SpVectorPlusEq (SpVectorP SpV1, SpVectorP SpV2);

int             Trivial (SpMatrixP A, XEQType * Perm, XEQType * InvP);
int             NestedDissections (SpMatrixP A, XEQType * Perm, XEQType * InvP);
int             ReverseCuthillMcKee (SpMatrixP A, XEQType * Perm, XEQType * InvP);

int             SymbolicFactorize (SpMatrixP A, XEQType * Perm, XEQType * InvP);
int             NumericalFactorize (SpMatrixP A);
int             DirectSolve (SpMatrixP L, SpMatrixP B, XEQType * Perm, XEQType * InvP);

extern FloatType Threshold;
extern FloatType InitThreshold;
extern long unsigned L_NNZ_Avail;
SpMatrixP       Factorize (SpMatrixP A, XEQType * Perm, XEQType * InvP);

extern FloatType Epsilon;
extern unsigned MaxIterNo;
int             IterativeSolve (SpMatrixP A, SpMatrixP L, SpMatrixP B, XEQType * Perm, XEQType * InvP);

SpMatrixP       SpMatrixMultiply (SpMatrixP A, SpMatrixP B);
SpMatrixP       SpMatrixDecrement (SpMatrixP A, SpMatrixP B);

int             SpMatrixCheckSymm (SpMatrixP A);
int             SpMatrixPrint (char *fname, SpMatrixP A);

/*
 *    Some low-level functions.
 */
int             LLTSolve (SpMatrixP L, FloatType * B);
int             VPlusEqAB (FloatType * V, SpMatrixP A, FloatType * B, XEQType * InvP);
int             VEqApB (FloatType * V, SpMatrixP A, FloatType * B, XEQType * InvP);
int             ApToA (SpMatrixP A, XEQType * Perm);
int             AToAp (SpMatrixP A, XEQType * InvP);
FloatType       ScalarProduct (XEQType N, FloatType * A, FloatType * B);
FloatType       Norm (XEQType N, FloatType * R);

extern char     strbuff[];	/*      buffer to sprintf messages      */

#ifdef	STAT_COUNT
extern long unsigned A_NNZ, L_NACTCOL, L_NNZ;

#endif
