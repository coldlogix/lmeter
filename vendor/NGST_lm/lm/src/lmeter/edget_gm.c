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
 *    Procedures to read top. info.
 *      Now use DXC file.
 */

#include"lmeter.h"
#include"head_gm.h"
#include<ctype.h>
#include<string.h>
#include <math.h>

#define	BUFFLEN	1024
char            buff[BUFFLEN];
char            ch;

/* those externs are from main_gm.c     */
extern FILE    *infile;
extern float    gridsize;
extern byte     doublestep;

extern int      currlay, currite;
extern xypoint  a, b;

/* Return ilay assoc with CAD layer name */
int
layname_ilay (char *name)
{
    int             ilay = 0;

    while (laynames[ilay])
    {
	if (strcmp (laynames[ilay], name) == 0)
	    return ilay;
	ilay++;
    }
    sprintf (strbuff, "No such layer, ignored: %s", name);
    warning(strbuff);
    return -1;
}

#define	skipblank()	while(isspace(ch))	ch=fgetc(infile);
/* always pre-read ch before skipblank() ! */

start_edgeget ()
{

    ch = fgetc (infile);
    skipblank ();

}

more_edge ()
{

    if (ch == EOF) return 0;

    if (ch == '$')		/* new keyword,prob. polyline   */
    {
	getkword (buff);
	if (strcmp (buff, "$POLYLINE") == 0)	/* new polyline */
	{
	    getilay ();
	    if (currlay == -1 ) /* unknown layer, skip to the next keyword */
	    {
		while (ch != '$' && ch != EOF) ch=fgetc(infile);
		return more_edge(); /* and try again */
	    }
	    getiterm ();
	    getpoint (&b);

	    return 1;
	}
	if (strcmp (buff, "$EOF") == 0)
	    return 0;
	if (strcmp (buff, "$ENTITIES") == 0)
	    return more_edge ();
	fprintf (stderr, "Unrecognized word '%s'\n", buff);
	return 0;
    }
    else
    {
	/* more edges exist in curr. pline        */
	return (1);
    }
}

get_edge ()
{

    a.x = b.x;
    a.y = b.y;
    getpoint (&b);

}

getpoint (xypoint * pnt)
{
    float           x, y;

    if (ungetc (ch, infile) == EOF)
	error ("Can't unget !?");
    if (fscanf (infile, " %f %f ", &x, &y) != 2)
	error ("Can't read point");
    if (fmod (x, gridsize) != 0.0 || fmod (y, gridsize) != 0.0)
	warning ("Coordinate is not multiple of step, rounding");
    x = x / gridsize;
    y = y / gridsize;
    /* 
     * We make conversion to int coord here                         
     */
    pnt->x = floor(x+0.5);
    pnt->y = floor(y+0.5);
    ch = fgetc (infile);
    skipblank ();

}

getilay ()
{
    int             i = 0;

    while (isalnum (ch) && i < BUFFLEN)
    {
	buff[i++] = toupper (ch);
	ch = fgetc (infile);
    }
    skipblank ();
    if (i == BUFFLEN)
	error ("Too long layname");
    buff[i] = '\0';
    currlay = layname_ilay (buff);
    if (currlay > (int)NLAYMAX)
	error ("Too many layers");

}
getiterm ()
{
    int             ite;

    ungetc (ch, infile);
    if (fscanf (infile, "%d", &ite) != 1)
	error ("Bad teminal number");
    ch = fgetc (infile);
    skipblank ();
    currite = ite;
    if (currite > NTRMMAX)
	error ("Terminal number too large");

}

getkword (char *buff)
{
    int             i = 0;

    if (ch != '$')
	error ("Keyword must begin from '$'");
    while (isgraph (ch) && i < BUFFLEN)
    {
	buff[i++] = ch;
	ch = fgetc (infile);
    }
    skipblank ();
    if (i == BUFFLEN)
	error ("Too long keyword");
    buff[i] = '\0';

}
