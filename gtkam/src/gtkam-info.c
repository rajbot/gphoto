/* gtkam-info.c
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
#include "gtkam-info.h"

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

#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktable.h>
#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkimage.h>

#include "gtkam-cancel.h"
#include "gtkam-error.h"
#include "gtkam-status.h"

struct _GtkamInfoPrivate
{
	Camera *camera;
	gboolean multi;

	CameraFileInfo info;
	CameraFileInfo info_new;

	gchar *path;
	gchar *new_path;

	GtkWidget *button_apply, *button_reset;
	gboolean needs_update;

	GtkWidget *check_read, *check_delete;
	GtkWidget *entry_name;
};

#define PARENT_TYPE GTK_TYPE_DIALOG
static GtkDialogClass *parent_class;

enum {
	INFO_UPDATED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
gtkam_info_destroy (GtkObject *object)
{
	GtkamInfo *info = GTKAM_INFO (object);

	if (info->priv->camera) {
		gp_camera_unref (info->priv->camera);
		info->priv->camera = NULL;
	}

	if (info->priv->path) {
		g_free (info->priv->path);
		info->priv->path = NULL;
	}

	if (info->priv->new_path) {
		g_free (info->priv->new_path);
		info->priv->new_path = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gtkam_info_finalize (GObject *object)
{
	GtkamInfo *info = GTKAM_INFO (object);

	g_free (info->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtkam_info_class_init (gpointer g_class, gpointer class_data)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtkam_info_destroy;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtkam_info_finalize;

	signals[INFO_UPDATED] = g_signal_new ("info_updated",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GtkamInfoClass, info_updated), NULL, NULL,
		g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		G_TYPE_POINTER);

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtkam_info_init (GTypeInstance *instance, gpointer g_class)
{
	GtkamInfo *info = GTKAM_INFO (instance);

	info->priv = g_new0 (GtkamInfoPrivate, 1);
}

GType
gtkam_info_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size     = sizeof (GtkamInfoClass);
		ti.class_init     = gtkam_info_class_init;
		ti.instance_size  = sizeof (GtkamInfo);
		ti.instance_init  = gtkam_info_init;

		type = g_type_register_static (PARENT_TYPE, "GtkamInfo",
					       &ti, 0);
	}

	return (type);
}

static void
gtkam_info_update_sensitivity (GtkamInfo *info)
{
	g_return_if_fail (GTKAM_IS_INFO (info));

	if (info->priv->needs_update) {
		gtk_widget_set_sensitive (info->priv->button_apply, TRUE);
		gtk_widget_set_sensitive (info->priv->button_reset, TRUE);
	} else {
		gtk_widget_set_sensitive (info->priv->button_apply, FALSE);
		gtk_widget_set_sensitive (info->priv->button_reset, FALSE);
	}
}

static gboolean
gtkam_info_update (GtkamInfo *info)
{
	int result;
	gchar *dir;
	GtkWidget *dialog, *s;
	GtkamInfoInfoUpdatedData data;

	g_return_val_if_fail (GTKAM_IS_INFO (info), FALSE);

	dir = g_dirname (info->priv->path);

	s = gtkam_status_new (_("Getting information for file '%s' in "
			      "'%s'..."), dir, g_basename (info->priv->path));
	gtk_widget_show (s);
	result = gp_camera_file_set_info (info->priv->camera, dir,
		g_basename (info->priv->path), info->priv->info_new,
		GTKAM_STATUS (s)->context->context);
	gp_camera_file_get_info (info->priv->camera, dir,
			g_basename (info->priv->path), &info->priv->info,
			NULL);

	/* Emit the signal */
	memset (&data, 0, sizeof (GtkamInfoInfoUpdatedData));
	data.camera = info->priv->camera;
	data.multi  = info->priv->multi;
	data.folder = dir;
	data.name = g_basename (info->priv->path);
	g_signal_emit (GTK_OBJECT (info), signals[INFO_UPDATED], 0, &data);

	g_free (dir);

	switch (result) {
	case GP_OK:
		break;
	case GP_ERROR_CANCEL:
		gtk_object_destroy (GTK_OBJECT (s));
		return FALSE;
	default:
		dialog = gtkam_error_new (result, GTKAM_STATUS (s)->context,
			GTK_WIDGET (info),
			_("Could not set file information for "
			"'%s'"), info->priv->path);
		gtk_widget_show (dialog);
		gtk_object_destroy (GTK_OBJECT (s));
		return (FALSE);
	}
	gtk_object_destroy (GTK_OBJECT (s));

	info->priv->needs_update = FALSE;
	gtkam_info_update_sensitivity (info);
	memset (&info->priv->info_new, 0, sizeof (CameraFileInfo));

	/* Check for name change */
	if (info->priv->new_path) {
		g_free (info->priv->path);
		info->priv->path = info->priv->new_path;
		info->priv->new_path = NULL;
		gtk_window_set_title (GTK_WINDOW (info),
				      g_basename (info->priv->path));
	}

	return (TRUE);
}

