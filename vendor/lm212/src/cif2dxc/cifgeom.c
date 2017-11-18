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
 *    Some "geometrical" procedures to work with cif objects.
 */
#define	MAIN

#include"cifgeom.h"
#include<math.h>

/*
 *    Function:       newpnt
 *      Definition:     Create a new 'point' with specifyed 'x' & 'y'.
 */
point          *
newpnt (coord x, coord y)
{
    point          *newp;

    allocateN (newp, 1);
    newp->x = x;
    newp->y = y;
    return newp;
}				/*      newpnt  */

/*
 *    Function:       path2pline
 *      Definition:     convert 'path' to 'pline'.
 */
geomprim       *
path2pline (node * path)
{
    geomprim       *newpline;

    allocateN (newpline, 1);
    newpline->type = PLINE;
    newpline->par.pline.layname = layname;
    newpline->par.pline.pntlist = cons (last (path), path);
    return newpline;
}				/*      path2pline      */

/*
 *    Function:       box2pline
 *      Definition:     Convert CIF box to the closed pline.
 */
geomprim       *
box2pline (coord L, coord W, coord Cx, coord Cy, coord Dx, coord Dy)
{
    geomprim       *newpline;
    point          *p1, *p2, *p3, *p4, *p5;
    coord           d = sqrt (Dx * Dx + Dy * Dy);

#define	Px(x0,y0)	Cx+((x0)*Dx-(y0)*Dy)/d/2
#define	Py(x0,y0)	Cy+((x0)*Dy+(y0)*Dx)/d/2
    p1 = newpnt (Px (-L, -W), Py (-L, -W));
    p2 = newpnt (Px (+L, -W), Py (+L, -W));
    p3 = newpnt (Px (+L, +W), Py (+L, +W));
    p4 = newpnt (Px (-L, +W), Py (-L, +W));
    p5 = newpnt (Px (-L, -W), Py (-L, -W));
#undef	Px
#undef	Py
    allocateN (newpline, 1);
    newpline->type = PLINE;
    newpline->par.pline.layname = layname;
    newpline->par.pline.pntlist = cons (p1, cons (p2, cons (p3, cons (p4, cons (p5, NULL)))));
    return newpline;
}				/*      box2pline       */

/*
 *    Function:       rflash2pline
 *      Definition:     Convert round flash to closed polyline
 *      (now to a square).
 */
geomprim       *
rflash2pline (coord D, coord Cx, coord Cy)
{
    geomprim       *newpline;
    point          *p1, *p2, *p3, *p4, *p5;
    coord           R = D / 2;

    p1 = newpnt (Cx - R, Cy - R);
    p2 = newpnt (Cx + R, Cy - R);
    p3 = newpnt (Cx + R, Cy + R);
    p4 = newpnt (Cx - R, Cy + R);
    p5 = newpnt (Cx - R, Cy - R);
    allocateN (newpline, 1);
    newpline->type = PLINE;
    newpline->par.pline.layname = layname;
    newpline->par.pline.pntlist = cons (p1, cons (p2, cons (p3, cons (p4, cons (p5, NULL)))));
    return newpline;
}				/*      rflash2pline    */

/* 
 * Find intersection of two lines (used in wire2pline)
 * Thanks to Mr. Wolfram and Mathematica for the expessions! ;-) 
 * Returns 0 if lines are parallel.
 */
static
int
intersect (coord * px, coord * py,	/* results */
	   coord ax0, coord ay0, coord ax1, coord ay1,	/* line A */
	   coord bx0, coord by0, coord bx1, coord by1 /* line B */ )
{
    coord           denom = ay1 * (bx0 - bx1) + ay0 * (-bx0 + bx1) + (ax0 - ax1) * (by0 - by1);

    if (denom == 0.0)
	return 0;		/* parallel */

    *px = (ax0 * (ay1 * (bx0 - bx1) + bx1 * by0 - bx0 * by1)
	   + ax1 * (ay0 * (-bx0 + bx1) - bx1 * by0 + bx0 * by1)) / denom;

    *py = (ax0 * ay1 * (by0 - by1) + ax1 * ay0 * (-by0 + by1)
	   + (ay0 - ay1) * (bx1 * by0 - bx0 * by1)) / denom;

    return 1;
}

/*
 *    Function:       wire2pline
 *      Definition:     Wires with zero width W are converted 
 *                      to open plines (used as terminals), 
 *                      Wires with non-zero width W are converted
 *                      to closed polylines.
 */
