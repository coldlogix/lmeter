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
 * 	This is to substitute point_eqs in eqs_lm.c when using OLDLM3.
\*/
point_eqs(coord x)
{	roundcnf	_HUGE *ownrcf,_HUGE *nbrrcfs[8],_HUGE *nbrrcf;
	unsigned	ownbasvar,nbrbasvars[8];
	byte		nbrsector[8];
	cnftype		notwritten=~0;	/* which elds have no PTE yet	*/
	byte		l,s,i,mylayup,mylaydown,nblayup,nblaydown,nl,ns;
	byte		ield,ownneld,term,imetl,t;
	double		coeff;
	layelinfo 	*layels;
	byte 		uplay,dnlay;
	unsigned 	basvar;
	int		st1,st2;

	eqtree		**pteqs;	/* [neld+1] PTEs for all elds	*/
	eqtree		**ptads;	/* [ntea+1] PTAs for all terms	*/
	eqtree		*eqp;		/* points to one PTE/PTA	*/

	ENTERPROC(point_eqs)

	assert(x<xmax);
	assert(eqsource[0][x].incnf<CNFARRLEN);

	ownrcf=configs+eqsource[0][x].incnf;
	ownbasvar=eqsource[0][x].varnum;
	ownneld=ownrcf->neld;

	/* initialise all PTE,PTA	*/
	allocate(pteqs,ownneld+1,sizeof(eqtree*));
	allocate(ptads,ntea+1,sizeof(eqtree*));
	for(i=0;i<=ownneld;i++)
	{	allocate(pteqs[i],1,sizeof(eqtree));
		pteqs[i]->trelen=8*4+6;
		/* such length will be allocated by first insertion	*/
	}
	for(i=0;i<=ntea;i++)
	{	allocate(ptads[i],1,sizeof(eqtree));
		ptads[i]->trelen=8*4+6;
		/* such length will be allocated by first insertion	*/
	}

	/* to find all neibourghs use information in pattern		*/
	for(s=0;s<8;s++)
	{	coord	deltax,deltay;
		enum	dtag	d_nd;
		short	l;

		if( (l=pattern[s].l) == 0 )	/* bounds to space */
			nbrrcfs[s]=NULL;
		else
		{	d_nd=pattern[s].d_nd;
			deltay = dy[d_nd][s] * l;
			deltax = dx[d_nd][s] * l;
			assert( abs(deltax)<=POW2(kmax) );
			assert( abs(deltay)<=POW2(kmax) );
			nbrrcfs[s]=configs+eqsource[deltay][x+deltax].incnf;
			nbrbasvars[s]=eqsource[deltay][x+deltax].varnum;
			nbrsector[s]=nbrsectors[d_nd][s];
		}	/*	else	*/
	}	/*	for	*/

	for(l=1;l<=nlay;l++)
	{	if( (ield=ownrcf->layels[l].ield) == 0 ) continue;

		curreq=ield+ownbasvar-1;

		if( notwritten & POW2(ield) )	/* need PTE	*/
		{	notwritten &= ~POW2(ield);
			if (ield==1) continue;	/* ground,no PTE*/
				eqp=pteqs[ield];
		}
		else if( (term=ownrcf->layels[l].term) > 0 && term!=ISOHOLE)
		{	/* generate PTA for terminal */
			eqp=ptads[term];
		}
		else	continue;
		/* near hole in isolation it can be 		*/
		/* no eld.,that need PTA couse of design rules	*/

		/* count currs from all sectors, add them to eqp-> */
		for(s=0;s<8;s++)
		{	/* now add two psi vars (phi's+list of terms)	*/

			nbrrcf=nbrrcfs[s];
			/* is it near Free space ?			*/
			if(nbrrcf==NULL) continue;

			/* find own up/down layers			*/
			imetl=ownrcf->layels[l].imetl;
			assert(imetl<=ownrcf->nmetl);
			mylayup=ownrcf->gaps[s][imetl].uplay;	nl=mylayup;
			mylaydown=ownrcf->gaps[s][imetl].downlay;

			/* is there both elds ?				*/
			if(mylayup==0 || mylaydown==0)	continue;

			assert(mylayup>mylaydown);
			coeff=ownrcf->gaps[s][imetl].coeff;

			/* find nbr up/down layers			*/
			ns=nbrsector[s];
			imetl=nbrrcf->layels[nl].imetl;
			assert(imetl<=nbrrcf->nmetl);
			nblayup=nbrrcf->gaps[ns][imetl].uplay;
			nblaydown=nbrrcf->gaps[ns][imetl].downlay;

			assert(nblayup && nblaydown);
			assert(nblayup>nblaydown);
			assert(coeff==nbrrcf->gaps[ns][imetl].coeff);
			/*	Add to current equation four sums:	*/
			/*	psi var from current point		*/
			/*      then psi var from nbr. point.		*/
			/*	Each psi var consists of parts from 	*/
			/*	up/down layers of the gap.		*/
			for	(	st1=0,
					layels=ownrcf->layels,
					uplay=mylayup,
					dnlay=mylaydown,
					basvar=ownbasvar;
					st1<2;
					st1++,
					layels=nbrrcf->layels,
					uplay=nblayup,
					dnlay=nblaydown,
					basvar=nbrbasvars[s],
					coeff=-coeff
				)
			{	unsigned	var;
				signed		term;
				byte		lay;

				assert(layels[uplay].ield);	/* both are metal	*/
				assert(layels[dnlay].ield);
				for	(	st2=0,
						lay=uplay;
						st2<2;
						st2++,
						lay=dnlay,
						coeff=-coeff
					)
					while(lay>0)
					{       if( (var=layels[lay].ield)==0)	lay--;  /* not metal,skip */
						else if( term=layels[lay].term )	/* add term phi	*/
							if(term==ISOHOLE)	lay--;	/* skip		*/
							else
							{	assert(term<=ntea);
								instree(eqp,-term,coeff);
								lay=layels[lay].nxtlay;
								assert(lay>0 && lay<=nlay);
							}
						else if( var>1 )	/* not grd, add phi var	*/
						{	assert(var+basvar-1);
							instree(eqp,var+basvar-1,coeff);
							lay--;
						}
						else	lay--;	/* ground with nothing to add	*/
					}	/*	while	*/
			}	/*	for	*/
		}	/*	for	*/
	}	/*	for	*/

	/* here add PTAs to PTEs if needs	*/
	for(i=2;i<=ownneld;i++)
		if(t=ownrcf->addpta[i])
			add_tree(pteqs[i],ptads[t],-1.0);
	/* add PTAs to term. eqs.		*/
	for(i=1;i<=ntea;i++)	add_tree(teaeqs[i],ptads[i],1.0);
	/* need'nt I add ptads[i] not only to i'th term	*/
	/* but to all lower terms in string too ??	*/
		/* write PTE to files A,B		*/
	for(i=2;i<=ownneld;i++)
	{	curreq=i+ownbasvar-1;
		outtree(pteqs[i]);
	}
	for(i=0;i<=ownneld;i++)		free_tree(pteqs[i]);
	for(i=0;i<=ntea;i++)		free_tree(ptads[i]);
	Free(pteqs);
	Free(ptads);
} /*  end of point_eqs */
