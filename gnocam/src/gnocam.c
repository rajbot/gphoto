#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <callbacks.h>
#include "preferences.h"
#include "gphoto-extensions.h"
#include "gnocam.h"

/********************/
/* Static Variables */
/********************/

static GladeXML *xml;

/**************/
/* Prototypes */
/**************/

void app_cameras_update (GladeXML* xml, GConfValue* value);
void camera_add_to_tree (GladeXML *xml, Camera *camera, gchar *name, gint camera_id, gint position);
void folder_build (GladeXML *xml, GtkWidget *item, Camera *camera, gint camera_id, gchar *path);

void on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera *camera, char *status) 
{
	gnome_appbar_set_status (GNOME_APPBAR (glade_xml_get_widget (xml, "appbar")), status);
	return (0);
}

int gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
	gnome_appbar_set_progress (GNOME_APPBAR (glade_xml_get_widget (xml, "appbar")), percentage / 100);
	return (0);
}

int gp_frontend_message (Camera *camera, char *message)
{
	gnome_dialog_run_and_close (GNOME_DIALOG (gnome_app_warning (GNOME_APP (glade_xml_get_widget (xml, "app")), message)));
	return (0);
}

int gp_frontend_confirm (Camera *camera, char *message)
{
	//FIXME
	return (0);
}

int gp_frontend_prompt (Camera *camera, CameraWidget *window)
{
	//FIXME
	return (0);
}

void
on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time)
{
        GList *filenames;
        guint i;
        gchar *message;

        filenames = gnome_uri_list_extract_filenames (selection_data->data);
        for (i = 0; i < g_list_length (filenames); i++) {
                message = g_strdup_printf ("Upload not implemented (filename: %s)", (gchar *) g_list_nth_data (filenames, i));
                gnome_dialog_run_and_close (GNOME_DIALOG (gnome_error_dialog (message)));
                g_free (message);
        }
        gnome_uri_list_free_strings (filenames);
}

static void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
	GladeXML*	xml;

	g_assert ((xml = user_data) != NULL);

	if (entry->value == NULL) {
		//FIXME: What happens here?
		g_assert_not_reached ();
	} else {
		app_cameras_update (xml, entry->value);
	}
}

void
folder_build (GladeXML *xml, GtkWidget *item, Camera *camera, gint camera_id, gchar *path)
{
        CameraList folder_list;
        CameraListEntry *folder_list_entry;
        guint count, i;
        GnomeApp *app;
        GtkWidget *subtree;

        g_assert (item != NULL);
        g_assert (camera != NULL);
        g_assert (path != NULL);
        g_assert (xml != NULL);
        app = GNOME_APP (glade_xml_get_widget (xml, "app"));
        g_assert (app != NULL);

        if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
                count = gp_list_count (&folder_list);

                /* Do we have subfolders? */
                if (count > 0) {
                        /* We have subfolders. */
                        subtree = gtk_tree_new ();
                        gtk_tree_item_set_subtree (GTK_TREE_ITEM (item), subtree);

                        for (i = 0; i < count; i++) {
                                folder_list_entry = gp_list_entry (&folder_list, i);

                                /* Add the subfolder to the tree. */
                                item = gtk_tree_item_new_with_label (folder_list_entry->name);
                                gtk_widget_show (item);
                                gtk_tree_append (GTK_TREE (subtree), item);
                                path = g_strdup_printf ("%s/%s", path, folder_list_entry->name);
                                gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
				gtk_object_set_data (GTK_OBJECT (item), "camera_id", GINT_TO_POINTER (camera_id));
                                gtk_object_set_data (GTK_OBJECT (item), "path", path);
                                folder_build (xml, item, camera, camera_id, path);
                        }
                }
        } else {
                gnome_app_error (app, _("Could not get folder list!"));
        }
}

