/* gtkam-list.c
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
#include "gtkam-list.h"

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

#include <stdio.h>
#include <string.h>

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-port-log.h>

#include "gtkam-close.h"
#include "gtkam-error.h"
#include "gtkam-exif.h"
//#include "../pixmaps/no_thumbnail.xpm"
#include "gtkam-save.h"
#include "gdk-pixbuf-hacks.h"
#include "gtkam-info.h"
#include "gtkam-delete.h"
#include "gtkam-status.h"
#include "gtkam-util.h"

typedef struct _GtkamListCameraData GtkamListCameraData;
struct _GtkamListCameraData {
	Camera *camera;
	gboolean multi;
	GtkTreePath *path;
};

struct _GtkamListPrivate
{
	GtkListStore *store;

	GPtrArray *cameras;

	gboolean thumbnails;
};

#define PARENT_TYPE GTK_TYPE_TREE_VIEW
static GtkTreeViewClass *parent_class;

enum {
	FILE_SELECTED,
	FILE_UNSELECTED,
	NEW_STATUS,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

enum {
	PREVIEW_COLUMN = 0,
	NAME_COLUMN,
	FOLDER_COLUMN,
	NUM_COLUMNS
};

static void
gtkam_list_destroy (GtkObject *object)
{
	GtkamList *list = GTKAM_LIST (object);
	guint i;
	GtkamListCameraData *data;

	if (list->priv->cameras) {
		for (i = 0; i < list->priv->cameras->len; i++) {
			data = list->priv->cameras->pdata[i];
			gp_camera_unref (data->camera);
			gtk_tree_path_free (data->path);
			g_free (data);
		}
		g_ptr_array_free (list->priv->cameras, TRUE);
		list->priv->cameras = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gtkam_list_finalize (GObject *object)
{
	GtkamList *list = GTKAM_LIST (object);

	g_free (list->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtkam_list_class_init (gpointer g_class, gpointer class_data)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtkam_list_destroy;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtkam_list_finalize;

	signals[FILE_SELECTED] = g_signal_new ("file_selected",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GtkamListClass, file_selected), NULL, NULL,
		g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		G_TYPE_POINTER);
	signals[FILE_UNSELECTED] = g_signal_new ("file_unselected",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GtkamListClass, file_unselected), NULL, NULL,
		g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		G_TYPE_POINTER);
	signals[NEW_STATUS] = g_signal_new ("new_status",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GtkamListClass, new_status), NULL, NULL,
		g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		G_TYPE_POINTER);

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtkam_list_init (GTypeInstance *instance, gpointer g_class)
{
	GtkamList *list = GTKAM_LIST (instance);

	list->priv = g_new0 (GtkamListPrivate, 1);
	list->priv->thumbnails = TRUE;
	list->priv->cameras = g_ptr_array_new ();
}

GType
gtkam_list_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size     = sizeof (GtkamListClass);
		ti.class_init     = gtkam_list_class_init;
		ti.instance_size  = sizeof (GtkamList);
		ti.instance_init  = gtkam_list_init;

		type = g_type_register_static (PARENT_TYPE, "GtkamList",
					       &ti, 0);
	}

	return (type);
}

static GtkamListCameraData *
gtkam_list_get_camera_data (GtkamList *list, GtkTreeIter *iter)
{
	guint i;
	GtkTreePath *path;
	GtkamListCameraData *data = NULL;

	g_return_val_if_fail (GTKAM_IS_LIST (list), NULL);

	path = gtk_tree_model_get_path (
			GTK_TREE_MODEL (list->priv->store), iter);
	for (i = 0; i < list->priv->cameras->len; i++) {
		data = list->priv->cameras->pdata[i];
		if (!gtk_tree_path_compare (path, data->path))
			break;
	}
	g_assert (i != list->priv->cameras->len);
	gtk_tree_path_free (path);

	return (data);
}

static Camera *
gtkam_list_get_camera_from_iter (GtkamList *list, GtkTreeIter *iter)
{
	GtkamListCameraData *data;

	g_return_val_if_fail (GTKAM_IS_LIST (list), NULL);

	data = gtkam_list_get_camera_data (list, iter);

	return (data->camera);
}

static gboolean
gtkam_list_get_multi_from_iter (GtkamList *list, GtkTreeIter *iter)
{
	GtkamListCameraData *data;

	g_return_val_if_fail (GTKAM_IS_LIST (list), FALSE);

	data = gtkam_list_get_camera_data (list, iter);

	return (data->multi);
}

static gchar *
gtkam_list_get_folder_from_iter (GtkamList *list, GtkTreeIter *iter)
{
	GValue value = {0};
	gchar *folder;

	g_return_val_if_fail (GTKAM_IS_LIST (list), NULL);

	gtk_tree_model_get_value (GTK_TREE_MODEL (list->priv->store), iter,
				  FOLDER_COLUMN, &value);
	folder = g_strdup (g_value_get_string (&value));
	g_value_unset (&value);

	return (folder);
}

static gchar *
gtkam_list_get_name_from_iter (GtkamList *list, GtkTreeIter *iter)
{
	GValue value = {0};
	gchar *name;

	g_return_val_if_fail (GTKAM_IS_LIST (list), NULL);
	
	gtk_tree_model_get_value (GTK_TREE_MODEL (list->priv->store), iter,
				  NAME_COLUMN, &value);
	name = g_strdup (g_value_get_string (&value));
	g_value_unset (&value);
	
	return (name);
}

typedef struct _GetThumbnailData GetThumbnailData;
struct _GetThumbnailData {
	Camera *camera;
	gboolean multi;
	gchar *folder;
	gchar *name;
	GtkamList *list;
	GtkTreeIter *iter;
};

static gboolean
get_thumbnail_idle (gpointer data)
{
	GetThumbnailData *d = data;
	CameraFile *file;
	GtkWidget *s;
	GdkPixbuf *pixbuf;
	int result;

	s = gtkam_status_new (_("Downloading thumbnail of '%s' from "
		"folder '%s'..."), d->name, d->folder);
	gtk_widget_show (s);
	g_signal_emit (G_OBJECT (d->list), signals[NEW_STATUS], 0, s);
	gp_file_new (&file);
	result = gp_camera_file_get (d->camera, d->folder, d->name,
			GP_FILE_TYPE_PREVIEW, file,
			GTKAM_STATUS (s)->context->context);
	if (result >= 0) {
		pixbuf = gdk_pixbuf_new_from_camera_file (file, 50, NULL);
		gtk_list_store_set (d->list->priv->store, d->iter,
				    PREVIEW_COLUMN, pixbuf, -1);
		gdk_pixbuf_unref (pixbuf);
	}

	gp_file_unref (file);
	gtk_object_destroy (GTK_OBJECT (s));

	gp_camera_unref (d->camera);
	g_free (d->name);
	g_free (d->folder);
	gtk_tree_iter_free (d->iter);
	g_free (d);

	return (TRUE);
}

static gboolean
show_thumbnails_foreach_func (GtkTreeModel *model, GtkTreePath *path,
                              GtkTreeIter *iter, gpointer data)
{
        GtkamList *list = GTKAM_LIST (data);
        Camera *camera;
        gboolean multi;
        gchar *folder, *name;
        CameraAbilities a;
	GetThumbnailData *d;

        camera = gtkam_list_get_camera_from_iter (list, iter);
        multi = gtkam_list_get_multi_from_iter (list, iter);
        folder = gtkam_list_get_folder_from_iter (list, iter);
        name = gtkam_list_get_name_from_iter (list, iter);

        gp_camera_get_abilities (camera, &a);
	if (a.file_operations & GP_FILE_OPERATION_PREVIEW) {
		d = g_new0 (GetThumbnailData, 1);
		d->camera = camera;
		gp_camera_ref (camera);
		d->multi = multi;
		d->folder = g_strdup (folder);
		d->name = g_strdup (name);
		d->iter = gtk_tree_iter_copy (iter);
		d->list = list;
		g_idle_add (get_thumbnail_idle, d);
	}
	g_free (folder);
	g_free (name);

        return (FALSE);
}

void
gtkam_list_show_thumbnails (GtkamList *list)
{
        g_return_if_fail (GTKAM_IS_LIST (list));

	list->priv->thumbnails = TRUE;

        gtk_tree_model_foreach (GTK_TREE_MODEL (list->priv->store),
                                show_thumbnails_foreach_func, list);
}

void
gtkam_list_hide_thumbnails (GtkamList *list)
{
	g_return_if_fail (GTKAM_IS_LIST (list));

	list->priv->thumbnails = FALSE;

	g_warning ("Fixme: gtkam_list_hide_thumbnails");
}

#if 0

static void
on_info_updated (GtkamInfo *info, Camera *camera, gboolean multi,
		 const gchar *folder, const gchar *name, GtkamList *list)
{
	gtkam_list_update_folder (list, camera, multi, folder);
}

typedef struct {
	GtkWidget *menu;
	GtkamList *list;
	GtkIconListItem *item;
} PopupData;

static void
on_info_activate (GtkMenuItem *i, PopupData *data)
{
	GtkamList *list = data->list;
	GtkIconListItem *item = data->item;
	GtkWidget *w, *info;
	gchar *path;
	const gchar *folder;
	Camera *camera;
	gboolean multi;

	folder = gtk_object_get_data (GTK_OBJECT (item->entry), "folder");
	camera = gtk_object_get_data (GTK_OBJECT (item->entry), "camera");
	multi = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item->entry),
						      "multi"));
	g_return_if_fail (folder && camera);

	w = gtk_widget_get_ancestor (GTK_WIDGET (list), GTK_TYPE_WINDOW);
	if (strlen (folder) == 1)
		path = g_strdup_printf ("/%s", item->label);
	else
		path = g_strdup_printf ("%s/%s", folder, item->label);
	info = gtkam_info_new (camera, path, w);
	g_free (path);
	if (info) {
		gtk_widget_show (info);
		gtk_signal_connect_while_alive (GTK_OBJECT (info),
			"info_updated", GTK_SIGNAL_FUNC (on_info_updated),
			list, GTK_OBJECT (list));
	}
}

static void
on_file_deleted (GtkamDelete *delete, Camera *camera, gboolean multi,
		 const gchar *folder, const gchar *name, GtkamList *list)
{
	gtkam_list_update_folder (list, camera, multi, folder);
}

static void
on_delete_activate (GtkMenuItem *menu_item, PopupData *data)
{
	GtkamList *list = data->list;
	GtkIconListItem *item = data->item;
	GtkWidget *delete, *w;

	delete = gtkam_delete_new (list->priv->status);
	gtkam_delete_add (GTKAM_DELETE (delete), 
		gtk_object_get_data (GTK_OBJECT (item->entry), "camera"),
		GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item->entry),
						"multi")),
		gtk_object_get_data (GTK_OBJECT (item->entry), "folder"),
		item->label);
	gtk_signal_connect_while_alive (GTK_OBJECT (delete), "file_deleted",
		GTK_SIGNAL_FUNC (on_file_deleted), list, GTK_OBJECT (list));

	w = gtk_widget_get_ancestor (GTK_WIDGET (list), GTK_TYPE_WINDOW);
	gtk_window_set_transient_for (GTK_WINDOW (delete), GTK_WINDOW (w));
	gtk_widget_show (delete);
}

static void
on_exif_activate (GtkMenuItem *menu_item, PopupData *data)
{
	GtkamList *list = data->list;
	GtkWidget *dialog, *w;
	GtkIconListItem *item = data->item;
	Camera *camera;
	const gchar *folder;
	gboolean multi;

	list = NULL;

	folder = gtk_object_get_data (GTK_OBJECT (item->entry), "folder");
	camera = gtk_object_get_data (GTK_OBJECT (item->entry), "camera");
	multi = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item->entry),
						      "multi"));

	w = gtk_widget_get_ancestor (GTK_WIDGET (data->list), GTK_TYPE_WINDOW);
	dialog = gtkam_exif_new (camera, multi, folder, item->label, w);
	if (dialog)
		gtk_widget_show (dialog);
}

static void
on_save_activate (GtkMenuItem *menu_item, PopupData *data)
{
	GtkamList *list = data->list;
	GtkIconListItem *item = data->item;
	GtkWidget *save;

	save = gtkam_save_new (list->priv->status);
	gtkam_save_add (GTKAM_SAVE (save),
		gtk_object_get_data (GTK_OBJECT (item->entry), "camera"),
		GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item->entry),
						      "multi")),
		gtk_object_get_data (GTK_OBJECT (item->entry), "folder"),
		item->label);
	gtk_widget_show (save);
}

static gboolean
kill_idle (gpointer user_data)
{
	PopupData *data = user_data;

	gtk_object_unref (GTK_OBJECT (data->menu));
	g_free (data);

	return (FALSE);
}

static void
kill_popup_menu (GtkWidget *widget, PopupData *data)
{
	gtk_idle_add (kill_idle, data);
}

static gboolean
on_select_icon (GtkIconList *ilist, GtkIconListItem *item,
		GdkEventButton *event, GtkamList *list)
{
	CameraAbilities a;
	CameraFile *file;
	GtkWidget *dialog, *w, *s;
	int result;
	GdkPixbuf *pixbuf;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	Camera *camera;
	const gchar *folder;
	gboolean multi;

	folder = gtk_object_get_data (GTK_OBJECT (item->entry), "folder");
	camera = gtk_object_get_data (GTK_OBJECT (item->entry), "camera");
	multi = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item->entry),
						      "multi")); 

	if (!event)
		return (TRUE);

	w = gtk_widget_get_ancestor (GTK_WIDGET (list), GTK_TYPE_WINDOW);

	gp_camera_get_abilities (camera, &a);
	if ((event->type == GDK_2BUTTON_PRESS) &&
	    (a.file_operations & GP_FILE_OPERATION_PREVIEW)) {
		CameraFile *file;

		/* Double-click: Get thumbnail */
		gp_file_new (&file);
		s = gtkam_status_new (_("Getting thumbnail of file '%s' in "
			"folder '%s'..."), item->label, folder);
		gtk_widget_show (s);
		gtk_box_pack_start (GTK_BOX (list->priv->status), s,
				    FALSE, FALSE, 0);
		result = gp_camera_file_get (camera, folder,
			item->label, GP_FILE_TYPE_PREVIEW, file,
			GTKAM_STATUS (s)->context->context);
		if (multi)
			gp_camera_exit (camera, NULL);
		switch (result) {
		case GP_OK:
			pixbuf = gdk_pixbuf_new_from_camera_file (file,
					ICON_WIDTH, w);
			if (pixbuf) {
				gdk_pixbuf_render_pixmap_and_mask (pixbuf,
						&pixmap, &bitmap, 127);
				gdk_pixbuf_unref (pixbuf);
				gtk_pixmap_set (GTK_PIXMAP (item->pixmap),
						pixmap, bitmap);
				item->state = GTK_STATE_SELECTED;
			}
			break;
		case GP_ERROR_NOT_SUPPORTED:
		case GP_ERROR_CANCEL:
			break;
		default:
			dialog = gtkam_error_new (result,
				GTKAM_STATUS (s)->context, w,
				_("Could not get preview of "
				"file '%s' in folder '%s'"),
				item->label, folder);
			gtk_widget_show (dialog);
			break;
		}
		gtk_object_destroy (GTK_OBJECT (s));
		gp_file_unref (file);

		while (gtk_events_pending ())
			gtk_main_iteration ();

		return (FALSE);
	} else if (event->type == GDK_BUTTON_PRESS) {
		if (event->button == 3) {
			CameraFileInfo info;
			PopupData *data;
			GtkWidget *i;

			while (gtk_events_pending ())
				gtk_main_iteration ();

			data = g_new0 (PopupData, 1);
			data->list = list;
			data->item = item;

			/* Right-click: Show popup menu */
			data->menu = gtk_menu_new ();
			gtk_widget_show (data->menu);
			i = gtk_menu_item_new_with_label (_("Info"));
			gtk_widget_show (i);
			gtk_container_add (GTK_CONTAINER (data->menu), i);
			gtk_signal_connect (GTK_OBJECT (i), "activate",
				GTK_SIGNAL_FUNC (on_info_activate), data);

			i = gtk_menu_item_new_with_label (_("Exif"));
#ifdef HAVE_EXIF
			gtk_widget_show (i);
#endif
			gtk_container_add (GTK_CONTAINER (data->menu), i);
			gtk_signal_connect (GTK_OBJECT (i), "activate",
				GTK_SIGNAL_FUNC (on_exif_activate), data);
			gp_file_new (&file);
			if (gp_camera_file_get (camera, folder,
				item->label, GP_FILE_TYPE_EXIF,
				file, NULL) < 0)
				gtk_widget_set_sensitive (i, FALSE);
			gp_file_unref (file);

			i = gtk_menu_item_new ();
			gtk_widget_show (i);
			gtk_container_add (GTK_CONTAINER (data->menu), i);
			gtk_widget_set_sensitive (i, FALSE);

			/* Save */
			i = gtk_menu_item_new_with_label (_("Save"));
			gtk_widget_show (i);
			gtk_container_add (GTK_CONTAINER (data->menu), i);
			gtk_signal_connect (GTK_OBJECT (i), "activate",
				GTK_SIGNAL_FUNC (on_save_activate), data);

			/* Delete */
			i = gtk_menu_item_new_with_label (_("Delete"));
			gtk_widget_show (i);
			if (!(a.file_operations & GP_FILE_OPERATION_DELETE) ||
			    (!gp_camera_file_get_info (camera, folder,
					item->label, &info, NULL) &&
			     (info.file.fields & GP_FILE_INFO_PERMISSIONS) && 
			     (!(info.file.permissions & GP_FILE_PERM_DELETE))))
				gtk_widget_set_sensitive (i, FALSE);
			gtk_container_add (GTK_CONTAINER (data->menu), i);
			gtk_signal_connect (GTK_OBJECT (i), "activate",
				GTK_SIGNAL_FUNC (on_delete_activate), data);

			gtk_signal_connect (GTK_OBJECT (data->menu), "hide",
					    GTK_SIGNAL_FUNC (kill_popup_menu),
					    data);
			gtk_menu_popup (GTK_MENU (data->menu), NULL, NULL,
					NULL, NULL, event->button, event->time);

			return (FALSE);
		}
	}

	return (TRUE);
}
#endif

