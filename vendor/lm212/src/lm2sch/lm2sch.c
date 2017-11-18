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

/* -*-C-*-
*******************************************************************************
*
* File:         lm2sch.c
* Description:  Process LMeter output to match it with a schematic graph.
* Author:       Paul Bunyk
* Created:      Tue Apr  7 19:03:11 1998
* Language:     C
* Package:      LMeter
* Status:       Experimental
*
* (C) Copyright 1998, Paul Bunyk, all rights reserved.
*
*******************************************************************************
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include "linalg.h"
#include <unistd.h>

#define MAX_EDGES  1024
#define MAX_NODES 1024
#define MAX_TERMS 64
#define MAX_NODE_DEGREE 4
#define MAX_NAME_LEN 16
#define MAX_LINE_LEN 256

#define ERROR_PLOT 1

/*
 * **************************
 * * GLOBAL DATA STRUCTURES *
 * **************************
 */

/*
 * Array describing all elements in the netlist.
 * First record in this array is not used (see 'nodes' below).
 */
int numEdges = 0; 
int numTerms = 0; 
int numInds = 0;
int numMutual = 0;
struct
{
	char name[MAX_NAME_LEN]; /* Element name, e.g., L1, PIN ... */
	enum {IND, TERM, MUTUAL, UNUSED} type; /* ind/term/etc. */
	/* inductances/terminals */
	int  nodes[2];           /* Two nodes elt is connected to */
	double flow[MAX_TERMS];  /* current flow in all "experiments" */
	int  knowflow;           /* Do we already know the current? */
	/* inductances/mutual inductances */
	double value;            /* computed inductance */
 	double oldvalue;         /* "netlist" inductance */
	/* terminals */
	int  termno;             /* LMeter No. of the terminal */
	/* mutual inductances */
	char indName[2][MAX_NAME_LEN]; /* Ind. names*/
	int  indNo[2];           /* Ind. #s in sequence */
	int  indIdx[2];          /* Ind. index in edges */
} edges[MAX_EDGES];

/*
 * Array describing which elements are connected to each node.  Each node
 * is represented with an array of the adjacent edges,
 * index is positive for connection as first node, negative for the
 * second one.
 *
 * We assume that nodes are numbered sequentially starting from 0 (ground). 
 */
int numNodes = 0;
struct 
{
	int numEdges; /* How many edges does enter this node ? */
	int edges[MAX_NODE_DEGREE]; /* What are their indices? */
	int numKnown; /* How many currents flowing into this node we know? */
} nodes[MAX_NODES]; 

/* 
 * LMeter reports results for terminal numbers, we work with names.
 * This table encodes number -> name + edge#
 * Reverse encoding is given by termno field in edge structure. 
 */
struct
{
	char name[MAX_NAME_LEN];
	int edge;
} terms[MAX_TERMS];
/* NB: LMeter supports notion of active terminals (ones that are set
 * to 1 at one of the "experiments" and passive terminals (currents on
 * them are reported). Presently everyone makes all terminals active,
 * have to look deeper into that.
 */

/*
 * Macros to iterate over tables correctly.
 * (So I do not forget a 1 here or there) :)
 * Note that obvious 'for' is excluded to make C mode understand me!
 */
#define ALL_EDGES(e)   e=1; e<=numEdges; e++
#define ALL_NODES(n)   n=1; n<=numNodes; n++
#define ALL_TERMS(t)   t=0; t<numTerms; t++

/*
 * *****************
 * * I/O FUNCTIONS *
 * *****************
 */

/*
 * Error reporting facility a-la PERL :) 
 */
int die(char *where, char *what)
{

	fprintf(stderr, "ERROR: %s", where);
	if (what)  fprintf(stderr, ": %s",what);
	if (errno) perror(" ");
	fprintf(stderr, "\n");
	exit(1);
}

/* 
 * Get netlist (i.e., edges table) from SPICE-type file
 */
