#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "preferences.h"
#include "gnocam.h"

/* Static Variables */

static GladeXML *xml;

/* Functions */

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

int main (int argc, char *argv[]) 
{
	gint debug_level;
	static GtkTargetEntry target_table[] = { 
		{"text/uri-list", 0, 0}
	};
	GError *gerror = NULL;
	GConfClient *client = NULL;

	/* Init GNOME. */
	gnome_init (PACKAGE, VERSION, argc, argv);

	/* Init glade. */
	glade_gnome_init ();

	/* Init gphoto2 backend. */
        gnome_config_push_prefix ("/" PACKAGE "/Other/");
        debug_level = gnome_config_get_int ("debug level");
	gnome_config_pop_prefix ();
        gp_init (debug_level);
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, gp_frontend_confirm, gp_frontend_prompt);

	/* Init gnome-vfs. */
	if (!gnome_vfs_init ()) {
		gnome_error_dialog (_("Could not initialize gnome-vfs!"));
		return (1);
	}

	/* Init gconf. */
	if (!gconf_init (argc, argv, &gerror)) {
		gnome_error_dialog (g_strdup_printf (_("Could not initialize gconf!\n\n%s"), gerror->message));
		return (1);
	}
	client = gconf_client_get_default ();
	gconf_client_add_dir (client, "/apps/GnoCam", GCONF_CLIENT_PRELOAD_NONE, NULL);

	/* Load the interface. */
	xml = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app");
	if (xml == NULL) g_error (_("Could not find " GNOCAM_GLADEDIR "gnocam.glade. Check if " PACKAGE " was installed correctly."));

	/* Get the preferences. */
	preferences_get (xml);

	/* connect the signals */
	glade_xml_signal_autoconnect (xml);

	/* Store some data. */
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "app")), "client", client);
	// Menu items:
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_previews")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_preview_as")), "xml", xml);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_files")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "save_file_as")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "delete")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "exit")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "preferences")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "about")), "xml", xml);
	// Camera tree
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "tree_cameras")), "xml", xml);
	// File list
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "clist_files")), "xml", xml);
	// Buttons:
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_files")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_file_as")), "xml", xml);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_previews")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_save_preview_as")), "xml", xml);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml, "button_delete")), "xml", xml);

	/* Drag'n drop stuff. */
	gtk_drag_source_set (
		glade_xml_get_widget (xml, "clist_files"), 
		GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, 
		target_table,
		1,
		GDK_ACTION_COPY);

	/* start the event loop */
	gtk_main ();

	/* Clean up. */
	gp_exit ();
	gtk_object_unref (GTK_OBJECT (client));
	return (0);
}
