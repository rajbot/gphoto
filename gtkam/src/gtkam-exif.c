/* gtkam-exif.c
 *
 * Copyright (C) 2001 Lutz M�ller <urc8@rz.uni-karlsruhe.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gtkam-exif.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include <string.h>

#include <gtk/gtkbutton.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkbox.h>
#include <gtk/gtklabel.h>

#ifdef HAVE_EXIF
#  include <libexif-gtk/gtk-exif-browser.h>
#endif

#include "gtkam-cancel.h"
#include "gtkam-error.h"

struct _GtkamExifPrivate
{
};

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class;

static void
gtkam_exif_destroy (GtkObject *object)
{
	GtkamExif *exif = GTKAM_EXIF (object);

	exif = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gtkam_exif_finalize (GObject *object)
{
	GtkamExif *exif = GTKAM_EXIF (object);

	g_free (exif->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtkam_exif_class_init (gpointer g_class, gpointer class_data)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtkam_exif_destroy;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtkam_exif_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtkam_exif_init (GTypeInstance *instance, gpointer g_class)
{
	GtkamExif *exif = GTKAM_EXIF (instance);

	exif->priv = g_new0 (GtkamExifPrivate, 1);
}

GType
gtkam_exif_get_type (void)
{
	GTypeInfo ti;

	memset (&ti, 0, sizeof (GTypeInfo));
	ti.class_size     = sizeof (GtkamExifClass);
	ti.class_init     = gtkam_exif_class_init;
	ti.instance_size  = sizeof (GtkamExif);
	ti.instance_init  = gtkam_exif_init; 

	return (g_type_register_static (PARENT_TYPE, "GtkamExif", &ti, 0));
}

static void
on_exif_close_clicked (GtkButton *button, GtkamExif *exif)
{
	gtk_object_destroy (GTK_OBJECT (exif));
}

GtkWidget *
gtkam_exif_new (Camera *camera, gboolean multi,
		const gchar *folder, const gchar *file, GtkWidget *opt_window)
{
	GtkamExif *exif;
	GtkWidget *button;
#ifdef HAVE_EXIF
	GtkWidget *dialog, *browser, *c;
	CameraFile *cfile;
	int result;
	const char *data;
	long int size;
	ExifData *edata;
#else
	GtkWidget *label;
#endif

	g_return_val_if_fail (camera != NULL, NULL);

#ifdef HAVE_EXIF
	/* Get exif data */
	gp_file_new (&cfile);
	c = gtkam_cancel_new (opt_window,
		_("Getting EXIF information for file '%s' in "
		"folder '%s'..."), file, folder);
	gtk_widget_show (c);
	result = gp_camera_file_get (camera, folder, file, GP_FILE_TYPE_EXIF,
			     cfile, GTKAM_CANCEL (c)->context->context);
	if (multi)
		gp_camera_exit (camera, NULL);
	switch (result) {
	case GP_OK:
		break;
	case GP_ERROR_CANCEL:
		gtk_object_destroy (GTK_OBJECT (c));
		return (NULL);
	default:
		gp_file_unref (cfile);
		dialog = gtkam_error_new (result, GTKAM_CANCEL (c)->context,
			opt_window, _("Could not get exif information for "
			"'%s' in folder '%s'"), file, folder);
		gtk_widget_show (dialog);
		gtk_object_destroy (GTK_OBJECT (c));
		return (NULL);
	}
	gtk_object_destroy (GTK_OBJECT (c));

	gp_file_get_data_and_size (cfile, &data, &size);
	edata = exif_data_new_from_data (data, size);
	gp_file_unref (cfile);
	if (!edata) {
		dialog = gtkam_error_new (GP_ERROR_CORRUPTED_DATA, NULL,
			opt_window, _("Could not interpret EXIF data."));
		gtk_widget_show (dialog);
		return (NULL);
	}
#endif

	exif = g_object_new (GTKAM_TYPE_EXIF, NULL);

#ifdef HAVE_EXIF
	browser = gtk_exif_browser_new ();
	gtk_exif_browser_set_data (GTK_EXIF_BROWSER (browser), edata);
	exif_data_unref (edata);
	gtk_widget_show (browser);
	gtk_container_set_border_width (GTK_CONTAINER (browser), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (exif)->vbox), browser,
			    TRUE, TRUE, 0);
#else
	label = gtk_label_new (_("Gtkam has been compiled without exif "
		"support."));
	gtk_widget_show (label);
	gtk_container_set_border_width (GTK_CONTAINER (label), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (exif)->vbox), label,
			    TRUE, TRUE, 0);
#endif

	button = gtk_button_new_with_label (_("Close"));
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_exif_close_clicked), exif);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (exif)->action_area),
			   button);
	gtk_widget_grab_focus (button);

	if (opt_window)
		gtk_window_set_transient_for (GTK_WINDOW (exif),
					      GTK_WINDOW (opt_window));

	return (GTK_WIDGET (exif));
}
