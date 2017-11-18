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
 *    Main file of the 'cif2ps' program.
 \*/

#include"cifgeom.h"
#include<assert.h>

#include<string.h>

FILE           *ps;
FILE           *term;

/*
 *    Table to store terminals' config info.
 *      NB: Terminals are counted starting from 1 (for historical reasons)
 */
#define	NAMLEN	32
#define	MAXNTRM	(8*sizeof(unsigned))
struct
{
    char            name[NAMLEN];
    char            uplay[NAMLEN];
    char            dnlay[NAMLEN];
    char            ttype;
}
termTab[MAXNTRM];
int             ntrm = 0;

#define	TERMINALS	"input.lm"
#define	TERMNAMES	"terminal.tab"

/*
 *    Function:       psout
 *      Definition:     Draw one pline in the output ps file.
 */
psout (geomprim * prim)
{
    point           pnt1;
    static int      itrm = 0;
    char           *op = "moveto";

    switch (prim->type)
    {
    case PLINE:
	foreach
	    (pnt, prim->par.pline.pntlist,
	     {
	     transpnt (&pnt1, pnt->car);
	     fprintf (ps, "\t%f\t%f %s\n",
		      (double) pnt1.x,
		      (double) pnt1.y,
		      op);
	     op = "lineto";
	     }
	);
	op = "moveto";
	fprintf (ps, "stroke\n");
	break;
    case UEXT:
	break;
    }
    return 0;
}				/*      psout   */

double          maxX = -1e6;
double          maxY = -1e6;
double          minX = 1e6;
double          minY = 1e6;

findBB (geomprim * prim)
{
    point           pnt1;

    switch (prim->type)
    {
    case PLINE:
	foreach
	    (pnt, prim->par.pline.pntlist,
	     {
	     transpnt (&pnt1, pnt->car);
	     if (pnt1.x > maxX) maxX = pnt1.x;
	     if (pnt1.y > maxY) maxY = pnt1.y;
	     if (pnt1.x < minX) minX = pnt1.x;
	     if (pnt1.y < minY) minY = pnt1.y;
	     }
	) ;
    }
    return 0;
}				/*      findBB  */

main (int argc, char **argv)
{
    extern FILE    *yyin;	/*      from cif.l      */
    int             t;

    if (argc > 3)
    {
	fprintf (stderr, "Call: %s [cifFilename [psFilename]]\n", argv[0]);
	return 1;
    }

    /*
     *    Read input file to internal data structures
     */
    if (argc > 1)
	fileopen (yyin, argv[1], "r");
    else
	yyin = stdin;
    yyparse ();
    fclose (yyin);

    walkroot (findBB);

    /*
     *    Write output file
     */
    if (argc > 2)
	fileopen (ps, argv[2], "w");
    else
	ps = stdout;
    fprintf (ps, "%f %f translate %f %f scale %f %f translate\n",
	     0.25 * 72, 0.25 * 72,
	     (8.0 * 72) / (maxX - minX),
	     (10.5 * 72) / (maxY - minY),
	     -minX, -minY);
    walkroot (psout);
    fprintf (ps, "showpage\n");
    fclose (ps);

    return 0;
}
int
userprocess ()
{				/*      fprintf(stderr,"User ext:%d %s\n",userextno,userexttext);       */
    switch (userextno)
    {
    case 9:
	fprintf (stderr, "Processing \"%s\" ...\n", userexttext);
	break;
    }

    return 0;
}
