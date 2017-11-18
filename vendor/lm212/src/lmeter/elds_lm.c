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
 *    This is to process point's RoundConfig (rcf) and fill
 *      some structures stored there.
 */

#include"lmeter.h"
#include"head_lm.h"
/*
 *    Find the point's Compound Electrodes using cnfs & tertlst,
 *      store this info in layels[] & neld.
 *      NB:     The lowest eld. has number 1, not 0.
 */
void 
compels (roundcnf * rcf)
{
    layelinfo      *layels;	/* [nlay+1] with comp. eld. info */
    cnftype        *cnfs;	/* configs in all corners (0-7) */
    slottrm        *trmlst;
    byte            nmetl, eldup, elddn, up, down, ite, neld;
    byte            laycnf;
    cnftype         laybit;
    byte            sectbit, l, l1, s, t;
    byte            wasmet, badiso;

    layels = rcf->layels;
    cnfs = rcf->cnfs;
    trmlst = rcf->trmlst;
    assert (layels);
    assert (cnfs);
    assert (trmlst || rcf->nte == 0);
    neld = 0;
    nmetl = 0;
    laybit = 1;
    /* for all layers initialize layinfo    */
    for (l = 0; l <= nlay; l++)
    {				/* set 1 in laycnf for metal sectors    */
	if (layinfo[l].type == MET)
	{
	    sectbit = 1;
	    laycnf = 0;
	    for (s = 0; s < 8; s++)
	    {
		if (cnfs[s] & laybit)
		    laycnf |= sectbit;
		sectbit <<= 1;
	    }
	    layels[l].laycnf = laycnf;
	    /* exist metal sector? */
	    layels[l].ield = (laycnf) ? ++neld : 0;
	    layels[l].imetl = (laycnf) ? ++nmetl : 0;

	}
	else
	{
	    layels[l].laycnf = 0;
	    layels[l].ield = 0;
	    layels[l].imetl = 0;
	}
	layels[l].terlen = 0;
	layels[l].term = 0;
	layels[l].nxtlay = 0;
	laybit <<= 1;
    }
    rcf->nmetl = nmetl;

    /* Does there any electrodes exist ?
     */
    if (neld == 0)
    {
	rcf->neld = 0;
	return;
    }

    /* use connections through holes is isolator                    */
    /* isolator layer has hole if it does not exist at some sector  */
    wasmet = 0;			/* first,no metal eld. can be reached   */
    for (l = 1; l <= nlay; l++)
    {
	laybit = POW2 (l);
	switch (layinfo[l].type)
	{
	case NIL:
	    break;
	case ISO:
	    if (wasmet)
		/* interesting iso. are next to metal   */
		/* is it good (complete) isolator?      */
	    {
		badiso = 0;
		/* think that iso is good       */
		for (s = 0; s < 8; s++)
		    if (!(cnfs[s] & laybit))
		    {		/* no iso. at slot s    */
			badiso = 1;
			break;
		    }
		if (!badiso)
		    wasmet = 0;
	    }
	    break;
	case MET:
	    if (!layels[l].laycnf)
		break;
	    /* existing layer */
	    if (!wasmet)
	    {
		wasmet = 1;
		elddn = layels[l].ield;
	    }
	    else
	    {			/* connect two layers   */
		eldup = layels[l].ield;
		layels[l].ield = elddn;
		layels[l].term = ISOHOLE;
		/* shift down ields     */
		for (l1 = l + 1; l1 <= nlay; l1++)
		    if (layels[l1].ield > eldup)
			layels[l1].ield--;
		neld--;
	    }
	}
    }

    /* use connections through terminals            */
    for (t = 0; t < rcf->nte; t++)
    {
	up = trmlst[t].up;
	down = trmlst[t].down;
	ite = trmlst[t].ite;
	assert (up > down && up <= nlay);
	assert (ite <= ntrm);
	if (terminfo[ite].itea > 0)	/* non dummy    */
	{
	    eldup = layels[up].ield;
	    elddn = layels[down].ield;
	    assert (eldup <= neld && elddn <= neld);
	    if (!eldup || !elddn)
	    {
		sprintf (strbuff, 
			 "LMeter: %s %s layer(s) missing at terminal %s (#%d)",
			 !eldup?"top":"",
			 !elddn?"bottom":"",
			 terminfo[ite].name, ite);
		error (strbuff);
	    }
	    if (eldup == elddn)
		if (layels[up].term != terminfo[ite].itea)
		{
		sprintf (strbuff, 
			 "LMeter: shared upper electrode or shorted terminal %s (#%d)",
			 terminfo[ite].name, ite);
		error (strbuff);
		}
		else		/* the same term. from the other slot */
		    continue;
	    layels[up].term = terminfo[ite].itea;
	    layels[up].nxtlay = down;
	    /* shift down ields */
	    for (l = 0; l <= nlay; l++)
		if (layels[l].ield > eldup)
		    layels[l].ield--;
		else if (layels[l].ield == eldup)
		    layels[l].ield = elddn;
	    neld--;
	}
    }

    rcf->neld = neld;

}				/* end of compels       */
/*
 *    Find all Gaps around the point, store them in gaps[].
 */
