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

#include<stdio.h>
#include<stdlib.h>
#include"misc.h"
#include"list.h"

typedef	double	coord;

typedef	struct	point
{	coord	x,y;
}	point;

point	*newpnt	( coord x, coord y );

typedef	coord	**TMType;

typedef	struct	geomprim
{	enum	primtype
	{	PLINE,
		CALL,
		UEXT
	}	type;
	union
	{	struct	plinepar
		{	char	*layname;
			node	*pntlist;	/*	List of 'point'		*/
		}	pline;
		struct	callpar
		{	int	symname;
			TMType	callTM;
		}	call;
		struct	uextpar
		{	int	no;	
			char	*text;
		}	uext;
	}       par;
}       geomprim;

geomprim	*path2pline	( node *path );
geomprim	*box2pline	( coord L, coord W, coord Cx, coord Cy, coord Dx, coord Dy );
geomprim	*rflash2pline	( coord R, coord Cx, coord Cy );
geomprim	*wire2pline	( coord W, node *path );
geomprim	*call2call	( int symname );
geomprim	*uext2uext	( int no, char *text );

EXTERN	TMType	CTM;

int	newCTM	( void );
int	translate	( coord a,coord b );
int	mirror_x	( void );
int	mirror_y	( void );
int	rotate		( coord a,coord b );
int	applyTM		( TMType TM );
int	transpnt	( point *pnt1, point *pnt );

#define	CALLDEPTH	16
int	walkroot	( int (*f)(geomprim *) );

EXTERN	char	dummylayname[]	INIT("ZZZZ");
EXTERN	char	*layname 	INIT(dummylayname);
EXTERN	int	symname;

EXTERN	node	*root		INIT(NULL);
EXTERN	node	*defs		INIT(NULL);

EXTERN	char	strbuff[160];

#define	USEREXTLEN	1024

EXTERN	int	userextno;
EXTERN	char	userexttext[USEREXTLEN];
	
int	printpntlist	(node *path);
int	printprimlist	(node *primlist);
int	printdeflist	(node *defs);



