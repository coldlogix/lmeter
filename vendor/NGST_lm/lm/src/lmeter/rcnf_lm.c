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
 *      Here are procedures to work with cnf. indexes of whole lines.
 *      fill_rcf fills incnf for all points in line
 */

#include"lmeter.h"
#include"head_lm.h"

#define POINTTERM	10	/* how many terms can cross in one point */
#define	CNFARRSTEP	1024

/*
 *    cnflevel==3 immediately after accepting new y-line, the complete
 *      config generation is needed;
 *
 *      cnflevel==2 when generate points on line next after y-line,
 *      generate new configs using the information from the old;
 *
 *      cnflevel==1 when generate points on line two lines after y-line;
 *      it means that the config generation is needed only for in points
 *      inside 45-sectors ( |/ & \| ), the other can use keys from the
 *      previous row;
 *
 *      else cnflevel==0, no new configs generated, but the keys from the
 *      previous row are used.
 */
byte            cnflevel;

#define SETKEY(x,key)	{assert(x>=0&&x<xmax);	assert(key<cnfarrlen);	\
			wavesource[0][x].incnf=key;	oneuses(key);   }
int             d45fl, ylfl;

extern edgelist oldael;		/* from ael_lm  */

int             acc_ter (cnftype cnf, int dir);

fill_rcf ()
{
    assert (cnflevel <= 3);
/*      printf("ael.len=%u oldael.len=%u\n",(unsigned)ael.len,(unsigned)oldael.len);
 */ switch (cnflevel)
    {
    case 3:			/* accept edges after yline     */
	ylfl = 1;
	accept_two_el ();
	Free (oldael.edges);
	oldael.len = 0;
	break;
    case 2:			/* accept edges from ael        */
	ylfl = 0;
	accept_edges ();
	break;
    case 1:			/* use last line keys except |/ */
	d45fl = 1;
	ylfl = 0;
	use_keys ();
	break;
    case 0:			/* use last line keys           */
	d45fl = 0;
	ylfl = 0;
	use_keys ();
	break;
    }
    if (cnflevel)
	cnflevel--;
    dump_rcfkey ();
}

/*      some global vars to store (accumulate) current config info      */
cnftype         cnfs[8], hortercnf;
slottrm         trmlst[POINTTERM];
byte            nte;

extern cnftype  space_cnf;	/* to init cnfs.        */

cnfkey          new_cnf ();

/*      Accept_edges generates new configs for all points on line,      */
/*      uses ael as source of information on edges in that line.        */
/*      in yline each edge comes from point, in ael - through point.    */

#define 	GET_X(e)	( ael.edges[e].c0 + ael.edges[e].dx)

accept_edges ()
{
    byte            e, s;
    coord           x, x1, xr;
    cnfkey          key;

    /*  clear cnf info      */
    for (s = 0; s < 8; cnfs[s++] = space_cnf) ;
    nte = 0;
    /*  scan all edges      */
    e = 0;
    while (e < ael.len)
    {
	nte = 0;		/* new point - new trmlst       */
	x = GET_X (e);
	while (e < ael.len && x == GET_X (e))
	    acc_e (ael.edges[e++]);
	assert (e == ael.len || x < GET_X (e));		/* sorted by x list */
	key = new_cnf ();
	SETKEY (x, key);
	if (e < ael.len)	/* exist more points    */
	{			/* fill line between two points */
	    hor_cnf ();
	    key = new_cnf ();
	    xr = GET_X (e);
	    if (xr < x + 2)
		continue;
	    for (x1 = x + 1; x1 < xr; x1++)
		SETKEY (x1, key);
	}

    }

    /* test empty cnf in end of line        
     * 
     * This was a useful assertion, but it does not hold when last edge 
     * on the line is at 45 degrees, then 6 or 1 can be filled.
     * If 0 or 7 is filled, then something should be REALLY wrong. 
     * P.B. 12/13/2002
     */
    assert (cnfs[0] == space_cnf && 
	    /* cnfs[1] == space_cnf &&
	       cnfs[6] == space_cnf && 
	    */
	    cnfs[7] == space_cnf);

}				/* accept_edges */

/*      Accept_two_el generates new configs for all points on line,     */
/*      uses ael & oldael as sources of information on edges.           */

#define 	GET_AEL_X(e)	( ael.edges[e].c0 + ael.edges[e].dx)
#define 	GET_OLD_X(e)	( oldael.edges[e].c0 + oldael.edges[e].dx)