static void
on_apply_clicked (GtkButton *button, GtkamInfo *info)
{
	gtkam_info_update (info);
}

static void
on_reset_clicked (GtkButton *button, GtkamInfo *info)
{
	if ((info->priv->info.file.fields & GP_FILE_INFO_NAME) &&
	    info->priv->entry_name)
		gtk_entry_set_text (GTK_ENTRY (info->priv->entry_name),
				    info->priv->info.file.name);
	if ((info->priv->info.file.fields & GP_FILE_INFO_PERMISSIONS) &&
	    info->priv->check_read && info->priv->check_delete) {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (info->priv->check_read),
			info->priv->info.file.permissions & GP_FILE_PERM_READ);
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (info->priv->check_delete),
			info->priv->info.file.permissions & GP_FILE_PERM_DELETE);
	}

	memset (&info->priv->info_new, 0, sizeof (CameraFileInfo));
	if (info->priv->new_path) {
		g_free (info->priv->new_path);
		info->priv->new_path = NULL;
	}
	info->priv->needs_update = FALSE;
	gtkam_info_update_sensitivity (info);
}

static void
on_ok_clicked (GtkButton *button, GtkamInfo *info)
{
	if (info->priv->needs_update) {
		if (gtkam_info_update (info))
			gtk_object_destroy (GTK_OBJECT (info));
	} else
		gtk_object_destroy (GTK_OBJECT (info));
}

static void
on_cancel_clicked (GtkButton *button, GtkamInfo *info)
{
	gtk_object_destroy (GTK_OBJECT (info));
}

static void
on_read_toggled (GtkToggleButton *toggle, GtkamInfo *info)
{
	info->priv->info_new.file.fields |= GP_FILE_INFO_PERMISSIONS;
	if (toggle->active)
		info->priv->info_new.file.permissions |= GP_FILE_PERM_READ;
	else
		info->priv->info_new.file.permissions &= ~GP_FILE_PERM_READ;

	info->priv->needs_update = TRUE;
	gtkam_info_update_sensitivity (info);
}

static void
on_delete_toggled (GtkToggleButton *toggle, GtkamInfo *info)
{
	info->priv->info_new.file.fields |= GP_FILE_INFO_PERMISSIONS;
	if (toggle->active)
		info->priv->info_new.file.permissions |= GP_FILE_PERM_DELETE;
	else
		info->priv->info_new.file.permissions &= ~GP_FILE_PERM_DELETE;

	info->priv->needs_update = TRUE;
	gtkam_info_update_sensitivity (info);
}

static void
on_name_changed (GtkEditable *editable, GtkamInfo *info)
{
	const gchar *name;
	gchar *dir;

	info->priv->info_new.file.fields |= GP_FILE_INFO_NAME;
	name = gtk_entry_get_text (GTK_ENTRY (editable));
	strncpy (info->priv->info_new.file.name, name,
		 sizeof (info->priv->info_new.file.name));
	info->priv->needs_update = TRUE;
	gtkam_info_update_sensitivity (info);

	dir = g_dirname (info->priv->path);
	if (info->priv->new_path)
		g_free (info->priv->new_path);
	if (strlen (dir) == 1)
		info->priv->new_path = g_strdup_printf ("/%s", name);
	else
		info->priv->new_path = g_strdup_printf ("%s/%s", dir, name);
	g_free (dir);
}

