#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include <gphoto2.h>
#include "gnocam.h"
#include "cameras.h"
#include "information.h"
#include "frontend.h"
#include "notification.h"
#include "gallery.h"
#include "preferences.h"
#include "preview.h"
#include "file-operations.h"

/***************/
/* Definitions */
/***************/

#define EOG_IMAGE_VIEWER_ID 		"OAFIID:eog_image_viewer:a30dc90b-a68f-4ef8-a257-d2f8ab7e6c9f"
#define NAUTILUS_IMAGE_VIEWER_ID	"OAFIID:nautilus-image-viewer:30686633-23d5-422b-83c6-4f1b06f8abcd"

/********************/
/* Global Variables */
/********************/

GConfClient*		gconf_client = NULL;
BonoboObjectClient*	viewer_client = NULL;
GtkTree*		main_tree = NULL;
GnoCamViewMode		view_mode = GNOCAM_VIEW_MODE_PREVIEW;
GList*			preview_list = NULL;
GladeXML*		xml_main = NULL;

/***************/
/* Prototypes. */
/***************/

void on_view_mode_activate		(GtkWidget* widget, gpointer user_data);

void on_save_previews_activate 		(GtkWidget* widget, gpointer user_data);
void on_save_previews_as_activate 	(GtkWidget* widget, gpointer user_data);
void on_save_files_activate		(GtkWidget* widget, gpointer user_data);
void on_save_files_as_activate		(GtkWidget* widget, gpointer user_data);
void on_delete_activate			(GtkWidget* widget, gpointer user_data);

/**************/
/* Callbacks. */
/**************/

void
on_view_mode_activate (GtkWidget* widget, gpointer user_data)
{
	view_mode = GPOINTER_TO_INT (user_data);
}

void
on_new_gallery_activate (GtkWidget* widget, gpointer user_data)
{
        gallery_new ();
}

void
on_exit_activate (GtkWidget* widget, gpointer user_data)
{
        gtk_main_quit ();
}

void
on_preferences_activate (GtkWidget* widget, gpointer user_data)
{
        preferences ();
}

void
on_about_activate (GtkWidget* widget, gpointer user_data)
{
        g_assert (glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "about") != NULL);
}

void
on_manual_activate (GtkWidget* widget, gpointer user_data)
{
        gchar*  manualfile;

        if ((manualfile = gnome_help_file_find_file ("gnocam", "index.html"))) {
                gchar* url = g_strconcat ("file:", manualfile, NULL);
                gnome_help_goto (NULL, url);
                g_free (url);
                g_free (manualfile);
        } else {
                dialog_information (
                        "Could not find the manual for " PACKAGE ". "
                        "Check if it has been installed correctly in "
                        "$PREFIX/share/gnome/help/gnocam.");
        }
}

void 
on_save_previews_activate (GtkWidget* widget, gpointer user_data)
{
        save_all_selected (main_tree, TRUE, FALSE, FALSE);
}

void
on_save_previews_as_activate (GtkWidget* widget, gpointer user_data)
{
	save_all_selected (main_tree, TRUE, TRUE, FALSE);
}

void
on_save_files_activate (GtkWidget* widget, gpointer user_data)
{
	save_all_selected (main_tree, FALSE, FALSE, FALSE);
}

void 
on_save_files_as_activate (GtkWidget* widget, gpointer user_data)
{
	save_all_selected (main_tree, FALSE, TRUE, FALSE);
}

void 
on_delete_activate (GtkWidget* widget, gpointer user_data)
{
	delete_all_selected (main_tree);
}

/*************/
/* Functions */
/*************/

