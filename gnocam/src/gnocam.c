#include <config.h>
#include <gphoto2.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include <gphoto2.h>

#include <gal/e-paned/e-hpaned.h>

#include "cameras.h"
#include "frontend.h"
#include "preferences.h"
#include "file-operations.h"
#include "gnocam-shortcut-bar.h"
#include "GnoCam.h"

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

void on_shortcut_bar_item_selected (EShortcutBar* shortcut_bar, GdkEvent* event, gint group_num, gint item_num);

/**************/
/* Callbacks. */
/**************/

void
on_shortcut_bar_item_selected (EShortcutBar* shortcut_bar, GdkEvent* event, gint group_num, gint item_num)
{
	gchar* 		url;
	gchar* 		name;
	GtkWidget*	widget;

	/* If there is an old viewer, destroy it. */
	if (main_paned->child2) gtk_container_remove (GTK_CONTAINER (main_paned), main_paned->child2);

	/* Get information about the item. */
	e_shortcut_model_get_item_info (shortcut_bar->model, group_num, item_num, &url, &name);

	/* Get widget */
        if (!(widget = bonobo_widget_new_control (url, corba_container))) {
                g_warning (_("Could not get widget for '%s'!"), url);
        } else {
                gtk_widget_show (widget);
                e_paned_pack2 (main_paned, widget, TRUE, TRUE);
        }
}															

void
on_about_activate (BonoboUIComponent* component, gpointer user_data, const gchar* path)
{
	g_return_if_fail (glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "about"));
}

/*************/
/* Functions */
/*************/

int main (int argc, char *argv[]) 
{
	GConfClient*		client;
	GError*			gerror = NULL;
	GtkWidget*		shortcut_bar;
	BonoboUIContainer*      container;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_UNSAFE_VERB ("Exit", gtk_main_quit),
		BONOBO_UI_UNSAFE_VERB ("Preferences", preferences),
		BONOBO_UI_UNSAFE_VERB ("About", on_about_activate),
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
                arg = bonobo_arg_new (TC_GNOME_GnoCam_DebugLevel);
                BONOBO_ARG_SET_GENERAL (arg, GNOME_GnoCam_DEBUG_LEVEL_NONE, TC_GNOME_GnoCam_DebugLevel, int, NULL);
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

	/* Create the window and hpaned. */
	main_window = GTK_WINDOW (bonobo_window_new (PACKAGE, PACKAGE));
	gtk_window_set_default_size (main_window, 705, 550);
	gtk_signal_connect (GTK_OBJECT (main_window), "delete_event", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
	gtk_widget_show (GTK_WIDGET (main_paned = E_PANED (e_hpaned_new ())));
	bonobo_window_set_contents (BONOBO_WINDOW (main_window), GTK_WIDGET (main_paned));

	/* Create the shortcut bar. */
	shortcut_bar = gnocam_shortcut_bar_new ();
	gnocam_shortcut_bar_refresh (GNOCAM_SHORTCUT_BAR (shortcut_bar));
	gtk_widget_show (shortcut_bar);
	e_paned_add1 (E_PANED (main_paned), shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (shortcut_bar), "item_selected", (GtkSignalFunc) on_shortcut_bar_item_selected, NULL);

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

	/* Show what we have done so far. */
	gtk_widget_show (GTK_WIDGET (main_window));

	/* Start the event loop. */
	bonobo_main ();

	/* Clean up the main window. */
	bonobo_object_unref (BONOBO_OBJECT (main_component));
	bonobo_object_unref (BONOBO_OBJECT (container));

	/* Clean up (gphoto). */
	gp_exit ();

	/* Clean up (gconf). */
	gtk_object_unref (GTK_OBJECT (client));

	return (0);
}