static gboolean
selection_func (GtkTreeSelection *selection, GtkTreeModel *model,
		GtkTreePath *path, gboolean path_currently_selected,
		gpointer data)
{
	GtkTreeIter iter;
	gchar *folder, *name;
	GtkamList *list = GTKAM_LIST (data);
	GtkamListFileSelectedData sd;
	GtkamListFileUnselectedData ud;

	gtk_tree_model_get_iter (model, &iter, path);
	folder = gtkam_list_get_folder_from_iter (list, &iter);
	name = gtkam_list_get_name_from_iter (list, &iter);
	if (path_currently_selected) {
		ud.camera = gtkam_list_get_camera_from_iter (list, &iter);
		ud.multi = gtkam_list_get_multi_from_iter (list, &iter);
		ud.folder = folder;
		ud.name = name;
		g_signal_emit (G_OBJECT (list), signals[FILE_UNSELECTED],
			       0, &ud);
	} else {
		sd.camera = gtkam_list_get_camera_from_iter (list, &iter);
		sd.multi = gtkam_list_get_multi_from_iter (list, &iter);
		sd.folder = folder;
		sd.name = name;
		g_signal_emit (G_OBJECT (list), signals[FILE_SELECTED],
			       0, &sd);
	}
	g_free (name);
	g_free (folder);

	return (TRUE);
}