void 
storegaps (roundcnf * rcf)
{
    layelinfo      *layels;	/* [nlay+1] with comp. eld. info */
    cnftype        *cnfs;	/* configs in all corners (0-7) */
    byte            nmetl, imetl, m, s, l;
    cnftype         laybit;
    gapinfo       **gaps;
    byte            downlay;
    byte            firstmet, justmet, waitgrd;
    double          c, d;

    gaps = rcf->gaps;
    nmetl = rcf->nmetl;
    layels = rcf->layels;
    cnfs = rcf->cnfs;

    assert (gaps);
    assert (layels);
    assert (cnfs);

    /* allocate gaps[][]    */
    for (s = 0; s < 8; s++)
	allocate (gaps[s], (nmetl + 1), sizeof (gapinfo));
    /* scan all sectors     */
    for (s = 0; s < 8; s++)
    {
	firstmet = 1;		/* was not metal        */
	downlay = 0;
	justmet = 0;
	waitgrd = 1;
	for (l = 0; l <= nlay; l++)
	{
	    laybit = POW2 (l);
	    switch (layinfo[l].type)
	    {
	    case NIL:
		break;
	    case ISO:
		if (cnfs[s] & laybit)
		{
		    justmet = 0;
		    d += layinfo[l].repar;
		}
		break;
	    case MET:
		if (cnfs[s] & laybit)
		{
		    if (firstmet)
		    {
			if (waitgrd)
			{	/*      First gap - (c==0.0)    */
			    imetl = layels[l].imetl;
			    m = imetl;
			    /*      Fill 'imetl's in the gap */
			    while (m > 0)
			    {
				gaps[s][m].coeff = 0.0;
				gaps[s][m].uplay = l;
				assert (gaps[s][m].downlay == 0);
				m--;
			    }
			    waitgrd = 0;
			}	/*      then    */
			/* open gap     */
			firstmet = 0;
			justmet = 1;
			d = layinfo[l].repar;
			downlay = l;
		    }
		    else
		    {		/* close gap    */
			imetl = layels[l].imetl;
			assert (imetl);
			if (justmet)	/*use last imetl */
			    gaps[s][imetl] = gaps[s][imetl - 1];
			else
			{
			    d += layinfo[l].repar;
			    c = 1 / d;
			    gaps[s][imetl].downlay = downlay;
			    m = imetl;
			    /*      Fill 'imetl's in the gap */
			    while (m > 0 &&
				   gaps[s][m].downlay == downlay)
			    {
				gaps[s][m].coeff = c;
				gaps[s][m].uplay = l;
				m--;
			    }
			    d = layinfo[l].repar;
			    downlay = l;
			    justmet = 1;
			}
		    }
		}
		else
		{
		    imetl = layels[l].imetl;
		    if (imetl)	/* exist some metal */
			gaps[s][imetl].downlay = downlay;
		    /* coeff,uplay will be stored later */
		}
		break;
	    default:
		error ("Internal inconsistency");
	    }
	}
    }

}				/* end of storegaps     */
/*
 *    Modify coeffs stored in 'gaps' structure to add edge inductances.
 */
