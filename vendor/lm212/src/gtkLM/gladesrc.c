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

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gladesig.h"
#include "gladesrc.h"

GtkWidget*
get_widget                             (GtkWidget       *widget,
                                        gchar           *widget_name)
{
  GtkWidget *parent, *found_widget;

  for (;;)
    {
      if (GTK_IS_MENU (widget))
        parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
      else
        parent = widget->parent;
      if (parent == NULL)
        break;
      widget = parent;
    }

  found_widget = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (widget),
                                                   widget_name);
  if (!found_widget)
    g_warning ("Widget not found: %s", widget_name);
  return found_widget;
}

/* This is an internally used function to set notebook tab widgets. */
void
set_notebook_tab                       (GtkWidget       *notebook,
                                        gint             page_num,
                                        GtkWidget       *widget)
{
  GtkNotebookPage *page;
  GtkWidget *notebook_page;

  page = (GtkNotebookPage*) g_list_nth (GTK_NOTEBOOK (notebook)->children, page_num)->data;
  notebook_page = page->child;
  gtk_widget_ref (notebook_page);
  gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
  gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), notebook_page,
                            widget, page_num);
  gtk_widget_unref (notebook_page);
}

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory                   (gchar           *directory)
{
  pixmaps_directories = g_list_prepend (pixmaps_directories, g_strdup (directory));
}

/* This is an internally used function to check if a pixmap file exists. */
#ifndef G_DIR_SEPARATOR_S
#define G_DIR_SEPARATOR_S "/"
#endif
gchar*
check_file_exists                      (gchar           *directory,
                                        gchar           *filename)
{
  gchar *full_filename;
  struct stat s;
  gint status;

  full_filename = g_malloc (strlen (directory) + 1 + strlen (filename) + 1);
  strcpy (full_filename, directory);
  strcat (full_filename, G_DIR_SEPARATOR_S);
  strcat (full_filename, filename);

  status = stat (full_filename, &s);
  if (status == 0 && S_ISREG (s.st_mode))
    return full_filename;
  g_free (full_filename);
  return NULL;
}

/* This is an internally used function to create pixmaps. */
GtkWidget*
create_pixmap                          (GtkWidget       *widget,
                                        gchar           *filename)
{
  gchar *found_filename = NULL;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;
  GList *elem;

  /* We first try any pixmaps directories set by the application. */
  elem = pixmaps_directories;
  while (elem)
    {
      found_filename = check_file_exists ((gchar*)elem->data, filename);
      if (found_filename)
        break;
      elem = elem->next;
    }

  /* If we haven't found the pixmap, try the source directory. */
  if (!found_filename)
    {
      found_filename = check_file_exists ("", filename);
    }

  if (!found_filename)
    {
      g_print ("Couldn't find pixmap file: %s", filename);
      return NULL;
    }

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
                                                   NULL, found_filename);
  g_free (found_filename);
  if (gdkpixmap == NULL)
    return NULL;
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

