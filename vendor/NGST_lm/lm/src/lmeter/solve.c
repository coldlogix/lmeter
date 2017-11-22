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

#include"lmeter.h"
#include"head_lm.h"

/*
 *    Function:       out_trmcur
 *      Definition:     Write out terminal current in the suitable format.
 */
static FILE    *curstxt = NULL;
out_trmcur (int iout, int itea, double trmcur)
{
/*      if      (curstxt==NULL) fileopen(curstxt,"curs.txt","w");
 *    fprintf (curstxt,"%4d\t%4d\t%16.8lf\n",iteaitrm[iout+1],iteaitrm[itea+1],trmcur);
 *      fflush  (curstxt);
 */ fprintf (stdout, "%4d\t%4d\t%16.8lf\n", iteaitrm[iout + 1], iteaitrm[itea + 1], trmcur);
    return 0;
}				/*      out_trmcur      */
/*
 *    Table to encode N to initial Threshold.
 */
struct
{
    XEQType         N;
    FloatType       Threshold;
}
Thrs[] =
{
  { 25000, 0.00000000001},
  { 50000, 0.0000000001},
  {100000, 0.000000001},
  {200000, 0.00000001},
  {400000, 0.0000001}
};
/*
 *    Function:       solve
 *      Definition:     Solve AX=B, compute CX-D, print it out.
 */
#ifdef SHOWFLOW
FILE *picture = NULL;
int    phase_point(int x, int y, int z, double value);
#endif
solve (void)
{
    XEQType         i, N = A->NCols;
    int             iout, itea;
    XEQType        *Perm, *InvP;
    SpMatrixP       L, currents;
    XEQType         TermNNZ = 0;

    allocateN (Perm, N);
    allocateN (InvP, N);
    NestedDissections (A, Perm, InvP);
    message ("LM: Variables were permuted");
    for (i = 0; i < sizeof (Thrs) / sizeof (Thrs[0]); i++)
    {
	Threshold = Thrs[i].Threshold;
	if (N <= Thrs[i].N)
	    break;
    }				/*      for     */
    InitThreshold = 2 * Threshold;
    sprintf (strbuff, "N=%lu, current Threshold is %lf", (long unsigned) N, (double) Threshold);
    message (strbuff);
    L = Factorize (A, Perm, InvP);
    message ("LM: Matrix was numerically factorized");
#ifdef	STAT_COUNT
    fprintf (stderr, "L_NNZ=%lu\tL_NACTCOL=%lu\n", L_NNZ, L_NACTCOL);
#endif
    Epsilon = 1e-10 ;
    for (itea = 0; itea < ntea; itea++)
	if (C->Cols[itea].NNZ > TermNNZ)
	    TermNNZ = C->Cols[itea].NNZ;
    Epsilon /= (double) TermNNZ;
    IterativeSolve (A, L, B, Perm, InvP);
    SpMatrixFree (A);
    SpMatrixFree (L);
    free (Perm);
    free (InvP);
    message ("LM: Systems were solved");
    currents = SpMatrixDecrement (SpMatrixMultiply (C, B), D);
    for (iout = 0; iout < nout; iout++)
	for (itea = 0; itea < ntea; itea++)
	    out_trmcur (iout, itea, currents->Cols[iout].Coeffs[itea]);
    message ("LM: Solution complete");

#ifdef SHOWFLOW
    for (iout = 0; iout < nout; iout++)
      {
	for (i = 0; i < N; i++)
	  phase_point((int) XYZ->Cols[0].Coeffs[i],
		      (int) XYZ->Cols[1].Coeffs[i],
		      (int) XYZ->Cols[2].Coeffs[i],
		      (double) B->Cols[iout].Coeffs[i]
		      );
	phase_newpage();
      }
#endif
    return 0;
}				/*      solve   */

#ifdef SHOWFLOW

#define PAPERX 612.0
#define PAPERY 828.0
#define	XOFFSET 50.0
#define	YOFFSET 50.0

static int prev_x=-1, prev_y=-1;
static double color[3];

phase_point(int x, int y, int z, double value)
{
  time_t now = time (NULL);
  if (picture==NULL)
    {
      picture = fopen("flow.ps", "w");
      fprintf (picture, "%s\n", "%!PS-Adobe-3.0 EPSF-3.0");
      fprintf (picture, "%s\n", "%%Creator: LMeter");
      fprintf (picture, "%s %g %g %g %g\n", "%%BoundingBox:",
	       0.0, 0.0,
	       PAPERX, PAPERY);
      fprintf (picture,"%s\n", 
	       "/p { % x y R G B -> ; show value at xy point \n "
	       "setrgbcolor \n" 
	       "moveto -0.5 -0.5 rmoveto 0 1 rlineto 1 0 rlineto "
	       "0 -1 rlineto -1 0 rlineto fill} def\n" 
	       );
      phase_newpage();
    }  
  assert(z < 3);
  if (x==prev_x && y==prev_y) /* same point, new z */
    color[z] = value;
  else 
    {
      if (prev_x!=-1 && prev_y!=-1) /* show point */
	{
	  fprintf(picture, "%i %i %lf %lf %lf p\n", 
		  x,y, color[0], color[2], color[1]);
	}
      color [0] = color [1] = color [2] = 0.0;
      prev_x = x; prev_y = y;
      color[z] = value;
    }
}

phase_newpage()
{
  static int firstpage = 1;
  float scale = MIN ((PAPERX - 2 * XOFFSET) / xmax / sqrt (2), 
		     (PAPERY - 2 * YOFFSET) / ymax / sqrt (2));
 
  prev_x = prev_y = -1;

  if (firstpage == 0)
 	fprintf(picture, "showpage\n");
  else  firstpage = 0;
  fprintf (picture, "%g %g translate\n", XOFFSET, YOFFSET);
  fprintf (picture, "0 setlinewidth\n");
  fprintf (picture, "%g %g scale\n", scale, scale);
  fprintf (picture, "%g 0 translate\n", ymax / sqrt (2));
}
      
#endif
