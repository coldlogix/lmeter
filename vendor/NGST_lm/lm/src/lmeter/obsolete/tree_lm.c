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
 *	That is the module to store equation vars & coeffs in trees
 *	and to write those trees to output files.
\*/

#include"lmeter.h"
#include"head_lm.h"

#ifdef	OLDLM3

#include<math.h>

/* Store part								    */

struct	titem		*tremem;/* unpacked *eqp for recursive proc.	*/
static	unsigned	trelen,treuse;
static	double		c;
static	signed		v;

instree( eqtree *eqp,	signed var,	double coeff)
{
	ENTERPROC(instree);

	if(eqp->tremem==NULL)	/* insert first rec.	*/
	{	allocate(eqp->tremem,eqp->trelen,sizeof(struct titem) );
		eqp->treuse=1;
		eqp->tremem[0].coeff=coeff;
		eqp->tremem[0].var=var;
	}
	else	/* search in tree */
	{       tremem=eqp->tremem;
		treuse=eqp->treuse;
		trelen=eqp->trelen;
		if(treuse==trelen)	/*	expand tremem	*/
		{	struct	titem		*newtremem;
			unsigned		i;
			trelen*=2;
			allocate(newtremem,trelen,sizeof(struct titem) );
			for(i=0;i<treuse;i++)	newtremem[i]=tremem[i];
			Free(tremem);
			tremem=newtremem;
			eqp->tremem=tremem;
			eqp->trelen=trelen;
		}
		c=coeff;
		v=var;
		if(var<tremem[0].var)	recins(&(tremem[0].left));
		else
		if(var>tremem[0].var)	recins(&(tremem[0].right));
		else			tremem[0].coeff+=coeff;
		eqp->treuse=treuse;
	}
	assert(eqp->treuse<=eqp->trelen);
	LEAVEPROC(instree);
}

recins(unsigned *indptr)
{	unsigned	ind=*indptr;

	if(ind==0)	/* null branch,insert new item	*/
	{       assert(treuse<trelen);
		tremem[treuse].coeff=c;
		tremem[treuse].var=v;
		*indptr=treuse++;
	}
	else	if(v<tremem[ind].var)	recins(&(tremem[ind].left));
	else	if(v>tremem[ind].var)	recins(&(tremem[ind].right));
	else	tremem[ind].coeff+=c;
}

add_tree(eqtree *eqto,eqtree *eqfrom, double sign)
{	unsigned	i;
	struct	titem	*tremem;
	tremem=eqfrom->tremem;
	for(i=0;i<eqfrom->treuse;i++)
		instree(eqto,tremem[i].var,sign*tremem[i].coeff);
	assert(eqto->treuse <= eqto->trelen);
}

free_tree(eqtree *eqt)
{	Free(eqt->tremem);
	Free(eqt);
}

/* Output part								    */

extern	byte		itea;
extern	unsigned	nnz;
extern	byte		outterfile;

FILE	*atext,*btext,*ctext,*dtext,*itext;
FILE	*abin,*bbin,*cbin,*dbin,*ibin;

FILE	*lhsbin,*lhstxt,*rhsbin,*rhstxt;

double	*rhsbuf;	/* buffer to store rhs for different tests	*/

