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


/* That is the module for creating and updating Active Edge List,       */
/* while scrolling yx-tree.                                             */

#include"lmeter.h"
#include"head_gm.h"
#include"avltree.h"

dcnf          **aelgm, **newaelgm;	/* array of pointers to 4-configs       */
coord           height;		/* distance between curr. and next y-lines      */
coord           curry;

extern tree     yx;
tree            curryline, nextyline;
void            (*act) ();

void            shift_line (tree yline);
void            accept_point (tree xpnt);
void            shift_aelgm ();

/*      Call action() after modifying aelgm on each yline               */

void
scroll_by_lines (void action ())
{

    allocateN (aelgm, xmax);
    allocateN (newaelgm, xmax);
    act = action;
    curryline = nextyline = NULL;
    apply_ascend (yx, shift_line);
    height = 0;
    curry = nextyline->key;
    apply_ascend (nextyline->info, accept_point);
    /* modify aelgm with last yline */
    (*act) ();
    curryline = nextyline = NULL;
    Free (aelgm);
    Free (newaelgm);
    aelgm = newaelgm = NULL;

}

/* Accept yline to aelgm                                                        */

void
shift_line (tree yline)
{

    assert (yline);
    if (nextyline == NULL)	/* called for the first time    */
	nextyline = yline;
    else
    {
	curryline = nextyline;
	nextyline = yline;
	height = nextyline->key - curryline->key;
	curry = curryline->key;
	apply_ascend (curryline->info, accept_point);
	/* modify aelgm with curryline  */
	(*act) ();
	shift_aelgm ();
    }

}

void
accept_point (tree xpnt)	/* add xpnt's edges to aelgm    */
{
    coord           x;
    dcnf           *pcnfs, *ecnfs;	/* cnfs[4] for yx-point,aelgm-item */
    byte            dir, empty;

    assert (xpnt);
    assert (xpnt->key < xmax);
    assert (aelgm);
    x = xpnt->key;
    pcnfs = xpnt->info;
    if (aelgm[x] == NULL)
	allocate (aelgm[x], 4, sizeof (dcnf));
    ecnfs = aelgm[x];
    empty = 1;
    for (dir = 0; dir < 4; dir++)
    {
	ecnfs[dir].tcnf ^= pcnfs[dir].tcnf;
	ecnfs[dir].ecnf ^= pcnfs[dir].ecnf;
	if (ecnfs[dir].tcnf || ecnfs[dir].ecnf)
	    empty = 0;		/* exist at least one edge/term */
    }
    if (empty)
    {
	assert (aelgm[x]);
	Free (aelgm[x]);
	aelgm[x] = NULL;
    }

}

/*      Change aelgm between two ylines                                 */

void
shift_aelgm ()
{
    coord           x, newx;
    byte            dir;
    dcnf          **oldaelgm;
    int             dx[4] =
    {0, 1, 0, -1};

    for (x = 0; x < xmax; x++)
	if (aelgm[x])
	{
	    for (dir = 1; dir < 4; dir++)
		if (aelgm[x][dir].ecnf || aelgm[x][dir].tcnf)
		{
		    newx = x + dx[dir] * height;
		    assert (newx < xmax);
		    if (newaelgm[newx] == NULL)
			allocate (newaelgm[newx], 4, sizeof (dcnf));
		    newaelgm[newx][dir].ecnf ^= aelgm[x][dir].ecnf;
		    newaelgm[newx][dir].tcnf ^= aelgm[x][dir].tcnf;
		}
	    Free (aelgm[x]);
	    aelgm[x] = NULL;
	}
    oldaelgm = aelgm;
    aelgm = newaelgm;
    newaelgm = oldaelgm;

}
