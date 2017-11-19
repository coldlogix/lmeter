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
 * This module implements all input for LM as well as interface between the 
 * geometrical part (GM) and LM itself.
 */

#include"lmeter.h"
#include"head_lm.h"
#include"head_gm.h"
#include"avltree.h"
#include<string.h>

#undef	USE_TERM0
#undef  DEBUG_TOPFILE

static int      nylines;	/* total nmb. of y lines        */
static int      iyline = 0;	/* nmb. of current y line       */

cnftype         space_cnf;	/* cnf of free space ( TERM 0 ) */

initparams_lm ()
{
    char           *buf, *b;
    char           *deftermup, *deftermdown;
    int             i;
    const double    pi = 3.14159265, pf = 2.067835;

    /* read misc. variables */
    GETPAR (buf, "KMAX", 1);
    kmax = atoi (buf);
    GETPAR (buf, "KMIN", 1);
    kmin = atoi (buf);
    GETPAR (buf, "CURRUNIT", 1);
    currunit = atof (buf);
    currunit = (currunit == 0) ? 4 * pi / 10 : 8 * pi * pi / 10 / pf * currunit;
    currunit = 2.0 * currunit;	/* 9-point Laplasian ???        */
    swapdir = getpar ("SWAPDIR");
    GETPAR (buf, "BULKLAMBDA", 1);
    bulklambda = atof (buf);
    rewind (topf);
    if ((fscanf (topf, "%d %d %d", &xmax, &ymax, &nylines)) != 3)
	error ("can't read TOP file ");
    iyline = 0;

    /* read linecnf */
    allocate (linecnf, ncolors, sizeof (*linecnf));
    for (i = 0; i < ncolors; i++)
    {
	char            name[50];
	cnftype         bit, cnf;
	byte            c;

	sprintf (name, "LAYCONF[%s]", mystrupr (laynames[i]));
	GETPAR (buf, name, 1);
	bit = 1;
	cnf = 0;
	c = 0;
	while (buf[c])
	{
	    switch (buf[c])
	    {
	    case '1':
		cnf |= bit;
	    case '0':
		bit <<= 1;
		break;
	    case '+':
		cnf |= bit;
	    case '-':
		bit <<= 1;
		break;
	    }
	    c++;
	}
	linecnf[i] = cnf;
    }
    /* _dump_linecnf(stderr); */
#ifdef	USE_TERM0
    space_cnf = 0;
#else
    /* 
     * The first in the list layer (used for TERMinals) also marks
     * free-space config. We do not use explicit name ("TERM") anymore.
     * space_cnf = linecnf[layname_ilay ("TERM")];
     */
    space_cnf = linecnf[0];
#endif

    /* read tech. par.      */
    GETPAR (buf, "NFILMS", 1);
    nlay = atoi (buf);
    if (nlay <= 0 || nlay > NLAYMAX)
	error ("Bad NLAY");
    allocate (layinfo, nlay + 1, sizeof (layerpar));
    GETPAR (buf, "FILMTYPE", 1);
    strtok (buf, SEPARATES);	/* skip layer 0 */
    metalcnf = 0;
    isocnf = 0;
    for (i = 1; i <= nlay; i++)
    {
	if ((b = strtok (NULL, SEPARATES)) == NULL)
	    error ("can't read LTYPE");
	switch (b[0])
	{
	case 'S':
	case 's':
	    layinfo[i].type = MET;
	    metalcnf |= (POW2 (i));
	    break;
	case 'I':
	case 'i':
	    layinfo[i].type = ISO;
	    isocnf |= (POW2 (i));
	    break;
	case 'N':
	case 'n':
	    layinfo[i].type = NIL;
	    break;
	default:
	    error ("LTYPE should be S,I,N");
	}
    }
    GETPAR (buf, "FILMPAR", 1);
    strtok (buf, SEPARATES);	/* skip layer 0 */
    for (i = 1; i <= nlay; i++)
    {
	if ((b = strtok (NULL, SEPARATES)) == NULL)
	    error ("can't read LRPAR");
	layinfo[i].repar = atof (b);
    }
    GETPAR (buf, "FILMMASK", 1);
    strtok (buf, SEPARATES);	/* skip layer 0 */
    for (i = 1; i <= nlay; i++)
    {
	if ((b = strtok (NULL, SEPARATES)) == NULL)
	    error ("can't read LMASK");
	layinfo[i].mask = atoi (b);
    }
    GETPAR (buf, "UNITSIZE", 1);
    unitsize = atof (buf) * gridsize;
#ifndef	NOINDCORR
    GETPAR (buf, "COEFF3D0", 1);
    Coeff3D0 = atof (buf);
    GETPAR (buf, "COEFF3D1", 1);
    Coeff3D1 = atof (buf);
    GETPAR (buf, "COEFF3D2", 1);
    Coeff3D2 = atof (buf);
    GETPAR (buf, "FILMSHIFT", 1);
    strtok (buf, SEPARATES);	/* skip layer 0 */
    sprintf (strbuff, "Using 3D coefficients: %lf %lf %lf",
	     (double) Coeff3D0, (double) Coeff3D1, (double) Coeff3D2);
    message (strbuff);
    for (i = 1; i <= nlay; i++)
    {
	float           shift;

	if ((b = strtok (NULL, SEPARATES)) == NULL)
	    error ("can't read SHIFT");
	/*      LM knows only how to use metal shift.   */
	if (layinfo[i].type == MET)
	{
	    shift = atof (b);
	    if (shift > unitsize / 2)
	    {
		warning ("Too large shift, truncated");
		shift = unitsize / 2;
	    }			/*      then    */
	    layinfo[i].shift = shift;
	}			/*      then    */
    }
#else
    Coeff3D0 = Coeff3D1 = Coeff3D2 = 0.0;
    for (i = 1; i <= nlay; i++)
	layinfo[i].shift = 0;
#endif
    /* _dump_layerpar(stderr); */
    /* read term. par. */
    GETPAR (buf, "NTERM", 1);
    ntrm = atoi (buf);
    if (ntrm <= 0)
	error ("Bad NTERM");
    allocate (terminfo, ntrm, sizeof (termpar));
    GETPAR (buf, "TERMTYPE", 1);
    for (i = 0; i < ntrm; i++)
    {
	if ((b = strtok (buf, SEPARATES)) == NULL)
	    error ("can't read TTYPE");
	buf = NULL;		/* to read from the same buf    */
	switch (b[0])
	{
	case 'S':
	case 's':
	    terminfo[i].type = SHT;
	    break;
	case 'I':
	case 'i':
	    terminfo[i].type = ISR;
	    break;
	case 'N':
	case 'n':
	    terminfo[i].type = DUM;
	    break;
	default:
	    error ("TERMTYPE should be S,I,N");
	}
    }

    buf = getpar("TERMNAME");
    if (buf == NULL)
    {
	warning("LMeter: No TERMNAME specified, unused");
    }
    else
	for (i = 0; i < ntrm; i++)
	{
	    if ((b = strtok (buf, SEPARATES)) == NULL)
		    error("LMeter: not enough TERMNAMEs");
	    buf = NULL;		/* to read from the same buf    */
	    terminfo[i].name = b;
	}

    ntea = 0;
    for (i = 0; i < ntrm; i++)
	if (terminfo[i].type == ISR)
	    terminfo[i].itea = ++ntea;
    nout = ntea;
    for (i = 0; i < ntrm; i++)
	if (terminfo[i].type == SHT)
	    terminfo[i].itea = ++ntea;
    allocateN (iteaitrm, ntea + 1);
    for (i = 0; i < ntrm; i++)
	iteaitrm[terminfo[i].itea] = i + 1;
    GETPAR (deftermup, "DEFTERMUP", 1);
    deftermup = strtok (deftermup, SEPARATES);
    GETPAR (deftermdown, "DEFTERMDOWN", 1);
    deftermdown = strtok (deftermdown, SEPARATES);
    GETPAR (buf, "TERMUP", 1);
    for (i = 0; i < ntrm; i++)
    {
	if ((b = strtok ((i == 0) ? buf : NULL, SEPARATES)) == NULL)
	    error ("Can't read TERMUP");
	terminfo[i].up = gettermlay ((b[0] == '*') ? deftermup : b, i);
    }
    GETPAR (buf, "TERMDOWN", 1);
    for (i = 0; i < ntrm; i++)
    {
	if ((b = strtok ((i == 0) ? buf : NULL, SEPARATES)) == NULL)
	    error ("Can't read TERMDOWN");
	terminfo[i].down = gettermlay ((b[0] == '*') ? deftermdown : b, i);
    }
    /* Fix fllipped-over terminals */
    for (i = 0; i < ntrm; i++)
    {
	int up = terminfo[i].up;
	int dn = terminfo[i].down;
	if (dn == up)
	{
	    sprintf(strbuff,
		    "LMeter: Terminal %s: both electrodes on the same layer",
		    terminfo[i].name);	    
	    error (strbuff);
	}
	if (dn > up)
	{
	    sprintf(strbuff,
		    "LMeter: Terminal %s: wrong electrodes order, fixed",
		    terminfo[i].name);
	    warning(strbuff);
	    terminfo[i].up = dn;
	    terminfo[i].down = up;
	}
    }    
}

