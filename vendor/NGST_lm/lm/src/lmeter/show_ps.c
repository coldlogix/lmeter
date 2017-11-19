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
 *    This file implements LMeter's debugging graphics using PostScript
 */

#include"lmeter.h"

#ifdef	PSGRAPH

#include"head_lm.h"
#include"head_gm.h"
#include"avltree.h"
#include<math.h>
#include<time.h>

#define	PICTURE	"grid.ps"

/* Is it a presentation print-out or a (somehow useful) 
 * grid debugging print-out?
 */
#define DEBUGGRID 1

/* Do we really want PostScript printout?       */
static int      skipPSp = 0;

/*      Paper: 8.5"x11", 72 points/inch, landscape      */
#define PAPERX 612.0
#define PAPERY 828.0
#define	XOFFSET 50.0
#define	YOFFSET 50.0

/*
 *    We are are going to draw two pictures: input geometry 
 *      and decimated grid. These pictures and input header
 *      are saved in three different files (to be able to draw
 *      the grid under the input geometry.
 */
static FILE    *pshead, *psgrid, *psgeom;

/*
 *    PS file creation/closing
 */
show_init ()
{
    /* NB: this procedure should be called only after 
     * xmax, ymax are defined and xmin=0, ymin=0
     */
    time_t          now = time (NULL);
    float           scale;

    if (skipPSp = (getpar ("PSPRINT") == NULL))
	return;

    fileopen (pshead, PICTURE, "w");
    psgrid = tmpfile ();
    psgeom = tmpfile ();

    /*
     *    PostScript file header and scaling info 
     */
    fprintf (pshead, "%s\n", "%!PS-Adobe-3.0 EPSF-3.0");
    fprintf (pshead, "%s\n", "%%Creator: LMeter");
    fprintf (pshead, "%s %s\n", "%%Title:", PICTURE);
    fprintf (pshead, "%s %s", "%%CreationDate:", ctime (&now));
    fprintf (pshead, "%s %g %g %g %g\n", "%%BoundingBox:",
	     0.0, 0.0,
	     PAPERX, PAPERY);
    fprintf (pshead, "%g %g translate\n", XOFFSET, YOFFSET);
    scale = MIN ((PAPERX - 2 * XOFFSET) / xmax / sqrt (2), (PAPERY - 2 * YOFFSET) / ymax / sqrt (2));
    fprintf (psgrid, "0 setlinewidth\n");
    fprintf (pshead, "%g %g scale\n", scale, scale);
#if DEBUGGRID==0
    fprintf (pshead, "%g 0 translate\n", ymax / sqrt (2));
    fprintf (pshead, "1 0.5 scale 45 rotate\n");
#endif
    fprintf (pshead, "2 setlinecap\n");

    /*
     *    PostScript procedures
     */
    fprintf (pshead, "/m  {newpath moveto}  			def\n");
    fprintf (pshead, "/rm {rmoveto} 				def\n");
    fprintf (pshead, "/rl {rlineto} 				def\n");
    fprintf (pshead, "/gs {gsave} 				def\n");
    fprintf (pshead, "/gr {grestore} 				def\n");
    fprintf (pshead, "/s  {stroke} 				def\n");
    fprintf (pshead, "/lr {2 copy rlineto neg exch neg exch rmoveto}	def\n");
    fprintf (pshead, "/t  {setlinewidth} 			def\n");
    fprintf (pshead, "/np {newpath}  			        def\n");
    fprintf (pshead, "/cp {closepath} 				def\n");

    /*
     *    Available colors:
     *      1       Red
     *      2       Green
     *      3       Blue
     *      4       Cyan
     *      5       Magenta
     *      6       Yellow
     *      7       Violet
     *      8       Brown
     *      9       Dark Red
     *      10      Dark Green
     *      11      Dark Blue
     *      12      Dark Cyan
     *      13      Dark Magenta
     *      14      Dark Yellow
     *      15      Dark Violet
     *      16      Black
     */

    fprintf (pshead, "\
/c0  {0 0 0 		setrgbcolor} def\n \
/c1  {1 0 0 		setrgbcolor} def\n \
/c2  {0 1 0 		setrgbcolor} def\n \
/c3  {0 0 1 		setrgbcolor} def\n \
/c4  {0 1 1 		setrgbcolor} def\n \
/c5  {1 0 1 		setrgbcolor} def\n \
/c6  {1 1 0 		setrgbcolor} def\n \
/c7  {0.5 0 1 		setrgbcolor} def\n \
/c8  {0.6 0.2 0.2 	setrgbcolor} def\n \
/c9  {0.5 0 0 		setrgbcolor} def\n \
/c10 {0 0.5 0 		setrgbcolor} def\n \
/c11 {0 0 0.5 		setrgbcolor} def\n \
/c12 {0 0.5 0.5 	setrgbcolor} def\n \
/c13 {0.5 0 0.5 	setrgbcolor} def\n \
/c14 {0.5 0.5 0 	setrgbcolor} def\n \
/c15 {0.25 0 0.5 	setrgbcolor} def\n \
/c16 {0 0 0 		setrgbcolor} def\n \
");

    fprintf (psgrid, "c16\n");

}				/* end of show_init */

show_end ()
{
    char            buffer[1024];

    if (skipPSp)
	return;
    rewind (psgrid);
    rewind (psgeom);
    while (fgets (buffer, 1024, psgrid))
	fputs (buffer, pshead);
    while (fgets (buffer, 1024, psgeom))
	fputs (buffer, pshead);
    fclose (psgrid);
    fclose (psgeom);
    fprintf (pshead, "showpage\n");
    fclose (pshead);
}				/* end of show_end */