accept_two_el ()
{
    byte            ea, eo, s;
    coord           x, xr;
    cnfkey          key;

    /*  clear cnf info      */
    for (s = 0; s < 8; cnfs[s++] = space_cnf) ;
    for (eo = 0; eo < oldael.len; eo++)
    {
	assert (oldael.edges[eo].dir < 4);
	oldael.edges[eo].dir += 4;
    }
    nte = 0;
    hortercnf = 0;
    /*  scan all edges      */
    ea = 0;
    eo = 0;
    if (ea < ael.len && eo < oldael.len)
	xr = min (GET_AEL_X (ea), GET_OLD_X (eo));
    else if (ea < ael.len)
	xr = GET_AEL_X (ea);
    else
	xr = GET_OLD_X (eo);
    while (ea < ael.len || eo < oldael.len)
    {
	nte = 0;		/* new point - new trmlst       */
	x = xr;
	if (hortercnf)		/* add to dir 4 terms from prev. points */
	    acc_ter (hortercnf, 4);
	while (ea < ael.len && x == GET_AEL_X (ea))
	    acc_e (ael.edges[ea++]);
	while (eo < oldael.len && x == GET_OLD_X (eo))
	    acc_e (oldael.edges[eo++]);
	assert (ea == ael.len || x < GET_AEL_X (ea));
	assert (eo == oldael.len || x < GET_OLD_X (eo));
	if (hortercnf)		/* add to dir 0 terms from curr. point  */
	    acc_ter (hortercnf, 0);
	key = new_cnf ();
	SETKEY (x, key);
	if (ea < ael.len || eo < oldael.len)	/* exist more points    */
	{			/* fill line between two points */
	    if (ea < ael.len && eo < oldael.len)
		xr = min (GET_AEL_X (ea), GET_OLD_X (eo));
	    else if (ea < ael.len)
		xr = GET_AEL_X (ea);
	    else		/* eo<oldael.len */
		xr = GET_OLD_X (eo);
	    hor_cnf ();
	    if (xr < x + 2)
		continue;
	    /*                      if( cnfs[0]==cnfs[7] && nte==0 && xr-x>3 )
				    key=wavesource[1][(x+xr)/2].incnf;
				    else
	    */ 
	    key = new_cnf ();
	    for (x = x + 1; x < xr; x++)
		SETKEY (x, key);
	}
    }

    /* test empty cnf in end of line
     * See above in accept_edges
     */
    assert (cnfs[0] == space_cnf && 
	    /* cnfs[1] == space_cnf &&
	       cnfs[6] == space_cnf && 
	    */
	    cnfs[7] == space_cnf);
    assert (!hortercnf);
}				/* accept_two_el */

#define 	GET_DIR(e)	(ael.edges[e].dir)
#define 	PREV_X(x,dir)	x-(2-dir)

/*      Use_keys fills incnf for all points on current line from those  */
/*      on previous line.                                               */
/*      When d45fl=1 then config info accumulated to generate new       */
/*      configs for points in 45deg. sectors (like  |/)                 */

use_keys ()
{
    byte            e, s;
    coord           xl, x, xr, xl1, xr1;
    byte            dirl, dirr;
    cnfkey          key;

    if (d45fl)
    {				/*  clear cnf info      */
	for (s = 0; s < 8; s++)
	    cnfs[s] = space_cnf;
	nte = 0;
    }
    /*  scan all edges      */
    e = 0;
    while (e < ael.len)
    {
	nte = 0;		/* new point - new trmlst       */
	xl = GET_X (e);
	dirl = GET_DIR (e);
	while (e < ael.len && xl == GET_X (e))
	{
	    if (d45fl)
		acc_e (ael.edges[e]);
	    e++;
	}
	assert (e == ael.len || xl < GET_X (e));	/* sorted list */
	key = wavesource[1][PREV_X (xl, dirl)].incnf;
	assert (key < cnfarrlen);
	assert (key);
	SETKEY (xl, key);
	if (e < ael.len)	/* exist more points    */
	{			/* fill line between two points */
	    xr = GET_X (e);
	    assert (xr > xl);
	    if (xr < xl + 2)
		continue;
	    dirr = GET_DIR (e);
	    xl1 = PREV_X (xl, dirl);
	    xr1 = PREV_X (xr, dirr);
	    assert (xr1 > xl1);
	    if (d45fl)
	    {
		hor_cnf ();
		assert (nte == 0);
		if (xr1 - xl1 == 1)	/* inside 45deg. sector */
		{
		    key = new_cnf ();
		    SETKEY (xl + 1, key);
		    continue;
		}
	    }
	    assert (xr1 - xl1 > 1);
	    key = wavesource[1][xl1 + 1].incnf;
	    assert (key < cnfarrlen);
	    assert (key);
	    for (x = xl + 1; x < xr; x++)
		SETKEY (x, key);
	}
    }
}				/*  end of use_keys */
/*
 *    Modify cnfs by accepting one edge.
 */
