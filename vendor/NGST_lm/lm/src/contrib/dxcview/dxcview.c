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

#include<stdio.h>
#include<stdlib.h>
#include<graphics.h>
/*	#include"svga256.h"	*/
#include<limits.h>

#define	MAIN
#include"misc.h"
#include"dxcview.h"

#define	ESC	0x1b
#define	CR	0x0d
#define	HOME	0x47
#define	UP	0x48
#define	PGUP	0x49
#define	LEFT	0x4b
#define	RIGHT	0x4d
#define	END	0x4f
#define	DOWN	0x50
#define	PGDN	0x51
#define	F1	0x3b

#define	STEP	0.1

int	scrmaxx,scrmaxy;

int	huge	detect()	{return	4;}

main(int argc, char **argv)
{	int	gd,gm;
	extern	FILE	*yyin;

	initlays();

	xmin=INT_MAX;
	xmax=INT_MIN;
	ymin=INT_MAX;
	ymax=INT_MIN;
	if	(argc>1)
	{	fileopen(yyin,argv[1],"r");
	}       /*	then	*/
	else	error("No input file specified");
	yyparse();
	fclose(yyin);

	/*	installuserdriver("Svga256",detect);	*/
	/*	installuserdriver("$dj",detect);	*/
	registerbgidriver(EGAVGA_driver);
	gd=DETECT;
	initgraph(&gd,&gm,"e:\bc\bgi");

	/* read result of initialization */
	{	int	errorcode = graphresult();
		if (errorcode != grOk)  /* an error occurred */
		{
		   printf("Graphics error: %s\n", grapherrormsg(errorcode));
		   printf("Press any key to halt:");
		   getch();
		   exit(1); /* terminate with an error code */
		}
	}
	scrmaxx=getmaxx();
	scrmaxy=getmaxy();
	zoom=MIN(scrmaxx/(xmax-xmin),scrmaxy/(ymax-ymin));
	shiftx=xmin;
	shifty=ymin;
	do
	{       setfillstyle(INTERLEAVE_FILL,DARKGRAY);
		bar(0,0,scrmaxx,scrmaxy);
		/*	setwritemode(TRANS_COPY_PUT);	*/
		show();
		switch	(getch())
		{	case	'+':    shiftx+=0.25*scrmaxx/zoom;
					shifty+=0.25*scrmaxy/zoom;
					zoom*=2.0;
					break;
			case	'-':    zoom/=2.0;
					shiftx-=0.25*scrmaxx/zoom;
					shifty-=0.25*scrmaxy/zoom;
					break;
			case	CR:	zoom=MIN(scrmaxx/(xmax-xmin),scrmaxy/(ymax-ymin));
					shiftx=xmin;
					shifty=ymin;
					break;
			case	ESC:	closegraph();	return	0;
			case	0x00:
				switch	(getch())
				{
					case	LEFT:	shiftx-=scrmaxx*STEP/zoom;
							break;
					case	RIGHT:	shiftx+=scrmaxx*STEP/zoom;
							break;
					case	UP:	shifty-=scrmaxy*STEP/zoom;
							break;
					case	DOWN:	shifty+=scrmaxy*STEP/zoom;
							break;
					case	HOME:	shiftx-=scrmaxx*STEP/zoom;
							shifty-=scrmaxy*STEP/zoom;
							break;
					case	PGUP:   shiftx+=scrmaxx*STEP/zoom;
							shifty-=scrmaxy*STEP/zoom;
							break;
					case	END:    shiftx-=scrmaxx*STEP/zoom;
							shifty+=scrmaxy*STEP/zoom;
							break;
					case	PGDN:   shiftx+=scrmaxx*STEP/zoom;
							shifty+=scrmaxy*STEP/zoom;
							break;
					case	F1:	showpic();
							if	(getch()==0x00)	getch();
							break;
					default:	putchar('\7');
				}	/*	switch	*/
				break;
			default:	putchar('\7');
		}	/*	switch	*/
	}	while	(1);
}
/*\
 * 	Colors for different layers.
\*/
struct	name2color
{	char	name[NAMELEN];
	enum	COLORS	color;
};
#define	CN(name)	{#name,name}
struct	name2color
	colname2color[]=
	{   	CN(BLACK),
		CN(BLUE),
		CN(GREEN),
		CN(CYAN),
		CN(RED),
		CN(MAGENTA),
		CN(BROWN),
		CN(LIGHTGRAY),
		CN(DARKGRAY),
		CN(LIGHTBLUE),
		CN(LIGHTGREEN),
		CN(LIGHTCYAN),
		CN(LIGHTRED),
		CN(LIGHTMAGENTA),
		CN(YELLOW),
		CN(WHITE)
	};