void ReadSpiceNetlist(char *fname)
{
	FILE *f;
	char line[MAX_LINE_LEN];
	char *lptr;
	char name[MAX_NAME_LEN];
	int  n1, n2; /* nodes for "normal" elements, ind. #s for mutuals */
	char l1[MAX_NAME_LEN],l2[MAX_NAME_LEN]; /* L1, L2 names for mutuals */
	double value;

	(f = fopen(fname, "r")) || die("opening",fname);

	while (fgets(line, MAX_LINE_LEN, f))
	{
	    int numfields;

	    /* skip whitespace */
	    lptr = line;
	    while (*lptr == ' ' || *lptr == '\t' || *lptr == '\n') lptr++;
	    if (strncmp(lptr,".END", 4)==0) 
		break;
	    if (lptr[0] == 0 
		|| lptr[0] == '.' 
		|| lptr[0] == '*'
		|| lptr[0] == '+') 
		continue; /* A special or empty SPICE card, ignored */

	    numfields = sscanf(lptr,"%s %s %s %lf", name, &l1, &l2, &value);
	    /* card should have at least name and two nodes */
	    if (numfields <  3) die("bad SPICE card",line); 
	    if (numfields == 3) value = 0.0;

	    numEdges++;
	    numEdges < MAX_EDGES || die("table overflow", 
					"too many circuit elements"); 
	    strncpy(edges[numEdges].name, name, MAX_NAME_LEN);
	    switch (toupper(name[0]))
	    {
	    case 'L':
		edges[numEdges].type = IND; 
		numInds++;
		break;
	    case 'P':
	    case 'J':
	    case 'I':
		edges[numEdges].type = TERM; 
		numTerms++;
		break;
	    case 'M':
	    case 'K':
		edges[numEdges].type = MUTUAL; 
		numMutual++;
		break;
	    default:
		fprintf(stderr,"# ReadSpiceNetlist: unknown element '%s' ignored\n", 
		       name);
		edges[numEdges].type = UNUSED; 		  
		break;
	    }
	    switch (edges[numEdges].type)
	    {
	    case UNUSED:
		numEdges--; /* Ignore it */
		break;
	    case IND:
		edges[numEdges].oldvalue = value;
	    case TERM:
		sscanf(l1, "%d", &n1) == 1 
		    || die("bad node number", l1);
		sscanf(l2, "%d", &n2) == 1 
		    || die("bad node number", l2);
		edges[numEdges].nodes[0] = n1;
		edges[numEdges].nodes[1] = n2;
		if (n1>numNodes) numNodes = n1;
		if (n2>numNodes) numNodes = n2;
		break;
	    case MUTUAL:
		strncpy(edges[numEdges].indName[0], l1, MAX_NAME_LEN);
		strncpy(edges[numEdges].indName[1], l2, MAX_NAME_LEN);
		edges[numEdges].oldvalue = value;
		break; /* !!! for now */
	    }
	}
}

/* 
 * Fill-in indNo field for all mutual inductances 
 */
void ConnectMutuals()
{
	int e1;
	
	for (ALL_EDGES(e1))
		if (edges[e1].type == MUTUAL)
		{
			int e2, indNo=0, numfound = 0;
			for (ALL_EDGES(e2))
				if (edges[e2].type == IND)
				{
					if (strcmp(edges[e2].name,
						   edges[e1].indName[0])==0)
					{
						edges[e1].indNo[0] = indNo;
						edges[e1].indIdx[0] = e2;
						numfound++;
					}
					else if (strcmp(edges[e2].name,
						   edges[e1].indName[1])==0)
					{
						edges[e1].indNo[1] = indNo;
						edges[e1].indIdx[1] = e2;
						numfound++;
					}
					if (numfound == 2) 
						break;
					indNo++;
				}
			(numfound == 2) 
				|| die("bad mutual",edges[e1].name);			
		}
	
}

/* 
 * Read terminal names (numbered sequentially). 
 */