acc_e (edgedata ed)
{
    byte            dir, s;
    cnftype         cnf;

    dir = ed.dir;
    assert (ylfl && dir < 8 || !ylfl && dir < 4);
    assert (dir != 4);
    if ((cnf = ed.ecnf) && dir > 0)	/* not term. && non-hor */
    {
	if (dir < 4)		/* edge up      */
	{
	    for (s = dir; s > 0; s--)
		cnfs[s - 1] ^= cnf;
	    if (!ylfl)
		for (s = 4 + dir; s <= 7; s++)
		    cnfs[s] ^= cnf;
	}
	else if (dir > 4)	/* edge down - can be in yline  */
	{
	    assert (ylfl);
	    for (s = dir; s <= 7; s++)
		cnfs[s] ^= cnf;
	}
    }
    if (cnf = ed.tcnf)
	if (dir == 0)		/* add hor. cnf */
	{
	    assert (ylfl);
	    hortercnf ^= cnf;
	}
	else
	    acc_ter (cnf, dir);
}				/*  end of acc_e */
/*
 *    Modify cnf by accepting the terminals.
 */
acc_ter (cnftype cnf, int dir)
{
    byte            ite, up, dn;
    cnftype         termcnf;

    for (ite = 0; ite < ntrm; ite++)
	if (cnf & POW2 (ite + 1))	/* exist term. ite      */
	{
	    if (nte == POINTTERM)
		error ("Too many terms in one point");
	    up = terminfo[ite].up;
	    dn = terminfo[ite].down;

	    assert(dn < up && up <= nlay);

	    trmlst[nte].slot = dir;
	    trmlst[nte].up = up;
	    trmlst[nte].down = dn;
	    trmlst[nte].ite = ite;
	    nte++;
	    if (!ylfl)
	    {
		if (nte == POINTTERM)
		    error ("Too many terms in one point");
		trmlst[nte].slot = 4 + dir;
		trmlst[nte].up = up;
		trmlst[nte].down = dn;
		trmlst[nte].ite = ite;
		nte++;
	    }
	}
}
/*
 *    Delete all configs exept those in sectors 0 and 7 (horizontal).
 */
hor_cnf ()
{
    byte            t;
    slottrm         newtrmlst[POINTTERM];
    byte            newnte;

    cnfs[3] = cnfs[2] = cnfs[1] = cnfs[0];
    cnfs[4] = cnfs[5] = cnfs[6] = cnfs[7];

    newnte = 0;
    for (t = 0; t < nte; t++)
	if (trmlst[t].slot == 0)
	    newtrmlst[newnte++] = trmlst[t];
    assert (newnte <= nte);
    for (t = 0; t < newnte; t++)
	trmlst[t] = newtrmlst[t];
    nte = newnte;

}				/*  end of hor_cnf */

/*
 *    Free configs of last line.
 */
free_rcf ()
{
    coord           x;

    for (x = 0; x < xmax; x++)
	onefree (endarea_2d[0][x].incnf);
}				/* end of free_rcf */

cnfkey 
new_cnf ()			/* new cnf.info. -> free key    */
{
    cnfkey          i;
    byte            s, t;

    /*      NB:     key==0 is used for the area outside the layout lines !  */
    i = 1;
    assert (configs && cnfarrlen);
    while (i < cnfarrlen && configs[i].uses)
	i++;
    if (i == cnfarrlen)
    {
	cnfarrlen += CNFARRSTEP;
	reallocateN (configs, cnfarrlen);
	message ("LM: Config table size increased.");
    }				/*      then    */
    assert (i);
    configs[i].uses = 0;	/* sets to 1 by next SETKEY     */
    /* copy trmlst  */
    configs[i].nte = nte;
    if (nte)
    {
	allocate (configs[i].trmlst, nte, sizeof (slottrm));
    }
    else
	configs[i].trmlst = NULL;
    allocate (configs[i].gaps, 8, sizeof (gapinfo *));
    allocate (configs[i].cnfs, 8, sizeof (cnftype));
    for (t = 0; t < nte; t++)
    {
	configs[i].trmlst[t].slot = trmlst[t].slot;
	configs[i].trmlst[t].up = trmlst[t].up;
	configs[i].trmlst[t].down = trmlst[t].down;
	configs[i].trmlst[t].ite = trmlst[t].ite;
    }
    /* copy cnfs    */
    for (s = 0; s < 8; s++)
	configs[i].cnfs[s] = cnfs[s];
    allocate (configs[i].layels, nlay + 1, sizeof (layelinfo));
    masklays (configs + i);
    compels (configs + i);
    storegaps (configs + i);
    store_edgeinds (configs + i);
    storeadds (configs + i);
    dump_rcf (i);
    return (i);
}

oneuses (cnfkey key)		/* inc field "uses"             */
{
    assert (key < cnfarrlen);
    configs[key].uses++;
}

