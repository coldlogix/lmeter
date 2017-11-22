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


void
on_cif2dxcButton_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_lmeterButton_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_lm2schButton_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_TopWindow_destroy                   (GtkObject       *object,
                                        gpointer         user_data);

void
on_TopWindow_destroy                   (GtkObject       *object,
                                        gpointer         user_data);

void
on_errorContinue_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_valuesUpdateButton_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_goButton_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_workingDir_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_workingDir_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_chDirButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_workingDir_activate                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_valuesClist_click_column            (GtkCList        *clist,
                                        gint             column,
                                        gpointer         user_data);

void
on_saveStateButton_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_restoreStateButton_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_cancelButton_clicked                (GtkButton       *button,
                                        gpointer         user_data);