GtkWidget*
create_TopWindow ()
{
  GtkWidget *TopWindow;
  GtkWidget *vbox6;
  GtkWidget *hbox1;
  GtkWidget *frame1;
  GtkWidget *vbox5;
  GtkWidget *notebook1;
  GtkWidget *table3;
  GtkWidget *cif2dxcButton;
  GtkWidget *pixmap25;
  GtkWidget *lmeterButton;
  GtkWidget *pixmap17;
  GtkWidget *lm2schButton;
  GtkWidget *pixmap26;
  GtkWidget *goButton;
  GtkWidget *pixmap24;
  GtkWidget *doImportCIF;
  GtkWidget *doRunLMeter;
  GtkWidget *doBackannotate;
  GtkWidget *cancelButton;
  GtkWidget *pixmap27;
  GtkWidget *table1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label26;
  GtkWidget *label5;
  GtkWidget *label7;
  GtkWidget *label32;
  GtkWidget *label8;
  GtkWidget *label33;
  GtkWidget *DXCname;
  GtkWidget *techname;
  GtkWidget *termname;
  GtkWidget *LMoutname;
  GtkWidget *SPICEname;
  GtkWidget *lm2schTermname;
  GtkWidget *lm2schoutname;
  GtkWidget *valuesname;
  GtkWidget *CIFname;
  GtkWidget *table4;
  GtkWidget *label22;
  GtkWidget *label23;
  GtkWidget *label24;
  GtkWidget *cif2dxcPath;
  GtkWidget *lmeterPath;
  GtkWidget *lm2schPath;
  GtkWidget *label13;
  GtkWidget *label12;
  GtkWidget *label21;
  GtkWidget *hbox3;
  GtkWidget *restoreStateButton;
  GtkWidget *saveStateButton;
  GtkWidget *frame2;
  GtkWidget *vbox4;
  GtkWidget *scrolledwindow5;
  GtkWidget *valuesClist;
  GtkWidget *label34;
  GtkWidget *label35;
  GtkWidget *label37;
  GtkWidget *label36;
  GtkWidget *valuesUpdateButton;
  GtkWidget *frame3;
  GtkWidget *scrolledwindow4;
  GtkWidget *logtext;
  GtkWidget *frame4;
  GtkWidget *vbox7;
  GtkWidget *workingDir;

  TopWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "TopWindow", TopWindow);
  gtk_signal_connect (GTK_OBJECT (TopWindow), "destroy",
                      GTK_SIGNAL_FUNC (on_TopWindow_destroy),
                      NULL);
  gtk_window_set_title (GTK_WINDOW (TopWindow), "LMeter Workbench");
  gtk_window_set_wmclass (GTK_WINDOW (TopWindow), "gtkLMeter", "gtkLMeter");

  vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "vbox6", vbox6);
  gtk_widget_show (vbox6);
  gtk_container_add (GTK_CONTAINER (TopWindow), vbox6);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "hbox1", hbox1);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox6), hbox1, TRUE, TRUE, 0);

  frame1 = gtk_frame_new ("Control Center");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "frame1", frame1);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (hbox1), frame1, FALSE, FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (frame1), 5);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "vbox5", vbox5);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (frame1), vbox5);

  notebook1 = gtk_notebook_new ();
  gtk_object_set_data (GTK_OBJECT (TopWindow), "notebook1", notebook1);
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox5), notebook1, FALSE, FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (notebook1), 10);

  table3 = gtk_table_new (3, 3, TRUE);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "table3", table3);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (notebook1), table3);

  cif2dxcButton = gtk_button_new ();
  gtk_object_set_data (GTK_OBJECT (TopWindow), "cif2dxcButton", cif2dxcButton);
  gtk_widget_show (cif2dxcButton);
  gtk_table_attach (GTK_TABLE (table3), cif2dxcButton, 2, 3, 0, 1,
                    (GtkAttachOptions) GTK_EXPAND, (GtkAttachOptions) GTK_EXPAND, 0, 0);
  gtk_signal_connect (GTK_OBJECT (cif2dxcButton), "clicked",
                      GTK_SIGNAL_FUNC (on_cif2dxcButton_clicked),
                      NULL);

  pixmap25 = create_pixmap (TopWindow, "cif2dxc.xpm");
  if (pixmap25 == NULL)
    g_error ("Couldn't create pixmap");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "pixmap25", pixmap25);
  gtk_widget_show (pixmap25);
  gtk_container_add (GTK_CONTAINER (cif2dxcButton), pixmap25);

  lmeterButton = gtk_button_new ();
  gtk_object_set_data (GTK_OBJECT (TopWindow), "lmeterButton", lmeterButton);
  gtk_widget_show (lmeterButton);
  gtk_table_attach (GTK_TABLE (table3), lmeterButton, 2, 3, 1, 2,
                    (GtkAttachOptions) GTK_EXPAND, (GtkAttachOptions) GTK_EXPAND, 0, 0);
  gtk_signal_connect (GTK_OBJECT (lmeterButton), "clicked",
                      GTK_SIGNAL_FUNC (on_lmeterButton_clicked),
                      NULL);

  pixmap17 = create_pixmap (TopWindow, "LMeter.xpm");
  if (pixmap17 == NULL)
    g_error ("Couldn't create pixmap");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "pixmap17", pixmap17);
  gtk_widget_show (pixmap17);
  gtk_container_add (GTK_CONTAINER (lmeterButton), pixmap17);

  lm2schButton = gtk_button_new ();
  gtk_object_set_data (GTK_OBJECT (TopWindow), "lm2schButton", lm2schButton);
  gtk_widget_show (lm2schButton);
  gtk_table_attach (GTK_TABLE (table3), lm2schButton, 2, 3, 2, 3,
                    (GtkAttachOptions) GTK_EXPAND, (GtkAttachOptions) GTK_EXPAND, 0, 0);
  gtk_signal_connect (GTK_OBJECT (lm2schButton), "clicked",
                      GTK_SIGNAL_FUNC (on_lm2schButton_clicked),
                      NULL);

  pixmap26 = create_pixmap (TopWindow, "lm2sch.xpm");
  if (pixmap26 == NULL)
    g_error ("Couldn't create pixmap");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "pixmap26", pixmap26);
  gtk_widget_show (pixmap26);
  gtk_container_add (GTK_CONTAINER (lm2schButton), pixmap26);

  goButton = gtk_button_new ();
  gtk_object_set_data (GTK_OBJECT (TopWindow), "goButton", goButton);
  gtk_widget_show (goButton);
  gtk_table_attach (GTK_TABLE (table3), goButton, 0, 1, 0, 1,
                    (GtkAttachOptions) 0, (GtkAttachOptions) 0, 0, 0);
  gtk_signal_connect (GTK_OBJECT (goButton), "clicked",
                      GTK_SIGNAL_FUNC (on_goButton_clicked),
                      NULL);

  pixmap24 = create_pixmap (TopWindow, "keepmoving.xpm");
  if (pixmap24 == NULL)
    g_error ("Couldn't create pixmap");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "pixmap24", pixmap24);
  gtk_widget_show (pixmap24);
  gtk_container_add (GTK_CONTAINER (goButton), pixmap24);

  doImportCIF = gtk_check_button_new_with_label (" Import CIF");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "doImportCIF", doImportCIF);
  gtk_widget_show (doImportCIF);
  gtk_table_attach (GTK_TABLE (table3), doImportCIF, 1, 2, 0, 1,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (doImportCIF), TRUE);

  doRunLMeter = gtk_check_button_new_with_label (" Run LMeter");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "doRunLMeter", doRunLMeter);
  gtk_widget_show (doRunLMeter);
  gtk_table_attach (GTK_TABLE (table3), doRunLMeter, 1, 2, 1, 2,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (doRunLMeter), TRUE);

  doBackannotate = gtk_check_button_new_with_label (" Backannotate");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "doBackannotate", doBackannotate);
  gtk_widget_show (doBackannotate);
  gtk_table_attach (GTK_TABLE (table3), doBackannotate, 1, 2, 2, 3,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (doBackannotate), TRUE);

  cancelButton = gtk_button_new ();
  gtk_object_set_data (GTK_OBJECT (TopWindow), "cancelButton", cancelButton);
  gtk_widget_show (cancelButton);
  gtk_table_attach (GTK_TABLE (table3), cancelButton, 0, 1, 2, 3,
                    (GtkAttachOptions) 0, (GtkAttachOptions) 0, 0, 0);
  gtk_signal_connect (GTK_OBJECT (cancelButton), "clicked",
                      GTK_SIGNAL_FUNC (on_cancelButton_clicked),
                      NULL);

  pixmap27 = create_pixmap (TopWindow, "cancel.xpm");
  if (pixmap27 == NULL)
    g_error ("Couldn't create pixmap");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "pixmap27", pixmap27);
  gtk_widget_show (pixmap27);
  gtk_container_add (GTK_CONTAINER (cancelButton), pixmap27);

  table1 = gtk_table_new (9, 2, TRUE);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "table1", table1);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (notebook1), table1);

  label2 = gtk_label_new ("CIF file");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label2", label2);
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label3 = gtk_label_new ("DXC file");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label3", label3);
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 1, 2,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label4 = gtk_label_new ("Technology file");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label4", label4);
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 2, 3,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label26 = gtk_label_new ("LMeter terminals file");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label26", label26);
  gtk_widget_show (label26);
  gtk_table_attach (GTK_TABLE (table1), label26, 0, 1, 3, 4,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label5 = gtk_label_new ("LMeter results");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label5", label5);
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 4, 5,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label7 = gtk_label_new ("SPICE netlist");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label7", label7);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 5, 6,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label32 = gtk_label_new ("lm2sch terminals file");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label32", label32);
  gtk_widget_show (label32);
  gtk_table_attach (GTK_TABLE (table1), label32, 0, 1, 6, 7,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label8 = gtk_label_new ("lm2sch results (readable)");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label8", label8);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table1), label8, 0, 1, 7, 8,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label33 = gtk_label_new ("lm2sch results (plain)");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label33", label33);
  gtk_widget_show (label33);
  gtk_table_attach (GTK_TABLE (table1), label33, 0, 1, 8, 9,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  DXCname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "DXCname", DXCname);
  gtk_widget_show (DXCname);
  gtk_table_attach (GTK_TABLE (table1), DXCname, 1, 2, 1, 2,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (DXCname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (DXCname);
  gtk_entry_set_text (GTK_ENTRY (DXCname), "lmeter.dxc");

  techname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "techname", techname);
  gtk_widget_show (techname);
  gtk_table_attach (GTK_TABLE (table1), techname, 1, 2, 2, 3,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_entry_set_text (GTK_ENTRY (techname), "lm.dfl");

  termname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "termname", termname);
  gtk_widget_show (termname);
  gtk_table_attach (GTK_TABLE (table1), termname, 1, 2, 3, 4,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_entry_set_text (GTK_ENTRY (termname), "lmeter.term");

  LMoutname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "LMoutname", LMoutname);
  gtk_widget_show (LMoutname);
  gtk_table_attach (GTK_TABLE (table1), LMoutname, 1, 2, 4, 5,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (LMoutname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (LMoutname);
  gtk_entry_set_text (GTK_ENTRY (LMoutname), "lmeter.out");

  SPICEname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "SPICEname", SPICEname);
  gtk_widget_show (SPICEname);
  gtk_table_attach (GTK_TABLE (table1), SPICEname, 1, 2, 5, 6,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (SPICEname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (SPICEname);
  gtk_entry_set_text (GTK_ENTRY (SPICEname), "netlist.spi");

  lm2schTermname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "lm2schTermname", lm2schTermname);
  gtk_widget_show (lm2schTermname);
  gtk_table_attach (GTK_TABLE (table1), lm2schTermname, 1, 2, 6, 7,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (lm2schTermname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (lm2schTermname);
  gtk_entry_set_text (GTK_ENTRY (lm2schTermname), "term.name");

  lm2schoutname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "lm2schoutname", lm2schoutname);
  gtk_widget_show (lm2schoutname);
  gtk_table_attach (GTK_TABLE (table1), lm2schoutname, 1, 2, 7, 8,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (lm2schoutname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (lm2schoutname);
  gtk_entry_set_text (GTK_ENTRY (lm2schoutname), "values.out");

  valuesname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "valuesname", valuesname);
  gtk_widget_show (valuesname);
  gtk_table_attach (GTK_TABLE (table1), valuesname, 1, 2, 8, 9,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (valuesname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (valuesname);
  gtk_entry_set_text (GTK_ENTRY (valuesname), "lm2sch.out");

  CIFname = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "CIFname", CIFname);
  gtk_widget_show (CIFname);
  gtk_table_attach (GTK_TABLE (table1), CIFname, 1, 2, 0, 1,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  GTK_WIDGET_SET_FLAGS (CIFname, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (CIFname);
  gtk_entry_set_text (GTK_ENTRY (CIFname), "lmeter.cif");

  table4 = gtk_table_new (9, 2, TRUE);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "table4", table4);
  gtk_widget_show (table4);
  gtk_container_add (GTK_CONTAINER (notebook1), table4);

  label22 = gtk_label_new ("CIF to DXC converter");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label22", label22);
  gtk_widget_show (label22);
  gtk_table_attach (GTK_TABLE (table4), label22, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label23 = gtk_label_new ("LMeter");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label23", label23);
  gtk_widget_show (label23);
  gtk_table_attach (GTK_TABLE (table4), label23, 0, 1, 1, 2,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  label24 = gtk_label_new ("Backannotator (lm2sch)");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label24", label24);
  gtk_widget_show (label24);
  gtk_table_attach (GTK_TABLE (table4), label24, 0, 1, 2, 3,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);

  cif2dxcPath = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "cif2dxcPath", cif2dxcPath);
  gtk_widget_show (cif2dxcPath);
  gtk_table_attach (GTK_TABLE (table4), cif2dxcPath, 1, 2, 0, 1,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_entry_set_text (GTK_ENTRY (cif2dxcPath), "../../bin/cif2dxc");

  lmeterPath = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "lmeterPath", lmeterPath);
  gtk_widget_show (lmeterPath);
  gtk_table_attach (GTK_TABLE (table4), lmeterPath, 1, 2, 1, 2,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_entry_set_text (GTK_ENTRY (lmeterPath), "../../bin/lmeter");

  lm2schPath = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "lm2schPath", lm2schPath);
  gtk_widget_show (lm2schPath);
  gtk_table_attach (GTK_TABLE (table4), lm2schPath, 1, 2, 2, 3,
                    (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_entry_set_text (GTK_ENTRY (lm2schPath), "../../bin/lm2sch");

  label13 = gtk_label_new ("Design Flow");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label13", label13);
  gtk_widget_show (label13);
  set_notebook_tab (notebook1, 0, label13);

  label12 = gtk_label_new ("File Names");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label12", label12);
  gtk_widget_show (label12);
  set_notebook_tab (notebook1, 1, label12);

  label21 = gtk_label_new ("Program Names");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label21", label21);
  gtk_widget_show (label21);
  set_notebook_tab (notebook1, 2, label21);

  hbox3 = gtk_hbox_new (TRUE, 0);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "hbox3", hbox3);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox5), hbox3, FALSE, FALSE, 0);

  restoreStateButton = gtk_button_new_with_label ("Restore");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "restoreStateButton", restoreStateButton);
  gtk_widget_show (restoreStateButton);
  gtk_box_pack_end (GTK_BOX (hbox3), restoreStateButton, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (restoreStateButton), 10);
  gtk_signal_connect (GTK_OBJECT (restoreStateButton), "clicked",
                      GTK_SIGNAL_FUNC (on_restoreStateButton_clicked),
                      NULL);

  saveStateButton = gtk_button_new_with_label ("Save");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "saveStateButton", saveStateButton);
  gtk_widget_show (saveStateButton);
  gtk_box_pack_end (GTK_BOX (hbox3), saveStateButton, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (saveStateButton), 10);
  gtk_signal_connect (GTK_OBJECT (saveStateButton), "clicked",
                      GTK_SIGNAL_FUNC (on_saveStateButton_clicked),
                      NULL);

  frame2 = gtk_frame_new ("Results");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "frame2", frame2);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (hbox1), frame2, TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (frame2), 5);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "vbox4", vbox4);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (frame2), vbox4);

  scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "scrolledwindow5", scrolledwindow5);
  gtk_widget_show (scrolledwindow5);
  gtk_box_pack_start (GTK_BOX (vbox4), scrolledwindow5, TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (scrolledwindow5), 10);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  valuesClist = gtk_clist_new (4);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "valuesClist", valuesClist);
  gtk_widget_show (valuesClist);
  gtk_container_add (GTK_CONTAINER (scrolledwindow5), valuesClist);
  gtk_signal_connect (GTK_OBJECT (valuesClist), "click_column",
                      GTK_SIGNAL_FUNC (on_valuesClist_click_column),
                      NULL);
  gtk_clist_set_column_width (GTK_CLIST (valuesClist), 0, 80);
  gtk_clist_set_column_width (GTK_CLIST (valuesClist), 1, 80);
  gtk_clist_set_column_width (GTK_CLIST (valuesClist), 2, 80);
  gtk_clist_set_column_width (GTK_CLIST (valuesClist), 3, 80);
  gtk_clist_column_titles_show (GTK_CLIST (valuesClist));

  label34 = gtk_label_new ("Name");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label34", label34);
  gtk_widget_show (label34);
  gtk_clist_set_column_widget (GTK_CLIST (valuesClist), 0, label34);

  label35 = gtk_label_new ("Layout");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label35", label35);
  gtk_widget_show (label35);
  gtk_clist_set_column_widget (GTK_CLIST (valuesClist), 1, label35);

  label37 = gtk_label_new ("Netlist");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label37", label37);
  gtk_widget_show (label37);
  gtk_clist_set_column_widget (GTK_CLIST (valuesClist), 2, label37);

  label36 = gtk_label_new ("Difference");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "label36", label36);
  gtk_widget_show (label36);
  gtk_clist_set_column_widget (GTK_CLIST (valuesClist), 3, label36);

  valuesUpdateButton = gtk_button_new_with_label ("Update");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "valuesUpdateButton", valuesUpdateButton);
  gtk_widget_show (valuesUpdateButton);
  gtk_box_pack_end (GTK_BOX (vbox4), valuesUpdateButton, FALSE, FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (valuesUpdateButton), 10);
  gtk_signal_connect (GTK_OBJECT (valuesUpdateButton), "clicked",
                      GTK_SIGNAL_FUNC (on_valuesUpdateButton_clicked),
                      NULL);

  frame3 = gtk_frame_new ("Run Log");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "frame3", frame3);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox6), frame3, TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (frame3), 5);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "scrolledwindow4", scrolledwindow4);
  gtk_widget_show (scrolledwindow4);
  gtk_container_add (GTK_CONTAINER (frame3), scrolledwindow4);
  gtk_container_border_width (GTK_CONTAINER (scrolledwindow4), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  logtext = gtk_text_new (NULL, NULL);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "logtext", logtext);
  gtk_widget_show (logtext);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), logtext);
  gtk_widget_set_usize (logtext, 640, 200);

  frame4 = gtk_frame_new ("Current directory");
  gtk_object_set_data (GTK_OBJECT (TopWindow), "frame4", frame4);
  gtk_widget_show (frame4);
  gtk_box_pack_end (GTK_BOX (vbox6), frame4, FALSE, FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (frame4), 5);

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "vbox7", vbox7);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (frame4), vbox7);
  gtk_container_border_width (GTK_CONTAINER (vbox7), 5);

  workingDir = gtk_entry_new_with_max_length (256);
  gtk_object_set_data (GTK_OBJECT (TopWindow), "workingDir", workingDir);
  gtk_widget_show (workingDir);
  gtk_box_pack_start (GTK_BOX (vbox7), workingDir, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (workingDir, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (workingDir);
  gtk_signal_connect (GTK_OBJECT (workingDir), "activate",
                      GTK_SIGNAL_FUNC (on_workingDir_activate),
                      NULL);
  gtk_entry_set_text (GTK_ENTRY (workingDir), ".");

  return TopWindow;
}

GtkWidget*
create_errorDialog ()
{
  GtkWidget *errorDialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox3;
  GtkWidget *pixmap21;
  GtkWidget *errorWhere;
  GtkWidget *errorMessage;
  GtkWidget *dialog_action_area1;
  GtkWidget *errorContinue;

  errorDialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorDialog", errorDialog);
  gtk_container_border_width (GTK_CONTAINER (errorDialog), 10);
  gtk_window_set_title (GTK_WINDOW (errorDialog), "Error");
  gtk_window_position (GTK_WINDOW (errorDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (errorDialog), FALSE, FALSE, TRUE);

  dialog_vbox1 = GTK_DIALOG (errorDialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (errorDialog), "dialog_vbox1", dialog_vbox1);
  gtk_widget_show (dialog_vbox1);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (errorDialog), "vbox3", vbox3);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox3, TRUE, TRUE, 10);

  pixmap21 = create_pixmap (errorDialog, "caution.xpm");
  if (pixmap21 == NULL)
    g_error ("Couldn't create pixmap");
  gtk_object_set_data (GTK_OBJECT (errorDialog), "pixmap21", pixmap21);
  gtk_widget_show (pixmap21);
  gtk_box_pack_start (GTK_BOX (vbox3), pixmap21, TRUE, TRUE, 0);

  errorWhere = gtk_label_new ("errorWhere");
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorWhere", errorWhere);
  gtk_widget_show (errorWhere);
  gtk_box_pack_start (GTK_BOX (vbox3), errorWhere, TRUE, TRUE, 10);

  errorMessage = gtk_label_new ("errorMessage");
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorMessage", errorMessage);
  gtk_widget_show (errorMessage);
  gtk_box_pack_start (GTK_BOX (vbox3), errorMessage, TRUE, TRUE, 10);

  dialog_action_area1 = GTK_DIALOG (errorDialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (errorDialog), "dialog_action_area1", dialog_action_area1);
  gtk_widget_show (dialog_action_area1);
  gtk_container_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  errorContinue = gtk_button_new_with_label ("Continue");
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorContinue", errorContinue);
  gtk_widget_show (errorContinue);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), errorContinue, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (errorContinue), "clicked",
                      GTK_SIGNAL_FUNC (on_errorContinue_clicked),
                      NULL);

  return errorDialog;
}

