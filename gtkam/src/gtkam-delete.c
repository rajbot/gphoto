/* gtkam-delete.c
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
#include "gtkam-delete.h"

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
#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>

#include "gtkam-error.h"
#include "gtkam-status.h"

typedef struct _GtkamDeleteData GtkamDeleteData;
struct _GtkamDeleteData {
	Camera *camera;
	gboolean multi;
	gchar *folder;
	gchar *name;
	GtkWidget *check;
};

struct _GtkamDeletePrivate
{
	Camera *camera;
	gboolean multi;

	GSList *data;
	GtkWidget *vbox;

	GtkWidget *msg;
};

#define PARENT_TYPE GTKAM_TYPE_DIALOG
static GtkamDialogClass *parent_class;

enum {
	ALL_DELETED,
	FILE_DELETED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

static void
gtkam_delete_destroy (GtkObject *object)
{
	GtkamDelete *delete = GTKAM_DELETE (object);
	gint i;
	GtkamDeleteData *data;

	if (delete->priv->data) {
		for (i = g_slist_length (delete->priv->data) - 1; i >= 0; i--) {
			data = g_slist_nth_data (delete->priv->data, i);
			gp_camera_unref (data->camera);
			g_free (data->folder);
			g_free (data->name);
			g_free (data);
		}
		g_slist_free (delete->priv->data);
		delete->priv->data = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gtkam_delete_finalize (GObject *object)
{
	GtkamDelete *delete = GTKAM_DELETE (object);

	g_free (delete->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtkam_delete_class_init (gpointer g_class, gpointer class_data)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtkam_delete_destroy;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtkam_delete_finalize;

	signals[FILE_DELETED] = g_signal_new ("file_deleted",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (GtkamDeleteClass, file_deleted), NULL, NULL,
		g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		G_TYPE_POINTER);
	signals[ALL_DELETED] = g_signal_new ("all_deleted",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (GtkamDeleteClass, all_deleted), NULL, NULL,
		g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		G_TYPE_POINTER);

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtkam_delete_init (GTypeInstance *instance, gpointer g_class)
{
	GtkamDelete *delete = GTKAM_DELETE (instance);

	delete->priv = g_new0 (GtkamDeletePrivate, 1);
}

GType
gtkam_delete_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo)); 
		ti.class_size     = sizeof (GtkamDeleteClass);
		ti.class_init     = gtkam_delete_class_init;
		ti.instance_size  = sizeof (GtkamDelete);
		ti.instance_init  = gtkam_delete_init;

		type = g_type_register_static (PARENT_TYPE, "GtkamDelete",
					       &ti, 0);
	}

	return (type);
}

static int
gp_list_lookup_name (CameraList *list, const char *name)
{
	int i;
	const char *n;

	for (i = 0; i < gp_list_count (list); i++) {
		gp_list_get_name (list, i, &n);
		if (!strcmp (n, name))
			return (i);
	}

	return (GP_ERROR);
}

static gboolean
delete_all (GtkamDelete *delete, Camera *camera, gboolean multi,
	    const gchar *folder)
{
	GtkWidget *d, *s;
	int result, r1, r2;
	CameraList l1, l2;
	const char *name;
	GtkamDeleteAllDeletedData add;
	GtkamDeleteFileDeletedData fdd;

	s = gtkam_status_new (_("Deleting all files in '%s'..."), folder);
	gtk_widget_show (s);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (delete)->vbox), s,
			    FALSE, FALSE, 0);
	r1 = gp_camera_folder_list_files (camera, folder, &l1, NULL);
	result = gp_camera_folder_delete_all (camera, folder,
					GTKAM_STATUS (s)->context->context);
	switch (result) {
	case GP_OK:
		add.camera = camera;
		add.multi = multi;
		add.folder = folder;
		g_signal_emit (G_OBJECT (delete),
			signals[ALL_DELETED], 0, &add);
		gtk_object_destroy (GTK_OBJECT (s));
		return (TRUE);
	case GP_ERROR_CANCEL:
		gtk_object_destroy (GTK_OBJECT (s));
		return (TRUE);
	default:
		d = gtkam_error_new (result, GTKAM_STATUS (s)->context,
			GTK_WIDGET (delete), _("Could not delete all files "
			"in '%s'."), folder);
		gtk_widget_show (d);
		gtk_object_destroy (GTK_OBJECT (s));

		/* See what files have been deleted */
		r2 = gp_camera_folder_list_files (camera, folder, &l2, NULL);
		if ((r1 == GP_OK) && (r2 == GP_OK)) {
			for (r1 = 0; r1 < gp_list_count (&l1); r1++) {
				gp_list_get_name (&l1, r1, &name);
				if (gp_list_lookup_name (&l2, name) >= 0) {
					fdd.camera = camera;
					fdd.multi = multi;
					fdd.folder = folder;
					fdd.name = name;
					g_signal_emit (GTK_OBJECT (delete),
						signals[FILE_DELETED], 0,
						&fdd);
				}
			}
		}
		return (FALSE);
	}
}

