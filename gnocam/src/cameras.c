//This file should be called utils.c or so.
//I'll change the name some day. 

#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include "gphoto-extensions.h"
#include "callbacks.h"
#include "gnocam.h"
#include "cameras.h"
#include "information.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;
extern GConfClient*	client;

/**
 * camera_tree_folder clean:
 *
 * This function removes all items from a tree and makes sure that 
 * the tree is still connected with its parent. Any associated
 * pages in the notebook are removed.
 **/
void
camera_tree_folder_clean (GtkTreeItem* folder)
{
	gint		i;
	GtkTree*	tree;

	g_assert (folder != NULL);
	g_assert ((tree = GTK_TREE (folder->subtree)) != NULL);

	/* Delete all items of tree. */
	for (i = g_list_length (tree->children) - 1; i >= 0; i--) camera_tree_item_remove (GTK_TREE_ITEM (g_list_nth_data (tree->children, i)));
}

void
camera_tree_item_remove (GtkTreeItem* item)
{
	gchar*			path;
	gchar*			filename;
	GtkNotebook*		notebook;
	GtkWidget*		page;
	GtkWidget*		owner;
	GtkTree*		tree;
	Camera*			camera;
	gboolean		root;
	frontend_data_t*	frontend_data = NULL;
	CameraFile*		file;

	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((tree = GTK_TREE (GTK_WIDGET (item)->parent)) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	/* Root folder needs special care. */
	root = ((!(filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) && (strcmp ("/", path) == 0));

	/* If item is a folder, clean it. */
	if (!filename) {
		g_assert (item->subtree);
		camera_tree_folder_clean (item);
		gtk_widget_unref (item->subtree);
	}

        /* Do we have to remove a notebook page? */
        if ((page = gtk_object_get_data (GTK_OBJECT (item), "page"))) gtk_notebook_remove_page (notebook, gtk_notebook_page_num (notebook, page));

	/* If it's the root folder, unref the camera. */
	if (root) gp_camera_unref (camera);

	/* If this is the last item, we have to make sure we don't loose the 	*/
	/* tree. Therefore, keep a reference to the tree owner. 		*/
	owner = tree->tree_owner;

	/* Clean up. */
	g_free (path);
	if (filename) g_free (filename);
	if ((file = gtk_object_get_data (GTK_OBJECT (item), "file"))) gp_file_free (file);
	if ((file = gtk_object_get_data (GTK_OBJECT (item), "preview"))) gp_file_free (file);
        gtk_container_remove (GTK_CONTAINER (tree), GTK_WIDGET (item));

	/* Make sure the tree does not get lost. */
	if (GTK_WIDGET (tree)->parent == NULL) {

		/* This does only happen with tree items. I hope so. */
		g_assert (GTK_IS_TREE_ITEM (owner));
		gtk_tree_item_set_subtree (GTK_TREE_ITEM (owner), GTK_WIDGET (tree));
	}
}

void
camera_tree_folder_add (GtkTree* tree, Camera* camera, gchar* path)
{
	GtkTreeItem*	item;
	GtkWidget*	subtree;
	gboolean 	root;
	CameraList	folder_list;
	CameraList	file_list;
	gint		folder_list_count;
	gint		file_list_count;
	GtkTargetEntry 	target_table[] = {{"text/uri-list", 0, 0}};
	
	g_assert (camera != NULL);
	g_assert (tree != NULL);
	g_assert (path != NULL);

	/* Root folder needs special care. */
	root = (strcmp ("/", path) == 0);

	/* Create the new item. */
	if (root) item = GTK_TREE_ITEM (gtk_tree_item_new_with_label (((frontend_data_t*) camera->frontend_data)->name));
	else item = GTK_TREE_ITEM (gtk_tree_item_new_with_label (path));
        gtk_widget_show (GTK_WIDGET (item));
        gtk_tree_append (tree, GTK_WIDGET (item));

        /* For drag and drop. */
        //FIXME: Right now, only drops onto the camera (= root folder) work. Why?!?
        gtk_drag_dest_set (GTK_WIDGET (item), GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (item), "drag_data_received", GTK_SIGNAL_FUNC (on_drag_data_received), NULL);
	gtk_signal_connect (GTK_OBJECT (item), "expand", GTK_SIGNAL_FUNC (on_tree_item_expand), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "collapse", GTK_SIGNAL_FUNC (on_tree_item_collapse), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "deselect", GTK_SIGNAL_FUNC (on_tree_item_deselect), NULL);
	if (root) gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_camera_button_press_event), NULL);
	else gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_folder_button_press_event), NULL);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (item), "path", g_strdup (path));
        gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));

        /* Do we have folders? */
        if (gp_camera_folder_list (camera, &folder_list, path) != GP_OK) {
                dialog_information ("Could not get folder list for folder '%s'!", path);
                folder_list_count = 0;
        } else folder_list_count = gp_list_count (&folder_list);
        gtk_object_set_data (GTK_OBJECT (item), "folder_list_count", GINT_TO_POINTER (folder_list_count));

        /* Do we have files? */
        if (gp_camera_file_list (camera, &file_list, path) != GP_OK) {
                dialog_information ("Could not get file list for folder '%s'!", path);
                file_list_count = 0;
        } else file_list_count = gp_list_count (&file_list);
        gtk_object_set_data (GTK_OBJECT (item), "file_list_count", GINT_TO_POINTER (file_list_count));

        /* Create the subtree. Don't populate it yet. */
        subtree = gtk_tree_new ();
        gtk_widget_ref (subtree);
        gtk_widget_show (subtree);
        gtk_tree_item_set_subtree (item, subtree);
}

/**
 * camera_tree_update:
 *
 * This function adjusts the camera tree to reflect the current settings
 * as indicated in value.
 **/