void ReadTerminalNames(char *fname)
{
	FILE *f;
	char line[MAX_LINE_LEN];
	char name[MAX_NAME_LEN];
	int number;

	(f = fopen(fname, "r")) || die("opening", fname);

	while (fgets(line, MAX_LINE_LEN, f))
	{
		sscanf(line,"%s %d", name, &number) == 2 
			|| die("bad terminal name line", line);
		strncpy(terms[number - 1].name, name, MAX_NAME_LEN);
		number < MAX_TERMS || die("table overflow",
					  "too many terminals"); 
	}
	number == numTerms 
		|| die (fname, "not enough or too many terminals");
}

/* 
 * Get terminal current matrix from LMeter-produced file. 
 */
void ReadTermCurrents(char *fname)
{
	FILE *f;
	char line[MAX_LINE_LEN];
	int t1, t2;
	double value;

	(f = fopen(fname, "r")) || die("opening",fname);

	while (fgets(line, MAX_LINE_LEN, f))
	{
		int e;

		sscanf(line,"%d %d %lf", &t1, &t2, &value) == 3
			|| die("bad LMeter line", line);
		t1--; t2--;
		e = terms[t2].edge;
		edges[e].type == TERM || die("Bad terminal number in", line);
		edges[e].flow[t1] = value;
	}
}

/*
 * ******************
 * * DEBUG PRINTOUT *
 * ******************
 */

/* 
 * Print netlist (edges)
 */
void PrintEdges()
{
	int e, t;

	for (ALL_EDGES(e))
		switch (edges[e].type)
		{
		case IND:
			fprintf(stderr, "L: %3d %16s(%3d,%3d) = %lf -> %lf\n", 
			e, 
			edges[e].name, 
			edges[e].nodes[0],
			edges[e].nodes[1],
			edges[e].oldvalue, 
			edges[e].value);
			break;
		case TERM:
			fprintf(stderr, "P: %3d %16s(%3d,%3d) = %d\n", 
			e, 
			edges[e].name, 
			edges[e].nodes[0],
			edges[e].nodes[1],
			edges[e].termno);
			break;
		case MUTUAL:
			fprintf(stderr, "M: %3d %16s(%s %3d,%s %3d) = %lf -> %lf\n", 
			e, 
			edges[e].name, 
			edges[e].indName[0], 
			edges[e].indNo[0],
			edges[e].indName[1], 
			edges[e].indNo[1],
			edges[e].oldvalue, 
			edges[e].value);
			break;
		}
	/*
	  for (ALL_TERMS(t))
	  fprintf(stderr, "%lf ", 
	  edges[e].flow[t]); 
	  fprintf(stderr, "\n");
	*/
}

/* 
 * Print nodes connectivity table 
 */
void PrintNodes()
{
	int n;

	for (ALL_NODES(n))
	{
		int ie;
		fprintf(stderr, " %d: ", n);
		for (ie=0; ie<nodes[n].numEdges; ie++)
		{
			int e = nodes[n].edges[ie];
			if (e > 0)
				fprintf(stderr, "+%s ",edges[e].name);
			else
				fprintf(stderr, "-%s ",edges[-e].name);
		}
		fprintf(stderr, "\n", n);
	}
}
/* 
 * Print terminals table
 */
void PrintTerms()
{
	int t;

	for (ALL_TERMS(t))
		fprintf(stderr, "%d %s -> %d\n",
			t, terms[t].name, terms[t].edge);
}

/*
 * ************************
 * * PROCESSING FUNCTIONS *
 * ************************
 */

void BuildNodes()
{
	int e, n;

	for (ALL_NODES(n))
		nodes[n].numEdges = nodes[n].numKnown = 0;

	for (ALL_EDGES(e))
	{
		int dir;
		for (dir = 0; dir <= 1; dir++)
		{
			int n = edges[e].nodes[dir];
			if (n==0) continue; /* Skip GROUND node */
			(nodes[n].numEdges < MAX_NODE_DEGREE)
				|| die("Too high node degree around", 
				       edges[e].name);
			nodes[n].edges[nodes[n].numEdges++] = (2*dir-1) * e;
		}
	}
}

