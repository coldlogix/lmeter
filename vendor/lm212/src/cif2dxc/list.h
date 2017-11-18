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


typedef	struct	node	
{	void	*car;
	void	*cdr;
}	node;

node	*cons	(void *car,void *cdr);
node	*assoc	(void *key,node *alist);
node	*reverse(node *list);
int	length	(node *list);
void	*last	(node *list);
int	snoc	(node *list, void **pcar, void **pcdr);

#define	foreach(name, list, operator)					\
	do                                                              \
	{       node	*name;                                          \
		for	(name=list;name!=NULL;name=name->cdr)           \
			operator;                                       \
	}	while(0)