void 
store_edgeinds (roundcnf * rcf)
{
    layelinfo      *layels;	/* [nlay+1] with comp. eld. info */
    cnftype        *cnfs;	/* configs in all corners (0-7) */
    gapinfo       **gaps;
    float           Coeff3D;
    byte            nmetl, s;

#ifndef	NOINDCORR

    gaps = rcf->gaps;
    nmetl = rcf->nmetl;
    layels = rcf->layels;
    cnfs = rcf->cnfs;

    assert (gaps);
    assert (layels);
    assert (cnfs);

    for (s = 0; s < 8; s++)
    {
	byte            s1 = (s + 1) % 8, l, nextl;
	cnftype         metaledges = (cnfs[s] ^ cnfs[s1]) & metalcnf;

	for (l = 1; metaledges && (l <= nlay); l = nextl)
	{
	    cnftype         laybit = POW2 (l);

	    if (metaledges & laybit)
	    {
		byte            sa, sb, l1, imetl, imetl1, TrueEdge, m, downlay,
		                downlay1;
		float           coeff, coeff1, newcoeff, newcoeff1;
		float           efflength, cuttedpart;
		cnftype         cnfa, cnfb;
		gapinfo        *gapptr, *gapptr1;
		byte            closeUp, closeDn;

		metaledges ^= laybit;
		/*      First, determine, which sector
		 *    ('s' or 's1') contains metal slice,
		 *      it will be 'sa' and the other - 'sb'.
		 */
		if (cnfs[s] & laybit)
		{
		    assert (!(cnfs[s1] & laybit));
		    sa = s;
		    sb = s1;
		}		/*      then    */
		else
		{
		    assert (cnfs[s1] & laybit);
		    sa = s1;
		    sb = s;
		}		/*      else    */
		cnfa = cnfs[sa];
		cnfb = cnfs[sb];
		/*      Check, whether it is a true edge,
		 *    i.e. there is no metal in 'sb' that
		 *      can connect to 'sa' slice.
		 *      First, look OVER 'l', also getting
		 *      'nextl' value.
		 */
		TrueEdge = 1;

		for (l1 = l + 1; l1 <= nlay; l1++)
		{
		    cnftype         lay1bit = POW2 (l1);

		    if (cnfa & lay1bit)
			if (metalcnf & lay1bit)
			{
			    if (cnfb & lay1bit)
				TrueEdge = 0;
			}
			else if (isocnf & lay1bit)
			    break;
		}		/*      for     */
		nextl = l1 + 1;
		/*      Then, look UNDER l.
		 */
		for (l1 = l - 1; TrueEdge && (l1 >= 1); l1--)
		{
		    cnftype         lay1bit = POW2 (l1);

		    if (cnfa & lay1bit)
			if (metalcnf & lay1bit)
			{
			    if (cnfb & lay1bit)
				TrueEdge = 0;
			}
			else if (isocnf & lay1bit)
			    break;
		}		/*      for     */

		/*      Continue with 'nextl' if no true edge.
		 */
		if (!TrueEdge)
		    continue;

		/*      Add additional inductance to 
		 *      gaps covered in 'sa' by layer 'l' 
		 *      AND by layer 'l1' - layer over 'l'.
		 *      'l1' is determined later.
		 */
		imetl = layels[l].imetl;
		assert (imetl <= nmetl);
		gapptr = gaps[sa] + imetl;
		coeff = gapptr->coeff;
		assert (gapptr->uplay == l);
		downlay = gapptr->downlay;

		if (downlay == 0)
		{
		    assert (coeff == 0.0);
		    closeDn = 0;
		}		/*      then    */
		else
		    closeDn = 1;

		closeUp = 0;
		for (l1 = nextl; l1 <= nlay; l1++)
		    if (cnfa & metalcnf & POW2 (l1))
		    {
			closeUp = 1;
			break;
		    }		/*      then    */
		if (closeUp)
		{
		    imetl1 = layels[l1].imetl;
		    assert (imetl1 <= nmetl);
		    gapptr1 = gaps[sa] + imetl1;
		    coeff1 = gapptr1->coeff;
		    assert (gapptr1->uplay == l1);
		    downlay1 = gapptr1->downlay;
		}
		else
		    coeff1 = 0.0;
		/*
		 *    Get the value of 3D coeff.
		 */
		switch (closeUp + closeDn)
		{
		case 0:
		    Coeff3D = Coeff3D0;
		    break;
		case 1:
		    Coeff3D = Coeff3D1;
		    break;
		case 2:
		    Coeff3D = Coeff3D2 / 2;
		    break;
		default:
		    error ("Internal error");
		}		/*      switch  */
		/*
		 *    Subtract cut-off, add Coeff3D
		 */
		efflength = unitsize / 2;	/* Thanks to D.Z. --- for 2     */
		cuttedpart = (layinfo[l].shift + bulklambda) / efflength;
		newcoeff = coeff * (1 - cuttedpart) + Coeff3D / efflength;
		if (closeUp)
		    newcoeff1 = coeff1 * (1 - cuttedpart) + Coeff3D / efflength;
		/*      
		 *    And give 'newcoeff' and 'newcoeff1' to all users.
		 */
		m = imetl;
		while (m > 0 && gaps[sa][m].downlay == downlay)
		    gaps[sa][m--].coeff = newcoeff;
		if (closeUp)
		{
		    m = imetl1;
		    while (m > 0 && gaps[sa][m].downlay == downlay1)
			gaps[sa][m--].coeff = newcoeff1;
		}

	    }			/*      then    */
	    else
		nextl = l + 1;
	}			/*      for     */
    }				/*      for     */

#endif /*        NOINDCORR       */

    return;
}				/*      store_edgeinds  */
/*
 *      Fill the 'addpta' structure of the point.
 */