void CheckNetlist()
{
	int n, e;
	/* this table is used to skip zero-degree nodes */
	int nodeTranslate[MAX_NODES];
	int numRealNodes = 0;

	/* 
	 * Get rid of all zero-degree nodes.
	 * First find them and build nodeTranslate table:
	 */
	nodeTranslate[0] = 0;
	for (ALL_NODES(n))
	{
		if (nodes[n].numEdges == 0)
		{
			fprintf(stderr,"# CheckNetlist: Node %d of degree 0 removed\n", n);
			nodeTranslate[n] = 0;
		}
		else
			nodeTranslate[n] = ++numRealNodes;
	}
	/* 
	 * Then renumber nodes in edges if necessary 
	 */
	if (numNodes != numRealNodes)
	{
		numNodes = numRealNodes;
		for (ALL_EDGES(e))
		{
			int dir;
			for (dir=0; dir <= 1; dir++)
			{
				int node = edges[e].nodes[dir];
				assert(node == 0 || nodeTranslate[node] > 0);
				edges[e].nodes[dir] = nodeTranslate[node];
			}
		}
		BuildNodes(); /* Again! */
	}
}

/*
 * Find which edge corresponds to which terminal.
 * Should be probably done in better that N^2 way! :) 
 */
void SetTerminalEdges()
{
	int t, e;
	
	for (ALL_EDGES(e))
		if (edges[e].type == TERM)
		{
			int found = 0;
			for (ALL_TERMS(t))
				if (strcmp(terms[t].name, edges[e].name)==0)
				{
					terms[t].edge == 0 || 
						die("terminal mentioned twice",
						    terms[t].name);
					terms[t].edge = e;
					edges[e].termno = t;
					found = 1;
					break;
				}
			found || die("undeclared netlist terminal",
				     edges[e].name);
		}
}

/*
 * Starting from known termninal currents, calculate currents through
 * all other elements.
 */
double CalculateCurrents()
{
	int n, e, t;
	int numKnown = 0;
	int numIter = 0;

	for (ALL_EDGES(e))
		switch (edges[e].type)
		{
		case TERM:
			edges[e].knowflow = 1;
			nodes[edges[e].nodes[0]].numKnown++;
			nodes[edges[e].nodes[1]].numKnown++;
			numKnown++;
			break;
		case IND:
			edges[e].knowflow = 0;
			break;
		}
	/* check that we really know currents through all terminals */
	for (ALL_TERMS(t))
	{
		edges[terms[t].edge].type == TERM
			|| die("Internal error: terminal is inductance???", 
			       terms[t].name);
		edges[terms[t].edge].knowflow 
			|| die("Internal error: unknown terminal current???", 
			       terms[t].name);
	}

	/* 
	 * Iterate until we know all currents:
	 *   find an node with d-1 known currents (d is node degree)
	 *   calculate current through the unknown element.
	 * This process should converge if we do not have inductive loops.
	 */
	while (numKnown < numEdges - numMutual)
	{
		int n;
		int converging = 0;

		numIter++;

		for (ALL_NODES(n))
			if (nodes[n].numKnown == nodes[n].numEdges - 1)
			{
				int ie, unk = 0;
				double unkdir;
				/* First find which one is unknown */
				for (ie = 0; ie<nodes[n].numEdges; ie++)
				{
					int e = nodes[n].edges[ie];
					assert(abs(e)<=numEdges);
					if (!edges[abs(e)].knowflow)
					{
						unkdir = (e<0)? -1.0:+1.0;
						unk = abs(e);
						for (ALL_TERMS(t))
							edges[unk].flow[t] = 0.0;
						break;
					}
				}
				assert(unk);
				/* Now add up all known ones */
				for (ie = 0; ie<nodes[n].numEdges; ie++)
				{
					int t, e = nodes[n].edges[ie];
					double dir = (e<0)? -1.0:+1.0;
					
					if (unk == abs(e)) continue;
					
					assert(edges[abs(e)].knowflow);
					for (ALL_TERMS(t))
						edges[unk].flow[t] -=
							dir * unkdir * 
							edges[abs(e)].flow[t];
				}
				/* Know one more! */
				edges[unk].knowflow = 1;
				nodes[edges[unk].nodes[0]].numKnown++;
				nodes[edges[unk].nodes[1]].numKnown++;
				numKnown++;
				converging = 1;
			}
		converging || die("CalculateCurrents not converging",
				  "check for inductive loops");
	}
	/*
	 * Check that now all node currents sum up to 0.0
	 */
	{
		double maxsum = 0;
		int worst = 0;
#if ERROR_PLOT
		char errname[] = "currents.err";
		FILE *errfile = fopen(errname,"w");

		errfile || die("opening", errname);
#endif
		for (ALL_NODES(n))
		{
			for (ALL_TERMS(t))
			{
				int ie;
				double sum = 0;
				for (ie = 0; ie<nodes[n].numEdges; ie++)
				{
					int e = nodes[n].edges[ie];
					double dir = (e<0)? -1.0:+1.0;
					assert(abs(e)<=numEdges);
					assert(edges[abs(e)].knowflow);
					sum += dir * edges[abs(e)].flow[t]; 
				}
				/* assert(fabs(sum) < 1e-6); */
#if ERROR_PLOT
				fprintf(errfile,"%lf\n", sum);
#endif
				sum = fabs(sum);
				if (sum > maxsum)
				{
					maxsum = sum;
					worst = t;
				}
			}
#if ERROR_PLOT
				fprintf(errfile,"\n");
#endif
		}
		fprintf(stderr,"# CalculateCurrents: Iterations: %d\n", numIter);
		fprintf(stderr,"# Worst currents accuracy: %lf in experiment with terminal \"%s\"\n", 
			maxsum, terms[worst].name);
		fprintf(stdout,"# Worst currents accuracy: %lf in experiment with terminal \"%s\"\n", 
			maxsum, terms[worst].name);
		return maxsum;
	}
}

