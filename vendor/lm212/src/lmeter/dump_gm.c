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

#include"lmeter.h"
#include"head_gm.h"
#include"avltree.h"

FILE           *d;
char           *
bits (char *buff, cnftype n)
{
    int             i;

    assert (buff);
    for (i = 15; i >= 0; i--)
	buff[i] = (n & (POW2 (i))) ? '1' : '0';
    buff[16] = 0;
    return buff;
}

void 
dumpx (tree t)
{
    dcnf           *info;
    int             i;
    char            bits1[17], bits2[17];

    assert (t);
    info = t->info;
    assert (info);
    fprintf (d, "\tX=%-5d\n", t->key);
    for (i = 0; i < 4; i++)
	fprintf (d, "\t\t TCNF:%-17s:%-6u ECNF:%-17s:%-6u\n",
		 bits (bits1, (unsigned) info[i].tcnf),
		 (unsigned) info[i].tcnf,
		 bits (bits2, (unsigned) info[i].ecnf),
		 (unsigned) info[i].ecnf);
}
void 
dumpyx (tree t)
{
    if (d == NULL)
	fileopen (d, "yx.dmp", "w");
    fprintf (d, "NEW Y-TREE Y=%-5d\n", t->key);
    apply_ascend (t->info, dumpx);
}
