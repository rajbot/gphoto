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
#include "cameras.h"
#include "properties.h"
#include "information.h"

/********************/
/* Static Variables */
/********************/

static GladeXML *xml;

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera *camera, char *status) 
{
	gnome_appbar_set_status (GNOME_APPBAR (glade_xml_get_widget (xml, "appbar")), status);
	return (GP_OK);
}

int gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
	gnome_appbar_set_progress (GNOME_APPBAR (glade_xml_get_widget (xml, "appbar")), percentage / 100);
	return (GP_OK);
}

int gp_frontend_message (Camera *camera, char *message)
{
	dialog_information (message);
	return (GP_OK);
}

int gp_frontend_confirm (Camera *camera, char *message)
{
	//FIXME
	return (GP_CONFIRM_NO);
}

int gp_frontend_prompt (Camera *camera, CameraWidget *window)
{
	frontend_data_t*	frontend_data;
	
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	/* Is the propertybox opened? */	
	if (frontend_data->xml_properties) {
		values_set (frontend_data->xml_properties, window);
		return (GP_PROMPT_OK);
	} else {
		camera_properties (xml, camera, window);
		return (GP_PROMPT_CANCEL);
	}
}

static void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
	GladeXML*	xml;

	g_assert ((xml = user_data) != NULL);

	if (entry->value == NULL) {
		
		/* No cameras configured. */
		cameras_update (xml, NULL);
	} else {
		cameras_update (xml, entry->value);
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

	/* Init threads. */
	g_thread_init (NULL);

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
	value = gconf_client_get (client, "/apps/" PACKAGE "/debug_level", NULL);
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
		cameras_update (xml, value);
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
	gdk_threads_enter ();
	gtk_main ();
	gdk_threads_leave ();

	/* Clean up. */
	gp_exit ();
	gtk_object_unref (GTK_OBJECT (client));
	return (0);
}