void CalculateInductances()
{
	int e, n, t;
	int ind, mut, term;
	
	/* Incidence matrices for inductances and terminals 
	 */ 
	LA_Matrix GL, GT; 
	/* Vectors of currents through inductances and 
	 * phases on terminals for each experiment
	 */	
	LA_Matrix *IL, *PT;
	/* Temporary storage
	 */
	LA_Matrix TGL, TGT, GLGT, TGLGT, Ginv, A, B, L;
	LA_Matrix GLGinv, A1, B1, E;
	LA_Matrix I, TI, A1I, TIA1I, B1PT, TIB1PT;

	(IL = malloc(sizeof(LA_Matrix)*numTerms)) 
		|| die("not enough memory", "IL");
	(PT = malloc(sizeof(LA_Matrix)*numTerms)) 
		|| die("not enough memory", "PT");

	GL = LA_create(numInds,  numNodes);
	GT = LA_create(numTerms, numNodes);

 	for (ALL_TERMS(t)) /* for all experiments */
	{
		IL[t] = LA_create(numInds, 1);
		PT[t] = LA_create(numTerms, 1);
	}

	ind = 0;
	term = 0;
 	for (ALL_EDGES(e))
		switch (edges[e].type)
		{
		case IND:
			/* Fill GL row */
			for (ALL_NODES(n))
			{
				if (n==edges[e].nodes[0])
					LA_VAL(GL, ind, n-1) = +1.0;
				else if (n==edges[e].nodes[1])
					LA_VAL(GL, ind, n-1) = -1.0;
			}
			/* Fill ILs */
			for (ALL_TERMS(t))
				LA_VAL(IL[t],ind,0) = edges[e].flow[t];
			ind++;
			break;
		case TERM:
			/* Fill GT row */
			for (ALL_NODES(n))
			{
				if (n==edges[e].nodes[0])
					LA_VAL(GT, term, n-1) = +1.0;
				else if (n==edges[e].nodes[1])
					LA_VAL(GT, term, n-1) = -1.0;
			}
			/* Fill PT */
			for (ALL_TERMS(t))
                                if (edges[e].termno == t)
					LA_VAL(PT[t], term, 0) = 1.0;
			term++;
			break;
		case MUTUAL:
			break; /* can not precompute anything, can we? */
		}
	
#if 0
	LA_print(GL);
	LA_print(GT);
 	for (ALL_TERMS(t))
		LA_print(IL[t]);
 	for (ALL_TERMS(t))
		LA_print(PT[t]);
#endif
	TGL = LA_transpose(NULL, GL);
	TGT = LA_transpose(NULL, GT);
	GLGT = LA_stack(GL, GT);
	TGLGT = LA_transpose(NULL, GLGT);
	Ginv = LA_inverse(LA_dot(NULL, TGLGT, GLGT));
	if (Ginv == NULL)
	    die("CalculateInductances",
		"Singular matrix, check topology");
#if 0
	LA_print(Ginv);
#endif
	
	GLGinv = LA_dot(NULL, GL, Ginv);
	E = LA_unit(numInds);
	A1 = LA_add(LA_dot(NULL, GLGinv, TGL), -1.0, E);
	B1 = LA_dot(NULL, GLGinv, TGT);
#if 0
	LA_print(A1);
	LA_print(B1);
#endif

	A = LA_create(numInds+numMutual, numInds+numMutual); 
	B = LA_create(numInds+numMutual, 1); 

	I = LA_create(numInds, numInds+numMutual);
	/* these will be allocated when needed */
	TI = NULL;
	A1I = NULL; 
	TIA1I = NULL; 
	B1PT = NULL; 
	TIB1PT = NULL; 
	
	for (ALL_TERMS(t)) /* all experiments */
	{

		ind = 0; 
		mut = 0;
		for (ALL_EDGES(e))
			switch (edges[e].type)
			{
				int i1, i2;

			case IND:
				LA_VAL(I,ind,ind)=LA_VAL(IL[t],ind,0);
				ind++;
				break;
			case MUTUAL:
				i1 = edges[e].indNo[0];
				i2 = edges[e].indNo[1];
				LA_VAL(I,i1,numInds+mut)= LA_VAL(IL[t],i2,0);
				LA_VAL(I,i2,numInds+mut)= LA_VAL(IL[t],i1,0);
				mut++;
				break;
			}
		TI = LA_transpose(TI, I);
		A1I = LA_dot(A1I, A1, I);
		TIA1I = LA_dot(TIA1I, TI, A1I);
		LA_add(A, +1.0, TIA1I);
		B1PT = LA_dot(B1PT, B1, PT[t]);	
		TIB1PT = LA_dot(TIB1PT, TI, B1PT);
		LA_add(B, +1.0, TIB1PT);
	}

#if 0
	LA_print(A);
	LA_print(B);
#endif
	
	A = LA_inverse(A);
	if (A == NULL)
	    die("CalculateInductances",
		"Singular matrix, check topology");

	L = LA_dot(NULL,A,B);
#if 0
	LA_print(L);
#endif

	/* Get rid of all the accumulated junk */
	LA_delete(GL);
	LA_delete(GT);
	LA_delete(TGL);
	LA_delete(TGT);
	LA_delete(GLGT);
	LA_delete(TGLGT);
	LA_delete(Ginv);
	LA_delete(A);
	LA_delete(B);
	LA_delete(GLGinv);
	LA_delete(A1);
	LA_delete(B1);
	LA_delete(E);
	LA_delete(I);
	LA_delete(TI);
	LA_delete(A1I);
	LA_delete(TIA1I);
	LA_delete(B1PT);
	LA_delete(TIB1PT);
 	for (ALL_TERMS(t)) /* for all experiments */
	{
		LA_delete(IL[t]);
		LA_delete(PT[t]);
	}
	free(IL); 
	free(PT);

	/* Now we know vector L, put it back into the edge info */
	ind = 0;
	mut = 0;
 	for (ALL_EDGES(e))		
		switch (edges[e].type)
		{
		case IND:
			edges[e].value = LA_VAL(L,ind,0);
			ind++;
			break;
		case MUTUAL:
			edges[e].value = LA_VAL(L,numInds+mut,0);
			mut++;
			break;
		}

	LA_delete(L);
}