geomprim       *
wire2pline (coord W, node * path)
{
    geomprim       *newpline;

    allocateN (newpline, 1);
    newpline->type = PLINE;
    newpline->par.pline.layname = layname;

    W /= 2.0;			/* work with half-width */
    if (W == 0.0)
	newpline->par.pline.pntlist = path;
    else
	/* fun starts! */
    {
	node            first =
	{NULL, NULL};		/* root of the list */
	node           *cursor = &first, *next;
	node           *i;

	/* Three points on path: previous, current, next
	 */
	point          *p0 = NULL, *p1 = NULL, *p2 = NULL;

	/* each point along the path adds TWO points to
	 * the pline (up and down).
	 * There are also previous/current/next pairs
	 */
	point           up0, down0, up1a, down1a, up1, down1, up2, down2;
	point          *pup, *pdown;	/* actual points inserted */

	for (i = path; i->cdr; i = i->cdr)	/* all but last */
	{
	    /* Work with next segment p1-p2 */
	    /* segment lengths */
	    coord           d, dx, dy;

	    /* tang/normal unit vectors along the segment */
	    point           t, n;

	    /* shift points */
	    p0 = p1;
	    p1 = i->car;
	    p2 = ((node *) (i->cdr))->car;
	    up0 = up1;
	    down0 = down1;
	    up1a = up2;
	    down1a = down2;

	    dx = p2->x - p1->x;
	    dy = p2->y - p1->y;
	    d = sqrt (dx * dx + dy * dy);
	    t.x = dx / d;
	    t.y = dy / d;
	    n.x = -t.y;
	    n.y = t.x;
	    up1.x = p1->x + W * (-t.x + n.x);
	    up1.y = p1->y + W * (-t.y + n.y);
	    up2.x = p2->x + W * (+t.x + n.x);
	    up2.y = p2->y + W * (+t.y + n.y);
	    down1.x = p1->x + W * (-t.x - n.x);
	    down1.y = p1->y + W * (-t.y - n.y);
	    down2.x = p2->x + W * (+t.x - n.x);
	    down2.y = p2->y + W * (+t.y - n.y);

	    /* For all but the first point we need to update
	     * up1 to be intersection of lines 
	     * up0-up1a and up1-up2, same with down1
	     */
	    if (p0)
	    {
		coord           upx, upy, downx, downy;

		if (!intersect (&upx, &upy,
				up0.x, up0.y,
				up1a.x, up1a.y,
				up1.x, up1.y,
				up2.x, up2.y)
		    ||
		    !intersect (&downx, &downy,
				down0.x, down0.y,
				down1a.x, down1a.y,
				down1.x, down1.y,
				down2.x, down2.y))
		    /* lines are parallel, do nothing */
		    continue;
		up1.x = upx;
		up1.y = upy;
		down1.x = downx;
		down1.y = downy;
	    }

	    /* insert current up/down points in list.
	     * Next points will be inserted after up1
	     * but before down1.
	     */
	    pup = newpnt (up1.x, up1.y);
	    pdown = newpnt (down1.x, down1.y);
	    next = cons (pup, cons (pdown, cursor->cdr));
	    cursor->cdr = next;
	    cursor = next;
	}

	/* insert last pair of points (up2/down2) */
	pup = newpnt (up2.x, up2.y);
	pdown = newpnt (down2.x, down2.y);
	next = cons (pup, cons (pdown, cursor->cdr));
	cursor->cdr = next;

	/* and store new pntlist 
	 * (with duplicated last point to make pline closed) 
	 */
	newpline->par.pline.pntlist = cons (last (first.cdr),
					    first.cdr);
    }
    return newpline;
}				/*      wire2pline      */

/*
 *    Function:       call2call
 *      Definition:     Return a newly created 'call' primitive.
 */
geomprim       *
call2call (int symname)
{
    geomprim       *newcall;

    allocateN (newcall, 1);
    newcall->type = CALL;
    newcall->par.call.symname = symname;
    newcall->par.call.callTM = CTM;
    return newcall;
}				/*      call2call       */
/*
 *    Function:       uext2uext
 *      Definition:     Store a uext string.
 */
geomprim       *
uext2uext (int no, char *text)
{
    geomprim       *newuext;

    allocateN (newuext, 1);
    newuext->type = UEXT;
    newuext->par.uext.no = no;
    allocateN (newuext->par.uext.text, strlen (text) + 1);
    strcpy (newuext->par.uext.text, text);
    return newuext;
}				/*      uext2uext       */

/*
 *    Functions to work with Transformation Matrixes.
 */
/*
 *    Function:       newCTM
 *      Definition:     CTM=    1       0       0
 *                              0       1       0
 *                              0       0       1
 */
