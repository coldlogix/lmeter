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
#include"head_gm.h"
#include"avltree.h"
#include<string.h>
#include<limits.h>
#include<ctype.h>

FILE           *infile;

int             currlay;	/* layer and ite for last readed edge */
int             currite;
xypoint         a;		/* two points for last readed edge    */
xypoint         b;		/* we read those four vars in EDGET_GM*/

/*
 *    Main structure to work:  y-tree, info - x-tree,
 *      its info - configs in all dirs.
 *      Here we use 4-item configs, for points in yx-tree they show
 *      the edge changes in four dirs (0-3) while scrolling through
 *      the point, for points in active edge list (ael) they show
 *      currently existing edges in four upper-right dirs.
 *      All points in yx-tree will be sent to LM with computed 8-dirs
 */
tree            yx = NULL;
extern tree     newyx;		/* news from crosser    */
linlist        *llyx;

int             dirtab[3][3] =
	/* dy    -1 0 1         dx */
{
    {1, 0, 3},			/* -1 */
    {2, 0, 2},			/*  0 */
    {3, 0, 1},			/*  1 */
};				/* what dir should have edge with dx,dy */

#define	dirind(x)	( (x>0)?2:((x<0)?0:1) )

void            intersect_edges ();	/* those from CROSS_GM                  */
void            add_news (tree newitem);
void            initparams_gm ();
void            start_edgeget ();
int             more_edge ();
void            get_edge ();
void            insert_edge ();
void            shift_yx ();
void            scroll_by_lines (void action ());
void            write_lm ();

#ifdef	PSGRAPH
void            show_aelgm ();
void            show_init ();

#endif

void 
main_gm ()
{
    int             i, j;

    initparams_gm ();
    message ("GM: parameters accepted.");
    xmax = ymax = INT_MIN;
    xmin = ymin = INT_MAX;
    start_edgeget ();
    while (more_edge ())
    {
	get_edge ();
	insert_edge ();
	if (xmax < b.x)
	    xmax = b.x;
	if (xmin > b.x)
	    xmin = b.x;
	if (ymax < b.y)
	    ymax = b.y;
	if (ymin > b.y)
	    ymin = b.y;
    }
    message ("GM: DXC file accepted.");
    sprintf (strbuff, "GM: Size: %g - %g X %g - %g",
	  gridsize * xmin, gridsize * xmax, gridsize * ymin, gridsize * ymax);
    message (strbuff);
/*      if(xmin<0 || ymin<0)    error("Please,move your layout to upper-left quadrant");
 */ shift_yx ();
    xmax -= xmin;
    ymax -= ymin;
    xmax++;
    ymax++;
    message ("GM: Layout was moved to 0,0");
    /* Now 0<=x<xmax && 0<=y<ymax   */
    scroll_by_lines (intersect_edges);
    message ("GM: lines intersected.");
#ifdef PSGRAPH
    show_init ();
    scroll_by_lines (show_aelgm);
#endif
    linecross ();
    apply_ascend (newyx, add_news);
    outdmp ();
    llyx = linearise_tree (yx);
    for (i = 0; i < llyx->len; i++)
	llyx->data[i].info = linearise_tree (llyx->data[i].info);
    write_lm ();
    /* and delete linarised trees   */
    for (i = 0; i < llyx->len; i++)
    {
	linlist        *llx = llyx->data[i].info;

	for (j = 0; j < llx->len; j++)
	    Free (llx->data[j].info);
	Free (llyx->data[i].info);
    }
    Free (llyx->data);
    Free (llyx);
    message ("GM: TOP file written.");

}

void 
initparams_gm ()
{
    char           *buf, *inname;
    int             i, n;

    inname = getpar ("DXCNAME");
    if (inname)
    {
	i = 0;
	n = strlen (inname);
	while (!isspace (inname[i]) && inname[i] != '\0')
	    i++;
	inname[i] = '\0';	/*      extract valid filename  */
	fileopen (infile, inname, "r");
    }
    else
	infile = stdin;
    GETPAR (buf, "GRIDSIZE", 1);
    gridsize = atof (buf);
    if (getpar ("EXIST45"))
	gridsize /= 2.0;
    GETPAR (buf, "NLAYERS", 1);
    ncolors = atoi (buf);
    if (ncolors == 0 || ncolors > NLAYMAX)
	error ("Bad number of layers");
    GETPAR (buf, "LAYNAMES", 1);
    for (i = 0; i < ncolors; i++)
	if ((laynames[i] = strtok ((i == 0) ? buf : NULL, SEPARATES)) == NULL)
	    error ("Can't read LAYNAMES");
#ifdef	PSGRAPH
    {
	extern int      laycolors[NLAYMAX];

	i = 0;
	if (buf = getpar ("LAYCOLORS"))
	{
	    buf = strtok (buf, SEPARATES);
	    while (buf)
	    {
		laycolors[i++] = atoi (buf);
		buf = strtok (NULL, SEPARATES);
	    }
	}
	while (i < NLAYMAX)
	    laycolors[i++] = 16;
    }
#endif

}

void 
insert_edge ()
{
    coord           dx, dy;
    byte            dir;	/* direction from a to b                */
    extern void     insdir (tree p, byte dir);
    tree           *xtreeptr;

    dx = a.x - b.x;
    dy = a.y - b.y;
    if (dx || dy)		/* not the same point                   */
    {
	if (dx && dy && abs (dx) != abs (dy))
	{
	    sprintf (strbuff, "At point %-6d %-6d non-45 edge", a.x, a.y);
	    error (strbuff);
	}
	dir = dirtab[dirind (dx)][dirind (dy)];
	xtreeptr = (tree *) & (search (a.y, yx)->info);
	insdir (_search (a.x, xtreeptr, &avlgrew), dir);
	xtreeptr = (tree *) & (search (b.y, yx)->info);
	insdir (_search (b.x, xtreeptr, &avlgrew), dir);
    }

}

/*      Add one more edge with dir,currlay,currite to point p->info     */

void 
insdir (tree p, byte dir)
{
    dcnf           *info;

    assert (p);
    assert (dir < 4);

    if (currlay == -1) /* unknown layer, must be ignored before! */
	error("Internal error: inserting edge in non-existing layer");

    if (p->info == NULL)
	allocate (p->info, 4, sizeof (dcnf));
    info = p->info;
    if (currlay == 0 && currite)
	info[dir].tcnf ^= (POW2 (currite));
    else
	info[dir].ecnf ^= (POW2 (currlay));	/* also TERM ite==0     */
    
}
/*
 *    Additions to AVLTREE to shift XY-tree by xmin, ymin.
 */
void 
tree_shift (tree p)
{
    if (p == NULL)
	return;
    p->key -= xmin;
    tree_shift (p->left);
    tree_shift (p->right);
}
void 
yx_tree_shift (tree p)
{
    if (p == NULL)
	return;
    p->key -= ymin;
    yx_tree_shift (p->left);
    yx_tree_shift (p->right);
    tree_shift ((tree) p->info);
}
void 
shift_yx ()
{
    yx_tree_shift (yx);
}

double 
getrealx (coord x)
{
    return (double) (x + xmin) * gridsize;
}
double 
getrealy (coord y)
{
    return (double) (y + ymin) * gridsize;
}
