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

/*\
 *	That is the module to build pattern - array,where described
 *	the direction and length to move to the neibourgh in all 8 sectors.
\*/

#include"lmeter.h"
#include"head_lm.h"

extern struct
{	enum	dtag	d_nd;	/* 1 if diagonal link, 0 if is not.	*/
	short l;
}	pattern[8];

extern	signed char dx[2][8];
extern	signed char dy[2][8];

build_pattern(coord	x)
{	enum		dtag	move45;
	byte		klookat,kpoint,kfinal,nsetted;
	unsigned	step;
	byte		s,s1;	/* slots */
	int		deltax,deltay;

	ENTERPROC(build_pattern)

	assert(x>=0&&x<xmax);

	for(s=0;s<8;s++)	pattern[s].l=0;

	assert(eqsource[0][x].isused);
	kpoint=eqsource[0][x].isused-1;	/* 1 was added while storing	*/
	assert(kpoint<=kmax);
/*	search neighbour at	:
 *	K-KPOINT:	-1 -1  0  0  1  1
 *	D_ND	:	ND  D ND  D ND  D
 */	klookat= (kpoint==0)	?0:kpoint-1;
	kfinal=	 (kpoint==kmax)	?kmax:kpoint+1;
	move45=ND;
	nsetted=0;
	step=POW2(klookat);
	while	( nsetted<8 && klookat<=kfinal )
	{	for ( s=move45; s<8; s+=2 )
		{	deltax=dx[move45][s]*step;
			deltay=dy[move45][s]*step;
			if	(	x+deltax >= xmax ||
					x+deltax <  0    ||
					deltay < to_back ||
					deltay > to_front	)	continue;
			if(!eqsource[deltay])	printf("%d %d %d\n",(int)deltay,(int)to_front,(int)to_back);
			assert(eqsource[deltay]);
			if ( eqsource[deltay][x+deltax].isused )
			{	s1=s;
				if ( pattern[s1].l==0 )
				{	pattern[s1].l=step;
					pattern[s1].d_nd=move45;
					nsetted++;
				}
				s1=(s)?s-1:7;
				if ( pattern[s1].l==0 )
				{	pattern[s1].l=step;
					pattern[s1].d_nd=move45;
					nsetted++;
				}
			}
		}
		if 	( move45==ND )	move45=DG; /* use the same step	*/
		else
		{	move45=ND;
			step=POW2(++klookat);
		}
	}

	LEAVEPROC(build_pattern)

} /* end of build_pat */
