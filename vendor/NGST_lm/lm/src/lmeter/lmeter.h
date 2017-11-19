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

#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include"sysdpnd.h"
#include"misc.h"

/*
 *    Caution! Variables of the following types are readed/writed using
 *      printf formats - so when changing those types be sure to change
 *      the formats too.
 */
typedef int     coord;
typedef unsigned char byte;
typedef signed char sbyte;

#ifdef LONGCNF
typedef unsigned long long cnftype;

#else
typedef unsigned long cnftype;

#endif

/* Perform shifts that are long enough */
#define POW2(k)         (((cnftype)1)<<(cnftype)(k))

#define	NLAYMAX		(8*sizeof(cnftype)-1)
#define	NTRMMAX		NLAYMAX

#define	NAMLEN		16

extern char     strbuff[];	/*      The buffer to sprintf messages. */
EXTERN char    *progname;	/*      argv[0], surely.                */
EXTERN char    *swapdir;	/*      Path to store output files.     */
EXTERN float    gridsize;	/*      ACAD units/LM grid unit         */
extern char    *mystrupr (char *);
extern char    *getpar (char *name);
EXTERN FILE    *topf;

#define	GETPAR(val,name,toupr)					\
{	if	( (val=getpar(name))==NULL )			\
	{	sprintf(strbuff," %s is undefined",(name));	\
		error(strbuff);					\
	}                                                       \
	if	( toupr )	mystrupr(val);			\
}
#define	SEPARATES		" ,\t\n\r"

/*
 *    Some data common for GM & LM.
 */
EXTERN byte     ncolors;
EXTERN coord    xmin, xmax, ymin, ymax;

#ifndef	NRMEM
#define	reportmem()	_reportmem()
#else
#define	reportmem()
#endif

#ifdef	BGIGRAPH
#undef	message
#define	message(s)
#endif
