%{

#include<stdio.h>
#include<stdlib.h>

#include"misc.h"
#include"dxcview.h"

#define	YYSTYPE	float

%}

%token	ENTITIES POLYLINE ENDOFFILE FLOAT

%%

dxc
:	ENTITIES plinelist ENDOFFILE	{	YYACCEPT;	}
;

plinelist
:	/*	Empty	*/
|	plinelist pline
;

pline
:	POLYLINE {getlayname();} FLOAT {thickness=$3;} pointlist
	{       struct	pline	*newpline;
		allocateN(newpline,1);
		newpline->layname=layname;
		newpline->points=points;
		newpline->pointnum=pointnum;
		newpline->thickness=thickness;
		plinelist=cons(newpline,plinelist);
		reallocateN(points,pointnum);
	}
;

pointlist
:	/*	Empty	*/
	{       allocateN(points,MAXPOINTNUM);
		pointnum=0;
	}
|		pointlist FLOAT FLOAT
	{       /*	determine limits	*/
		if	(xmax<$2)	xmax=$2;
		if	(xmin>$2)	xmin=$2;
		if	(ymax<$3)	ymax=$3;
		if	(ymin>$3)	ymin=$3;
		/*	insert to pline		*/
		if	(pointnum<MAXPOINTNUM)
		{	points[pointnum].x=$2;
			points[pointnum].y=$3;
			pointnum++;
		}	/*	then	*/
		else	error("Too many points in pline");
	}
;

%%

yyerror(char	*s)
{	fprintf(stderr,"Syntax error: %s\n",s);
	return	0;
}