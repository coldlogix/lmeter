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
 *    This is the main header for the equation building (and solving)
 *      part of LMeter (LM itself).
 */

#include"csspdms.h"

typedef enum
{
    NIL = 0, ISO = 1, MET = 2
}
isomet;
typedef enum
{
    DUM = 0, ISR = 2, SHT = 1
}
srcsht;

enum dtag
{
    ND, DG
};

typedef unsigned cnfkey;

typedef struct
{
    coord           x;		/* x coord of edge beginning            */
    coord           c0;		/* x +/- y = c0 defines edge            */
    coord           dx;		/* x offset from edge beginning (c0,0)  */
    byte            dir;	/* edge direction (0-7)                 */
    cnftype         ecnf, tcnf;	/* edge's config        */
}
edgedata;

typedef struct
{
    byte            len;
    edgedata       *edges;
}
edgelist;

typedef struct
{
    coord           y;
    edgelist        el;
    short           isnotlast;
}
ylinedata;

typedef struct
{
    unsigned        varnum;	/* first var. nmb. in point     */
    byte            isused;	/* 0 or point order /k/ + 1     */
    cnfkey          incnf;	/* index in configs             */
}
point;

typedef struct
{
    short           mask;
    isomet          type;
    float           repar;
    float           shift;	/* shift in um.         */
}
layerpar;

typedef struct
{
    byte            up, down;
    srcsht          type;
    byte            itea;
    char            *name;
}
termpar;

typedef struct
{
    byte            slot;
    byte            up, down;
    byte            ite;
}
slottrm;

typedef struct
{
    byte            ield;	/* nmb. of compound electrode   */
    byte            laycnf;	/* sect. with metal marked by 1 */
    byte            imetl;	/* nmb. of metal layer          */
    byte            terlen;	/* nmb. of terms. to lowest pnt. */
    byte            nxtlay;	/* layer to which is connected  */
    byte            term;	/* itea of term.,that connects  */
}
layelinfo;

#define	ISOHOLE	(byte)-1
/* value for term to sign that that elds are shorted by hole in isolator */

typedef struct
{
    double          coeff;	/* 1/gap_thickness              */
    byte            uplay, downlay;	/* up,down layers in gap        */
}
gapinfo;

typedef struct
{
    unsigned        uses;	/* how many points use that item */
    cnftype        *cnfs;	/* configs in all corners [8]   */
    byte            nte;	/* length of trmlst             */
    slottrm        *trmlst;	/* list of terminals in point   */
    byte            neld;	/* number of compound electrodes */
    layelinfo      *layels;	/* [nlay+1] with comp. eld. info */
    byte            nmetl;	/* total nmb. of metal layers.  */
    gapinfo       **gaps;	/*gaps[8][nmetl] stores info on */
    /* gap,covered by imetl in sect. */
    byte           *addpta;	/* addpta[neld+1] - itea to add */
    /* its PTA to ield's equation   */
}
roundcnf;

typedef point  *area_1d;	/* one row of points            */

/*      THAT EXTERNS ARE DEFINED AT MAIN_LM.C                           */
/*      note,that pipeline rows with smaller indexes have greater y's!  */

EXTERN area_1d *area_2d;	/* main pipeline                        */
EXTERN area_1d *wavesource;	/* middle line in interference area     */
EXTERN area_1d *eqsource;	/* middle line in pattern build area    */
EXTERN area_1d *endinterf;	/* end of interference area             */
EXTERN area_1d *endarea_2d;	/* end of the pipeline                  */
EXTERN area_1d *back, *front;	/* active pipeline's head and tail      */
EXTERN int      to_front,	/* distances from eqsource to front ... */
                to_back;	/* ... and back.                        */

EXTERN edgelist ael;		/* global active edge list              */
EXTERN ylinedata yline;		/* global next yline                    */

EXTERN cnftype *linecnf;	/* cnf vectors of  ACAD colors         */
EXTERN cnftype  metalcnf;	/* metal layers are marked here.       */
EXTERN cnftype  isocnf;		/* and iso. layers are marked here.    */

EXTERN layerpar *layinfo;	/* information about phis. layers   */
EXTERN termpar *terminfo;	/*  ---//-----      terminals       */
EXTERN byte    *iteaitrm;	/* encode itea to itrm numbers      */
EXTERN roundcnf _HUGE *configs;	/* table for config info storing    */
EXTERN unsigned cnfarrlen;	/* and its length.                  */
EXTERN float    currunit;
EXTERN float    unitsize;	/* one LM grid unit in um.          */
EXTERN float    bulklambda;	/* lambda to cut-off metal edges    */
EXTERN float    Coeff3D0;
EXTERN float    Coeff3D1;
EXTERN float    Coeff3D2;
EXTERN byte     nlay;
EXTERN byte     ncolors;
EXTERN byte     ntrm;
EXTERN byte     ntea;
EXTERN byte     nout;
EXTERN unsigned curreq;
EXTERN unsigned nnz;
EXTERN unsigned lastvar;
EXTERN byte     kmax, kmin;
EXTERN coord    y;		/* y coord corresponded to wavesource   */

EXTERN SpMatrixP A, B, C, D;

EXTERN char    *laynames[NLAYMAX + 1]; /* CAD layers table */

#if	defined(BGIGRAPH) || defined(PSGRAPH)
#define initgrout()	_initgrout()
#define show_ael()	_show_ael()
/* #    define show_pattern(x)  _show_pattern(x) */
#define show_var(v,c)	_show_var(v,c)
#define closegrout()	_closegrout()
#else
#define initgrout()
#define show_ael()
#define show_pattern(x)
#define show_var(v,c)
#define closegrout()
#endif

#ifndef NDUMP
#define dump_rcfkey()	_dump_rcfkey()
#define	dump_rcf(x)	_dump_rcf(x)
#define	dump_linecnf(f)	_dump_linecnf(f)
#define	dump_layerpar(f) _dump_layerpar(f)
#else
#define dump_rcfkey()
#define	dump_rcf(x)
#define	dump_linecnf(f)
#define	dump_layerpar(f)
#endif