/* Write items from eqp-> to files lhsfile & rhsfile (pos & neg vars)	*/
outtree( eqtree *eqp)
{	byte	i;

	ENTERPROC(outtree);

	tremem=eqp->tremem;
	if(tremem==NULL)
	{	fprintf(stderr,"dumping empty tree\n");
		return 0 ;
	}

	allocate(rhsbuf,ntea+1,sizeof(double));

	if(outterfile)
	{	lhsbin=cbin;	lhstxt=ctext;
		rhsbin=dbin;	rhstxt=dtext;
#		ifndef	NTEXT
		fprintf(lhstxt,"%5u:\t",itea);
		fprintf(rhstxt,"%5u:\t",itea);
#		endif
	}
	else
	{	lhsbin=abin;	lhstxt=atext;
		rhsbin=bbin;	rhstxt=btext;
#		ifndef	NTEXT
		fprintf(lhstxt,"%5u:\t",curreq);
		fprintf(rhstxt,"%5u:\t",curreq);
#		endif
	}
	assert(eqp->treuse <= eqp->trelen );
	recout(tremem[0].left);
	varout(0);
	recout(tremem[0].right);
	assert(eqp->treuse <= eqp->trelen );
	if(	fwrite(rhsbuf+1,sizeof(double),nout,rhsbin)!=nout	)
		error("output write error");
#	ifndef	NTEXT
	for(i=1;i<=nout;i++)	fprintf(rhstxt,"%5lf\t",rhsbuf[i]);
	fputc('\n',lhstxt);
	fputc('\n',rhstxt);
#	endif
	Free(rhsbuf);

	LEAVEPROC(outtree);
}
recout(unsigned ind)
{	if(ind>0)
	{	recout(tremem[ind].left);
		varout(ind);
		recout(tremem[ind].right);
	}
}
varout(unsigned ind)
{	signed	var;
	double	coeff;
	struct
	{	double	coeff;
		long	ini,inj;	/* out matrix indexes	*/
	}	outrec;

	var=tremem[ind].var;
	coeff=tremem[ind].coeff;
	assert(var);
	if( fabs(coeff)<1e-7 )	return	0;
	if(var>0)	/* normal variable	*/
	{
#		ifndef	NTEXT
			fprintf(lhstxt,"<%3d> %4.1lf  ",var,coeff);
#		endif
		if(outterfile)
		{	outrec.coeff=coeff;
			outrec.ini=itea;
			outrec.inj=var;
			if(fwrite(&outrec,sizeof(outrec),1,lhsbin)!=1)
				error("output write error");
		}
		else
		{
			/* show_var(var,coeff); */
#			ifndef	NSYMM
				_test_symm(var,coeff);
#			endif
			if(curreq<=var)		/* coeff from upper triangle	*/
			{	nnz++;
				outrec.coeff=coeff;
				outrec.ini=curreq;
				outrec.inj=var;
				if( fwrite(&outrec,sizeof(outrec),1,lhsbin)!=1 )
					error("output write error");
			}
		}
	}
	else		/* term. index		*/
	{	var=-var;
		rhsbuf[var]-=coeff/currunit;
	}
}

char	*mknam(char *prefix,char *suffix)
{       static	char	buffer[256];
	if(prefix==NULL)	return	suffix;
	if(suffix==NULL)	return	prefix;
	strcpy(buffer,prefix);
	strcat(buffer,suffix);
	return	buffer;
}

openout()
{
#	ifndef	NTEXT
		fileopen(atext,mknam(swapdir,"lm_a.txt"),"w");
		fileopen(btext,mknam(swapdir,"lm_b.txt"),"w");
		fileopen(ctext,mknam(swapdir,"lm_c.txt"),"w");
		fileopen(dtext,mknam(swapdir,"lm_d.txt"),"w");
#	endif
	fileopen(itext,mknam(swapdir,"lm_i.txt"),"w");
#	ifdef	__NDPC__
#		define	MD	"w"
		_pmode=O_BINARY;
#	else
#		define	MD	"wb"
#	endif
	fileopen(abin,mknam(swapdir,"lm_a.dat"),MD);
	fileopen(bbin,mknam(swapdir,"lm_b.dat"),MD);
	fileopen(cbin,mknam(swapdir,"lm_c.dat"),MD);
	fileopen(dbin,mknam(swapdir,"lm_d.dat"),MD);
	fileopen(ibin,mknam(swapdir,"lm_i.dat"),MD);
#	undef	MD

	assert(sizeof(double)==8);
	assert(sizeof(long)==4);
	/* for compatibility with prev. version of LM3	*/
}

