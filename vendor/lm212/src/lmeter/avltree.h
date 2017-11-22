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

/*      AVL tree header /see N.Wirth "Mir" '85,p.250    /               */

typedef int     keys;		/* keys to access trees         */

typedef struct node
{
    keys            key;
    struct node    *left;
    struct node    *right;
    int             bal;	/* -1,0,+1      */
    void           *info;	/* user info    */
}
tnode;

typedef tnode  *tree;		/* tree root,type of left,right */

static int      avlgrew;

/* Local for a file which includes this header, it is used in macros */

/************************************************************************/
/*      Procedure to find (or insert new) node in tree *tptr by key x.  */
/*      Returns pointer to node,if allocated new then info==NULL        */
/*      if(*tptr==NULL) allocates it by first insertion                 */
/************************************************************************/

tree            _search (keys x, tree * tptr, int *hp);

#define	search(x,p)	(_search((x),&(p),&avlgrew))

/************************************************************************/
/*      Procedure to apply function f to all members of tree t          */
/*      in ascending (by key) order                                     */
/************************************************************************/

void             apply_ascend (tree t, void f (tree t));

/************************************************************************/
/*      Procedure to count all members of tree t                        */
/************************************************************************/

int             count_all (tree t);

/************************************************************************/
/*      Two dumping procedures                                          */
/************************************************************************/

void            printnode (tree t);
void            printtree (tree t);

/************************************************************************/
/*      Procedures to work with linlists                                */
/************************************************************************/

typedef struct
{
    keys            key;
    void           *info;
}
lnode;

typedef struct
{
    int             len;
    lnode          *data;
}
linlist;

/************************************************************************/
/*      Procedure to convert tree to  linlists (tree will be deleted)   */
/************************************************************************/

linlist        *linearise_tree (tree t);
