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


/* That module implements updating of Active Edge List                  */

#include"lmeter.h"
#include"head_lm.h"

/* change_ael accepts new y-line in ael                                 */

#define	GETX_AEL	(ael.edges[iael].c0+ael.edges[iael].dx)
#define	GETX_YEL	(yelptr->edges[iyel].x)
#define	GETDIR_AEL	(ael.edges[iael].dir)
#define	GETDIR_YEL	(yelptr->edges[iyel].dir)

edgelist        oldael;		/* for filling bottom (dirs 5,6,7) part of rcf  */
edgelist        newael;		/* ael with hor. edges,they are deleted later.  */

void change_ael ()
{
    edgelist       *yelptr;
    sbyte           inew, iyel, iael;
    sbyte           nremoved;	/*      <- just for debugging   */

    oldael = ael;		/* save current ael     */

    printf("Debugging ael 1\n");
    dump_ael ();

    yelptr = &(yline.el);
    newael.len = 0;
    if (yelptr->len == 0 && ael.len == 0)
	return;
    allocate (newael.edges, yelptr->len + ael.len, sizeof (edgedata));

    assert (test_ael_sort ());

    printf("Debugging ael 2: %d\n", ael.len);
    dump_ael ();
    nremoved = inew = iyel = iael = 0;
    while (iyel < yelptr->len && iael < ael.len)
    {
    printf ("DD2: %d %d %d %d\n", iyel,  yelptr->len, iael, ael.len);
	if (GETX_AEL < GETX_YEL)
	    newael.edges[inew++] = ael.edges[iael++];
	else if (GETX_YEL < GETX_AEL)
	    newael.edges[inew++] = yelptr->edges[iyel++];
	else if (GETDIR_AEL > GETDIR_YEL)	/* have equal x's       */
	    newael.edges[inew++] = ael.edges[iael++];
	else if (GETDIR_YEL > GETDIR_AEL)
	    newael.edges[inew++] = yelptr->edges[iyel++];
	else
	    /* here both have equal xs and dirs       */
	{
	    assert (GETX_YEL == GETX_AEL && GETDIR_YEL == GETDIR_AEL);
	    newael.edges[inew] = ael.edges[iael++];
	    nremoved++;
	    newael.edges[inew].ecnf ^= yelptr->edges[iyel].ecnf;
	    newael.edges[inew].tcnf ^= yelptr->edges[iyel].tcnf;
	    if (newael.edges[inew].ecnf || newael.edges[inew].tcnf)
		inew++;		/* still exist  */
	    else
		nremoved++;
	    iyel++;
	}
    }

    printf("Debugging ael 3... inew:%d\n", inew);
    dump_ael ();
    printf("the 4: %d %d %d %d\n", iyel , yelptr->len, iael , ael.len);

    /* only one 'while' will be entered     */
    while (iyel < yelptr->len)
	newael.edges[inew++] = yelptr->edges[iyel++];
    while (iael < ael.len)
	newael.edges[inew++] = ael.edges[iael++];

    printf("change_ael: inew=%u nremoved=%u\n",(unsigned)inew,(unsigned)nremoved);
    printf("change_ael: iael=%u iyel=%u\n",(unsigned)iael,(unsigned)iyel);

    assert (inew + nremoved == iael + iyel);
    /* oldael is freed in fill_rcf  */
    ael.len = newael.len = inew;
    ael.edges = newael.edges;
    assert (test_ael_sort ());

}				/*  end of change_ael */

remove_hor ()			/* copy newael to ael deleting hor. edges       */
{
    int             inew = newael.len;
    int             iael, e;

    if (inew)
    {
	allocate (ael.edges, inew, sizeof (edgedata));
    }
    else
	ael.edges = NULL;

    iael = 0;
    for (e = 0; e < inew; e++)
	if (newael.edges[e].dir != 0)
	    ael.edges[iael++] = newael.edges[e];

/*      printf("remove_hor: inew=%u iael=%u\n",(unsigned)inew,(unsigned)iael);
 */
    ael.len = iael;
    Free (newael.edges);
    newael.len = 0;
}

/* shift_ael implements changing of X's in ael                           */

shift_ael ()
{
    byte            e;

    for (e = 0; e < ael.len; e++)
    {				/*note:before changing compare with next,after-with prev.       */
	assert (e == ael.len - 1 || ael.edges[e].c0 + ael.edges[e].dx
		<= ael.edges[e + 1].c0 + ael.edges[e + 1].dx);
	ael.edges[e].dx += 2 - ael.edges[e].dir;
	assert (e == 0 || ael.edges[e].c0 + ael.edges[e].dx
		>= ael.edges[e - 1].c0 + ael.edges[e - 1].dx);
    }

}				/*    end of shift_ael */

resort_ael ()			/* by dir in all x-points       */
{
    int             e, i, i1, maxpos, maxd, n;
    coord           x;
    edgedata       *xeds[4], tmped;
    int             need = 0;

    e = 0;
    while (e < ael.len)
    {
	xeds[0] = ael.edges + e;
	x = xeds[0]->c0 + xeds[0]->dx;
	e++;
	n = 0;
	while (e < ael.len && (ael.edges[e].c0 + ael.edges[e].dx == x))
	    xeds[++n] = ael.edges + (e++);
	assert (n < 4);
	if (n == 0)
	    continue;
	/* now xeds[] points to up to 4 edges with same x       */
	/* sort them by dir in order 3210.                      */
	for (i = 0; i < n; i++)
	    if (xeds[i]->dir < xeds[i + 1]->dir)	/* need sort    */
	    {
		need = 1;
		break;
	    }
	if (need)
	{
	    for (i = 0; i < n; i++)
	    {			/* in [i+1 .. n]'th elems. find max dir,        */
		/* exchange it with i'th.                       */
		maxd = xeds[i]->dir;
		maxpos = i;
		for (i1 = i + 1; i1 <= n; i1++)
		    if (xeds[i1]->dir > maxd)
		    {
			maxd = xeds[i1]->dir;
			maxpos = i1;
		    }
		if (maxpos != i)	/* exchange     */
		{
		    tmped = *xeds[i];
		    *xeds[i] = *xeds[maxpos];
		    *xeds[maxpos] = tmped;
		}
	    }
	    need = 0;
	}
	for (i = 0; i < n; i++)
	    assert (xeds[i]->dir > xeds[i + 1]->dir);
    }
    assert (test_ael_sort ());

}				/*    end of resort_ael */

#ifndef	NDEBUG
test_ael_sort ()
{
    int             e;

    for (e = 1; e < ael.len; e++)
	if (ael.edges[e].c0 + ael.edges[e].dx
	    < ael.edges[e - 1].c0 + ael.edges[e - 1].dx ||
	    ael.edges[e].c0 + ael.edges[e].dx
	    == ael.edges[e - 1].c0 + ael.edges[e - 1].dx &&
	    ael.edges[e].dir > ael.edges[e - 1].dir)
	{
	    printf ("unsort edge=%-4d\n", e);
	    dump_ael ();
	    return 0;
	}
    return 1;
}

dump_ael ()
{
    int             e;
    edgedata        ed;

    printf ("\n");
    for (e = 0; e < ael.len; e++)
    {
	ed = ael.edges[e];
	printf ("e=%-4d x=%-4d c0=%-4d dx=%-4d c0+dx=%-4d dir=%-4d ecnf=%-8u tcnf=%-8u\n",
		e, ed.x, ed.c0, ed.dx, ed.c0 + ed.dx, (int) ed.dir, (unsigned) ed.ecnf, (unsigned) ed.tcnf);
    }
}
#endif
