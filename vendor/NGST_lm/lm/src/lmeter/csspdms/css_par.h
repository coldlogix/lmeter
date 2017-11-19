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
 *    This file contains some parameters to tune the CSSPDMS.
 *      It is used while compiling CSSPDMS library, not by subroutines
 *      users.
 */

#ifndef NUMTHREADS
#define NUMTHREADS 1
#endif

/*
 *    if Ljj<EPS an error occures - matrix is not positive definite.
 */
#define	EPS			1.0e-8

/*
 *    The next five defines control the incomplete factorization.
 *      Note, that first three determines only the INITIAL values of
 *      Threshold and L_NNZ_Avail, the direct assignment to those variables
 *      overrides those defaults.
 */
#define	LNNZMEM			256
/*      This number of M's of memory can be used for L storing.         */
#define	THRESHOLD_DEFAULT       1e-8
/*      The default initial value of Threshold, rather small.           */
#define	INITTHRESHOLD_DEFAULT	1e-6
/*      The default value of Threshold after the first incrementation.  */
#define	CHECK_TIME		10000
/*      After each CHECK_TIME colomns factorized the L_NNZ number is    */
/*      checked, if it is too large, the Threshold is multiplied by ... */
#define  DELTA_THR		5.0
/*      ... this value.                                                 */

/*
 *    This value determines the default maximum number of iterations while
 *      system solving. The direct assignment to MaxIterNo overrides this
 *      default.
 */
#define	MAXITER_DEFAULT		64