void 
storeadds (roundcnf * rcf)
{
    byte            e, l, down, eup, itea;
    byte           *addpta;
    layelinfo      *layels;

    allocate (addpta, rcf->neld + 1, sizeof (*addpta));
    layels = rcf->layels;
    assert (layels);
    rcf->addpta = addpta;
    l = nlay;
    while (l > 1)
    {
	itea = layels[l].term;
	assert (itea <= ntea || itea == ISOHOLE);
	if (itea > 0 && itea != ISOHOLE)
	{
	    down = layels[l].nxtlay;
	    eup = layels[l].ield;
	    assert (down < l);
	    l--;
	    /* all elds on lays from l-1 to 'down' need PTA */
	    while (l > down)
	    {
		e = rcf->layels[l].ield;
		if (e >= eup)	/* e needs PTA from itea     */
		    addpta[e] = itea;
		l--;
	    }
	}
	else
	    l--;
    }

}				/* end of storeadds     */

/*  masklay uses the mask information to mask some layers       */
void 
masklays (roundcnf * rcf)
{
    cnftype        *cnfs = rcf->cnfs;	/* configs in all sectors (0-7) */
    int             l, s;
    cnftype         laybit, maskbit, notlaybit;

    for (l = 0; l <= nlay; l++)
    {
	if (layinfo[l].mask > 0)	/* lay l is and-masked  */
	{
	    laybit = POW2 (l);
	    notlaybit = ~laybit;
	    maskbit = POW2 (layinfo[l].mask);
	    for (s = 0; s < 8; s++)
		if ((cnfs[s] & laybit) && !(cnfs[s] & maskbit))
		    cnfs[s] &= notlaybit;
	    /* if exist masked layer and not exist mask     */
	    /* delete masked layer.                         */
	}
	if (layinfo[l].mask < 0)	/* lay l is or-masked   */
	{
	    laybit = POW2 (l);
	    maskbit = POW2 (-layinfo[l].mask);
	    for (s = 0; s < 8; s++)
		if (cnfs[s] & maskbit)
		    cnfs[s] |= laybit;
	    /* if exist mask create or-masked layer */
	}
    }
}				/* end of masklays      */