void CalculateEnergies()
{
	int e, n, t;
	double worst_dE = 0.0;
	double dE;
	int worst = 0;

	for (ALL_TERMS(t)) /* all experiments */
	{
		int edge = terms[t].edge;
		double total = edges[edge].flow[t]/2.0; /* when it is active */
		double sum = 0.0;
		
		for (ALL_EDGES(e))
			switch (edges[e].type)
			{
				double I1, I2, I;
			case IND:
				I = edges[e].flow[t]; 
				sum += edges[e].value*I*I/2.0;
				break;
			case MUTUAL:
				I1 = edges[edges[e].indIdx[0]].flow[t];
				I2 = edges[edges[e].indIdx[1]].flow[t];
				sum += edges[e].value*I1*I2;
				break;
			}
		/* fprintf(stderr,"Energy: %3d %4.3lf %4.3lf %8.5lf\n", 
		   t, total, sum, (total-sum)/total); */
		dE = fabs((total-sum)/total);
		if (worst_dE < dE)
		{
			worst_dE = dE;
			worst = t;
		}
	}	
	fprintf(stderr,"# Worst dE: %lg in experiment with terminal \"%s\"\n",
 	       worst_dE, terms[worst].name);
	fprintf(stdout,"# Worst dE: %lg in experiment with terminal \"%s\"\n",
 	       worst_dE, terms[worst].name);
}