static gboolean
delete_one (GtkamDelete *delete, Camera *camera, gboolean multi,
	    const gchar *folder, const gchar *name)
{
	GtkWidget *d, *s;
	int result;
	GtkamDeleteFileDeletedData fdd;

	s = gtkam_status_new (_("Deleting file '%s' from folder '%s'..."),
			      name, folder);
	gtk_widget_show (s);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (delete)->vbox), s,
			    FALSE, FALSE, 0);
	result = gp_camera_file_delete (camera, folder, name,
				        GTKAM_STATUS (s)->context->context);
	switch (result) {
	case GP_OK:
		gtk_object_destroy (GTK_OBJECT (s));
		fdd.camera = camera;
		fdd.multi = multi;
		fdd.folder = folder;
		fdd.name = name;
		g_signal_emit (GTK_OBJECT (delete), signals[FILE_DELETED], 0,
			       &fdd);
		return (TRUE);
	case GP_ERROR_CANCEL:
		gtk_object_destroy (GTK_OBJECT (s));
		return (FALSE);
	default:
		d = gtkam_error_new (result, GTKAM_STATUS (s)->context,
			GTK_WIDGET (delete), _("Could not delete "
			"file '%s' in folder '%s'."), name, folder);
		gtk_widget_show (d);
		gtk_object_destroy (GTK_OBJECT (s));
		return (FALSE);
	}
}

static void
on_delete_clicked (GtkButton *button, GtkamDelete *delete)
{
	gboolean success = TRUE;
	gint i;
	GtkamDeleteData *data;

	for (i = g_slist_length (delete->priv->data) - 1; i >= 0; i--) {
		data = g_slist_nth_data (delete->priv->data, i);
		if (!GTK_TOGGLE_BUTTON (data->check)->active)
			continue;
		if (data->name) {
			if (delete_one (delete, data->camera, data->multi,
					data->folder, data->name)) {
				gp_camera_unref (data->camera);
				g_free (data->name);
				g_free (data->folder);
				delete->priv->data = g_slist_remove (
					delete->priv->data, data);
				gtk_container_remove (
					GTK_CONTAINER (delete->priv->vbox),
					GTK_WIDGET (data->check));
				g_free (data);
			} else
				success = FALSE;
		} else if (!delete_all (delete, data->camera, data->multi,
					data->folder))
			success = FALSE;
	}

	if (success)
		gtk_object_destroy (GTK_OBJECT (delete));
}

static void
on_cancel_clicked (GtkButton *button, GtkamDelete *delete)
{
	gtk_object_destroy (GTK_OBJECT (delete));
}

GtkWidget *
gtkam_delete_new (void)
{
	GtkamDelete *delete;
	GtkWidget *button, *scrolled;

	delete = g_object_new (GTKAM_TYPE_DELETE, NULL);

	/* Message */
	delete->priv->msg = gtk_label_new ("");
	gtk_widget_show (delete->priv->msg);
	gtk_label_set_justify (GTK_LABEL (delete->priv->msg), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (delete->priv->msg), TRUE);
	gtk_box_pack_start (GTK_BOX (GTKAM_DIALOG (delete)->vbox),
			    delete->priv->msg, FALSE, FALSE, 0);

	/* Scrolled window */
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolled);
	gtk_box_pack_start (GTK_BOX (GTKAM_DIALOG (delete)->vbox),
			    scrolled, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	delete->priv->vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (delete->priv->vbox);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					       delete->priv->vbox);

	button = gtk_button_new_from_stock (GTK_STOCK_DELETE);
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_delete_clicked), delete);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (delete)->action_area),
			   button);

	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_cancel_clicked), delete);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (delete)->action_area),
			   button);
	gtk_widget_grab_focus (button);

	return (GTK_WIDGET (delete));
}

void
gtkam_delete_add (GtkamDelete *delete, Camera *camera, gboolean multi,
		  const gchar *folder, const gchar *name)
{
	GtkamDeleteData *data;
	gchar *label, *msg;

	data = g_new0 (GtkamDeleteData, 1);
	data->camera = camera;
	gp_camera_ref (camera);
	data->multi = multi;
	data->folder = g_strdup (folder);
	data->name = g_strdup (name);
	delete->priv->data = g_slist_append (delete->priv->data, data);

	if (g_slist_length (delete->priv->data) == 1)
		msg = g_strdup_printf (_("Do you really want to "
					 "delete the following file?"));
	else
		msg = g_strdup_printf (_("Do you really want to "
					 "delete the following %i files?"),
					g_slist_length (delete->priv->data));
	gtk_label_set_text (GTK_LABEL (delete->priv->msg), msg);
	g_free (msg);

	if (name)
		label = g_strdup_printf (_("'%s' in folder '%s'"), name,
					 folder);
	else
		label = g_strdup_printf (_("All files in folder '%s'"),
					 folder);
	data->check = gtk_check_button_new_with_label (label);
	g_free (label);
	gtk_widget_show (data->check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->check), TRUE);
	gtk_box_pack_start (GTK_BOX (delete->priv->vbox), data->check,
			    FALSE, FALSE, 0);
}