newCTM (void)
{
    allocateN (CTM, 3);
    allocateN (CTM[0], 9);	/*      Just to save some time & core;  */
    CTM[1] = CTM[0] + 3;	/*      see also 'memcpy's below.       */
    CTM[2] = CTM[1] + 3;
    CTM[0][0] = 1;
    CTM[0][1] = 0;
    CTM[0][2] = 0;
    CTM[1][0] = 0;
    CTM[1][1] = 1;
    CTM[1][2] = 0;
    CTM[2][0] = 0;
    CTM[2][1] = 0;
    CTM[2][2] = 1;
    return 0;
}				/*      newCTM  */

/*
 *    Function:       dupCTM
 *      Definition:     Create a new CTM, copy the values from the old one.
 */
dupCTM (void)
{
    TMType          TM = CTM;

    allocateN (CTM, 3);
    allocateN (CTM[0], 9);	/*      Just to save some time & core;  */
    CTM[1] = CTM[0] + 3;	/*      see also 'memcpy's below.       */
    CTM[2] = CTM[1] + 3;
    memcpy (CTM[0], TM[0], 9 * sizeof (coord));
    return 0;
}				/*      dupCTM  */

/*
 *    Function:       delCTM
 *      Definition:     Free the CTM.
 */
delCTM (void)
{
    free (CTM[0]);
    free (CTM);
    return 0;
}				/*      delCTM  */

/*
 *    Function:       translate
 *      Definition:     TM=     1       0       0
 *                              0       1       0
 *                              a       b       1
 */
translate (coord a, coord b)
{
    CTM[0][0] += a * CTM[0][2];
    CTM[0][1] += b * CTM[0][2];
    CTM[1][0] += a * CTM[1][2];
    CTM[1][1] += b * CTM[1][2];
    CTM[2][0] += a * CTM[2][2];
    CTM[2][1] += b * CTM[2][2];
    return 0;
}				/*      translate       */

/*
 *    Function:       mirror_x
 *      Definition:     TM=     -1      0       0
 *                              0       1       0
 *                              0       0       1
 */
mirror_x (void)
{
    CTM[0][0] *= -1;
    CTM[1][0] *= -1;
    CTM[2][0] *= -1;
    return 0;
}				/*      mirror_x        */

/*
 *    Function:       mirror_y
 *      Definition:     TM=     1       0       0
 *                              0       -1      0
 *                              0       0       1
 */
mirror_y (void)
{
    CTM[0][1] *= -1;
    CTM[1][1] *= -1;
    CTM[2][1] *= -1;
    return 0;
}				/*      mirror_y        */

/*
 *    Function:       rotate
 *      Definition:     TM=     a/d     b/d     0
 *                              -b/d    a/d     0
 *                              0       0       1
 */
rotate (coord a, coord b)
{
    coord           d = sqrt (a * a + b * b);
    coord           TmpTM[3][3];

    TmpTM[0][0] = (a * CTM[0][0] - b * CTM[0][1]) / d;
    TmpTM[1][0] = (a * CTM[1][0] - b * CTM[1][1]) / d;
    TmpTM[2][0] = (a * CTM[2][0] - b * CTM[2][1]) / d;
    TmpTM[0][1] = (a * CTM[0][1] + b * CTM[0][0]) / d;
    TmpTM[1][1] = (a * CTM[1][1] + b * CTM[1][0]) / d;
    TmpTM[2][1] = (a * CTM[2][1] + b * CTM[2][0]) / d;
    TmpTM[0][2] = CTM[0][2];
    TmpTM[1][2] = CTM[1][2];
    TmpTM[2][2] = CTM[2][2];
    memcpy (CTM[0], TmpTM, 9 * sizeof (coord));
    return 0;
}				/*      rotate  */

/*
 *    Function:       applyTM
 *      Definition:     CTM=TM*CTM
 */
applyTM (TMType TM)
{
    coord           TmpTM[3][3];
    int             i, j, k;

    for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	{
	    coord           val = 0;

	    for (k = 0; k < 3; k++)
		val += TM[i][k] * CTM[k][j];
	    TmpTM[i][j] = val;
	}			/*      for     */
    memcpy (CTM[0], TmpTM, 9 * sizeof (coord));
    return 0;
}				/*      applyTM */

/*
 *    Function:       transpnt
 *      Definition:     Apply the CTM to a point 'pnt', save a result
 *      in 'pnt1'.
 */
transpnt (point * pnt1, point * pnt)
{
    coord           x = pnt->x;
    coord           y = pnt->y;

    /*      Apply CTM:
     *    (xt,yt,1)=(x,y,1)*CTM,
     */
    pnt1->x = x * CTM[0][0] + y * CTM[1][0] + CTM[2][0];
    pnt1->y = x * CTM[0][1] + y * CTM[1][1] + CTM[2][1];
    return 0;
}				/*      transpnt        */