onefree (cnfkey key)		/* dec "uses",if 0 - free key   */
{
    byte            s;

    assert (key < cnfarrlen);
    if (configs[key].uses)
	if ((--configs[key].uses) == 0)
	{
	    Free (configs[key].trmlst);
	    Free (configs[key].layels);
	    Free (configs[key].addpta);
	    for (s = 0; s < 8; s++)
		Free (configs[key].gaps[s]);
	    Free (configs[key].gaps);
	    Free (configs[key].cnfs);
	    configs[key].nte = 0;
	    configs[key].neld = 0;
	}
}
/*
 *    Key==0 is used to mark free space points.
 *      make_null_cnf creates this item.
 */
make_null_cnf ()
{
    byte            s;

    cnfarrlen = CNFARRSTEP;
    allocateN (configs, cnfarrlen);
    configs[0].uses = 0;
    configs[0].nte = nte;
    allocate (configs[0].gaps, 8, sizeof (gapinfo *));
    allocate (configs[0].cnfs, 8, sizeof (cnftype));
    for (s = 0; s < 8; s++)
	configs[0].cnfs[s] = space_cnf;
    allocate (configs[0].layels, nlay + 1, sizeof (layelinfo));
    masklays (configs);
    compels (configs);
    storegaps (configs);
    storeadds (configs);
    dump_rcf (0);
}				/* end of make_null_cnf */

#ifndef	NDUMP

/*      dumps line of round configs keys to file.                       */
static FILE    *drcfkey;

_dump_rcfkey ()
{
    coord           x;
    cnfkey          key;
    extern char    *swapdir;
    extern char    *mknam (char *, char *);

    if (drcfkey == NULL)
	fileopen (drcfkey, mknam (swapdir, "rcf_key.dmp"), "w");
    key = cnfarrlen - 1;
    for (x = 0; x < xmax; x++)
    {
	key = wavesource[0][x].incnf;
	fprintf (drcfkey, "%4u", key);
    }
    fprintf (drcfkey, "\n");
    fflush (drcfkey);
}

/*      dumps rcf info to file.                                         */
static FILE    *drcf;

_dump_rcf (cnfkey key)
{
    byte            s, l;
    int             t;
    cnftype         cnf;
    byte            imetl;
    byte            e, neld;
    extern char    *swapdir;
    extern char    *mknam (char *, char *);

    if (drcf == NULL)
    {
	fileopen (drcf, mknam (swapdir, "rcf.dmp"), "w");
	dump_linecnf (drcf);
	dump_layerpar (drcf);
    }

/*      for(key=0;key<cnfarrlen;key++)  */
    {
	fprintf (drcf, "KEY: %4u USED %6u\n\nCNFS:\n", key, (unsigned) configs[key].uses);

	for (s = 0; s < 8; s++)
	{
	    fprintf (drcf, "\t%u:", s);
	    cnf = configs[key].cnfs[s];
	    write_bits (drcf, cnf);
	}

#define	TF(f)	( (unsigned) configs[key].trmlst[t]. f )
	fprintf (drcf, "\n\tNTE: %4u\n", (unsigned) configs[key].nte);
	for (t = 0; t < configs[key].nte; t++)
	    fprintf (drcf, "\tSLOT:%4u UP:%4u DN:%4u ITE:%4u\n",
		     TF (slot), TF (up), TF (down), TF (ite));
	fprintf (drcf, "\n\tNELD: %4u\tNMETL: %4u\n",
		 (unsigned) (neld = configs[key].neld),
		 (unsigned) configs[key].nmetl);
	fprintf (drcf, "\tADDPTA:\t");
	for (e = 0; e <= neld; e++)
	    fprintf (drcf, "%4u", (unsigned) configs[key].addpta[e]);

#define	EF(f)	( (unsigned) configs[key].layels[l]. f )
	fprintf (drcf, "\n\tIELD:\tIMETL:\tTERM:\tNXTLAY:\tLAYCNF:\n");
	for (l = 0; l <= nlay; l++)
	{
	    fprintf (drcf, "\t%-u\t%-u\t%-u\t%-u\t",
		     EF (ield), EF (imetl), EF (term), EF (nxtlay));
	    write_bits (drcf, EF (laycnf));
	}

#define	GP(f)	( configs[key].gaps[s][imetl]. f )
	for (s = 0; s < 8; s++)
	{
	    if (configs[key].nmetl == 0)
		break;
	    fprintf (drcf, "GAPS IN %u:\t", (unsigned) s);
	    for (imetl = 1; imetl <= configs[key].nmetl; imetl++)
	    {
		fprintf (drcf, "%4lf %4u %4u\t",
		  GP (coeff), (unsigned) GP (downlay), (unsigned) GP (uplay));
	    }
	    fputc ('\n', drcf);
	}

	fputc ('\n', drcf);
    }
    fflush (drcf);
}

#endif