GtkWidget *
gtkam_info_new (Camera *camera, const gchar *path, GtkWidget *opt_window)
{
	GtkamInfo *info;
	GtkWidget *button, *image, *hbox, *dialog, *notebook, *page, *label;
	GtkWidget *check, *entry, *c;
	gchar *dir, *msg;
	int result;
	CameraFileInfo i;

	g_return_val_if_fail (camera != NULL, NULL);
	g_return_val_if_fail (path != NULL, NULL);

	/* Get file info */
	dir = g_dirname (path);
	c = gtkam_cancel_new (opt_window,
		_("Getting information about file '%s' in "
		"folder '%s'..."), g_basename (path), dir);
	result = gp_camera_file_get_info (camera, dir, g_basename (path),
		&i, GTKAM_CANCEL (c)->context->context);
	g_free (dir);
	switch (result) {
	case GP_OK:
		break;
	case GP_ERROR_CANCEL:
		gtk_object_destroy (GTK_OBJECT (c));
		return (NULL);
	default:
		dialog = gtkam_error_new (result, GTKAM_CANCEL (c)->context,
			opt_window, _("Could not get information about file "
			"'%s' in folder '%s'."), path);
		gtk_widget_show (dialog);
		gtk_object_destroy (GTK_OBJECT (c));
		return (NULL);
	}
	gtk_object_destroy (GTK_OBJECT (c));

	info = g_object_new (GTKAM_TYPE_INFO, NULL);
	g_signal_connect (GTK_OBJECT (info), "delete_event",
			    GTK_SIGNAL_FUNC (gtk_object_destroy), NULL);
	gtk_window_set_title (GTK_WINDOW (info), g_basename (path));

	info->priv->camera = camera;
	gp_camera_ref (camera);
	info->priv->path = g_strdup (path);
	memcpy (&info->priv->info, &i, sizeof (CameraFileInfo));

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (info)->vbox), hbox,
			    TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);

	image = gtk_image_new_from_file (IMAGE_DIR "/gtkam-camera.png");
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	notebook = gtk_notebook_new ();
	gtk_widget_show (notebook);
	gtk_box_pack_start (GTK_BOX (hbox), notebook, TRUE, TRUE, 0);

	if (info->priv->info.file.fields) {
		page = gtk_table_new (5, 2, FALSE);
		gtk_widget_show (page);
		gtk_container_set_border_width (GTK_CONTAINER (page), 5);
		gtk_table_set_col_spacings (GTK_TABLE (page), 5);
		gtk_table_set_row_spacings (GTK_TABLE (page), 2);
		label = gtk_label_new (_("File"));
		gtk_widget_show (label);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

		/* Name */
		if (info->priv->info.file.fields & GP_FILE_INFO_NAME) {
			label = gtk_label_new (_("Name:"));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 0, 1, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);

			entry = gtk_entry_new ();
			gtk_entry_set_max_length (GTK_ENTRY (entry),
				sizeof (info->priv->info.file.name));
			gtk_widget_show (entry);
			gtk_table_attach_defaults (GTK_TABLE (page), entry,
						   1, 2, 0, 1);
			gtk_entry_set_text (GTK_ENTRY (entry),
					    info->priv->info.file.name);
			info->priv->entry_name = entry;
			g_signal_connect (GTK_OBJECT (entry), "changed",
				GTK_SIGNAL_FUNC (on_name_changed), info);
		}

		/* Mime type */
		if (info->priv->info.file.fields & GP_FILE_INFO_TYPE) {
			label = gtk_label_new (_("Mime type:"));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 1, 2, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
			label = gtk_label_new (info->priv->info.file.type);
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label, 1, 2, 1, 2,
					  GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
		}

		/* Time */
		if (info->priv->info.file.fields & GP_FILE_INFO_MTIME) {
			label = gtk_label_new (_("Last modified:"));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 2, 3, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
			msg = g_strdup (ctime (&info->priv->info.file.mtime));
			msg[strlen (msg) - 1] = '\0';
			label = gtk_label_new (msg);
			g_free (msg);
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  1, 2, 2, 3, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
		}

		/* Size */
		if (info->priv->info.file.fields & GP_FILE_INFO_SIZE) {
			label = gtk_label_new (_("Size:"));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 3, 4, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
			msg = g_strdup_printf (_("%li bytes"),
						info->priv->info.file.size);
			label = gtk_label_new (msg);
			g_free (msg);
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  1, 2, 3, 4, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
		}

		/* Permissions */
		if (info->priv->info.file.fields & GP_FILE_INFO_PERMISSIONS) {
			label = gtk_label_new (_("Permissions:"));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 4, 5, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);

			hbox = gtk_hbox_new (FALSE, 5);
			gtk_widget_show (hbox);
			gtk_table_attach_defaults (GTK_TABLE (page), hbox,
						   1, 2, 4, 5);
			check = gtk_check_button_new_with_label (_("Read"));
			gtk_widget_show (check);
			gtk_container_add (GTK_CONTAINER (hbox), check);
			if (info->priv->info.file.permissions &
							GP_FILE_PERM_READ)
				gtk_toggle_button_set_active (
					GTK_TOGGLE_BUTTON (check), TRUE);
			info->priv->check_read = check;
			g_signal_connect (GTK_OBJECT (check), "toggled",
				GTK_SIGNAL_FUNC (on_read_toggled), info);

			check = gtk_check_button_new_with_label (_("Delete"));
			gtk_widget_show (check);
			gtk_container_add (GTK_CONTAINER (hbox), check);
			if (info->priv->info.file.permissions &
							GP_FILE_PERM_DELETE)
				gtk_toggle_button_set_active (
					GTK_TOGGLE_BUTTON (check), TRUE);
			info->priv->check_delete = check;
			g_signal_connect (GTK_OBJECT (check), "toggled",
				GTK_SIGNAL_FUNC (on_delete_toggled), info);
		}
	}

	if (info->priv->info.preview.fields) {
		page = gtk_table_new (5, 2, FALSE);
		gtk_widget_show (page);
		gtk_container_set_border_width (GTK_CONTAINER (page), 5);
		gtk_table_set_col_spacings (GTK_TABLE (page), 5);
		gtk_table_set_row_spacings (GTK_TABLE (page), 2);
		label = gtk_label_new (_("Preview"));
		gtk_widget_show (label);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

		/* Mime type */
		if (info->priv->info.preview.fields & GP_FILE_INFO_TYPE) {
			label = gtk_label_new (_("Mime type:"));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 0, 1, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
			label = gtk_label_new (info->priv->info.preview.type);
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach_defaults (GTK_TABLE (page), label,
						   1, 2, 0, 1);
		}

		/* Size */
                if (info->priv->info.preview.fields & GP_FILE_INFO_SIZE) {
                        label = gtk_label_new (_("Size:"));
                        gtk_widget_show (label);
                        gtk_label_set_justify (GTK_LABEL (label),
                                               GTK_JUSTIFY_LEFT);
                        gtk_table_attach (GTK_TABLE (page), label,
                                          0, 1, 1, 2, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
                        msg = g_strdup_printf ("%li bytes",
                                                info->priv->info.preview.size);
                        label = gtk_label_new (msg);
                        g_free (msg);
                        gtk_widget_show (label);
                        gtk_label_set_justify (GTK_LABEL (label),
                                               GTK_JUSTIFY_LEFT);
                        gtk_table_attach_defaults (GTK_TABLE (page), label,
                                                   1, 2, 1, 2);
                }
	}

	if (info->priv->info.audio.fields) {
		page = gtk_table_new (5, 2, FALSE);
		gtk_widget_show (page); 
		gtk_container_set_border_width (GTK_CONTAINER (page), 5);
		gtk_table_set_col_spacings (GTK_TABLE (page), 5);
		gtk_table_set_row_spacings (GTK_TABLE (page), 2);
		label = gtk_label_new (_("Audio"));
		gtk_widget_show (label); 
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

		/* Mime type */
		if (info->priv->info.audio.fields & GP_FILE_INFO_TYPE) {
			label = gtk_label_new (_("Mime type: "));
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach (GTK_TABLE (page), label,
					  0, 1, 0, 1, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
			label = gtk_label_new (info->priv->info.audio.type);
			gtk_widget_show (label);
			gtk_label_set_justify (GTK_LABEL (label),
					       GTK_JUSTIFY_LEFT);
			gtk_table_attach_defaults (GTK_TABLE (page), label,
						   1, 2, 0, 1);
		}

		/* Size */
                if (info->priv->info.audio.fields & GP_FILE_INFO_SIZE) {
                        label = gtk_label_new (_("Size:"));
                        gtk_widget_show (label);
                        gtk_label_set_justify (GTK_LABEL (label),
                                               GTK_JUSTIFY_LEFT);
                        gtk_table_attach (GTK_TABLE (page), label,
                                          0, 1, 1, 2, GTK_FILL, 0, 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
                        msg = g_strdup_printf ("%li bytes",
                                                info->priv->info.audio.size);
                        label = gtk_label_new (msg);
                        g_free (msg);
                        gtk_widget_show (label);
                        gtk_label_set_justify (GTK_LABEL (label),
                                               GTK_JUSTIFY_LEFT);
                        gtk_table_attach_defaults (GTK_TABLE (page), label,
                                                   1, 2, 1, 2);
                }
	}

	button = gtk_button_new_with_label (_("Ok"));
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_ok_clicked), info);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (info)->action_area),
			   button);
	gtk_widget_grab_focus (button);

	button = gtk_button_new_with_label (_("Apply"));
	gtk_widget_show (button);
	gtk_widget_set_sensitive (button, FALSE);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_apply_clicked), info);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (info)->action_area),
			   button);
	info->priv->button_apply = button;

	button = gtk_button_new_with_label (_("Reset"));
	gtk_widget_show (button);
	gtk_widget_set_sensitive (button, FALSE);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_reset_clicked), info);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (info)->action_area),
			   button);
	info->priv->button_reset = button;

	button = gtk_button_new_with_label (_("Cancel"));
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_cancel_clicked), info);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (info)->action_area),
			   button);

	if (opt_window)
		gtk_window_set_transient_for (GTK_WINDOW (info),
					      GTK_WINDOW (opt_window));

	return (GTK_WIDGET (info));
}
