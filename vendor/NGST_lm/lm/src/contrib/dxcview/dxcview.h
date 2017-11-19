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

#include"list.h"

#define	MAXPOINTNUM	300
EXTERN	struct	point	{	float	x,y;	}	*points;
EXTERN	int	pointnum;

#define	NAMELEN		20
#define	MAXNLAYS	20

EXTERN	char	*layname;
EXTERN	float	thickness;

struct	pline
{	char	*layname;
	struct	point	*points;
	int	pointnum;
	float	thickness;
};

EXTERN	node*	plinelist;

EXTERN	float	zoom,shiftx,shifty;
EXTERN 	float	xmin,xmax,ymin,ymax;