closeout()
{	/* write to ifile here */
	struct
	{	long	nnz,neq,nout,ntea;
	}	ihead;
#	define	FL1	20
#	define	FL2	40
	long	ftea[FL1],ftrm[FL2];
	int	t;

	ihead.nnz=nnz;
	ihead.neq=curreq;
	ihead.nout=nout;
	ihead.ntea=ntea;

	for(t=0;t<FL1;t++)	ftea[t]=0L;
	for(t=0;t<FL2;t++)	ftrm[t]=0L;
	for(t=0;t<nout;t++)	ftea[t]=(long)t+1;
	for(t=0;t<ntrm;t++)	if(terminfo[t].itea)
					ftrm[terminfo[t].itea-1]=(long)t+1;
	if(	fwrite(&ihead ,sizeof(ihead),1,ibin)!=1	||
		fwrite(ftea,sizeof(*ftea),FL1,ibin)!=FL1||
		fwrite(ftrm,sizeof(*ftrm),FL2,ibin)!=FL2	)
		error("output write error");
	fprintf(itext,"NNZ:%4u\tNEQ:%4u\tNOUT:%4u\tNTEA:%4u\n",
			nnz,curreq,nout,ntea);
	fprintf(itext,"\tFTEA(IOUT)\tFTRM(ITEA)\n");
	for(t=0;t<ntrm;t++)
		fprintf(itext,"\t%4u\t\t%4u\n",
				(unsigned)ftea[t],(unsigned)ftrm[t]);
#	ifndef	NTEXT
		fflush(atext);fclose(atext);
		fflush(btext);fclose(btext);
		fflush(ctext);fclose(ctext);
		fflush(dtext);fclose(dtext);
#	endif
	if(	fflush(itext)	||	fclose(itext)	||
		fflush(abin)	||	fclose(abin)	||
		fflush(bbin)	||	fclose(bbin)	||
		fflush(cbin)	||	fclose(cbin)	||
		fflush(dbin)	||	fclose(dbin)	||
		fflush(ibin)	||	fclose(ibin)	)
		error("closing file error");

#	ifndef	NSYMM
		_final_test_symm();
#	endif

}

#ifndef	NSYMM

struct	nzstruct
{	float		coeff;
	unsigned	i,j;
}	*nzsaves=NULL;		/*	array to store non-zero coeffs	*/

unsigned	nnzsave=0;	/*	number of stored coeffs.	*/
unsigned	nzslen=50;	/*	initial length of nzsaves	*/

_test_symm(unsigned	var,double coeff)
{       unsigned	inzsave;

	if(curreq==var)		/*	diagonal entry	*/
		if(coeff<=0)
			error("Bad diagonal entry");
		else	return;

	if(nzsaves==NULL)	allocate(nzsaves,nzslen,sizeof(*nzsaves));
	/*	nzsaves is allocated by the first call of _test_symm	*/

	inzsave=0;
	while
	(	( nzsaves[inzsave].i!=var || nzsaves[inzsave].j!=curreq )
		&&	inzsave<nnzsave
	)	inzsave++;
	if(inzsave==nnzsave )
	{       /*	insert new coeff	*/
		if(nnzsave==nzslen)
		{	struct	nzstruct	*newnzs;
			nzslen*=2;
			allocate(newnzs,nzslen,sizeof(*nzsaves));
			for(inzsave=0;inzsave<nnzsave;inzsave++)
				newnzs[inzsave]=nzsaves[inzsave];
			Free(nzsaves);
			nzsaves=newnzs;
		}
		nzsaves[inzsave].i=curreq;
		nzsaves[inzsave].j=var;
		nzsaves[inzsave].coeff=coeff;
		nnzsave++;
	}
	else	/*	the symm. coeff. was found	*/
	{	assert(nzsaves[inzsave].i==var && nzsaves[inzsave].j==curreq);
		assert(inzsave<nnzsave);
		if(fabs(nzsaves[inzsave].coeff-coeff)>1e-6)
		{       fprintf(stderr,"At %u %u %f %f\n",curreq,var,nzsaves[inzsave].coeff,c);
			error("unsymmetric matrix");
		}
		else	/*	symm. coeffs are equal	*/
		{       /*	remove that coeff.	*/
			nnzsave--;
			while(inzsave<nnzsave)
			{       inzsave++;
				nzsaves[inzsave-1]=nzsaves[inzsave];
			}
		}

	}

}

_final_test_symm()
{
	if(nnzsave>0)	error("final symm. test fails");
	else		message("Symmetry test is OK");
	Free(nzsaves);
}

#endif

#endif
