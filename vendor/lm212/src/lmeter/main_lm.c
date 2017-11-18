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
 *    Main module implements the area pipeline and its main cycle.
 */

#include"lmeter.h"
#include"head_lm.h"

extern byte     cnflevel;	/* level to compute round cnf:0,1,2,3   */
byte            itea;
byte            outterfile;

main_lm ()
{
    byte            more_dots;
    byte            t, i;

    initparams_lm ();
    message ("LM: parameters accepted.");
    init_pipeline ();
    message ("LM: work area initialized.");

    build_eqs_init ();
    A = SpMatrixCreate (2500);
    B = SpMatrixCreate (nout);
    C = SpMatrixCreate (ntea);
    D = SpMatrixCreate (nout);
    for (i = 0; i < nout; i++)
	SpVectorStartInsertion (B->Cols + i);
    for (i = 0; i < ntea; i++)
	SpVectorStartInsertion (C->Cols + i);
    for (i = 0; i < nout; i++)
	SpVectorStartInsertion (D->Cols + i);

    back = front = wavesource;
    to_front = front - eqsource;
    to_back = back - eqsource;
    make_null_cnf ();
    readline ();		/* read first line as next              */
    if (!(more_dots = yline.isnotlast))
	error ("only one y-line");
    y = yline.y;		/* accept at next step                  */
    do
    {
	if (y == yline.y)
	{
	    dotter_1d (ael);
	    resort_ael ();	/* on yline ael can be unsorted */
	    change_ael ();	/* accept new yline             */
	    cnflevel = 3;
	    fill_rcf ();	/* use oldael,ael               */
	    remove_hor ();
	    dotter_2d (yline.el);
	    shift_ael ();	/* shift by 1                   */
	    if (more_dots = yline.isnotlast)
		readline ();
	}
	else if (more_dots)
	{
	    dotter_1d (ael);
	    fill_rcf ();
	    shift_ael ();
	}
	else
	{
	    back++;
	    to_back++;
	}
	if (front >= eqsource)
	    build_eqs ();
	if (front >= endinterf && back <= endinterf)
	    num_vars ();
	if (front >= endarea_2d)
	    free_rcf ();
	front++;
	to_front++;
	y++;
	shift ();		/* pipeline     */
    }
    while (back < eqsource);

#ifdef PSGRAPH
    show_end ();
#endif

    while (back++ <= endarea_2d)
	free_rcf ();
    free_pipeline ();
    message ("LM: main pipeline finished ...");
    sprintf (strbuff, "LM: numvars %6u", lastvar);
    message (strbuff);

    build_eqs_finish ();
    SpMatrixResize (A, lastvar);
    /*      SpMatrixPrint("matrixA.txt",A); */
#ifndef	NSYMM
    SpMatrixCheckSymm (A);
#endif
    for (i = 0; i < nout; i++)
	SpVectorEndInsertion (B->Cols + i);
    for (i = 0; i < ntea; i++)
	SpVectorEndInsertion (C->Cols + i);
    for (i = 0; i < nout; i++)
	SpVectorEndInsertion (D->Cols + i);
    solve ();

    return 0;

}				/*    end of main */

unsigned        pipelen;

init_pipeline ()
{
    unsigned        stepmax = (POW2 (kmax));
    unsigned        l;

    pipelen = 2 * 3 * stepmax + 1 + 2 * stepmax + 1;
    /* interference area and pattern build area */
    allocate (area_2d, pipelen, sizeof (point *));
    for (l = 0; l < pipelen; l++)
	allocateN (area_2d[l], xmax);
    wavesource = area_2d + 3 * stepmax;
    endinterf = area_2d + 6 * stepmax;
    eqsource = endinterf + stepmax + 1;
    endarea_2d = area_2d + pipelen - 1;

}				/*      end of init_pipeline */

free_pipeline ()
{
    unsigned        l;

    for (l = 0; l < pipelen; l++)
	Free (area_2d[l]);
    Free (area_2d);
}				/*      end of free_pipeline */

shift ()
{
    unsigned        l = pipelen - 1;
    coord           x;
    area_1d         sav_area_2d_l = area_2d[l];

    do
	area_2d[l] = area_2d[l - 1];
    while (--l);
    area_2d[0] = sav_area_2d_l;
/*      for     (x=0;x<xmax;x++)
   {    point   *pnt=area_2d[0]+x;
   pnt->varnum=0;
   pnt->isused=0;
   pnt->incnf=0;
   }
 */ memset (area_2d[0], 0x00, sizeof (point) * xmax);

}				/*    end of shift */

write_bits (FILE * f, cnftype u)
{
    cnftype         bit = 1;
    byte            pos;

    for (pos = 0; pos < 8 * sizeof (u); pos++, bit <<= 1)
	fputc ((u & bit) ? '1' : '0', f);
    fputc ('\n', f);
}
