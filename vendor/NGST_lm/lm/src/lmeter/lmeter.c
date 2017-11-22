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


#define	MAIN
#include"lmeter.h"
#include"head_lm.h"

#define	Ver	13

char            CopyRightMessage[] = 
"* L-Meter 2.%02d Copyright (C) 1992-2001 Paul I. Bunyk\n"
"* pbunyk@trw.com\n"
"* Based on the algorithms by Sergey V. Rylov\n"
"* RSFQ Laboratory, Department of Physics\n" 
"*\tSUNY, Stony Brook, NY, 11794-3800\n"
"* Laboratory of Cryoelectronics, Department of Physics\n"
"*\tMoscow State University, Moscow, Russia, 119899\n"
"* Superconducting Electronics Organisation, TRW\n"
"*\tOne Space Park, Redondo Beach, CA, 90278\n"
"* This is free software, and you are welcome to redistribute it\n"
"* under the conditions of Gnu Public License (GPL)\n"
"* This software comes with ABSOLUTELY NO WARRANTY\n";

CopyRight ()
{
    fprintf (stderr, CopyRightMessage, Ver);
}

char            strbuff[256];

main (int argc, char **argv)
{
    CopyRight ();

    progname = argv[0];
    message ("LMeter: Execution begins.");
    reportmem ();
    sprintf (strbuff, "LMeter: Accepting up to %d terminals.", NTRMMAX);
    message (strbuff);
    accept_input (argc, argv);
    errno = 0;
    message ("LMeter: Input information was accepted.");
    reportmem ();
    main_gm ();
    errno = 0;
    message ("LMeter: Geometrical part passed.");
    reportmem ();
    main_lm ();
    errno = 0;
    message ("LMeter: Execution ends.");
    reportmem ();

#ifdef	__BORLANDC__
    if (heapcheck () == -1)
	error ("HEAP corrupted ...");
    else
	message ("LMeter: Heap seems to be OK")
#endif
#if	defined(__GNUC__) && defined (MSTATS)
	    mstats ("LMeter");
#endif

    atof ("0");			/* BC needs it to link in float. formats !!!    */

    return 0;
}

/*
 *    Some system-dependent stuff is here:
 */

#ifdef	__BORLANDC__
extern unsigned _stklen = 32000;

	/* it is needed by BorlandC++ to set big enough stack   */
#endif

#include<ctype.h>

#ifdef	__NDPC__
int             __k;		/* NDP C's ctype needs it!              */

#endif

char           *
mystrupr (char *s)		/* strupr not exist in NDP      */
{
    int             i = 0;

    assert (s);
    while (s[i])
    {
	s[i] = toupper (s[i]);
	i++;
    }
    return s;
}

#ifndef	NRMEM
#ifndef	__BORLANDC__
#error	REPORTMEM FOR BORLAND C ONLY
#endif
#include<alloc.h>
_reportmem ()
{
    sprintf (strbuff, "CORE LEFT %10lu \n", coreleft ());
    message (strbuff);
}
#endif