int main (int argc, char *argv[]) 
{
	GError*			gerror = NULL;
	GConfValue*		value = NULL;
	guint 			notify_id_cameras;
	gchar*			prefix = NULL;
	GtkWidget*		window;
	GtkWidget*		widget;
	GtkWidget*      	viewer;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	gint			i;
	BonoboUIContainer*	container;
	BonoboUIComponent*	component;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_UNSAFE_VERB ("NewGallery", on_new_gallery_activate),
		BONOBO_UI_UNSAFE_VERB ("Exit", on_exit_activate),
		BONOBO_UI_UNSAFE_VERB ("Preferences", on_preferences_activate),
		BONOBO_UI_UNSAFE_VERB ("About", on_about_activate),
		BONOBO_UI_UNSAFE_VERB ("Manual", on_manual_activate),
		BONOBO_UI_UNSAFE_VERB ("SavePreviews", on_save_previews_activate),
		BONOBO_UI_UNSAFE_VERB ("SavePreviewsAs", on_save_previews_as_activate),
		BONOBO_UI_UNSAFE_VERB ("SaveFiles", on_save_files_activate),
		BONOBO_UI_UNSAFE_VERB ("SaveFilesAs", on_save_files_as_activate),
		BONOBO_UI_UNSAFE_VERB ("Delete", on_delete_activate),
		BONOBO_UI_VERB_END};

	/* Use translated strings. */
	bindtextdomain (PACKAGE, GNOME_LOCALEDIR);
	textdomain (PACKAGE);

	/* Init GNOME, glade, gnome-vfs, gconf. */
	gnome_init (PACKAGE, VERSION, argc, argv);
	oaf_init (argc, argv);
	g_assert (bonobo_init (CORBA_OBJECT_NIL, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL));
	glade_gnome_init ();
	if (!gnome_vfs_init ()) {
		gnome_error_dialog (_("Could not initialize gnome-vfs!"));
		return (1);
	}
	if (!gconf_init (argc, argv, &gerror)) {
		gnome_error_dialog (g_strdup_printf (_("Could not initialize gconf!\n\n%s"), gerror->message));
		return (1);
	}
        gconf_client = gconf_client_get_default ();
	gconf_client_add_dir (gconf_client, "/apps/" PACKAGE "", GCONF_CLIENT_PRELOAD_NONE, NULL);

	/* Init gphoto2 backend with debug level as stored in database.	*/ 
	/* If there is no debug level stored, init with GP_DEBUG_NONE. 	*/
	if ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/debug_level", NULL))) {
		g_assert (value->type == GCONF_VALUE_INT);
		gp_init (gconf_value_get_int (value));
		gconf_value_free (value);
	} else gp_init (GP_DEBUG_NONE);
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, gp_frontend_confirm, gp_frontend_prompt);

	/* Create the window. We cannot do it with libglade as bonobo-support in libglade misses some features like toolbars and menus. */
	gtk_widget_show (window = bonobo_window_new (PACKAGE, PACKAGE));
	g_assert ((xml_main = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "main_vbox")));
	gtk_widget_show (widget = glade_xml_get_widget (xml_main, "main_vbox"));
	bonobo_window_set_contents (BONOBO_WINDOW (window), widget);
        container = bonobo_ui_container_new ();
        bonobo_ui_container_set_win (container, BONOBO_WINDOW (window));
        component = bonobo_ui_component_new (PACKAGE);
        bonobo_ui_component_set_container (component, bonobo_object_corba_objref (BONOBO_OBJECT (container)));
        bonobo_ui_component_add_verb_list (component, verb);
        bonobo_ui_util_set_ui (component, "", "gnocam-main.xml", PACKAGE);

	/* Add the view mode selection to the toolbar. */
	gtk_widget_show (widget = gtk_option_menu_new ());
	gtk_widget_show (menu = gtk_menu_new ());
	gtk_widget_show (menu_item = gtk_menu_item_new_with_label (_("None")));
	gtk_menu_append (GTK_MENU (menu), menu_item);
	gtk_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (on_view_mode_activate), GINT_TO_POINTER (GNOCAM_VIEW_MODE_NONE));
	gtk_widget_show (menu_item = gtk_menu_item_new_with_label (_("Preview")));
	gtk_menu_append (GTK_MENU (menu), menu_item);
	gtk_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (on_view_mode_activate), GINT_TO_POINTER (GNOCAM_VIEW_MODE_PREVIEW));
	gtk_widget_show (menu_item = gtk_menu_item_new_with_label (_("File")));
	gtk_menu_append (GTK_MENU (menu), menu_item);
	gtk_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (on_view_mode_activate), GINT_TO_POINTER (GNOCAM_VIEW_MODE_FILE));
	gtk_option_menu_set_menu (GTK_OPTION_MENU (widget), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (widget), 1);
	bonobo_ui_component_object_set (component, "/Toolbar/ViewMode", bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (widget))), NULL);

	/* Create the viewer. */
	if ((viewer = bonobo_widget_new_control (EOG_IMAGE_VIEWER_ID, bonobo_object_corba_objref (BONOBO_OBJECT (container))))) {
		gtk_widget_show (viewer);
		gtk_paned_pack2 (GTK_PANED (glade_xml_get_widget (xml_main, "main_hpaned")), viewer, TRUE, TRUE);
		viewer_client = bonobo_widget_get_server (BONOBO_WIDGET (viewer));
	} else dialog_information (_("Could not start the eog image viewer!"));

	/* Set the global variables. */
	main_tree = GTK_TREE (glade_xml_get_widget (xml_main, "main_tree"));

	/* Connect the signals. */
	glade_xml_signal_autoconnect (xml_main);

	/* Do we already have a prefix in the database? */
	if (!(value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL))) {
		
		/* Set prefix to HOME by default. */
		value = gconf_value_new (GCONF_VALUE_STRING);
		prefix = g_strdup_printf ("file:%s", g_get_home_dir ());
		gconf_value_set_string (value, prefix);
		gconf_client_set (gconf_client, "/apps/" PACKAGE "/prefix", value, NULL);
		g_free (prefix);

		/* Popup a welcome message. */
		g_assert ((glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "welcome_messagebox")));
	} 
	gconf_value_free (value);

        /* Populate the camera tree. */
	if ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/cameras", NULL))) {
		camera_tree_update (main_tree, value);
		gconf_value_free (value);
	}

	/* Notifications. */
	notify_id_cameras = gconf_client_notify_add (gconf_client, "/apps/" PACKAGE "/cameras", on_camera_setup_changed, NULL, NULL, NULL);

	/* Start the event loop. */
	bonobo_main ();

	/* Clean up the previews. */
	for (i = g_list_length (preview_list) - 1; i >= 0; i--) preview_free (g_list_nth_data (preview_list, i));

	/* Clean up the main window. */
        for (i = g_list_length (main_tree->children) - 1; i >= 0; i--) camera_tree_item_remove (g_list_nth_data (main_tree->children, i));
	bonobo_object_unref (BONOBO_OBJECT (component));
	bonobo_object_unref (BONOBO_OBJECT (container));
	gtk_widget_destroy (window);

	/* Clean up (gphoto). */
	gp_exit ();

	/* Clean up (gconf). */
	gconf_client_notify_remove (gconf_client, notify_id_cameras);
	gerror = NULL;
	gconf_client_suggest_sync (gconf_client, &gerror);
	if (gerror) g_warning ("GConf Error: %s", gerror->message);
	gtk_object_unref (GTK_OBJECT (gconf_client));

	return (0);
}