GtkWidget *
gtkam_list_new (void)
{
        GtkamList *list;
	GtkTreeSelection *selection;
	GtkCellRenderer *renderer;

        list = g_object_new (GTKAM_TYPE_LIST, NULL);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_set_select_function (selection, selection_func,
						list, NULL);

	list->priv->store = gtk_list_store_new (NUM_COLUMNS,
				GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (list),
				 GTK_TREE_MODEL (list->priv->store));

	/* Column for previews */
	renderer = gtk_cell_renderer_pixbuf_new ();
	g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list),
		-1, _("Preview"), renderer, "pixbuf", PREVIEW_COLUMN, NULL);

	/* Column for file names */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list),
			-1, _("Name"), renderer, "text", NAME_COLUMN, NULL);

        return (GTK_WIDGET (list));
}

void
gtkam_list_add_folder (GtkamList *list, Camera *camera, gboolean multi,
		       const gchar *folder)
{
	GtkWidget *dialog, *s;
	CameraList flist;
	int result;
	const char *name;
	gint i;
	GtkTreeIter iter;
	GtkamListCameraData *data;

	g_return_if_fail (GTKAM_IS_LIST (list));

	s = gtkam_status_new (_("Listing files in folder '%s'..."), folder);
	g_signal_emit (G_OBJECT (list), signals[NEW_STATUS], 0, s);
	result = gp_camera_folder_list_files (camera, folder, &flist,
					GTKAM_STATUS (s)->context->context);
	switch (result) {
	case GP_OK:
		break;
	case GP_ERROR_CANCEL:
		if (multi)
			gp_camera_exit (camera, NULL);
		gtk_object_destroy (GTK_OBJECT (s));
		return;
	default:
		if (multi)
			gp_camera_exit (camera, NULL);
		dialog = gtkam_error_new (result, GTKAM_STATUS (s)->context,
			NULL, _("Could not get file list for folder "
			"'%s'"), folder);
		gtk_widget_show (dialog);
		gtk_object_destroy (GTK_OBJECT (s));
		return;
	}
	gtk_object_destroy (GTK_OBJECT (s));

	for (i = 0; i < gp_list_count (&flist); i++) {
		gp_list_get_name (&flist, i, &name);
		gtk_list_store_append (list->priv->store, &iter);
		gtk_list_store_set (list->priv->store, &iter, NAME_COLUMN,
				    name, FOLDER_COLUMN, folder, -1);
		data = g_new0 (GtkamListCameraData, 1);
		data->camera = camera;
		gp_camera_ref (camera);
		data->multi = multi;
		data->path = gtk_tree_model_get_path (
				GTK_TREE_MODEL (list->priv->store), &iter);
		g_ptr_array_add (list->priv->cameras, data);
	}

	if (list->priv->thumbnails)
		gtkam_list_show_thumbnails (list);

#if 0
	gp_camera_get_abilities (camera, &a);
	for (i = 0; i < gp_list_count (&flist); i++) {
		gp_list_get_name (&flist, i, &name);

		/*
		 * First step: Show the plain icon
		 */
		pixbuf = gdk_pixbuf_new_from_xpm_data (
					(const char**) no_thumbnail_xpm);
		gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap,
						   127);
		gtk_icon_list_freeze (GTK_ICON_LIST (list));
		item = gtk_icon_list_add_from_pixmap (GTK_ICON_LIST (list),
						pixmap, bitmap, name, NULL);
		gtk_icon_list_thaw (GTK_ICON_LIST (list));

		/* Remember some data */
		gtk_object_set_data_full (GTK_OBJECT (item->entry), "camera",
					  camera,
					  (GtkDestroyNotify) gp_camera_unref);
		gp_camera_ref (camera);
		gtk_object_set_data_full (GTK_OBJECT (item->entry), "folder",
					  g_strdup (folder),
					  (GtkDestroyNotify) g_free);
		gtk_object_set_data (GTK_OBJECT (item->entry), "multi",
				     GINT_TO_POINTER (multi));

		/*
		 * Second step: Get information about the image.
		 */
		result = gp_camera_file_get_info (camera, folder,
						  name, &info, NULL);
		if (result != GP_OK) {
			gp_log (GP_LOG_DEBUG, PACKAGE, "Could not get "
				"information on '%s' in '%s': %s",
				folder, name, gp_result_as_string (result));
			continue;
		}

		/*
		 * Third step: Show the preview if there is one and
		 * 	       if it has been requested.
		 */
		if (list->priv->thumbnails &&
		    (a.file_operations & GP_FILE_OPERATION_PREVIEW) &&
		    info.preview.fields) {
			gp_file_new (&file);
			s = gtkam_status_new (_("Getting preview of file '%s' "
				"in folder '%s'..."), name, folder);
			gtk_widget_show (s);
			gtk_box_pack_start (GTK_BOX (list->priv->status), s,
				FALSE, FALSE, 0);
			result = gp_camera_file_get (camera, folder,
				name, GP_FILE_TYPE_PREVIEW, file,
				GTKAM_STATUS (s)->context->context);
			switch (result) {
			case GP_OK:
				tmp = gdk_pixbuf_new_from_camera_file (file,
						ICON_WIDTH, win);
				if (tmp) {
					gdk_pixbuf_unref (pixbuf);
					pixbuf = tmp;
					gdk_pixbuf_render_pixmap_and_mask (
						pixbuf, &pixmap, &bitmap, 127);
					gtk_pixmap_set (
						GTK_PIXMAP (item->pixmap),
						pixmap, bitmap);
				}
				break;
			case GP_ERROR_NOT_SUPPORTED:
			case GP_ERROR_CANCEL:
				break;
			default:
				dialog = gtkam_error_new (result,
					GTKAM_STATUS (s)->context, win,
					_("Could not get preview of file '%s' "
					"in folder '%s'."), name, folder);
				gtk_widget_show (dialog);
			}
			gtk_object_destroy (GTK_OBJECT (s));
			gp_file_unref (file);
		}

		/*
		 * Third step: Show additional information
		 */
		if (!gdk_pixbuf_get_has_alpha (pixbuf)) {
			tmp = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
			gdk_pixbuf_unref (pixbuf);
			pixbuf = tmp;
		}

		/* Check for audio data */
		if (info.audio.fields) {
			tmp = gdk_pixbuf_new_from_file (
				IMAGE_DIR "/gtkam-audio.png");
			gdk_pixbuf_add (pixbuf, 0, 0, tmp);
			gdk_pixbuf_unref (tmp);
		}

		/* Check for read-only flag */
		if ((info.file.fields & GP_FILE_INFO_PERMISSIONS) &&
		    !(info.file.permissions & GP_FILE_PERM_DELETE)) {
			tmp = gdk_pixbuf_new_from_file (
				IMAGE_DIR "/gtkam-lock.png");
			w = gdk_pixbuf_get_width (tmp);
			h = gdk_pixbuf_get_height (tmp);
			gdk_pixbuf_add (pixbuf,
				gdk_pixbuf_get_width (pixbuf) - w,
				gdk_pixbuf_get_height (pixbuf) - h,
				tmp);
			gdk_pixbuf_unref (tmp);
		}

		/* Check for downloaded flag */
		if ((info.file.fields & GP_FILE_INFO_STATUS) &&
		    (info.file.status & GP_FILE_STATUS_NOT_DOWNLOADED)){
			tmp = gdk_pixbuf_new_from_file (
				IMAGE_DIR "/gtkam-new.png");
			w = gdk_pixbuf_get_width (tmp);
			gdk_pixbuf_add (pixbuf,
				gdk_pixbuf_get_width (pixbuf) - w, 0, tmp);
			gdk_pixbuf_unref (tmp);
		}

		gdk_pixbuf_render_pixmap_and_mask (pixbuf,
					&pixmap, &bitmap, 127);
		gtk_pixmap_set (GTK_PIXMAP (item->pixmap), pixmap, bitmap);
	}

	if (!GTKAM_IS_LIST (list))
		return;
	
	if (multi)
		gp_camera_exit (camera, NULL);
	gtk_signal_emit (GTK_OBJECT (list), signals[CHANGED]);
