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


typedef struct
{
    coord           x, y;
}
xypoint;

typedef struct
{
    cnftype         tcnf, ecnf;	/* term and edge cnf    */
}
dcnf;				/* of one direct. (0-7) */

extern char    *laynames[NLAYMAX + 1]; /* CAD layers table from head_lm.h */

#ifdef	NEEDLCROSS
void            intersect_edge_line ();

#define	linecross()	scroll_by_lines(intersect_edge_line);
#else
#define	linecross()
#endif

#ifdef	NEEDDMP
void            dumpyx (tree t);

#define	outdmp()	apply_ascend(yx,dumpyx);
#else
#define	outdmp()
#endif
