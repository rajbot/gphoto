#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include <gphoto2.h>
#include <gal/e-paned/e-hpaned.h>
#include "gnocam.h"
#include "cameras.h"
#include "frontend.h"
#include "preferences.h"
#include "file-operations.h"

/********************/
/* Global Variables */
/********************/

GConfClient*		gconf_client 	= NULL;
Bonobo_UIContainer	corba_container = CORBA_OBJECT_NIL;
BonoboUIComponent*      main_component	= NULL;
GtkTree*		main_tree 	= NULL;
GnoCamViewMode		view_mode 	= GNOCAM_VIEW_MODE_PREVIEW;
GtkWindow*		main_window	= NULL;
gint			counter		= 0;
EPaned*			main_paned	= NULL;

/***************/
/* Prototypes. */
/***************/

void on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data);

void on_view_mode_activate		(GtkWidget* widget, gpointer user_data);
void on_save_previews_activate 		(GtkWidget* widget, gpointer user_data);
void on_save_previews_as_activate 	(GtkWidget* widget, gpointer user_data);
void on_save_files_activate		(GtkWidget* widget, gpointer user_data);
void on_save_files_as_activate		(GtkWidget* widget, gpointer user_data);
void on_gnocam_delete_activate		(GtkWidget* widget, gpointer user_data);

/**************/
/* Callbacks. */
/**************/

void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
        main_tree_update (entry->value);
}

void
on_view_mode_activate (GtkWidget* widget, gpointer user_data)
{
	view_mode = GPOINTER_TO_INT (user_data);
}

void
on_about_activate (BonoboUIComponent* component, gpointer user_data, const gchar* path)
{
	g_return_if_fail (glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "about"));
}

void 
on_save_previews_activate (GtkWidget* widget, gpointer user_data)
{
        save_all_selected (main_tree, TRUE, FALSE);
}

void
on_save_previews_as_activate (GtkWidget* widget, gpointer user_data)
{
	save_all_selected (main_tree, TRUE, TRUE);
}

void
on_save_files_activate (GtkWidget* widget, gpointer user_data)
{
	save_all_selected (main_tree, FALSE, FALSE);
}

void 
on_save_files_as_activate (GtkWidget* widget, gpointer user_data)
{
	save_all_selected (main_tree, FALSE, TRUE);
}

void 
on_gnocam_delete_activate (GtkWidget* widget, gpointer user_data)
{
	gint 			i;
	
	for (i = 0; i < g_list_length (GTK_TREE_SELECTION (main_tree)); i++) delete (GTK_TREE_ITEM (g_list_nth_data (GTK_TREE_SELECTION (main_tree), i)));
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
	GtkWidget*		widget;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	GtkWidget*		scrolledwindow;
	BonoboUIContainer*      container;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_UNSAFE_VERB ("Exit", gtk_main_quit),
		BONOBO_UI_UNSAFE_VERB ("Preferences", preferences),
		BONOBO_UI_UNSAFE_VERB ("About", on_about_activate),
		BONOBO_UI_UNSAFE_VERB ("SavePreviews", on_save_previews_activate),
		BONOBO_UI_UNSAFE_VERB ("SavePreviewsAs", on_save_previews_as_activate),
		BONOBO_UI_UNSAFE_VERB ("SaveFiles", on_save_files_activate),
		BONOBO_UI_UNSAFE_VERB ("SaveFilesAs", on_save_files_as_activate),
		BONOBO_UI_UNSAFE_VERB ("Delete", on_gnocam_delete_activate),
		BONOBO_UI_VERB_END};

	/* Use translated strings. */
	bindtextdomain (PACKAGE, GNOME_LOCALEDIR);
	textdomain (PACKAGE);

	/* Init GNOME, glade, gnome-vfs, gconf. */
	gnome_init (PACKAGE, VERSION, argc, argv);
	oaf_init (argc, argv);
	g_return_val_if_fail (bonobo_init (CORBA_OBJECT_NIL, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL), 1);
	glade_gnome_init ();
	g_return_val_if_fail (gnome_vfs_init (), 1);
	if (!gconf_init (argc, argv, &gerror)) {
		gnome_error_dialog (g_strdup_printf (_("Could not initialize gconf!\n\n%s"), gerror->message));
		return (1);
	}
        g_return_val_if_fail (gconf_client = gconf_client_get_default (), 1);
	gconf_client_add_dir (gconf_client, "/apps/" PACKAGE "", GCONF_CLIENT_PRELOAD_NONE, NULL);

	/* Init gphoto2 backend with debug level as stored in database.	*/ 
	/* If there is no debug level stored, init with GP_DEBUG_NONE. 	*/
	if ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/debug_level", NULL))) {
		g_assert (value->type == GCONF_VALUE_INT);
		gp_init (gconf_value_get_int (value));
		gconf_value_free (value);
	} else gp_init (GP_DEBUG_NONE);
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, NULL, NULL);

	/* Create the window. */
	gtk_widget_show (GTK_WIDGET (main_window = GTK_WINDOW (bonobo_window_new (PACKAGE, PACKAGE))));
	gtk_signal_connect (GTK_OBJECT (main_window), "delete_event", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	/* Create the tree. */
	gtk_widget_show (GTK_WIDGET (main_paned = E_PANED (e_hpaned_new ())));
	gtk_widget_show (scrolledwindow = gtk_scrolled_window_new (NULL, NULL));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	e_paned_pack1 (main_paned, scrolledwindow, TRUE, TRUE);
	gtk_widget_show (widget = gtk_viewport_new (NULL, NULL));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), widget);
	gtk_widget_show (GTK_WIDGET (main_tree = GTK_TREE (gtk_tree_new ())));
	gtk_tree_set_selection_mode (GTK_TREE (main_tree), GTK_SELECTION_MULTIPLE);
	gtk_container_add (GTK_CONTAINER (widget), GTK_WIDGET (main_tree));
	bonobo_window_set_contents (BONOBO_WINDOW (main_window), GTK_WIDGET (main_paned));

	/* Create the component. */
	corba_container = bonobo_object_corba_objref (BONOBO_OBJECT (container = bonobo_ui_container_new ()));
        bonobo_ui_container_set_win (container, BONOBO_WINDOW (main_window));
        bonobo_ui_component_set_container (main_component = bonobo_ui_component_new (PACKAGE), corba_container);
        bonobo_ui_component_add_verb_list (main_component, verb);
        bonobo_ui_util_set_ui (main_component, GNOCAM_DATADIR, "gnocam-main.xml", PACKAGE);

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
	bonobo_ui_component_object_set (main_component, "/Toolbar/ViewMode", bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (widget))), NULL);

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
		main_tree_update (value);
		gconf_value_free (value);
	}

	/* Notifications. */
	notify_id_cameras = gconf_client_notify_add (gconf_client, "/apps/" PACKAGE "/cameras", on_camera_setup_changed, NULL, NULL, NULL);

	/* Start the event loop. */
	bonobo_main ();

	/* Clean up the main window. */
	bonobo_object_unref (BONOBO_OBJECT (main_component));
	bonobo_object_unref (BONOBO_OBJECT (container));
	gtk_widget_destroy (GTK_WIDGET (main_window));

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

