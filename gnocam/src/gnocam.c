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
#include "Gphoto.h"

/********************/
/* Global Variables */
/********************/

Bonobo_UIContainer	corba_container = CORBA_OBJECT_NIL;
BonoboUIComponent*      main_component	= NULL;
GtkTree*		main_tree 	= NULL;
GtkWindow*		main_window	= NULL;
gint			counter		= 0;
EPaned*			main_paned	= NULL;

/***************/
/* Prototypes. */
/***************/

void on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data);

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
        main_tree_update ();
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
	GConfClient*		client;
	GError*			gerror = NULL;
	GtkWidget*		widget;
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
	gint			result;
	Bonobo_Unknown		bag, property;
	CORBA_Environment	ev;
	CORBA_any*		value;

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
	g_return_val_if_fail (client = gconf_client_get_default (), 1);

	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Do we already have a debug level stored in our preferences? */
	bag = bonobo_get_object ("config:/apps/" PACKAGE, "IDL:Bonobo/PropertyBag:1.0", &ev);
	if (BONOBO_EX (&ev)) g_error (_("Could not get property bag for " PACKAGE "! (%s)"), bonobo_exception_get_text (&ev));
	property = Bonobo_PropertyBag_getPropertyByName (bag, "debug_level", &ev);
	if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level! (%s)"), bonobo_exception_get_text (&ev));
	if (property == CORBA_OBJECT_NIL) {
                Bonobo_Unknown  property;
		BonoboArg*	arg;

                property = bonobo_get_object ("config:/apps/" PACKAGE "/debug_level", "IDL:Bonobo/Property:1.0", &ev);
                if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level'! (%s)"), bonobo_exception_get_text (&ev));
                arg = bonobo_arg_new (TC_GNOME_Gphoto_DebugLevel);
                BONOBO_ARG_SET_GENERAL (arg, GNOME_Gphoto_DEBUG_LEVEL_NONE, TC_GNOME_Gphoto_DebugLevel, int, NULL);
                Bonobo_Property_setValue (property, arg, &ev);
		bonobo_arg_release (arg);
                if (BONOBO_EX (&ev)) g_error (_("Could not set property 'debug_level'! (%s)"), bonobo_exception_get_text (&ev));
	}

	/* Init gphoto2 backend with debug level as stored in database.	*/ 
	property = Bonobo_PropertyBag_getPropertyByName (bag, "debug_level", &ev);
	if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level! (%s)"), bonobo_exception_get_text (&ev));
	g_assert (property != CORBA_OBJECT_NIL);
	value = Bonobo_Property_getValue (property, &ev);
	if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level'! (%s)"), bonobo_exception_get_text (&ev));
	//FIXME: How do I cast CORBA_any to the debug level???
	if ((result = gp_init (GP_DEBUG_NONE)) != GP_OK) {
		g_warning (_("Could not initialize gphoto! (%s)"), gp_result_as_string (result));
	}
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, gp_frontend_confirm, NULL);

	/* Create the window. */
	main_window = GTK_WINDOW (bonobo_window_new (PACKAGE, PACKAGE));
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

	if (!gconf_client_get_string (gconf_client_get_default (), "/apps/" PACKAGE "/prefix", NULL)) {
		gchar*			prefix;
		
		/* Set prefix to HOME by default. */
		prefix = g_strconcat ("file:", g_get_home_dir (), NULL);
		gconf_client_set_string (gconf_client_get_default (), "/apps/" PACKAGE "/prefix", prefix, NULL);
		g_free (prefix);

		/* Popup a welcome message. */
		g_assert ((glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "welcome_messagebox")));
	} 

        /* Populate the camera tree. */
	main_tree_update ();

	/* Show what we have done so far. */
	gtk_widget_show (GTK_WIDGET (main_window));

	/* Start the event loop. */
	bonobo_main ();

	/* Clean up the main window. */
	bonobo_object_unref (BONOBO_OBJECT (main_component));
	bonobo_object_unref (BONOBO_OBJECT (container));
	gtk_widget_destroy (GTK_WIDGET (main_window));

	/* Clean up (gphoto). */
	gp_exit ();

	/* Clean up (gconf). */
	gtk_object_unref (GTK_OBJECT (client));

	return (0);
}