gettermlay (char *name, int i)	/* return laynmb. for ACAD layname      */
{
    int             ilay = 0, l1, l2;
    cnftype         cnf;
    char           *err = NULL;

    while (laynames[ilay] && strcmp (laynames[ilay], name) != 0)
	ilay++;
    if (laynames[ilay] == NULL)
	err = "Terminal %s refers to nonexisting layer \"%s\"";
    cnf = linecnf[ilay];
    for (l1 = NLAYMAX; l1 > 0; l1--)
	if (cnf & (POW2 (l1)))
	    break;
    for (l2 = 1; l2 <= NLAYMAX; l2++)
	if (cnf & (POW2 (l2)))
	    break;
    if (l1 != l2)
	err = "Terminal %s refers to layer \"%s\" with two metal films";
    if (layinfo[l1].type != MET)
	err = "Terminal %s refers to layer \"%s\" with no metal films";
    if (err)
    {
	sprintf (strbuff, err, terminfo[i + 1].name, name);
	/* fprintf(stderr,"layers: %s %d %d\n",name, l1,l2);
	 * write_bits(stderr, POW2(l1));
	 * write_bits(stderr, POW2(l2));
	 * write_bits(stderr,cnf);
	 * write_bits(stderr,linecnf[ilay]);
	 */
	error (strbuff);
    }
    return l1;
}

