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

/*  Note: You are free to use whatever license you want.
    Eventually you will be able to edit it within Glade. */

/*  gtkLMeter
 *  Copyright (C) <YEAR> <AUTHORS>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <gtk/gtk.h>
#include "gladesrc.h"
#include "gladesig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <signal.h>

GtkWidget *TopWindow;
GtkWidget *ErrorDialog;

GdkColor red;
GdkColor green;
GdkColor blue;
GdkColor purple;
GdkFont *fixed_font;

int HadError = 0;

int
main (int argc, char *argv[])
{
    char buffer[1024];

    gtk_set_locale ();
    gtk_init (&argc, &argv);

    /* 
     * Setup fonmts and colors
     */
    fixed_font = gdk_font_load ("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");

    red.red = 0xffff;
    red.green = 0;
    red.blue = 0;
    green.red = 0;
    green.green = 0xffff/2;
    green.blue = 0;
    blue.red = 0;
    blue.green = 0;
    blue.blue = 0xffff;
    purple.red = 0xffff;
    purple.green = 0;
    purple.blue = 0xffff;
    
    add_pixmap_directory("../gtkLM/");

    TopWindow = create_TopWindow ();
    ErrorDialog = create_errorDialog();
    gtk_widget_show (TopWindow);

    /* "click" on restore state button */
    gtk_signal_emit_by_name(GTK_OBJECT(get_widget(TopWindow, 
					       "restoreStateButton")), 
			    "clicked");
    /* Set CWD */
    gtk_entry_set_text(GTK_ENTRY(get_widget(TopWindow, 
					    "workingDir")), 
		       getcwd(buffer,sizeof(buffer)));
    
    
    gtk_main ();
    return 0;
}

/*
 * Get text from a named entry field (by name)
 */
gchar *
entryGetText(char *entryName)
{
    return gtk_entry_get_text(GTK_ENTRY(get_widget(TopWindow, entryName)));
}

/*
 * Show an error message 
 */
void
showError(char *where, char *message)
{    
    GtkLabel *whereLabel = GTK_LABEL(get_widget(ErrorDialog, "errorWhere"));
    GtkLabel *messageLabel = GTK_LABEL(get_widget(ErrorDialog, "errorMessage"));
    
    HadError = 1;

    gtk_label_set(whereLabel, where);
    gtk_label_set(messageLabel, message);
    gtk_widget_show (ErrorDialog);
}

/*
 * Function used in runProgram: tell parent (via stderr pipe) 
 * about the problem and _exit (can not use exit() under GTK 
 * because it will screw up the parent). 
 */
void
childError(char *where)
{
    fprintf(stderr, "ERROR: %s: %s\n", 
	    where, strerror(errno));
    _exit(1);
}

/* 
 * Run an external program with specified stdin/stdout, 
 * show stderr in log window
 */
pid_t childPid;

void
runProgram(char *program, char *argv[], char *infile, char *outfile)
{
    GtkText *logtext = GTK_TEXT(get_widget(TopWindow, "logtext"));
    char buffer[1024];
    int errpipe[2];
    int i = 0;

    sprintf(buffer, "\n%s ", program);
    gtk_text_insert (logtext, fixed_font, &green , NULL, 
		     buffer, strlen(buffer));
    if (argv) 
	for (i=0; argv[i]; i++) 
	{
	    sprintf(buffer, "%s ", argv[i]);
	    gtk_text_insert (logtext, fixed_font, &green , NULL, 
			     buffer, strlen(buffer));
	}
    sprintf(buffer, " < %s > %s\n", infile, outfile);
    gtk_text_insert (logtext, fixed_font, &green , NULL, 
		     buffer, strlen(buffer));

    if (pipe(errpipe))
    {
	showError("pipe", strerror(errno));
	return;
    }
    
    if ((childPid = fork()) == -1)
    {
	showError("fork", strerror(errno));
	return;
    }
    if (childPid==0) /* child */
    {
	/* switch stderr to errpipe 
	 */
	close(errpipe[0]);
	if (dup2 (errpipe[1], STDERR_FILENO) == -1) 
	    childError("dup"); 
	/* well, this one will print error message 
	 * on terminal since dup2 was not successful :( 
	 */

	if (infile && freopen(infile, "r", stdin) == NULL)
	    childError(infile);

	if (outfile && freopen(outfile, "w", stdout) == NULL)
	    childError(outfile);

       	if (execvp(program, argv) == -1)
	    childError(program);
    }
    else /* parent */
    {
	FILE *errfile;

	close(errpipe[1]);
	/* I want to do string I/O, thus switch to FILE interface
	 */
	if ((errfile = fdopen(errpipe[0], "r"))== NULL)
	{
	    showError("fdopen", strerror(errno));
	    return; /* have to kill child here !!! */
	}
	
	while (fgets(buffer, sizeof(buffer), errfile))
	{
	    if (strncmp(buffer, "ERROR", 3) == 0)
	    {
		char *what, *where, *why;

		gtk_text_insert (logtext, fixed_font, &red , NULL, 
				 buffer, strlen(buffer));
		what = strtok(buffer, ":");
		where = strtok(NULL, ":");
		why = strtok(NULL, "\n");
		showError(where, why);
	    }
	    else if (strncmp(buffer, "WARNING", 4) == 0)
		gtk_text_insert (logtext, fixed_font, &purple , NULL, 
				 buffer, strlen(buffer));
	    else
		gtk_text_insert (logtext, fixed_font, &blue, NULL, 
				 buffer, strlen(buffer));
	    while (gtk_events_pending())
		gtk_main_iteration();
	}
	sprintf(buffer, "%s finished", program);
	gtk_text_insert (logtext, fixed_font, &green , NULL, 
			 buffer, strlen(buffer));

	childPid = 0;
    }
}


