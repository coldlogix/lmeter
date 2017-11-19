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
 *	File to collect my own interfaces to C library functions
 *	together with some other unsorted stuff. If you find it necessary
 *	to add here something new - do it !
 *							Paul Bunyk.
\*/

#ifndef	___MISC_H
#define	___MISC_H

#include<errno.h>

#define	STDMESS	stderr

#define	error(str)							\
	do {	fprintf(STDMESS,"\"%s\":\t%4u: ERROR:\t%s\n",		\
		__FILE__,__LINE__,str);					\
		if	(errno)	perror("DUE TO");			\
		exit(1);						\
	}	while(0)

#define	message(str)							\
	do {	fprintf(STDMESS,"\"%s\":\t%4u: MESSAGE:\t%s\n",		\
		__FILE__,__LINE__,(str));				\
	}	while(0)

#define	allocate(ptr,num,size)						\
	do {	ptr=(void *)calloc((size_t)(num),(size_t)(size));	\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
	sprintf(errs,"Can't allocate Num:%6u Size:%6u",(num),(size));	\
			error(errs);                                    \
		}                                                       \
	}	while(0)

#define	allocateN(ptr,num)						\
	do {	ptr=(void *)calloc((size_t)(num),sizeof(*ptr));		\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
			sprintf(errs,"Can't allocate Num:%6u Size:%6u",	\
				(num),sizeof(*ptr));			\
			error(errs);                                    \
		}                                                       \
	}	while(0)

#define	reallocate(ptr,num,size)					\
	do {	ptr=(void *)realloc((ptr),(size_t)(num)*(size));	\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
	sprintf(errs,"Can't allocate Num:%6u Size:%6u",(num),(size));	\
			error(errs);                                    \
		}                                                       \
	}	while(0)

#define	reallocateN(ptr,num)						\
	do {	ptr=(void *)realloc((ptr),(size_t)(num)*sizeof(*ptr));	\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
			sprintf(errs,"Can't allocate Num:%6u Size:%6u",	\
				(num),sizeof(*ptr));			\
			error(errs);                                    \
		}                                                       \
	}	while(0)

#define	fileopen(fptr,nm,md)						\
	if( ((fptr)=fopen((nm),(md)))==NULL )				\
	{	char	errs[256];					\
		sprintf	(errs,"Can't open file name %s mode %s.",	\
			(nm),(md));					\
		error(errs);						\
	}

#define	fileread(obj,sz,nm,fl)						\
	if	( fread((obj),(sz),(nm),(fl))!=(nm) )			\
	{	char	errs[256];					\
		sprintf(errs,"Can't read file %s.",#fl);		\
		error(errs);						\
	}
	
#define	filewrite(obj,sz,nm,fl)						\
	if	( fwrite((obj),(sz),(nm),(fl))!=(nm) )			\
	{	char	errs[256];					\
		sprintf(errs,"Can't write to file %s.",#fl);		\
		error(errs);						\
	}

#define	fileseek(fl,ln,md)						\
	if	( fseek((fl),(ln),(md)) )				\
	{	char	errs[256];					\
		sprintf(errs,"Can't seek file %s.",#fl);		\
		error(errs);						\
	}

#ifdef	min
#	undef	min
#	undef	max
#endif

#define	min(a,b)	(((a)<(b))?(a):(b))
#define	max(a,b)	(((a)>(b))?(a):(b))
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#define	MAX(a,b)	(((a)>(b))?(a):(b))

#define	POW2(k)		(1<<(k))

#define	Free(p)		if	(p)	free(p)

#ifdef	MAIN
#	define	EXTERN
#else
#	define	EXTERN	extern
#endif

#endif