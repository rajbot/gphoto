#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "gphoto-extensions.h"
#include "callbacks.h"
#include "gnocam.h"
#include "cameras.h"
#include "information.h"

/*************/
/* Functions */
/*************/

void
cameras_clean_subtree (GtkTree* tree)
{
	GtkTreeItem*	item;
	gint		i;
	GtkWidget*	owner;

	g_assert (tree != NULL);

	/* Delete all items of tree. */
	owner = tree->tree_owner;
	for (i = g_list_length (tree->children) - 1; i >= 0; i--) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
		if (item->subtree) {
			cameras_clean_subtree (GTK_TREE (item->subtree));
			gtk_object_unref (GTK_OBJECT (item->subtree));
		}
		g_free (gtk_object_get_data (GTK_OBJECT (item), "path"));
		gtk_container_remove (GTK_CONTAINER (tree), GTK_WIDGET (item));
	}

	/* Make sure the subtree itself does not get lost. */
	if (GTK_WIDGET (tree)->parent == NULL) gtk_tree_item_set_subtree (GTK_TREE_ITEM (owner), GTK_WIDGET (tree));
}

void
cameras_update (GladeXML* xml, GConfValue* value)
{
        GtkTree*        tree;
	GtkWidget*	sub_tree;
        GSList*         list_cameras = NULL;
        GtkObject*      object;
	GtkWidget*	item;
        GList*          tree_list;
        gchar*          description;
	gchar*		path;
        gint            i;
        gint            j;
        guint           id;
        Camera*         camera;
	CameraList	folder_list;
	GnomeApp*	app;
        GtkTargetEntry target_table[] = {
                {"text/uri-list", 0, 0}
        };

        g_assert (xml != NULL);
        g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);
	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);

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
                                if (gp_camera_update_by_description (xml, &camera, description)) {
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
			        path = g_strdup ("/");
			        item = gtk_tree_item_new_with_label (((frontend_data_t*) camera->frontend_data)->name);
			        gtk_widget_show (item);
			        gtk_tree_append (tree, item);
			
			        /* For drag and drop. */
			        //FIXME: Right now, only drops onto the camera (= root folder) work. Why?!?
			        gtk_drag_dest_set (item, GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
			
			        /* Connect the signals. */
			        gtk_signal_connect (GTK_OBJECT (item), "drag_data_received", GTK_SIGNAL_FUNC (on_drag_data_received), NULL);
			        gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_cameras_button_press_event), NULL);
				gtk_signal_connect (GTK_OBJECT (item), "expand", GTK_SIGNAL_FUNC (on_tree_item_expand), NULL);
				gtk_signal_connect (GTK_OBJECT (item), "collapse", GTK_SIGNAL_FUNC (on_tree_item_collapse), NULL);
			
			        /* Store some data. */
			        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
			        gtk_object_set_data (GTK_OBJECT (item), "path", path);
			        gtk_object_set_data (GTK_OBJECT (item), "xml", xml);
			        gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));
	
			        /* Do we have folders? */
			        if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
					if (gp_list_count (&folder_list) > 0) {

						/* Create the subtree. Don't populate it yet. */
						sub_tree = gtk_tree_new ();
						gtk_widget_ref (sub_tree);
						gtk_widget_show (sub_tree);
						gtk_tree_item_set_subtree (GTK_TREE_ITEM (item), sub_tree);
					}
			        } else dialog_information ("Could not get folder list for folder '%s'!", path);
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
				cameras_clean_subtree (GTK_TREE (GTK_TREE_ITEM (item)->subtree));
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


