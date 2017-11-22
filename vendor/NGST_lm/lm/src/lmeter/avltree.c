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


/*      AVL tree implementation /see N.Wirth "Mir" '85,p.250    /       */

#include"avltree.h"
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#define	allocate(ptr,num,size)					   	\
	{	ptr=(void *)calloc((size_t)(num),(size_t)(size));       \
		if(ptr==NULL)                                           \
		{	fprintf(stderr,"Can't allocate num:%6u size:%6u\n",(num),(size));	\
			fprintf(stderr,"File:%s\tLine:%6u\n",__FILE__,__LINE__); 		\
			exit(1);                                        \
		}                                                       \
	}

tree
_search (keys x, tree * tptr, int *hp)
{
    tree            p1, p2, p, finded;
    int             h;

    assert (tptr);
    assert (hp);

    p = *tptr;
    *hp = 0;
    if (p == NULL)		/* empty tree,insert key        */
    {
	*hp = 1;		/* heigth changed               */
	allocate (p, 1, sizeof (tnode));
	p->key = x;		/* others=0 after calloc */
	p->left = p->right = NULL;
	p->bal = 0;
	*tptr = p;		/* set new branch       */
	return p;
    }
    if (x < p->key)		/* search in left sub-tree      */
    {
	finded = _search (x, &(p->left), hp);
	if (*hp)		/* grew left branch             */
	{
	    switch (p->bal)
	    {
	    case +1:
		p->bal = 0;
		*hp = 0;
		break;
	    case 0:
		p->bal = -1;
		break;
	    case -1:		/* balance      */
		p1 = p->left;
		if (p1->bal == -1)	/* LL   */
		{
		    p->left = p1->right;
		    p1->right = p;
		    p->bal = 0;
		    p = p1;
		}
		else
		    /* LR     */
		{
		    p2 = p1->right;
		    p1->right = p2->left;
		    p2->left = p1;
		    p->left = p2->right;
		    p2->right = p;
		    p->bal = (p2->bal == -1) ? +1 : 0;
		    p1->bal = (p2->bal == +1) ? -1 : 0;
		    p = p2;
		}
		p->bal = 0;
		*hp = 0;
		break;
	    default:
		fprintf (stderr, "Internal error\n");
		exit (1);
	    }
	}
    }
    else if (x > p->key)	/* search in right sub-tree     */
    {
	finded = _search (x, &(p->right), hp);
	if (*hp)		/* grew right branch            */
	{
	    switch (p->bal)
	    {
	    case -1:
		p->bal = 0;
		*hp = 0;
		break;
	    case 0:
		p->bal = +1;
		break;
	    case +1:		/* balance      */
		p1 = p->right;
		if (p1->bal == +1)	/* RR   */
		{
		    p->right = p1->left;
		    p1->left = p;
		    p->bal = 0;
		    p = p1;
		}
		else
		    /* RL     */
		{
		    p2 = p1->left;
		    p1->left = p2->right;
		    p2->right = p1;
		    p->right = p2->left;
		    p2->left = p;
		    p->bal = (p2->bal == +1) ? -1 : 0;
		    p1->bal = (p2->bal == -1) ? +1 : 0;
		    p = p2;
		}
		p->bal = 0;
		*hp = 0;
		break;
	    default:
		fprintf (stderr, "Internal error\n");
		exit (1);
	    }
	}
    }
    else
	/* x==p->key                      */
    {
	*hp = 0;
	assert (p);
	return (p);
    }
    *tptr = p;
    assert (p);
    assert (finded);
    return (finded);

}

void apply_ascend (tree p, void f (tree t))
{
    tree            pr;

    if (p == NULL)
	return;
    apply_ascend (p->left, f);
    pr = p->right;
    /*      save nessesary pointer if f(p) deletes p        */
    f (p);
    apply_ascend (pr, f);
}

int
count_all (tree p)
{
    if (p == NULL)
	return 0;
    else
	return 1 + count_all (p->left) + count_all (p->right);
}

void
pt (tree t, int offs)
{
    int             i;

    if (t == NULL)
	return;
    if (t->left)
	pt (t->left, offs + 1);
    for (i = 0; i < offs; i++)
	putchar ('\t');
    printf ("%2d:%-5d\n", (int) t->bal, (int) t->key);
    if (t->right)
	pt (t->right, offs + 1);
}

void
printnode (tree t)
{
    printf ("Ptr:%-10pKey:%-5dBal:%-5dLeft:%-10hpRight:%-10hpInfo:%-10hp\n",
	    t, (int) t->key, (int) t->bal, t->left, t->right, t->info);
}

void
printtree (tree t)
{
    printf ("------- ------- ------- ------- ------- ------- -------\n");
    pt (t, 0);
    printf ("------- ------- ------- ------- ------- ------- -------\n");
}

/*********/

static int      len;
static linlist *llp;
void            add_del (tree p);

linlist        *
linearise_tree (tree p)
{
    allocate (llp, 1, sizeof (*llp));
    llp->len = count_all (p);
    if (llp->len)
    {
	allocate (llp->data, llp->len, sizeof (*(llp->data)));
	len = 0;
	apply_ascend (p->left, add_del);
	llp->data[len].key = p->key;
	llp->data[len].info = p->info;
	len++;
	apply_ascend (p->right, add_del);
	/*      not to del not allocated root p */
	assert (len == llp->len);
    }
    else
	llp->data = NULL;
    return llp;
}

void
add_del (tree p)
{
    llp->data[len].key = p->key;
    llp->data[len].info = p->info;
    len++;
    free (p);
}