/*
 *    Decode the ACAD config (colors) to LM config (existing layers)
 */
cnftype 
ecnf_decode (cnftype colorcnf)
{
    byte            c;
    cnftype         ecnf = 0;

    for (c = 0; c <= ncolors; c++)
	if (colorcnf & (POW2 (c)))
	    ecnf ^= linecnf[c];
    return ecnf;
}

/*
 * Misc functions to read/write variables of cnftype. Of course on a GNU
 * system (e.g., Linux) we would use just "%Lu", but we should not
 * assume that we have GNU IO lib. Historically top. file is not binary,
 * so these tricks are required...
 */

static int 
incnf (FILE * f, cnftype * cp)
{
#ifdef LONGCNF
    long           *ap = (long *) cp;
    long           *bp = ((long *) cp) + 1;

    return fscanf (f, " %lu %lu ", ap, bp);
#else
    return fscanf (f, " %lu ", cp);
#endif
}

static void 
outcnf (FILE * f, cnftype c)
{
#ifdef LONGCNF
    long           *ap = (long *) &c;
    long           *bp = ((long *) &c) + 1;

    fprintf (f, " %lu %lu ", *ap, *bp);
#else
    fprintf (f, " %lu ", c);
#endif
}

/*
 *    Read one y-line to internal structures.
 */