void
camera_tree_update (GtkTree* tree, GConfValue* value)
{
        GSList*         list_cameras = NULL;
        GtkObject*      object;
	GtkTreeItem*	item;
        GList*          tree_list;
        gchar*          description;
        gint            i;
        gint            j;
        guint           id;
        Camera*         camera;

	g_assert (tree != NULL);

        if (value) {
                g_assert (value->type == GCONF_VALUE_LIST);
                g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
                list_cameras = gconf_value_get_list (value);
        }

        tree_list = g_list_first (tree->children);

        /* Mark each camera in the tree as unchecked. */
        for (i = 0; i < g_list_length (tree_list); i++) gtk_object_set_data (GTK_OBJECT (g_list_nth_data (tree_list, i)), "checked", GINT_TO_POINTER (0));

	/* Investigate if the new cameras are in the tree. */
        for (i = 0; i < g_slist_length (list_cameras); i++) {
                tree_list = g_list_first (tree->children);
                value = g_slist_nth_data (list_cameras, i);
                g_assert (value->type == GCONF_VALUE_STRING);
                description = g_strdup (gconf_value_get_string (value));

                /* Get the camera id from the description.      */
                /* Remember, the description is "id\n...        */
                id = (guint) atoi (description);

                /* Do we have this camera in the tree? */
                for (j = 0; j < g_list_length (tree_list); j++) {
                        g_assert (GTK_IS_OBJECT (g_list_nth_data (tree_list, j)));
                        object = GTK_OBJECT (g_list_nth_data (tree_list, j));
                        g_assert ((camera = gtk_object_get_data (object, "camera")) != NULL);
                        if (id == ((frontend_data_t*) camera->frontend_data)->id) {

                                /* We found the camera. Do we have to update? */
                                if (gp_camera_update_by_description (&camera, description)) {
                                        /* Update the label. */
                                        gtk_label_set_text (GTK_LABEL (GTK_BIN (object)->child), ((frontend_data_t*) camera->frontend_data)->name);

                                        /* Mark camera as checked. */
                                        g_assert (GPOINTER_TO_INT (gtk_object_get_data (object, "checked")) == 0);
                                        gtk_object_set_data (object, "checked", GINT_TO_POINTER (1));
                                }
                                break;
                        }
                }
                if (j == g_list_length (tree_list)) {

                        /* We don't have the camera in the tree (yet). */
                        if ((camera = gp_camera_new_by_description (description))) {

				/* Add the camera to the tree. */
				gp_camera_ref (camera);
				camera_tree_folder_add (tree, camera, "/");
			}
                }
                g_free (description);
        }

        /* Delete all unchecked cameras. */
        tree_list = g_list_first (tree->children);
        for (j = g_list_length (tree_list) - 1; j >= 0; j--) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree_list, j));
                if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "checked")) == 0) {
			camera_tree_item_remove (item);
                        tree_list = g_list_first (tree->children);
                }
        }
}

void
update_pixmap (GtkPixmap* pm, CameraFile* file)
{
	GdkInterpType		interpolation = 0;
	gfloat			magnification;
	GConfValue*		value;
        GdkPixbuf*              pixbuf;
        GdkPixbufLoader*        loader;
        GdkPixmap*              pixmap;
        GdkBitmap*              bitmap;

	g_assert (file != NULL);
	g_assert (pm != NULL);

        g_assert ((loader = gdk_pixbuf_loader_new ()) != NULL);
        if (gdk_pixbuf_loader_write (loader, file->data, file->size)) {
                gdk_pixbuf_loader_close (loader);
                g_assert ((pixbuf = gdk_pixbuf_loader_get_pixbuf (loader)) != NULL);

                /* Magnification? */
                value = gconf_client_get_without_default (client, "/apps/" PACKAGE "/magnification", NULL);
                if (value) {
                        g_assert (value->type = GCONF_VALUE_FLOAT);
                        magnification = gconf_value_get_float (value);
                } else magnification = 1;

                /* Interpolation? */
                value = gconf_client_get_without_default (client, "/apps/" PACKAGE "/interpolation", NULL);
                if (value) {
                        g_assert (value->type = GCONF_VALUE_INT);
                        switch (gconf_value_get_int (value)) {
                        case 0:
                                interpolation = GDK_INTERP_NEAREST;
                                break;
                        case 1:
                                interpolation = GDK_INTERP_TILES;
                                break;
                        case 2:
                                interpolation = GDK_INTERP_BILINEAR;
                                break;
                        case 3:
                                interpolation = GDK_INTERP_HYPER;
                                break;
                        default:
                                g_assert_not_reached ();
                        }
                } else interpolation = GDK_INTERP_BILINEAR;
                gdk_pixbuf_render_pixmap_and_mask (gdk_pixbuf_scale_simple (
                        pixbuf,
                        magnification * gdk_pixbuf_get_width (pixbuf),
                        magnification * gdk_pixbuf_get_height (pixbuf),
                        interpolation), &pixmap, &bitmap, 127);
                gdk_pixbuf_unref (pixbuf);
                gtk_pixmap_set (pm, pixmap, bitmap);
        } else dialog_information (_("Could not load image '%s'!"), file->name);
}

void
camera_tree_update_pixmaps (GtkTree* tree)
{
        GtkTreeItem*    item;
        gint            i;
	GtkPixmap*	pixmap;

	g_assert (tree != NULL);

        for (i = 0; i < g_list_length (tree->children); i++) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
                if (item->subtree) camera_tree_update_pixmaps (GTK_TREE (item->subtree));
                else if ((pixmap = gtk_object_get_data (GTK_OBJECT (item), "pixmap"))) 
			update_pixmap (pixmap, gtk_object_get_data (GTK_OBJECT (item), "preview"));
        }
} 