void
camera_add_to_tree (GladeXML *xml, Camera *camera, gchar *name, gint camera_id, gint position)
{
        gchar *path;
        GnomeApp *app;
        GtkWidget *item;
        CameraList folder_list;
        GtkTree *tree;
        GtkTargetEntry target_table[] = {
                {"text/uri-list", 0, 0}
        };

        g_assert (xml != NULL);
        g_assert (camera != NULL);
        g_assert (name != NULL);
        g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);
        g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);

        path = g_strdup ("/");
        item = gtk_tree_item_new_with_label (name);
        gtk_widget_show (item);
        if (position == -1) gtk_tree_append (tree, item);
        else gtk_tree_insert (tree, item, position);
        
        /* For drag and drop. */
	//FIXME: Right now, only drops onto the camera (= root folder) work. Why?!?
	gtk_drag_dest_set (item, GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
 
        /* Connect the signals. */
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_received", GTK_SIGNAL_FUNC (on_drag_data_received), NULL);
        gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_cameras_button_press_event), NULL);
        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
	gtk_object_set_data (GTK_OBJECT (item), "camera_id", GINT_TO_POINTER (camera_id));
        gtk_object_set_data (GTK_OBJECT (item), "path", path);
        gtk_object_set_data (GTK_OBJECT (item), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (item), "checked", GINT_TO_POINTER (1));
        
        /* Do we have folders? */
        if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
                folder_build (xml, item, camera, camera_id, path);
        } else {
                gnome_app_error (app, _("Could not get folder list!"));
        }
}

void 
app_cameras_update (GladeXML* xml, GConfValue* value)
{
	GtkTree*	tree;
	GSList*		list_cameras;
	GtkObject*	object;
	GList*		tree_list;
	gchar*		camera_description;
	gchar*		description;
	gchar*		name;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gint		camera_id;
	gint		i;
	gint		count;
	gint		j;
	Camera*		camera;

	g_assert (xml != NULL);
	g_assert (value != NULL);
	g_assert (value->type == GCONF_VALUE_LIST);
	g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);	
        g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);

	list_cameras = gconf_value_get_list (value);
        tree_list = g_list_first (tree->children);

        /* Mark each camera in the tree as unchecked. */
        for (i = 0; i < g_list_length (tree_list); i++)
                gtk_object_set_data (GTK_OBJECT (g_list_nth_data (tree_list, i)), "checked", GINT_TO_POINTER (0));

        for (i = 0; i < g_slist_length (list_cameras); i++) {
                tree_list = g_list_first (tree->children);
                count = 0;
                value = g_slist_nth_data (list_cameras, i);
                g_assert (value->type == GCONF_VALUE_STRING);
                description = g_strdup (gconf_value_get_string (value));
		camera_description = g_strdup (description);
                for (j = 0; camera_description[j] != '\0'; j++) {
                        if (camera_description[j] == '\n') {
                                camera_description[j] = '\0';
                                count++;
                        }
                }
                g_assert (count == 4);
                camera_id = atoi (camera_description);
                for (j = 0; camera_description[j] != '\0'; j++);
                name = g_strdup (&camera_description[++j]);
		for (; camera_description[j] != '\0'; j++);
                model = g_strdup (&camera_description[++j]);
        	for (; camera_description[j] != '\0'; j++);
                port = g_strdup (&camera_description[++j]);
                for (; camera_description[j] != '\0'; j++);
                speed = g_strdup (&camera_description[++j]);

		/* Do we have this camera in the tree? */
                for (j = 0; j < g_list_length (tree_list); j++) {
                        g_assert (GTK_IS_OBJECT (g_list_nth_data (tree_list, j)));
                        object = GTK_OBJECT (g_list_nth_data (tree_list, j));
                        if (GPOINTER_TO_INT (gtk_object_get_data (object, "camera_id")) == camera_id) {
                                //FIXME: Check if the port/speed/model/name changed!
                                g_assert (GPOINTER_TO_INT (gtk_object_get_data (object, "checked")) == 0);
                                gtk_object_set_data (object, "checked", GINT_TO_POINTER (1));
                                break;
                        }
                }
                if (j == g_list_length (tree_list)) {

                        /* We don't have the camera in the tree (yet). */
                        camera = gp_camera_new_by_description (description);
                        if (camera) {
                                camera_add_to_tree (xml, camera, name, camera_id, -1);
                        }
                }

                g_free (camera_description);
		g_free (description);
                g_free (name);
                g_free (model);
                g_free (port);
                g_free (speed);
        }

        /* Delete all unchecked cameras. */
        tree_list = g_list_first (tree->children);
        for (j = 0; j < g_list_length (tree_list); j++) {
                object = GTK_OBJECT (g_list_nth_data (tree_list, j));
                if (GPOINTER_TO_INT (gtk_object_get_data (object, "checked")) == 0) {
                        gp_camera_free (gtk_object_get_data (object, "camera"));
                        //FIXME: There is more to free - perhaps a whole subtree...
                        gtk_tree_remove_item (tree, GTK_WIDGET (object));
                        tree_list = g_list_first (tree->children);
                }
        }
}

