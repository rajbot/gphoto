#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "gphoto-extensions.h"
#include "cameras.h"
#include "information.h"
#include "frontend.h"
#include "notification.h"

/********************/
/* Global Variables */
/********************/

GConfClient*	client = NULL;
GladeXML*	xml;

/*************/
/* Functions */
/*************/

int main (int argc, char *argv[]) 
{
//	static GtkTargetEntry target_table[] = {{"text/uri-list", 0, 0}};
	GError*		gerror = NULL;
	GConfValue*	value = NULL;
	guint 		notify_id_cameras, notify_id_magnification, notify_id_interpolation;
	gchar*		prefix = NULL;
	gchar*		home = NULL;
	gchar*		message = NULL;

#ifdef GNOCAM_USES_THREADS
	/* Init threads. */
	g_thread_init (NULL);
#endif

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

	/* Init gphoto2 backend with debug level as stored in database.	*/ 
	/* If there is no debug level stored, init with GP_DEBUG_NONE. 	*/
	if ((value = gconf_client_get (client, "/apps/" PACKAGE "/debug_level", NULL))) {
		g_assert (value->type == GCONF_VALUE_INT);
		gp_init (gconf_value_get_int (value));
		gconf_value_free (value);
	} else gp_init (GP_DEBUG_NONE);
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, gp_frontend_confirm, gp_frontend_prompt);

	/* Load the interface. */
	if (!(xml = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app")))
		g_error (_("Could not find " GNOCAM_GLADEDIR "gnocam.glade. Check if " PACKAGE " was installed correctly."));

	/* Do we already have a prefix in the database? */
	if (!(value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL))) {
		
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
                        "Enjoy!\n\n(Default prefix has been set to '%s'.)"), prefix);
		g_free (prefix);
		gnome_app_message (GNOME_APP (glade_xml_get_widget (xml, "app")), message);
		g_free (message);
	} 
	gconf_value_free (value);

	/* connect the signals */
	glade_xml_signal_autoconnect (xml);

        /* Populate the camera tree. */
        value = gconf_client_get (client, "/apps/" PACKAGE "/cameras", NULL);
        if (value) {
                camera_tree_update (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), value);
                gconf_value_free (value);
        }

	/* Notifications. */
	notify_id_cameras = gconf_client_notify_add (client, "/apps/" PACKAGE "/cameras", on_camera_setup_changed, NULL, NULL, NULL);
	notify_id_magnification = gconf_client_notify_add (client, "/apps/" PACKAGE "/magnification", on_preview_setup_changed, NULL, NULL, NULL);
	notify_id_interpolation = gconf_client_notify_add (client, "/apps/" PACKAGE "/interpolation", on_preview_setup_changed, NULL, NULL, NULL);

	/* Drag'n drop stuff. */
//FIXME
//	gtk_drag_source_set (glade_xml_get_widget (xml, "clist_files"), GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, target_table, 1, GDK_ACTION_COPY);

	/* Start the event loop. */
#ifdef GNOCAM_USES_THREADS
	gdk_threads_enter ();
#endif
	gtk_main ();
#ifdef GNOCAM_USES_THREADS
	gdk_threads_leave ();
#endif

	/* Clean up. */
	gp_exit ();
	gconf_client_notify_remove (client, notify_id_cameras);
	gconf_client_notify_remove (client, notify_id_magnification);
	gconf_client_notify_remove (client, notify_id_interpolation);
	gtk_object_unref (GTK_OBJECT (client));
	return (0);
}