void readline ()
{
    coord           cy, x;
    int             ne, nx, ix, nex;
    byte            e, elast;
    edgelist       *elsp;	/* array of edgelists for x points */

    iyline++;
    if (fscanf (topf, "%d %d", &cy, &nx) != 2)
	error ("can't read yline header ");
    printf("Deugging readline: %d %d\n", cy, nx);
    yline.y = cy;
    yline.isnotlast = (iyline < nylines);
    if (nx == 0)
    {
	Free (yline.el.edges);
	yline.el.edges = NULL;
	yline.el.len = 0;
	return;
    }
    allocate (elsp, nx, sizeof (edgelist));

    ne = 0;
    for (ix = 0; ix < nx; ix++)
    {
	if (fscanf (topf, "%d %d", &x, &nex) != 2)
	    error ("can't read xrec  header ");
    printf("Deugging readline2: %d %d\n", x, nex);
	ne += nex;
	elsp[ix].len = nex;
	if (nex)
	    allocate (elsp[ix].edges, nex, sizeof (edgedata));
	for (e = 0; e < nex; e++)
	{
	    unsigned        dir;
	    cnftype         colorcnf, tcnf;

	    if (fscanf (topf, "%d", &dir) != 1
		|| incnf (topf, &colorcnf) == 0
		|| incnf (topf, &tcnf) == 0)
		error ("can't read edge info");
	    if (dir > 7)
		error ("bad direction");
	    elsp[ix].edges[e].x = x;
	    elsp[ix].edges[e].dx = ((int) 2 - dir) * cy;
	    elsp[ix].edges[e].c0 = x - elsp[ix].edges[e].dx;
	    elsp[ix].edges[e].dir = (byte) dir;
	    elsp[ix].edges[e].ecnf = ecnf_decode (colorcnf);
	    elsp[ix].edges[e].tcnf = tcnf;
	}
    }
    Free (yline.el.edges);
    yline.el.len = ne;
    if (ne > 0)
    {
	allocate (yline.el.edges, ne, sizeof (edgedata));
    }
    else
	yline.el.edges = NULL;

    elast = 0;
    for (ix = 0; ix < nx; ix++)
    {
	for (e = 0; e < elsp[ix].len; e++)
	    yline.el.edges[elast + e] = elsp[ix].edges[e];
	elast += elsp[ix].len;
	if (elsp[ix].edges)
	    Free (elsp[ix].edges);
    }
    Free (elsp);

#ifndef	BGIGRAPH
#if 1
    {
	extern double   getrealy (coord y);
	float           y = getrealy (cy);

	sprintf (strbuff, 
		 "LM: Y-line (%9.2lf) accepted: %3d'th of %3d.", 
		 y, iyline, nylines);
	message (strbuff);
    }
#endif
#endif

}				/*    end of readline */

/* 
 * Maybe you will find these usefull ;-)
 */
_dump_linecnf (FILE * f)
{
    int             i;

    fprintf (f, "LINECNF ACCEPTED:\n");
    for (i = 0; i < ncolors; i++)
	write_bits (f, linecnf[i]);
}
_dump_layerpar (FILE * f)
{
    int             l;

    fprintf (f, "LAYINFO ACCEPTED:\n");
    fprintf (f, "MASK:    TYPE:    REPAR:\n");
    for (l = 0; l <= nlay; l++)
    {
	fprintf (f, "%8d %8d %8f\n",
	      (int) layinfo[l].mask, (int) layinfo[l].type, layinfo[l].repar);
    }
}

/*      
 *    Module to write yx-tree to a file for LM
 */

extern linlist *llyx;

write_lm ()
{
    int             i, j;

#ifdef  DEBUG_TOPFILE
    topf = fopen ("TOPFILE", "w+");
#else
    topf = tmpfile ();
#endif
    if (topf == NULL)
	error ("can not open temporary file");
    fprintf (topf, "%d %d\n%d\n", xmax, ymax, llyx->len);

    for (i = 0; i < llyx->len; i++)
    {
	lnode           yline;

	yline = llyx->data[i];
	fprintf (topf, "\t%d\t%d\n", yline.key, ((linlist *) yline.info)->len);
	for (j = 0; j < ((linlist *) yline.info)->len; j++)
	{
	    lnode           xpnt;
	    dcnf           *info;
	    int             d, count = 0;

	    xpnt = ((linlist *) yline.info)->data[j];
	    info = xpnt.info;

	    for (d = 0; d < 4; d++)
		if (info[d].ecnf || info[d].tcnf)
		    count++;
	    fprintf (topf, "\t\t%d\t%d\n", xpnt.key, count);
	    /* also write count==0 (edge cross points)      */
	    /*      need dir. order 3210                    */
	    if (count)
		for (d = 3; d >= 0; d--)
		    if (info[d].ecnf || info[d].tcnf)
		    {
			fprintf (topf, "\t\t\t%d", d);
			outcnf (topf, info[d].ecnf);
			outcnf (topf, info[d].tcnf);
			fprintf (topf, "\n");
		    }
	}
    }
    fflush (topf);
    /*      
     *    It is tmpfile() now --- shouldn't close()       
     *      fclose(topf);
     */

}
