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
 *      Read input parameters 
 */

#include"lmeter.h"
#include<string.h>
#include<ctype.h>

const char      defaultfile[] = "lm.dfl";
const char      defaultname[] = "LM_DEFAULTS";
const char      helpline[] = "CALL:	lmeter {-s \"<var> <value>\"|<parameter file name>}";

#define	LINELEN	1024
#define	PARSLEN		1024	/* max. nmb of parameter lines    */

struct
{
    char           *name;
    char           *value;
}
pars[PARSLEN];
int             npars;

assign (char *line)
{
    char           *name, *value;

    name = strtok (line, " \t");
    value = strtok (NULL, "#");
    mystrupr (name);
    if (npars == PARSLEN)
	error ("Parameter table overflow");
    allocate (pars[npars].name, 1, strlen (name) + 1);
    allocate (pars[npars].value, 1, (value ? strlen (value) : 0) + 1);
    strcpy (pars[npars].name, name);
    if (value)
	strcpy (pars[npars].value, value);
    ++npars;
}

char           *
getpar (char *name)
{
    int             i = 0;

    while (i < npars)
	if (strcmp (name, pars[i].name) == 0)
	    return pars[i].value;
	else
	    i++;
    return NULL;
}

accept_input (int argc, char **argv)
{
    int             a;
    char           *lm_defaults;
    FILE           *tmp;

    for (a = 1; a < argc; a++)
    {
	if (argv[a][0] != '-')	/* file name    */
	    accept_file (argv[a]);
	else			/* comm.line switch       */
	    switch (argv[a][1])
	    {
	    case 'S':
	    case 's':
		assign (argv[a] + 2);
		break;
	    default:
		fprintf (stderr, helpline);
		exit (0);
		break;
	    }
    }
    if ((tmp = fopen (defaultfile, "r")) != NULL)
    {
	fclose (tmp);
	accept_file (defaultfile);
    }
    lm_defaults = getenv (defaultname);
    if (lm_defaults)
	accept_file (lm_defaults);
}

accept_file (char *name)
{
    FILE           *sets;
    char            line[LINELEN];
    int             i, n;

    if ((sets = fopen (name, "r")) == NULL)
    {
	sprintf (strbuff, "Can't open input file %s", name);
	warning (strbuff);
	errno = 0;
    }
    else
    {
	sprintf (strbuff, "LMeter: Reading \"%s\"... ", name);
	message (strbuff);
	while (!feof (sets))
	{
	    fgets (line, LINELEN - 1, sets);
	    /*
	     *    Clean up leading spaces and trailing '\n'
	     */
	    i = 0;
	    n = strlen (line);
	    if (n)
	    {
		line[--n] = '\0';
		while (isspace (line[i]) && i < n)
		    i++;
		if (i < n && line[i] != '#')
		    assign (line + i);
	    }
	}
	fclose (sets);
    }
}
