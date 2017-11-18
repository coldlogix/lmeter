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
 *    File to collect my own interfaces to C library functions
 *      together with some other unsorted stuff. If you find it necessary
 *      to add here something new - do it !
 *                                                      Paul Bunyk.
 */

#ifndef	___MISC_H
#define	___MISC_H

#include<errno.h>
#include<string.h>

#if	defined(__TURBOC__)	&&	!defined(NDEBUG)
#define	CHECKFORSEGMENT(size)					\
	if	((long)size>0xFFFF)					\
		error("Can't allocate more then segment");
#else
#define	CHECKFORSEGMENT(void)
#endif

#define	error(str)							\
	{	fprintf(stderr,"ERROR: %s: %s (%s:%d)\n",		\
		(str),((errno)?strerror(errno):""), __FILE__,__LINE__);	\
		exit(1);						\
	}

#define	message(str)							\
	do {	fprintf(stderr,"%s (%s:%d)\n",			        \
		(str),__FILE__,__LINE__);				\
	}	while(0)

#define	warning(str)							\
	{	fprintf(stderr,"WARNING: %s: %s (%s:%d)\n",		\
		(str),((errno)?strerror(errno):""), __FILE__,__LINE__);	\
	}

#define	allocate(ptr,num,size)						\
	{    CHECKFORSEGMENT((num)*(size));				\
		ptr=(void *)calloc((size_t)(num),(size_t)(size));	\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
	sprintf(errs,"Can't allocate Num:%6u Size:%6u",(num),(size));	\
			error(errs);                                    \
		}                                                       \
	}

#define	allocateN(ptr,num)						\
	{    CHECKFORSEGMENT((num)*sizeof(*ptr));			\
		ptr=(void *)calloc((size_t)(num),sizeof(*ptr));		\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
			sprintf(errs,"Can't allocate Num:%6u Size:%6u",	\
				(num),sizeof(*ptr));			\
			error(errs);                                    \
		}                                                       \
	}

#define	reallocate(ptr,num,size)					\
	{	CHECKFORSEGMENT((num)*(size));				\
		ptr=(void *)realloc((ptr),(size_t)(num)*(size));	\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
	sprintf(errs,"Can't allocate Num:%6u Size:%6u",(num),(size));	\
			error(errs);                                    \
		}                                                       \
	}

#define	reallocateN(ptr,num)						\
	{	CHECKFORSEGMENT((num)*(sizeof(*ptr)));			\
		ptr=(void *)realloc((ptr),(size_t)(num)*sizeof(*ptr));	\
		if(ptr==NULL)                                           \
		{       char	errs[256];                              \
			sprintf(errs,"Can't allocate Num:%6u Size:%6u",	\
				(num),sizeof(*ptr));			\
			error(errs);                                    \
		}                                                       \
	}

#define	fileopen(fptr,nm,md)						\
	{if	( ((fptr)=fopen((nm),(md)))==NULL )			\
	{	char	errs[256];					\
		sprintf	(errs,"Can't open file name %s mode %s.",	\
			(nm),(md));					\
		error(errs);						\
	}}

#define	fileread(obj,sz,nm,fl)						\
	{if	( fread((obj),(sz),(nm),(fl))!=(nm) )			\
	{	char	errs[256];					\
		sprintf(errs,"Can't read file %s.",#fl);		\
		error(errs);						\
	}}

#define	filewrite(obj,sz,nm,fl)						\
	{if	( fwrite((obj),(sz),(nm),(fl))!=(nm) )			\
	{	char	errs[256];					\
		sprintf(errs,"Can't write to file %s.",#fl);		\
		error(errs);						\
	}}

#define	fileseek(fl,ln,md)						\
	{if	( fseek((fl),(ln),(md)) )				\
	{	char	errs[256];					\
		sprintf(errs,"Can't seek file %s.",#fl);		\
		error(errs);						\
	}}

#ifdef	min
#undef	min
#undef	max
#endif

#define	min(a,b)	(((a)<(b))?(a):(b))
#define	max(a,b)	(((a)>(b))?(a):(b))
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#define	MAX(a,b)	(((a)>(b))?(a):(b))

/*      Moved to lmeter.h to take into account long long shifts
 *    #define POW2(k)         (1<<(k)) 
 */

#define	Free(p)		if	(p)	free(p)

#ifdef  MAIN
#define  EXTERN
#define  INIT( everything )      = everything
#else
#define  EXTERN  extern
#define  INIT( void )
#endif

#endif
