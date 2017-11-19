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
 *    This is to mark dots of the non-regular grid. Each layout line
 *      emits the dot waves with decreasing density. Waves from different
 *      sources and their density decreasing are syncronised using the
 *      x and y coordinate value parity (i.e. the max k: x % POW2(k)==0 ).
 */

#include"lmeter.h"
#include"head_lm.h"

#undef	NO_SUBBND_DEC		/* undef this to decimate subboundary points      */

#define PSYNC(x)	do { if	( (((x) + step)%(2*step)) ) 	x +=step; } while (0)
#define PSYNC45(x)	do { if	( !(((x) + step)%(2*step)) )	x +=step; } while (0)
#define NSYNC(x)	do { if	( (((x) - step)%(2*step)) ) 	x -=step; } while (0)
#define NSYNC45(x)	do { if	( !(((x) - step)%(2*step)) )	x -=step; } while (0)

#define KSET( x , knew )						\
	do {	byte	k1;						\
		assert(x>=0&&x<xmax);	assert(knew<=kmax);		\
		if(k1=yptr[0][x].isused) 				\
			{ 	assert(k1<=kmax+1);			\
				yptr[0][x].isused=min(knew+1,k1);	\
			}						\
		else	yptr[0][x].isused=knew+1;			\
	} while (0)

static line_dots (coord yr, coord xl, coord xr, coord dx, byte kmin, int is45);

/*  compute the parity of coordinate y  */
static inline int parity (coord y)
{
  byte ky = 0;
  
  while ((ky < kmax) && (y % POW2 (ky) == 0))
    ky++;
  return ky-1;
}

/*
 *    Emit dots in x-direction from the points from ael
 *      by calling line_dots for each edge crossing current y-line.
 */
dotter_1d (edgelist el)		/*      el==ael */
{
    byte            e, step, k1;
    point          *p, *pmax;
    coord           x;
    edgedata       *ep;

    step = POW2 (kmax);
    if (y % step == 0)		/* fill regular grid with kmax  */
	for (x = 0; x < xmax; x += step)
	    if (k1 = wavesource[0][x].isused)
		wavesource[0][x].isused = min (k1, kmax + 1);
	    else
		wavesource[0][x].isused = kmax + 1;
    if (kmax == 0)
	return;
    for (e = 0; e < el.len; e++)
    {
	ep = el.edges + e;
	line_dots (0, ep->c0, ep->c0, ep->dx, 0, ep->dir != 2);
    }

}				/*    end of dotter_1d */

/*
 *    Emit dots in x and y directions from points and hor. edges on yline
 *      by calling line_dots for different ys. Those ys are chosen by the
 *      algorithm similar to one used in line_dots.
 */
