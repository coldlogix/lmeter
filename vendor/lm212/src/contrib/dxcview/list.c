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
 *	Module to deal with (untyped!) lists in a LISP-like way ...
\*/
#include"list.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
/*\
 * 	Function:	cons
 *	Definition:	Return a pointer to a new node
 *	with given 'car' & 'cdr' or NULL if can't do it.
\*/
node	*cons	(void *car,void *cdr)
{	node	*nd=malloc(sizeof(node));
	if	(nd)
	{    	nd->car=car;
		nd->cdr=cdr;
		return	nd;
	}	/*	then	*/
	return NULL;
}	/*	cons	*/

/*\
 * 	Function:	assoc
 *	Definition:	Search in 'alist' node with '*(nd->car->car)==key',
 *	return 'nd->car' or NULL if this key doesn't exist.
\*/
node	*assoc	(void *key,node *alist)
{       foreach
	(	nd,alist,
		if	(key==((node*)nd->car)->car)
			return	nd->car;
	);
	return NULL;
}	/*	assoc	*/
/*\
 * 	Function:	last
 *	Definition:     Return the last element in 'list'.
\*/
void	*last	(node *list)
{	foreach
	(	tmp,list,
		if	(tmp->cdr==NULL)	return	tmp->car;
	);
	return NULL;
}	/*	last	*/

/*\
 * 	Function:	length
 *	Definition:	Return the number of elements in 'list'.
\*/
int	length	( node *list )
{	int	n=0;
	foreach	(tmp,list,n++);
	return	n;
}	/*	length	*/
/*\
 * 	Function:	snoc
 *	Definition:	Reverse 'cons' - destroy the head node,
 *	saving 'car' & 'cdr' at 'pcar' & 'pcdr'.
\*/
int	snoc	(node *list, void **pcar, void **pcdr)
{       if	(list)
	{       *pcar=list->car;
		*pcdr=list->cdr;
		free(list);
	}	/*	then	*/
	return 0;
}	/*	snoc	*/