#endif
}

typedef struct _GtkamListRemoveData GtkamListRemoveData;
struct _GtkamListRemoveData {
	GtkamList *list;
	Camera *camera;
	gboolean multi;
	const gchar *folder;
	GtkTreeIter *iter;
};

static gboolean
remove_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
		     gpointer data)
{
	GtkamListRemoveData *rd = data;
	Camera *camera;
	gchar *folder;

	camera = gtkam_list_get_camera_from_iter (rd->list, iter);
	folder = gtkam_list_get_folder_from_iter (rd->list, iter);
	g_return_val_if_fail (folder != NULL, FALSE);

	if ((camera == rd->camera) && (!strcmp (folder, rd->folder))) {
		rd->iter = gtk_tree_iter_copy (iter);
		g_free (folder);
		return (TRUE);
	}

	g_free (folder);
	return (FALSE);
}

void
gtkam_list_remove_folder (GtkamList *list, Camera *camera,
			  gboolean multi, const gchar *folder)
{
	GtkamListRemoveData rd;

	g_return_if_fail (GTKAM_IS_LIST (list));

	rd.camera = camera;
	rd.multi = multi;
	rd.folder = folder;
	rd.list = list;
	rd.iter = NULL;
	gtk_tree_model_foreach (GTK_TREE_MODEL (list->priv->store),
				remove_foreach_func, &rd);
	while (rd.iter) {
		gtk_list_store_remove (list->priv->store, rd.iter);
		gtk_tree_iter_free (rd.iter);
		rd.iter = NULL;
		gtk_tree_model_foreach (GTK_TREE_MODEL (list->priv->store),
					remove_foreach_func, &rd);
	}
}