/*
 *    Procedures to draw the input geometry.
 */

int             laycolors[NLAYMAX];
float           layelev[NLAYMAX];
float           laythk[NLAYMAX];

extern dcnf   **aelgm;		/* array of pointers to 4-configs               */
extern coord    height;		/* distance between curr. and next y-lines      */
extern coord    curry;		/* these externs are from ael_gm.c              */

/*
 *    Some macros to write PostScript operators
 */
#define	MOVETO(x,y) 	fprintf(psgeom,"%g %g m\n",(float)x,(float)y)
#define	RMOVETO(x,y) 	fprintf(psgeom," %g %g rm\n",(float)x,(float)y)
#define	RLINETO(x,y) 	fprintf(psgeom," %g %g rl\n",(float)x,(float)y)
#define	GS()		fprintf(psgeom,"gs\n")
#define	GR()		fprintf(psgeom,"gr\n")
#define	STROKE()	fprintf(psgeom," s\n")

#define	TWOLINES(x,y)						\
        {							\
		if(currentcolor!=l)				\
		{						\
			currentcolor=l;				\
			fprintf(psgeom,"c%d ",laycolors[l]);	\
			fprintf(psgeom,"%f t\n",laythk[l]);	\
		}						\
		GS();						\
		RMOVETO(layelev[l],layelev[l]);			\
		RLINETO((x),(y));				\
/*		RLINETO(laythk[l],laythk[l]);			\
		RLINETO(-(x),-(y));				\
		RLINETO(-laythk[l],-laythk[l]);			\
*/		STROKE();					\
		GR();						\
	}

void 
show_aelgm ()
{
    coord           x, prevx = 0;
    int             horedge[NLAYMAX], termedge = 0;
    byte            d, l;
    int             currentcolor = 0;

    if (skipPSp)
	return;

#if DEBUGGRID
    return;
#endif
    for (l = 0; l < ncolors; l++)
    {
	horedge[l] = 0;
	laythk[l] = 0.9;
	layelev[l] = l;
    }

    GS ();

    for (x = 0; x < xmax; x++)
	if (aelgm[x])
	{
	    MOVETO (x, curry);
	    for (l = 0; l < ncolors; l++)
	    {
		if (horedge[l])
		    TWOLINES (prevx - x, 0);
		if (aelgm[x][0].ecnf & (POW2 (l)))
		    horedge[l] = !horedge[l];
		if (aelgm[x][1].ecnf & (POW2 (l)))
		    TWOLINES (height, height);
		if (aelgm[x][2].ecnf & (POW2 (l)))
		    TWOLINES (0, height);
		if (aelgm[x][3].ecnf & (POW2 (l)))
		    TWOLINES (-height, height);
	    }

/*                      MOVETO(x,curry); */
/*                      if(termedge)    LINEREL(prevx-x,0); */
/*                      for(d=0;d<4;d++) */
/*                              if(aelgm[x][d].tcnf) */
/*                              {        */
/*                                      switch(d) */
/*                                      {        */
/*                                      case 0: termedge=!termedge; */
/*                                              break; */
/*                                      case 1: LINEREL(height,height); */
/*                                              break; */
/*                                      case 2: LINEREL(0,height); */
/*                                              break; */
/*                                      case 3: LINEREL(-height,height); */
/*                                              break; */
/*                                      } */
/*                              } */

	    prevx = x;
	}
    GR ();
}

/*
 *    Procedures to draw the decimated grid.
 */

/* these externs are from eqs_lm.c              */
extern signed char dx[2][8];
extern signed char dy[2][8];
extern          usedline;
struct patlink
{
    enum dtag       d_nd;	/* 1 if diagonal link, 0 if is not.     */
    short           l;
};

#undef	MOVETO
#undef	STROKE
#define	MOVETO(x,y) 	fprintf(psgrid,"%g %g m\n",(float)x,(float)y)
#define	STROKE()	fprintf(psgrid," s\n")
#define	LINEREL(x,y)	fprintf(psgrid," %g %g lr\n",(float)x,(float)y)
#define	CIRCLE(x,y,r,c)	fprintf(psgrid," np %g %g %g 0 360 arc cp c%d fill c0 \n",(float)x,(float)y,(float)r, (int)c)

#define OFFSET 0.25

show_pattern (coord x, struct patlink *pattern, int rank)
{
    int             s, l;
    int             gx = x, gy = usedline;
    enum dtag       thisd, otherd;
    float           len;

    if (skipPSp)
	return;

    /*      Draw grid pattern       */
    MOVETO (gx, gy);
    for (s = 0; s < 8; s++)
    {
	thisd = pattern[s].d_nd;
	otherd = (thisd+1)%2;
	/*
	 * Do not ask me why (and how) this works, but it does. Maybe an extra but of
	 * effort would achieve the desired result with one simple formula.
	 */
	len = 0.5*pattern[s].l;
	if (len > 0.0)
	  if (thisd == 0)
	    LINEREL (+(dx[thisd][s] != 0 ?  dx[thisd][s] : OFFSET*dx[otherd][s]) * len , 
		     -(dy[thisd][s] != 0 ?  dy[thisd][s] : OFFSET*dy[otherd][s]) * len);
	  else
	    LINEREL (+(dx[thisd][s] + OFFSET*dx[otherd][s])/sqrt(1+OFFSET) * len , 
		     -(dy[thisd][s] + OFFSET*dy[otherd][s])/sqrt(1+OFFSET) * len);
    }
    STROKE ();
    /* Draw a filled circle around node */
    CIRCLE (gx, gy, 0.25, rank);
}				/*end of show_pattern */

#endif