/*
 * *************** Signals generated by GLADE **********************
 */

void
on_cif2dxcButton_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    runProgram(entryGetText("cif2dxcPath"),
	       NULL,
	       entryGetText("CIFname"),
	       entryGetText("DXCname"));
}


void
on_lmeterButton_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    char *argv[3];
    
    argv[0] = entryGetText("techname");
    argv[1] = entryGetText("termname");
    argv[2] = NULL;

    runProgram(entryGetText("lmeterPath"),
	       argv,
	       entryGetText("DXCname"),
	       entryGetText("LMoutname"));
}


void
on_lm2schButton_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    char *argv[9];
    GtkButton *valuesUpdateButton = GTK_BUTTON(get_widget(TopWindow, 
					       "valuesUpdateButton"));

    /* lm2sch -n netlist -t terminal names \
     *        -c LMeter output -o output file 
     */
    argv[0] = "-n"; 
    argv[1] = entryGetText("SPICEname");
    argv[2] = "-t"; 
    argv[3] = entryGetText("lm2schTermname");
    argv[4] = "-c"; 
    argv[5] = entryGetText("LMoutname");
    argv[6] = "-o"; 
    argv[7] = entryGetText("valuesname");
    argv[8] = NULL;

    runProgram(entryGetText("lm2schPath"),
	       argv,
	       NULL, 
	       entryGetText("lm2schoutname"));
    
    /* "press" valuesUpdateButton button */
    
    gtk_signal_emit_by_name(GTK_OBJECT(valuesUpdateButton), "clicked");
}


void
on_TopWindow_destroy                   (GtkObject       *object,
                                        gpointer         user_data)
{
    gtk_main_quit();
}


void
on_errorContinue_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide (ErrorDialog);    
}


void
on_valuesUpdateButton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkCList *valuesCList = GTK_CLIST(get_widget(TopWindow, "valuesClist"));
    char *values[4];
    char *valsname = entryGetText("lm2schoutname");
    FILE *valsfile = fopen(valsname, "r");
    char buffer[1024];

    gtk_clist_clear(valuesCList);

    if (valsfile == NULL)
    {
	showError(valsname, strerror(errno));
	return;
    }

    while(fgets(buffer, sizeof(buffer), valsfile))
    {
	char *bptr = buffer;
	while (isspace(*bptr)) bptr++;
	if (bptr[0] == '\0' || bptr[0] == '#') continue; 
	values[0] = strtok(bptr," \t\n");
	values[1] = strtok(NULL," \t\n");
	values[2] = strtok(NULL," \t\n");
	values[3] = strtok(NULL," \t\n");
	gtk_clist_append(valuesCList, values);
    }

    fclose(valsfile);
}


void
on_goButton_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkButton *cif2dxcButton = GTK_BUTTON(get_widget(TopWindow, 
					  "cif2dxcButton"));
    GtkButton *lmeterButton = GTK_BUTTON(get_widget(TopWindow, 
					  "lmeterButton"));
    GtkButton *lm2schButton = GTK_BUTTON(get_widget(TopWindow, 
					  "lm2schButton"));    
    GtkToggleButton *doImportCIF = GTK_TOGGLE_BUTTON(get_widget(TopWindow, 
					  "doImportCIF"));
    GtkToggleButton *doRunLMeter = GTK_TOGGLE_BUTTON(get_widget(TopWindow, 
					  "doRunLMeter"));
    GtkToggleButton *doBackannotate = GTK_TOGGLE_BUTTON(get_widget(TopWindow, 
					  "doBackannotate"));


    HadError = 0;

    /* "press" all buttons in sequence if doXXX is set */
    if (doImportCIF->active)
    {
	gtk_signal_emit_by_name(GTK_OBJECT(cif2dxcButton), "clicked");
	if (HadError) return;
    }

    if (doRunLMeter->active)
    {
	gtk_signal_emit_by_name(GTK_OBJECT(lmeterButton), "clicked");
	if (HadError) return;
    }

    if (doBackannotate->active)
    {
	gtk_signal_emit_by_name(GTK_OBJECT(lm2schButton), "clicked");
	if (HadError) return;
    }

}


