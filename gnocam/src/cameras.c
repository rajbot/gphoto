#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "gphoto-extensions.h"
#include "callbacks.h"
#include "gnocam.h"
#include "cameras.h"
#include "information.h"

/**************/
/* Prototypes */
/**************/

void folders_create_recursive (GladeXML *xml, GtkWidget *item, Camera *camera, gchar *path);

/*************/
/* Functions */
/*************/

void
folders_create_recursive (GladeXML *xml, GtkWidget *item, Camera *camera, gchar *path)
{
        CameraList 		folder_list;
        CameraListEntry*	folder_list_entry;
        gint 			count, i, j;
        GnomeApp*		app;
        GtkWidget*		subtree;
	gchar*			new_path;

        g_assert (item != NULL);
        g_assert (camera != NULL);
        g_assert (path != NULL);
        g_assert (xml != NULL);
        g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);

        if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
                count = gp_list_count (&folder_list);
                /* Do we have subfolders? */
                if (count > 0) {
//FIXME (Directory Browse): I can't parse the whole file tree. If I try to do so, I get the following error:
//GLib-ERROR **: Cannot create pipe main loop wake-up: Too many open files
//and the program crashes.
//Therefore, we'll go not deeper than two levels, that is two times '/' in the path.
j = 0;
for (i = 0; path[i] != '\0'; i++) if (path[i] == '/') j++;
if (j > 1) {
g_warning ("Can't go deeper than two levels in tree!");
return;
}
//
// We should probably fix this in the future.
//
                        /* We have subfolders. */
                        subtree = gtk_tree_new ();
                        gtk_tree_item_set_subtree (GTK_TREE_ITEM (item), subtree);

                        for (i = 0; i < count; i++) {
                                folder_list_entry = gp_list_entry (&folder_list, i);

                                /* Add the subfolder to the tree. */
                                item = gtk_tree_item_new_with_label (folder_list_entry->name);
                                gtk_widget_show (item);
                                gtk_tree_append (GTK_TREE (subtree), item);

				/* Construct the new path. */
				if (strcmp (path, "/") == 0) new_path = g_strdup_printf ("/%s", folder_list_entry->name);
                                else new_path = g_strdup_printf ("%s/%s", path, folder_list_entry->name);

				/* Store some data. */
				gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
				gtk_object_set_data (GTK_OBJECT (item), "path", new_path);

				/* Look in path for subfolders. */
                                folders_create_recursive (xml, item, camera, new_path);
                        }
                }
        } else {
                dialog_information ("Could not get folder list of folder '%s'!", path);
        }
}

void
cameras_update (GladeXML* xml, GConfValue* value)
{
        GtkTree*        tree;
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
			
			        /* Store some data. */
			        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
			        gtk_object_set_data (GTK_OBJECT (item), "path", path);
			        gtk_object_set_data (GTK_OBJECT (item), "xml", xml);
			        gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));
	
			        /* Do we have folders? */
			        if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
			                folders_create_recursive (xml, item, camera, path);
			        } else {
			                gnome_app_error (app, _("Could not get folder list!"));
			        }
			}
                }
                g_free (description);
        }

        /* Delete all unchecked cameras. */
        tree_list = g_list_first (tree->children);
        for (j = 0; j < g_list_length (tree_list); j++) {
                item = GTK_WIDGET (g_list_nth_data (tree_list, j));
                if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "checked")) == 0) {

			/* Remove all sub-folders. */
//FIXME: Shouldn't we free all paths first? -> g_free (gtk_object_get_data (item, "path")) for all items below this item...
			if (GTK_TREE_ITEM (item)->subtree) gtk_tree_item_remove_subtree (GTK_TREE_ITEM (item));

			/* Remove the entry. */
			gp_camera_free (gtk_object_get_data (GTK_OBJECT (item), "camera"));
			g_free (gtk_object_get_data (GTK_OBJECT (item), "path"));
                        gtk_tree_remove_item (tree, item);

                        tree_list = g_list_first (tree->children);
                }
        }
}


