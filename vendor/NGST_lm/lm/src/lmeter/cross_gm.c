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


/*      Procedure to add edge crosspoints to yx-tree                    */

#include"lmeter.h"
#include"head_gm.h"
#include"avltree.h"

extern dcnf   **aelgm;		/* array of pointers to 4-configs               */
extern coord    height;		/* distance between curr. and next y-lines      */
extern coord    curry;
extern tree     curryline;
extern tree     yx;
tree            newyx;

#define	ADDPNT(x,y)                  					\
	{       tree	xpnt;						\
		tree	*xtreeptr;					\
		xtreeptr=(tree*)&(_search((y),&newyx,&avlgrew)->info);	\
		xpnt=_search(x,xtreeptr,&avlgrew);			\
		if(xpnt->info==NULL)                           		\
			allocate(xpnt->info,4,sizeof(dcnf));   	 	\
	}
#define	EXIST(x,dir)	(aelgm[x][dir].tcnf || aelgm[x][dir].ecnf)

/* while calling in scroll_by_lines adds to yx-tree two edges   */
/* intersect points.                                            */
void
intersect_edges ()
{
    coord           x, x1, xstop;

    for (x = 0; x < xmax; x++)
	if (aelgm[x])
	{
	    if (EXIST (x, 2))
	    {
		xstop = x + height + 1;
		if (xstop > xmax)
		    xstop = xmax;
		for (x1 = x + 1; x1 < xstop; x1++)
		    if (aelgm[x1] && EXIST (x1, 3))
			ADDPNT (x, curry + x1 - x);
	    }
	    if (EXIST (x, 1))
	    {
		xstop = x + height + 1;
		if (xstop > xmax)
		    xstop = xmax;
		for (x1 = x + 1; x1 < xstop; x1++)
		    if (aelgm[x1] && EXIST (x1, 2))
			ADDPNT (x1, curry + x1 - x);
		xstop = x + 2 * height + 1;
		if (xstop > xmax)
		    xstop = xmax;
		for (x1 = x + 1; x1 < xstop; x1++)
		    if (aelgm[x1] && EXIST (x1, 3))
		    {
			assert ((x1 + x) % 2 == 0);
			assert ((x1 - x) % 2 == 0);
			ADDPNT ((x1 + x) / 2, curry + (x1 - x) / 2);
		    }
	    }
	}

}

/* while calling in scroll_by_lines adds to yx-tree             */
/* edge-with-line intersect points.                             */

void
intersect_edge_line ()
{
    coord           x;
    tree           *xtreeptr;

    /* add to curryline all aelgm items     */
    xtreeptr = (tree *) & (curryline->info);
    for (x = 0; x < xmax; x++)
	if (aelgm[x])
	{
	    tree            xpnt;

	    xpnt = _search (x, xtreeptr, &avlgrew);
	    if (xpnt->info == NULL)
		allocate (xpnt->info, 4, sizeof (dcnf));
	}

}

static coord    y;
void            add_xpnt (tree xpnt);

void
add_news (tree ytree)
{
    y = ytree->key;
    assert (ytree);
    apply_ascend (ytree->info, add_xpnt);
}

void
add_xpnt (tree xpnt)
{
    tree            xp;
    tree           *xtreeptr;

    assert (xpnt);
    xtreeptr = (tree *) & (search (y, yx)->info);
    xp = _search (xpnt->key, xtreeptr, &avlgrew);
    if (xp->info == NULL)
	allocate (xp->info, 4, sizeof (dcnf));
}
