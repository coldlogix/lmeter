%{

#include<stdio.h>
#include<stdlib.h>
#include"misc.h"
#include"dxcview.h"

#define	YYSTYPE	float

#include"dxc_tab.h"

%}

%%

"$ENTITIES"		return	ENTITIES;
"$EOF"			return	ENDOFFILE;
"$POLYLINE"		return	POLYLINE;
[\+\-]?[0-9]*"."?[0-9]*	{	yylval=atof(yytext);
				return	FLOAT;
			}
[ \t\n]*		/*	Skip blanks	*/
.			error("Lexical error");

%%

#include<ctype.h>
getlayname()
{	char	c;
	int	p=0;
	allocateN(layname,NAMELEN);
	while	(isspace(c=input()));
	unput(c);
	while	(!isspace(c=input()))
		if	(p<NAMELEN-1)	layname[p++]=c;
		else	error("Too long 'layname'");
	layname[p]='\0';
	unput(c);
	return	0;
}