int main (int argc, char *argv[]) 
{
	static GtkTargetEntry target_table[] = { 
		{"text/uri-list", 0, 0}
	};
	GError*		gerror = NULL;
	GConfClient*	client = NULL;
	GConfValue*	value = NULL;
	guint 		notify_id_cameras;
	gchar*		prefix = NULL;
	gchar*		home = NULL;
	gchar*		message = NULL;

	/* Init GNOME, glade, gnome-vfs, gconf. */
	gnome_init (PACKAGE, VERSION, argc, argv);
	glade_gnome_init ();
	if (!gnome_vfs_init ()) {
		gnome_error_dialog (_("Could not initialize gnome-vfs!"));
		return (1);
	}
	if (!gconf_init (argc, argv, &gerror)) {
		gnome_error_dialog (g_strdup_printf (_("Could not initialize gconf!\n\n%s"), gerror->message));
		return (1);
	}
        client = gconf_client_get_default ();
	gconf_client_add_dir (client, "/apps/" PACKAGE "", GCONF_CLIENT_PRELOAD_NONE, NULL);

	/* Init gphoto2 backend. */
	value = gconf_client_get (client, "/app/" PACKAGE "/debug_level", NULL);
	if (value) {

		/* We already have a value for debug_level in the database. */
		g_assert (value->type == GCONF_VALUE_INT);
		gp_init (gconf_value_get_int (value));
		gconf_value_free (value);
	} else {

		/* We don't have a value for debug_level in the database. */
		gp_init (GP_DEBUG_NONE);
	}
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, gp_frontend_confirm, gp_frontend_prompt);

	/* Load the interface. */
	xml = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app");
	if (xml == NULL) g_error (_("Could not find " GNOCAM_GLADEDIR "gnocam.glade. Check if " PACKAGE " was installed correctly."));

	/* Populate the camera tree. */
	value = gconf_client_get (client, "/apps/" PACKAGE "/cameras", NULL);
	if (value) {
		app_cameras_update (xml, value);
		gconf_value_free (value);
	} 

	value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL);
	if (!value) {
		
		/* Set prefix to HOME by default. */
		value = gconf_value_new (GCONF_VALUE_STRING);
		home = getenv ("HOME");
		prefix = g_strdup_printf ("file:%s", home);
		g_free (home);
		gconf_value_set_string (value, prefix);
		gconf_client_set (client, "/apps/" PACKAGE "/prefix", value, NULL);
		message = g_strdup_printf (
			_("It seems that you are running\n"
                        PACKAGE " for the first time.\n"
                        PACKAGE " is a small but powerful\n"
                        "front-end to gphoto. Welcome to\n"
                        "the wunderful world of gphoto!\n\n"
                        "Before you do anything else, you \n"
                        "should probably first open the \n"
                        "preferences dialog and add a \n"
                        "camera.\n\n"
                        "Enjoy!\n\n(Default prefix has been set to '%s')"), prefix);
		g_free (prefix);
		gnome_app_message (GNOME_APP (glade_xml_get_widget (xml, "app")), message);
		g_free (message);
	} 
	gconf_value_free (value);

	/* connect the signals */
	glade_xml_signal_autoconnect (xml);

	/* Store some data. */
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "app")), "client", client);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_previews")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_preview_as")), "xml", xml);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_files")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_file_as")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "delete")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "exit")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "preferences")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "about")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "tree_cameras")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "clist_files")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_files")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_file_as")), "xml", xml);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_previews")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_preview_as")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_delete")), "xml", xml);

	/* Notification in case the camera setup changes. */
	notify_id_cameras = gconf_client_notify_add (client, "/apps/" PACKAGE "/cameras", on_camera_setup_changed, xml, NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "app")), "notify_id_cameras", GUINT_TO_POINTER (notify_id_cameras));

	/* Drag'n drop stuff. */
	gtk_drag_source_set (glade_xml_get_widget (xml, "clist_files"), GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

	/* Start the event loop. */
	gtk_main ();

	/* Clean up. */
	gp_exit ();
	gtk_object_unref (GTK_OBJECT (client));
	return (0);
}