void
gtkam_list_save_selected (GtkamList *list)
{
#if 0
	GtkIconListItem *item;
	GtkWidget *save;
	guint i;

	g_return_if_fail (GTKAM_IS_LIST (list));

	if (!g_list_length (GTK_ICON_LIST (list)->selection))
		return;

	save = gtkam_save_new (list->priv->status);
	for (i = 0; i < g_list_length (GTK_ICON_LIST (list)->selection); i++) {
		item = g_list_nth_data (GTK_ICON_LIST (list)->selection, i);
		gtkam_save_add (GTKAM_SAVE (save),
			gtk_object_get_data (GTK_OBJECT (item->entry),
				"camera"),
			GPOINTER_TO_INT (
				gtk_object_get_data (GTK_OBJECT (item->entry),
					"multi")),
			gtk_object_get_data (GTK_OBJECT (item->entry),
				"folder"),
			gtk_entry_get_text (GTK_ENTRY (item->entry)));
	}
	gtk_widget_show (save);
#endif
}

void
gtkam_list_save_all (GtkamList *list)
{
	g_return_if_fail (GTKAM_IS_LIST (list));
}

guint
gtkam_list_count_all (GtkamList *list)
{
	g_return_val_if_fail (GTKAM_IS_LIST (list), 0);

	return (gtk_tree_model_iter_n_children (
				GTK_TREE_MODEL (list->priv->store), NULL));
}

static void
count_foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
		    gpointer data)
{
	guint *n = data;

	(*n)++;
}

guint
gtkam_list_count_selected (GtkamList *list)
{
	GtkTreeSelection *selection;
	guint n = 0;

	g_return_val_if_fail (GTKAM_IS_LIST (list), 0);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list));
	gtk_tree_selection_selected_foreach (selection, count_foreach_func, &n);

	return (n);
}

gboolean
gtkam_list_has_folder (GtkamList *list, Camera *camera, const gchar *folder)
{
	g_return_val_if_fail (GTKAM_IS_LIST (list), FALSE);

	g_warning ("Fixme: gtkam_list_has_folder");

	return (FALSE);
}

void
gtkam_list_add_file (GtkamList *list, Camera *camera, gboolean multi,
		     const gchar *folder, const gchar *name)
{
	g_return_if_fail (GTKAM_IS_LIST (list));

	g_warning ("Fixme: gtkam_list_add_file");
}