struct	name2color	lay2color[MAXNLAYS];
int	nlays=0;
/*\
 * 	Function:	initlays
 *	Definition:   	read the lay2color array.
\*/
#include<dir.h>
initlays	( void )
{	FILE	*f;
	int	i;
	char	layname[NAMELEN];
	char	colname[NAMELEN];
	char	fname[MAXPATH];

	if	((f=fopen("dxcview.pal","r"))==NULL)
	{       strcpy(fname,getenv("PSCAN"));
		strcat(fname,"\\lib\\dxcview.pal");
		fileopen(f,fname,"r");
	}	/*	then	*/
	while	(!feof(f))
	{	fscanf(f,"%s %s",layname,colname);
		strupr(layname);
		strupr(colname);
		if	(nlays==MAXNLAYS)
			error("Too many laynames defined");
		strcpy(lay2color[nlays].name,layname);
		lay2color[nlays].color=WHITE;
		for	(i=0;i<sizeof(colname2color)/sizeof(colname2color[0]);i++)
			if	(strcmp(colname2color[i].name,colname)==0)
			{	lay2color[nlays].color=colname2color[i].color;
				break;
			}	/*	then	*/
		nlays++;
	}
	fclose(f);
	return 0;
}	/*	initlays	*/
/*\
 * 	Function:	show
 *	Definition:	show every pline in plinelist.
\*/
show	( void )
{       struct	{int x,y;}	transpoints[MAXPOINTNUM];
	int	pnt;
	foreach
	(	p,plinelist,
		{       struct	pline	*pp=p->car;
			layname=pp->layname;
			thickness=pp->thickness;
			points=pp->points;
			pointnum=pp->pointnum;
			for	(pnt=0;pnt<pointnum;pnt++)
			{	transpoints[pnt].x=zoom*(points[pnt].x-shiftx);
				transpoints[pnt].y=scrmaxy-zoom*(points[pnt].y-shifty);
			}	/*	for	*/
			showpoly(transpoints,pointnum);
		}
	);
	return 0;
}	/*	show	*/

/*\
 * 	Function:	showpoly
 *	Definition:     just show a polygon (described as in BGI's 'drawpoly')
 *	in layer 'layname'.
\*/
showpoly	(struct {int x,y;} points[],int pointnum)
{       int	i;
	if	(thickness>0.0 && strcmp("TERM",layname)==0)
	{       long	x=0,y=0;	/*	terminal center	*/
		char	str[17];
/*		The right way to calculate the pline's center
 *		was suggested by Dima Zinovieff (he insisted on
 *		the insertion of this remark here).
 */
		for	(i=0;i<pointnum-1;i++)
		{	x+=points[i].x;
			y+=points[i].y;
		}	/*	for	*/
		x/=pointnum-1;
		y/=pointnum-1;
		setlinestyle(DOTTED_LINE,0,THICK_WIDTH);
		setcolor(WHITE);
		drawpoly(pointnum,points);
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		setcolor(YELLOW);
		outtextxy(x,y,itoa((int)thickness,str,10));
		setlinestyle(SOLID_LINE,0,NORM_WIDTH);
		setcolor(BLACK);
	}	/*	then	*/
	else
	{	for	(i=0;i<nlays;i++)
			if	(strcmp(layname,lay2color[i].name)==0)
			{	setcolor(lay2color[i].color);
				break;
			}
		if	(i==nlays)
			setcolor(WHITE);
		drawpoly(pointnum,(int *)points);
	}	/*	else	*/
	return 0;
}	/*	showpoly	*/
/*\
 * 	Function:	showpic
 *	Definition:	Attracts the user's attention ...
\*/
#include"roach135.xbm"
showpic	( )
{	int	i,j,p=0,b=8,x,y;
	char	c;
	x=getmaxx()-roach135_width;
	for	(i=0;i<roach135_width;i++)
	{       y=getmaxy()-roach135_height;
		for	(j=0;j<roach135_height;j++)
		{       if	(b==8)
			{	c=roach135_bits[p++];
				b=0;
			}       /*	then	*/
			if	((1<<b++)&c)	putpixel(x,y,BROWN);
			y++;
		}	/*	for	*/
		x++;
	}	/*	for	*/
	return 0;
}	/*	showpic	*/
