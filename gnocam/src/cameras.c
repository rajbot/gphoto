#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "gphoto-extensions.h"
#include "callbacks.h"
#include "gnocam.h"
#include "cameras.h"
#include "information.h"

/**
 * camera_tree_clean:
 *
 * This function removes all items from a tree and makes sure that 
 * the tree is still connected with its parent.
 **/
void
camera_tree_clean (GtkTree* tree)
{
	GtkTreeItem*	item;
	gint		i;
	GtkWidget*	owner;
	gchar*		filename;
	gchar*		path;
	GtkWidget*	page;
	GladeXML*	xml;
	GtkNotebook*	notebook;

	g_assert (tree != NULL);
	g_assert ((xml = gtk_object_get_data (GTK_OBJECT (tree), "xml")) != NULL);	
	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);

	/* Delete all items of tree. */
	owner = tree->tree_owner;
	for (i = g_list_length (tree->children) - 1; i >= 0; i--) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
		g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);

		/* Do we have to remove a notebook page? */
		if ((page = gtk_object_get_data (GTK_OBJECT (item), "page"))) gtk_notebook_remove_page (notebook, gtk_notebook_page_num (notebook, page));

		/* Folder or file? */
		if ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {
			g_free (filename);
		} else {

			/* It could be that gphoto had been unable to get the file/folder list. */
			if (item->subtree) {
				camera_tree_clean (GTK_TREE (item->subtree));
				gtk_object_unref (GTK_OBJECT (item->subtree));
			} 
		}

		/* Clean up. */
		g_free (path);
		gtk_container_remove (GTK_CONTAINER (tree), GTK_WIDGET (item));
	}

	/* Make sure the subtree itself does not get lost. */
	if (GTK_WIDGET (tree)->parent == NULL) gtk_tree_item_set_subtree (GTK_TREE_ITEM (owner), GTK_WIDGET (tree));
}

void
camera_tree_remove_file (GtkTreeItem* item)
{
	GladeXML*	xml;
	gchar*		path;
	gchar*		filename;
	GtkNotebook*	notebook;
	GtkWidget*	page;
	GtkTree*	tree;

	g_assert ((xml = gtk_object_get_data (GTK_OBJECT (item), "xml")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);
	g_assert ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((tree = GTK_TREE (GTK_WIDGET (item)->parent)) != NULL);

	if (g_list_length (tree->children) == 1) camera_tree_clean (tree);
	else {
                /* Do we have to remove a notebook page? */
                if ((page = gtk_object_get_data (GTK_OBJECT (item), "page"))) gtk_notebook_remove_page (notebook, gtk_notebook_page_num (notebook, page));

		/* Remove the item. */
		g_free (filename);
		g_free (path);
		gtk_container_remove (GTK_CONTAINER (tree), GTK_WIDGET (item));
	}
}

void
camera_tree_folder_add (GtkTree* tree, Camera* camera, gchar* path)
{
	GladeXML*	xml;
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
	g_assert ((xml = gtk_object_get_data (GTK_OBJECT (tree), "xml")) != NULL);

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
        gtk_object_set_data (GTK_OBJECT (item), "xml", xml);
        gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));

        /* Do we have folders? */
        if (gp_camera_folder_list (camera, &folder_list, path) != GP_OK) {
                dialog_information ("Could not get folder list for folder '%s'!", path);
                folder_list_count = 0;
        } else folder_list_count = gp_list_count (&folder_list);
        gtk_object_set_data (GTK_OBJECT (item), "folder_list_count", GINT_TO_POINTER (folder_list_count));

        /* Do we have files (in the root directory)? */
        if (gp_camera_file_list (camera, &file_list, path) != GP_OK) {
                dialog_information ("Could not get file list for folder '%s'!", path);
                file_list_count = 0;
        } else file_list_count = gp_list_count (&file_list);
        gtk_object_set_data (GTK_OBJECT (item), "file_list_count", GINT_TO_POINTER (file_list_count));

        /* Do we have to create a subtree? */
        if (file_list_count + folder_list_count > 0) {

                /* Create the subtree. Don't populate it yet. */
                subtree = gtk_tree_new ();
                gtk_widget_ref (subtree);
                gtk_widget_show (subtree);
                gtk_tree_item_set_subtree (item, subtree);
                gtk_object_set_data (GTK_OBJECT (subtree), "xml", xml);
        }
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
	GtkWidget*	item;
        GList*          tree_list;
        gchar*          description;
        gint            i;
        gint            j;
        guint           id;
        Camera*         camera;
	GladeXML*	xml;

	g_assert (tree != NULL);
	g_assert ((xml = gtk_object_get_data (GTK_OBJECT (tree), "xml")) != NULL);

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
                        if ((camera = gp_camera_new_by_description (xml, description))) {

				/* Add the camera to the tree. */
				camera_tree_folder_add (tree, camera, "/");
			}
                }
                g_free (description);
        }

        /* Delete all unchecked cameras. */
        tree_list = g_list_first (tree->children);
        for (j = g_list_length (tree_list) - 1; j >= 0; j--) {
                item = GTK_WIDGET (g_list_nth_data (tree_list, j));
                if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "checked")) == 0) {

			/* Remove subtree. */
			if (GTK_TREE_ITEM (item)->subtree) {
				camera_tree_clean (GTK_TREE (GTK_TREE_ITEM (item)->subtree));
				gtk_object_unref (GTK_OBJECT (GTK_TREE_ITEM (item)->subtree));
			}

			/* Remove the entry. */
			gp_camera_free (gtk_object_get_data (GTK_OBJECT (item), "camera"));
			g_free (gtk_object_get_data (GTK_OBJECT (item), "path"));
                        gtk_tree_remove_item (tree, item);

                        tree_list = g_list_first (tree->children);
                }
        }
}