void
on_workingDir_activate                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
    char *dir = entryGetText("workingDir");

    if (chdir(dir) == -1)
	showError(dir, strerror(errno));
}


/*
 * This is an utility comparing function (for qsort) which fetches
 * values from a row and compares them (using other params from global
 * variables.
 */
static GtkCList  *sortClist;
static int sortColumn;
static int doNumeric;

static int
rowCompare(const void *pa, const void *pb)
{
    /* a and b are just pointers to row numbers */
    int ra = *(int *)pa;
    int rb = *(int *)pb;
    
    char *va, *vb; /* values */ 

    gtk_clist_get_text(sortClist, ra, sortColumn, &va);
    gtk_clist_get_text(sortClist, rb, sortColumn, &vb);

    if (doNumeric)
    {
	/* N.B. We want absolute value sorting */

	double a = fabs(atof(va)), b = fabs(atof(vb));
	
	if (a<b) return +1;
	if (a>b) return -1;
	return 0;
    }
    else
	return(strcmp(va, vb));
}

/* 
 * Sort by the values in the column (0 - text, 1,2,3 - float) 
 */
void
on_valuesClist_click_column            (GtkCList        *clist,
                                        gint             column,
                                        gpointer         user_data)
{
    int nrows = clist->rows, i;
    int *array;

    sortClist = clist;
    sortColumn = column;

    doNumeric = (column != 0);

    array = g_new(int, nrows);
    
    for (i=0; i<nrows; i++) array[i] = i;

    qsort(array, nrows, sizeof(int), rowCompare);
    
    gtk_clist_freeze(clist);

    /* now append new rows in the right order */
    for (i=0; i<nrows; i++)
    {
	gchar    *text[4];
	int j;
	for (j=0; j<4; j++)
	    gtk_clist_get_text(clist, array[i], j, &(text[j]));
    
	gtk_clist_append(clist, text);
    }
    
    /* and delete the head of the list (old values) */
    for (i=0; i<nrows; i++)
       gtk_clist_remove(clist, 0);

    gtk_clist_thaw(clist);
    
    g_free(array);
}

/*
 * The next pair of functions saves/restores the state to/from 
 * a resource file in user's $HOME.
 */
const char *rcname = ".gtkLMrc";
FILE *rcfileOpen(char *mode)
{
    char *home = getenv("HOME");
    char fname[1024];
    FILE *rcfile;

    sprintf(fname,"%s/%s", home, rcname);
    if ((rcfile = fopen(fname, mode))==NULL)
	showError(fname, strerror(errno));

    return rcfile;
}

void
on_saveStateButton_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
    FILE *rcfile = rcfileOpen("w");

    /* produced by
     * grep entryGetText gladesig.c|sed 's|[^"]*\("[^"]*"\)[^"]*|\1,|g'|sort|uniq 
     */
    char *entryboxes[] = 
    {
	"CIFname",
	"DXCname",
	"LMoutname",
	"SPICEname",
	"cif2dxcPath",
	"lm2schPath",
	"lm2schTermname",
	"lm2schoutname",
	"lmeterPath",
	"techname",
	"termname",
	"valuesname",
	"workingDir",
	NULL
    };
    char *checkboxes[] = 
    {
	"doImportCIF",
	"doRunLMeter",
	"doBackannotate",
	NULL
    };
    int i;
    
    if (rcfile == NULL) return;

    for (i=0; entryboxes[i]; i++)
	fprintf(rcfile,"%s %s\n", 
		entryboxes[i], entryGetText(entryboxes[i]));
    for (i=0; checkboxes[i]; i++)
	    fprintf(rcfile,"%s %d\n", 
		    checkboxes[i], 
		    GTK_TOGGLE_BUTTON(
			get_widget(TopWindow,checkboxes[i]))->active);
    fclose(rcfile);
}

void
on_restoreStateButton_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    FILE *rcfile = rcfileOpen("r");
    char buffer[1024], name[1024], value[1024];

    if (rcfile == NULL) return;
    
    while (fgets(buffer, sizeof(buffer), rcfile))
    {
	int num = sscanf(buffer, "%s %s", name, value);
	GtkWidget *widget = get_widget(TopWindow, name);

	if (num != 2 || widget == NULL) 
	{
	    showError("Bad .gtkLMrc line", buffer);
	    fclose(rcfile);
	    return;
	}
	    
	if (GTK_IS_ENTRY(widget)) /* entrybox */
	    gtk_entry_set_text(GTK_ENTRY(widget), value);

	if (GTK_IS_TOGGLE_BUTTON(widget)) /* check button */
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 
					 atoi(value));
    }
    
}

/*
 * Kill childPid
 */
void
on_cancelButton_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    if (childPid == 0)
	showError("No process running", "");    
    else if (kill(childPid,SIGTERM) == -1)
	showError("kill", strerror(errno));
    else
	showError("Process killed","");
}

