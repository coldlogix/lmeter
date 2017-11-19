%{

#include"cifgeom.h"

#define	YYDEBUG	0

    static	char	*savedlayname;
    static	node	*def;
    static	coord	scaling=1.0;
    static      int     rootgeometry = 0;

    extern	int	lineno;	/*	lex scanner's	*/

    extern	int	userprocess();	/*	to process user extensions.	*/

%}

%union
{	
    int		intval;
    point   	*pntptr;
    node	*pntlist;
    geomprim	*primitive;
}

/*	In CIF 'integer' is unsigned, for signed we are using 'sInteger'.
 */
%token	<intval> PINTEGER_TOK NINTEGER_TOK
%type	<intval> integer sInteger

%token	P_TOK B_TOK R_TOK W_TOK L_TOK DS_TOK DF_TOK DD_TOK C_TOK E_TOK T_TOK MX_TOK MY_TOK
%token	COMMENTTEXT_TOK SEMI_TOK
%token	UNK_TOK

%start	cifFile

%type	<pntptr> 	point
%type	<pntlist>       path
%type	<primitive>     primCommand
			polygonCommand
			boxCommand
			roundFlashCommand
			wireCommand
			callCommand
			userExtCommand
%%

cifFile:	commandList	endCommand
{       
    if (!rootgeometry)
    {
	fprintf(stderr,
		"Empty root cell, using symbol 0\n");
	newCTM(); /* next call is with unit CTM */
	root = cons(call2call(0),root);
    }
    
    root = reverse(root);
    fprintf(stderr,
	    "CIF file was processed, total parse errors: %d\n",
	    yynerrs);
    YYACCEPT;
}

commandList:		/*	Empty	*/
|		commandList command SEMI_TOK
|		commandList error SEMI_TOK
{	
    fprintf(stderr,
	    "Error in command in definition of 'root', line %d\n",
	    lineno);
    yyerrok;
}
;

command:	primCommand
{       /*	Here - add to 'root' list. 	*/
    if	($1!=NULL)	/*	not miscCommand	*/
    {	
	if ($1->type != UEXT) /* some real geometry */
	    rootgeometry = 1;
	root=cons($1,root);
    }	/*	then	*/
}
|		defDelCommand
|		defCommand
;

primCommand:	polygonCommand		{	$$=$1;		}
|		boxCommand		{	$$=$1;		}
|		roundFlashCommand       {	$$=$1;		}
|		wireCommand             {	$$=$1;		}
|		callCommand             {	$$=$1;		}
|		miscCommand             {	$$=NULL;	}
|		userExtCommand
;

miscCommand:	layerCommand
|		commentCommand
;

polygonCommand:	P_TOK path
{
    $$=path2pline($2);
}

boxCommand:	B_TOK integer sep integer sep point sep
{
    $$=box2pline(scaling*$2,scaling*$4,$6->x,$6->y,1.0,0.0);
}
|		B_TOK integer sep integer sep point sep point
{
    $$=box2pline(scaling*$2,scaling*$4,$6->x,$6->y,$8->x,$8->y);
}

roundFlashCommand:	R_TOK integer sep point
{
    $$=rflash2pline(scaling*$2,$4->x,$4->y);
}

wireCommand:	W_TOK integer sep path
{
    $$=wire2pline(scaling*$2,$4);
}

callCommand:	C_TOK integer transformation
{
    $$=call2call($2);
}

transformation:	/*	Empty	*/
{
    newCTM();
}
|		transformation transform
;

transform:	T_TOK point
{	
    translate($2->x,$2->y); 
}
|		MX_TOK
{	
    mirror_x();
}
|		MY_TOK
{	
    mirror_y();
}
|		R_TOK point
{	
    rotate($2->x,$2->y);
}

layerCommand:	L_TOK	/*	layname is readed in lexical analyser,	*/ ;

commentCommand:	COMMENTTEXT_TOK	/*	comment text too. 		*/ ;

userExtCommand:	integer		
{
    userextno=$1;
    read_usertext();
    $$ = uext2uext(userextno, userexttext);
    userprocess();
}

defDelCommand:	DD_TOK integer;

defCommand:	defStartCommand SEMI_TOK primCommandList defFinCommand
;

defStartCommand:DS_TOK integer sep defStartScale
{
    symname=$2;
    /*	save current 'layname'.		*/
    savedlayname=layname;
    layname=dummylayname;
    /*	Start the new 'def' list.	*/
    def=NULL;
}

defStartScale:	/*	Empty	*/	{	scaling=1.0;		}
|		integer sep integer	{	scaling=(coord)$1/$3;	}

primCommandList:/*	Empty	*/
|		primCommandList primCommand SEMI_TOK
{
    /*	Here - add to 'def' list.	*/
    if	($2!=NULL)	/*	not miscCommand	*/
    {	
	def=cons($2,def);
    }	/*	then	*/
}
|		primCommandList error  SEMI_TOK
{
    fprintf(stderr,
	    "Error in command in definition of symbol \"%d\", line %d\n",
	    symname,lineno);
    yyerrok;
}


defFinCommand:	DF_TOK
{
    /*	Save current 'def' to assoc list 'defs'
     *	as 'symbname'.
     *	(with int converted to void *)
     */
    defs=cons(cons((void *)symname,reverse(def)),defs);
    /*	Restore the previous 'layname'.	*/
    layname=savedlayname;
    scaling=1.0;
    fprintf(stderr,"Symbol \"%d\" was defined\n",symname);
}

endCommand:	E_TOK;

point:		sInteger sep sInteger
{
    $$=newpnt(scaling*$1,scaling*$3);
}

path:		point			{	$$=cons($1,NULL);	}
|		path sep point		{	$$=cons($3,$1);		}

sep:		/*	Empty	*/		{	skip_sep();	}

integer:	PINTEGER_TOK			{	$$=$1;		}

sInteger:	PINTEGER_TOK | NINTEGER_TOK	{	$$=$1;		}

%%

yyerror(char *s)
{
    fprintf(stderr,"Parser: %s : ",s);
}