/*
 *    Function:       walkroot
 *      Definition:     Calls function 'f' on all plines in 'root' and
 *      symbols called from it. Function 'f' MUST use 'transpnt' to apply
 *      CTM to all points on those plines.
 *      Saved UEXTS are also processed.
 */
static int      (*savedf) (geomprim *);
static int      walk (node * root);
int
walkroot (int   (*f) (geomprim *))
{
    savedf = f;
    newCTM ();
    walk (root);
    delCTM ();
    return 0;
}				/*      walkroot        */

/*
 *    Function:       walk
 *      Definition:     Performs symbol calling and gives the resulted
 *      plines to 'savedf'.
 */
static TMType   TMStack[CALLDEPTH];
static int      TMSP = 0;	/*      Stack pointer.  */
static node    *def;
static
walk (node * root)
{
    foreach
	(primnode, root,
	 {
	     geomprim * prim = primnode->car;
	     switch (prim->type)
	     {
	     case PLINE:
	     case UEXT:
		 savedf (prim);
		 break;
	     case CALL:
		 def = assoc ((void *) prim->par.call.symname, defs);
		 if (def == NULL)
		 {
		     sprintf (strbuff, "Undefined symbol \"%d\"",
			      prim->par.call.symname);
		     error (strbuff);
		 }			/*      then    */
		 else
		 {			/*      Push CTM.       */
		     TMStack[TMSP++] = CTM;
		     if (TMSP == CALLDEPTH)
			 error ("Calls are nested too deeply");
		     dupCTM ();
		     applyTM (prim->par.call.callTM);
		     walk (def->cdr);
		     /*      Pop CTM.        */
		     delCTM ();
		     CTM = TMStack[--TMSP];
		 }			/*      else    */
		 break;
	     default:
		 error ("Internal error: Bad 'geomprim' type");
	     }			/*      switch  */
	 }
	    ) ;
    return 0;
}				/*      walk    */

/*
 *    Functions to perform some debugging printing ...
 */
static int      shift = 0;
static char     sline[10];

/*
 *    Function:       printpntlist
 *      Definition:     Print out the point list
 */
printpntlist (node * path)
{
    fprintf (stderr, "%sPointlist:\n", sline);
    sline[shift++] = '\t';
    foreach
	(nd, path,
	 {
	 fprintf (stderr, "%spoint: %f %f\n", sline,
		  ((point *) (nd->car))->x,
		  ((point *) (nd->car))->y
	 );
	 }
    );
    sline[--shift] = '\0';
    fprintf (stderr, "%sAll\n", sline);
    return 0;
}				/*      printpntlist    */

/*
 *      Function:     printrimlist
 *      Definition:
 */
printprimlist (node * primlist)
{
    foreach
	(tmp, primlist,
	 {
	 geomprim * prim = tmp->car;
	 switch (prim->type)
	 {
case PLINE:
	 fprintf (stderr, "%sAt Layer:%s\n",
		  sline, prim->par.pline.layname);
	 sline[shift++] = '\t';
	 printpntlist (prim->par.pline.pntlist);
	 sline[--shift] = '\0';
	 break;
case CALL:
	 fprintf (stderr, "%sCalling:%d\n",
		  sline, prim->par.call.symname);
	 printTM (prim->par.call.callTM);
	 break;
case UEXT:
	 fprintf (stderr, "%sUEXT:%d %s\n",
		  sline, prim->par.uext.no,
		  prim->par.uext.text);
	 break;
default:
	 error ("Internal error: Bad 'geomprim' type");
	 }			/*      switch  */
	 }
    ) ;
    return 0;
}				/*      printprimlist   */

/*
 *    Function:       printdeflist
 *      Definition:     Print the assoc list of block definitions.
 */
printdeflist (node * defs)
{
    foreach
	(tmp, defs,
	 {
	 fprintf (stderr, "%sSymbol:%d\n ",
		  sline, (int) ((node *) (tmp->car))->car);
	 sline[shift++] = '\t';
	 printprimlist (((node *) (tmp->car))->cdr);
	 sline[--shift] = '\0';
	 fprintf (stderr, "%sAll\n", sline);
	 }
    );
    return 0;
}				/*      printdeflist    */

/*
 *    Function:       printTM
 *      Definition:
 */
printTM (TMType TM)
{
    fprintf (stderr, "%s\t%f\t%f\t%f\n", sline, TM[0][0], TM[0][1], TM[0][2]);
    fprintf (stderr, "%s\t%f\t%f\t%f\n", sline, TM[1][0], TM[1][1], TM[1][2]);
    fprintf (stderr, "%s\t%f\t%f\t%f\n", sline, TM[2][0], TM[2][1], TM[2][2]);
    return 0;
}				/*      printTM */