dotter_2d (edgelist el)		/*      el==yline.el    */
{
    coord           xl, xr, xl1, xr1;
    byte            e;
    cnftype         ecnf, tcnf;
    coord           dy, dy1, d;
    unsigned        step, k, dymax;

    if (kmax == 0)
	return;

    for (e = 0; e < el.len; e++)
    {
	switch (el.edges[e].dir)
	{
	case 1:
	case 2:
	case 3:		/*  wave up     */
	    /*                      case 5: case 6: case 7: *//*  wave down  */
	    xr = xl = el.edges[e].x;
	    break;
	case 4:
	    error ("no left point for horizontal edge");
	case 0:
	    xl = el.edges[e].x;
	    /*  need not work at points on hor. edge */
	    /*  skip them , but find right point    */
	    ecnf = el.edges[e].ecnf;
	    tcnf = el.edges[e].tcnf;
	    while ((ecnf || tcnf) && (++e < el.len))
	      if (el.edges[e].dir == 0)
		{
		    ecnf ^= el.edges[e].ecnf;
		    tcnf ^= el.edges[e].tcnf;
		}
	    if (e == el.len)
		error ("no right point for horizontal edge ");
	    xr = el.edges[e].x;
	    break;
	default:
	    error ("Internal error");
	}
	dymax = 3 * POW2 (kmax);
	/*  wave up  */
	xl1 = xl;
	xr1 = xr;
	for (k = dy = 0; (k < kmax) && (dy <= dymax); k++)
	{
	    step = POW2 (k);
	    dy1 = dy;
#ifdef	NO_SUBBND_DEC
	    dy += step;
#endif
	    if ((y + dy + step) % (2 * step))
		dy += step;
	    for (d = dy1; d <= dy; d += step)
		line_dots (d, xl1, xr1, 0, k, 0);
#ifdef	NO_SUBBND_DEC
	    xl1 -= step;
	    xr1 += step;
#endif
	    NSYNC (xl1);
	    xl1 -= step;
	    PSYNC (xr1);
	    xr1 += step;
	    if (xl1 < 0)
		xl1 = 0;
	    if (xr1 > xmax - 1)
	    {
		xr1 = xmax - 1;
		xr1 -= xr1 % (2 * step);
	    }
	    dy += step;
	}			/* for  k */
	if ((dy > dymax) && (k <= kmax))
	    error ("interference area violated\n");
	/*  wave down   */
	xl1 = xl;
	xr1 = xr;
	for (k = dy = 0; (k < kmax) && (-dy <= dymax); k++)
	{
	    step = POW2 (k);
	    dy1 = dy;
#ifdef	NO_SUBBND_DEC
	    dy -= step;
#endif
	    if ((y + dy - step) % (2 * step))
		dy -= step;
	    for (d = dy1; d >= dy; d -= step)
		line_dots (d, xl1, xr1, 0, k, 0);
#ifdef	NO_SUBBND_DEC
	    xl1 -= step;
	    xr1 += step;
#endif
	    NSYNC (xl1);
	    xl1 -= step;
	    PSYNC (xr1);
	    xr1 += step;
	    if (xl1 < 0)
		xl1 = 0;
	    if (xr1 > xmax - 1)
	    {
		xr1 = xmax - 1;
		xr1 -= xr1 % (2 * step);
	    }
	    dy -= step;
	}			/* for  k */
	if ((-dy > dymax) && (k <= kmax))
	    error ("interference area violated\n");
    }

}				/*    end of dotter_2d */

/*
 *    Emit the dot waves in x direction - the inner procedure of the Dotter.
 */
static
line_dots (coord yr, coord xl, coord xr, coord dx, byte kmin, int is45)
{
    area_1d        *yptr = wavesource - yr;
    unsigned        step = POW2 (kmin);
    coord           xl1, xr1, x;
    byte            ky, k;

    assert ((xl + dx) < xmax && (xr + dx) < xmax);
    assert (kmin <= kmax);

    /*  fill between xl and xr, assume they have such parity        */
    assert (!((xl + dx) % step || (xr + dx) % step));
    for (x = xl + dx; x <= xr + dx; x += step)
	KSET (x, kmin);

    ky = parity(y+yr);

    /*  left wave  */
    for (k = kmin; (k <= ky) && (xl + dx >= 0); k++)
    {
	step = POW2 (k);
	xl1 = xl;
#ifdef	NO_SUBBND_DEC
	xl -= step;
#endif
	if (is45)
	    NSYNC45 (xl);
	else
	    NSYNC (xl);
	for (x = xl1 + dx; x >= max (xl + dx, 0); x -= step)
	  KSET (x, k);
	if (is45)
	    xl -= 2 * step;
	else
	    xl -= step;
    }				/* for  k */

    /*  right wave  */
    for (k = kmin; (k <= ky) && (xr + dx < xmax); k++)
    {
	step = POW2 (k);
	xr1 = xr;
#ifdef	NO_SUBBND_DEC
	xr += step;
#endif
	if (is45)
	    PSYNC45 (xr);
	else
	  PSYNC (xr);
	for (x = xr1 + dx; x <= min (xr + dx, xmax - 1); x += step)
	  KSET (x, k);
	if (is45)
	    xr += 2 * step;
	else
	    xr += step;
    }				/* for  k */

}				/*    end of line_dots */
