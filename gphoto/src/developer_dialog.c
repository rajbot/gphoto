/* $Id$
 *
 * gPhoto - free digital camera utility - http://www.gphoto.org/
 *
 * Copyright (C) 2000 Scott Fritzinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Log$
 * Revision 1.8  2000/03/24 12:28:28  ole
 * More markups for i18n support.
 *
 * Revision 1.7  1999/06/22 16:13:58  gdr
 * Remove DOCDIR from src/Makefile.am and put into config.h. Well
 * defines.h really but that gets included into config.h.
 *
 * Revision 1.6  1999/06/22 12:17:12  pauljenn
 *
 *
 * included config.h in sources requiring VERSION to be defined
 *
 * Revision 1.5  1999/06/21 18:04:11  ole
 * 1999-06-21  Ole Aamot  <oleaa@ifi.uio.no>
 *
 * 	* callbacks.c: changed activate_button(..) a bit
 * 	* developer_dialog.c: added a clickable webpage button. :o)
 * 	* gallery.c: added browse_gallery();
 * 	* main.c: added browse_gphoto();
 * 	* menu.c: added menu-links to www.gphoto.org (loads in a BROWSER)
 * 	* toolbar.c: added/changed icons (by tigert) for the plugins
 * 	* util.c: added url_send_browser(..), and browse_* web routines.
 *
 * Revision 1.4  1999/06/18 15:57:35  gdr
 * Get version number from configure.in rather than hardcode it in a string.
 *
 * Revision 1.3  1999/06/15 16:05:03  scottf
 * removed the #ifdef's for gtk < 1.2
 * windows show in middle of screen now instead of random places :)
 *
 * Revision 1.2  1999/06/04 15:37:33  gdr
 * Fix for DOCDIR directory.
 *
 * Revision 1.1.1.1  1999/05/27 18:32:07  scottf
 * gPhoto- digital camera utility
 *
 * Revision 1.10  1999/05/09 15:23:38  ole
 * AUTHORS for developers, CREDITS for beta-testers. :-)
 *
 * Revision 1.9  1999/05/09 05:47:27  gdr
 * Changed hard coded path (/usr/doc) to Makefile variable (DOCDIR)
 *
 * Revision 1.8  1999/05/08 04:10:14  scottf
 * added -I/usr/lib/glib/include to paths in some Makefiles
 * and fixed a path in developer_dialog.c (/usr/doc/gphoto-0.3).
 * finally, added "image_type" to the Image struct
 *
 * Revision 1.7  1999/05/05 16:57:19  ole
 * Second changes...
 *
 * Revision 1.6  1999/05/05 16:55:29  ole
 * Misc. changes.
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gtk/gtk.h"
#include "developer.h"
#include "config.h"
#include "gphoto.h"

#define FILE_NAME "AUTHORS"

static int  developer_dialog_hide (GtkWidget *widget, gpointer data);
static int  developer_show_next (GtkWidget *widget, gpointer data);
static void read_developer_file(char *filename);

static GtkWidget *developer_dialog = NULL;
static GtkWidget *developer_label;
static char **    developer_text = NULL;
static int        developer_count = 0;
static int        old_show_developer;

int show_developer = TRUE;
int last_developer = -1;

extern char *gphotoDir;
extern void browse_team();

void
developer_dialog_create ()
{
  GtkWidget *vbox, *hbox1, *hbox2, *bbox, *vbox_bbox2, *bbox2;
  GtkWidget *frame, *preview;
  GtkWidget *button_close, *button_next, *button_prev, *button_team;
  guchar *   utemp;
  guchar *   src;
  guchar *   dest;
  int        x;
  int        y;

  if (developer_count == 0)
    {
      gchar  *   temp;
      temp = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s",
			      DOCDIR, FILE_NAME);

      read_developer_file (temp);
      g_free (temp);
    }
  
  if (last_developer >= developer_count || last_developer < 0)
    last_developer = 0;
  
  if (!developer_dialog)
    {
      char title[256];
      
      developer_dialog = gtk_window_new (GTK_WINDOW_DIALOG);
      gtk_window_set_wmclass (GTK_WINDOW (developer_dialog), "developer", "gPhoto");
      sprintf(title, N_("gPhoto release %s was brought to you by"), VERSION);
      gtk_window_set_title (GTK_WINDOW (developer_dialog), title);
      gtk_window_set_position (GTK_WINDOW (developer_dialog), GTK_WIN_POS_CENTER);
      gtk_signal_connect (GTK_OBJECT (developer_dialog), "delete_event",
			  GTK_SIGNAL_FUNC (developer_dialog_hide), NULL);
      gtk_quit_add_destroy (1, GTK_OBJECT (developer_dialog));

      vbox = gtk_vbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (developer_dialog), vbox);
      gtk_widget_show (vbox);

      hbox1 = gtk_hbox_new (FALSE, 5);
      gtk_container_set_border_width (GTK_CONTAINER (hbox1), 10);
      gtk_box_pack_start (GTK_BOX (vbox), hbox1, FALSE, TRUE, 0);
      gtk_widget_show (hbox1);

      hbox2 = gtk_hbox_new (FALSE, 5);
      gtk_container_set_border_width (GTK_CONTAINER (hbox2), 10);
      gtk_box_pack_end (GTK_BOX (vbox), hbox2, FALSE, TRUE, 0);
      gtk_widget_show (hbox2);
      
      bbox = gtk_hbutton_box_new ();
      gtk_box_pack_end (GTK_BOX (hbox2), bbox, FALSE, FALSE, 0);
      gtk_widget_show (bbox);

      vbox_bbox2 = gtk_vbox_new (FALSE, 0);
      gtk_box_pack_end (GTK_BOX (hbox2), vbox_bbox2, FALSE, FALSE, 15);
      gtk_widget_show (vbox_bbox2);

      bbox2 = gtk_hbox_new (TRUE, 5); 
      gtk_box_pack_end (GTK_BOX (vbox_bbox2), bbox2, TRUE, FALSE, 0);
      gtk_widget_show(bbox2);

      preview = gtk_preview_new (GTK_PREVIEW_COLOR);
      gtk_preview_size (GTK_PREVIEW (preview), developer_width, developer_height);
      utemp = g_new (guchar, developer_width * 3);
      src = (guchar *)developer_data;
      for (y = 0; y < developer_height; y++)
	{
	  dest = utemp;
	  for (x = 0; x < developer_width; x++)
	    {
	      HEADER_PIXEL(src, dest);
	      dest += 3;
	    }
	  gtk_preview_draw_row (GTK_PREVIEW (preview), utemp,
				0, y, developer_width); 
	}
      g_free(utemp);

      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
      gtk_box_pack_end (GTK_BOX (hbox1), frame, FALSE, TRUE, 3);
      gtk_container_add (GTK_CONTAINER (frame), preview);
      gtk_widget_show (preview);
      gtk_widget_show (frame);

      developer_label = gtk_label_new (developer_text[last_developer]);
      gtk_label_set_justify (GTK_LABEL (developer_label), GTK_JUSTIFY_LEFT);
      gtk_box_pack_start (GTK_BOX (hbox1), developer_label, TRUE, TRUE, 3);
      gtk_widget_show (developer_label);


      button_team = gtk_button_new_with_label (N_("Web Page"));
      GTK_WIDGET_UNSET_FLAGS (button_team, GTK_RECEIVES_DEFAULT);
      gtk_signal_connect (GTK_OBJECT (button_team), "clicked",
			  GTK_SIGNAL_FUNC (browse_team), NULL);
      gtk_container_add (GTK_CONTAINER (bbox2), button_team);
      gtk_widget_show (button_team);
      
      button_prev = gtk_button_new_with_label (N_("Previous"));
      GTK_WIDGET_UNSET_FLAGS (button_prev, GTK_RECEIVES_DEFAULT);
      gtk_signal_connect (GTK_OBJECT (button_prev), "clicked",
			  GTK_SIGNAL_FUNC (developer_show_next),
			  (gpointer) "prev");
      gtk_container_add (GTK_CONTAINER (bbox2), button_prev);
      gtk_widget_show (button_prev);

      button_next = gtk_button_new_with_label (N_("Next"));
      GTK_WIDGET_UNSET_FLAGS (button_next, GTK_RECEIVES_DEFAULT);
      gtk_signal_connect (GTK_OBJECT (button_next), "clicked",
			  GTK_SIGNAL_FUNC (developer_show_next),
			  (gpointer) "next");
      gtk_container_add (GTK_CONTAINER (bbox2), button_next);
      gtk_widget_show (button_next);

      button_close = gtk_button_new_with_label (N_("Close"));
      GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
      gtk_window_set_default (GTK_WINDOW (developer_dialog), button_close);
      gtk_signal_connect (GTK_OBJECT (button_close), "clicked",
			  GTK_SIGNAL_FUNC (developer_dialog_hide), NULL);
      gtk_container_add (GTK_CONTAINER (bbox), button_close);
      gtk_widget_show (button_close);

      old_show_developer = show_developer;
    }

  if (!GTK_WIDGET_VISIBLE (developer_dialog))
    {
      gtk_widget_show (developer_dialog);
    }
  else
    {
      gdk_window_raise (developer_dialog->window);
    }
}
static int
developer_dialog_hide (GtkWidget *widget,
		  gpointer data)
{
  gtk_widget_hide (developer_dialog);
  return TRUE;
}

static int
developer_show_next (GtkWidget *widget,
		gpointer  data)
{
  if (!strcmp ((char *)data, "prev"))
    {
      last_developer--;
      if (last_developer < 0)
	last_developer = developer_count - 1;
    }
  else
    {
      last_developer++;
      if (last_developer >= developer_count)
	last_developer = 0;
    }
  gtk_label_set (GTK_LABEL (developer_label), developer_text[last_developer]);
  return FALSE;
}

static void
store_developer (char *str)
{
  developer_count++;
  developer_text = g_realloc(developer_text, sizeof(char *) * developer_count);
  developer_text[developer_count - 1] = str;
}

static void
read_developer_file (char *filename)
{
  
  FILE *fp;
  char *developer = NULL;
  char *str = NULL;

  fp = fopen (filename, "r");
  if (!fp)
    {
      char msg[1024];	
      sprintf(msg,
              N_("The gPhoto AUTHORS file appears to be missing!\n\
	      There should be a file called " FILE_NAME " in the\n\
	      %s directory.\nPlease check your installation."),
	      DOCDIR);
      store_developer(msg);
      return;
    }
  
  str = g_new (char, 1024);
  while (!feof (fp))
    {
      if (!fgets (str, 1024, fp))
	continue;
      
      if (str[0] == '#' || str[0] == '\n')
	{
	  if (developer != NULL)
	    {
	      developer[strlen (developer) - 1] = '\000';
	      store_developer (developer);
	      developer = NULL;
	    }
	}
      else
	{
	  if (developer == NULL)
	    {
	      developer = g_malloc (strlen (str) + 1);
	      strcpy (developer, str);
	    }
	  else
	    {
	      developer = g_realloc (developer, strlen (developer) + strlen (str) + 1);
	      strcat (developer, str);
	    }
	}
    }
  if (developer != NULL)
    store_developer (developer);
  g_free (str);
  fclose (fp);
}
