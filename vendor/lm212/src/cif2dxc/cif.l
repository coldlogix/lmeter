%{

#include<stdlib.h>
#include<stdio.h>
#include"cifgeom.h"
#include"cif.tab.h"

int	lineno=1;

#define	LINECNT	{int i; for(i=0;i<yyleng;i++) if (yytext[i]=='\n') lineno++; }

%}

blank		[^0-9A-Za-z\-();]

%%

"P"			return	P_TOK;
"B"			return	B_TOK;
"R"			return	R_TOK;
"W"			return	W_TOK;
"L"{blank}*		{       /*	Read 'shortname' (up to 4 chars)*/
#				define	SNAMELEN	4
				char	*newname;
				int	pos=0;
				char	c;
				LINECNT;
				allocateN(newname,SNAMELEN+1);
				c=input();
				while	(	pos<SNAMELEN
					&& 	(c>='0' && c<='9' 
 						|| c>='A' && c<='Z'
						|| c>='a' && c<='z')
					)
				{	newname[pos++]=c;
					c=input();
				}	/*	while	*/
				if	(c==EOF)	return	0;
				else
				{       unput(c);
					newname[pos]='\0';
					layname=newname;
					return  L_TOK;
				}	/*	else	*/
			}
"D"{blank}*"S"		{	LINECNT;	return	DS_TOK; }
"D"{blank}*"F"		{	LINECNT;	return	DF_TOK; }
"D"{blank}*"D"		{	LINECNT;	return	DD_TOK; }
"C"						return	C_TOK;
"E"{blank}*		{	LINECNT;	return	E_TOK;  }
"T"						return	T_TOK;
"M"{blank}*"X"		{	LINECNT;	return	MX_TOK; }
"M"{blank}*"Y"		{	LINECNT;	return	MY_TOK; }
";"						return	SEMI_TOK;
"("						return	skip_comment();
("-"?)[0-9]+		{	if	((yylval.intval=atoi(yytext))<0)
					return	NINTEGER_TOK;
				else	return	PINTEGER_TOK;
			}
{blank}*		{	LINECNT;	}

.			return  UNK_TOK;

%%

int	skip_comment()
{	int	depth=1;
	do	switch	(input())
		{	case	EOF:	return	0;
			case	'(':	depth++;	break;
			case	')':	if	(--depth==0)
						return	COMMENTTEXT_TOK;
			case	'\n':	lineno++;	break;
		}	/*	switch	*/
	while	(1);
	return	0;	/*	never used.	*/
}	/*	skip_comment	*/

int	read_usertext()
{	char	c;
	int	i=0;
	while	((c=input())!=EOF)
	{	if	(c==';')
		{	unput(c);
			userexttext[i]='\0';
			return	0;
		}
		else	if	(c=='\n')	lineno++;
		userexttext[i++]=c;
		if	(i==USEREXTLEN)	error("Too long user extension");
	}	/*	while	*/
	userexttext[i]='\0';
	return	0;
}	/*	read_usertext	*/

int	skip_sep()
{	char	c;
	while	((c=input())!=EOF)
		if	(	c>='0' && c<='9' 	||
				c=='-'			||
				c=='('			||
				c==')'			||
				c==';'
			)
		{	unput(c);
			return	0;
		}
		else	if	(c=='\n')	lineno++;
	return	0;
}

#ifndef yywrap
int	yywrap()
{
	return 1;
}
#endif