void WriteReport(char *outname)
{
	int e;
	FILE *out; 

	(out = fopen(outname, "w")) || die("opening", outname);
	fprintf(stdout,"# %-14s  %-6s %-6s %-6s\n", "name", "layout", "schem", "diff"); 
 	for (ALL_EDGES(e))
		if (edges[e].type == IND || edges[e].type == MUTUAL )
		{
			fprintf(out, "%-16s %6.3lf\n",
			       edges[e].name, 
			       /* edges[e].oldvalue, */
			       edges[e].value);
			printf("%-16s %6.3lf %6.3lf %+6.3lf\n",
			       edges[e].name, 
			       edges[e].value,
			       edges[e].oldvalue,
			       edges[e].oldvalue - edges[e].value);
		}
}

static char version[] = "$Id: lm2sch.c,v 1.3 1998/10/30 16:47:29 paul Exp paul $";

main(int argc, char **argv)
{
	double curracc;

	char *netlistname = "netlist.spi";
	char *termname = "term.name";
	char *lmoutname = "lmeter.out";
	char *outname = "lm2sch.out";
	char c;

	while ((c=getopt(argc, argv, "hn:t:c:o:")) != EOF)
	{
		switch(c)
		{
		case 'n': netlistname = optarg; break;
		case 't': termname = optarg; break;
		case 'c': lmoutname = optarg; break;
		case 'o': outname = optarg; break;
		case 'h':
			fprintf(stderr, 
				"%s [-n netlist] [-t termnames] [-c lmcurrents] [-o output]\n", argv[0]); 
			exit(0);
			break;
		default:
			exit(1);
		}
	}

	fprintf(stderr,"# %s\n", version);
	fprintf(stderr,"# using: %s %s %s %s\n", 
	       netlistname, termname, lmoutname, outname);
	ReadSpiceNetlist(netlistname);
	ConnectMutuals();
	ReadTerminalNames(termname);
	SetTerminalEdges();
	ReadTermCurrents(lmoutname);
	BuildNodes();
	CheckNetlist();

	fprintf(stderr,"# lm2sch: Elements: %d, Nodes: %d, Terms: %d, Inds: %d, Mutuals: %d\n", 
		numEdges, numNodes, numTerms, numInds, numMutual);

	curracc = CalculateCurrents();
	curracc < 1.0 
		|| die("current accuracy is too bad", 
		       "upside-down terminal or bad topology");
	CalculateInductances();
	CalculateEnergies();

	WriteReport(outname);

	return 0;
/* 	PrintNodes(); */
/* 	PrintTerms(); */
/* 	PrintEdges(); */
	
}
