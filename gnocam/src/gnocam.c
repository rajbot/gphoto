#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>
#include <gphoto2.h>

#include <GnoCam.h>

#include "gnocam-main.h"

/***************/
/* Log-Handler */
/***************/

static void
log_handler (const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
{
	GtkWindow*	window;
	
	window = GTK_WINDOW (user_data);

	if ((log_level == G_LOG_LEVEL_ERROR) || (log_level == G_LOG_LEVEL_CRITICAL)) gnome_error_dialog_parented (message, window);
	else if (log_level == G_LOG_LEVEL_WARNING) gnome_warning_dialog_parented (message, window);
	else if ((log_level == G_LOG_LEVEL_INFO) || (log_level == G_LOG_LEVEL_MESSAGE)) gnome_ok_dialog_parented (message, window);
}

/*************/
/* Functions */
/*************/

static void
load_settings (GConfClient* client)
{
	CORBA_Environment	ev;
	Bonobo_Unknown          bag, property;
	CORBA_any*              value;
	gint			result;

        /* Init exception. */
        CORBA_exception_init (&ev);

        /* Do we already have a debug level stored in our preferences? */
        bag = bonobo_get_object ("config:/apps/" PACKAGE, "IDL:Bonobo/PropertyBag:1.0", &ev);
        if (BONOBO_EX (&ev)) g_error (_("Could not get property bag for " PACKAGE "! (%s)"), bonobo_exception_get_text (&ev));
        property = Bonobo_PropertyBag_getPropertyByName (bag, "debug_level", &ev);
        if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level! (%s)"), bonobo_exception_get_text (&ev));
        if (property == CORBA_OBJECT_NIL) {
                BonoboArg*      arg;

                property = bonobo_get_object ("config:/apps/" PACKAGE "/debug_level", "IDL:Bonobo/Property:1.0", &ev);
                if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level'! (%s)"), bonobo_exception_get_text (&ev));
                arg = bonobo_arg_new (TC_GNOME_GnoCam_DebugLevel);
                BONOBO_ARG_SET_GENERAL (arg, GNOME_GnoCam_DEBUG_LEVEL_NONE, TC_GNOME_GnoCam_DebugLevel, int, NULL);
                Bonobo_Property_setValue (property, arg, &ev);
                bonobo_arg_release (arg);
                if (BONOBO_EX (&ev)) g_error (_("Could not set property 'debug_level'! (%s)"), bonobo_exception_get_text (&ev));
        }

        /* Init gphoto2 backend with debug level as stored in database. */
        property = Bonobo_PropertyBag_getPropertyByName (bag, "debug_level", &ev);
        if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level! (%s)"), bonobo_exception_get_text (&ev));
        g_assert (property != CORBA_OBJECT_NIL);
        value = Bonobo_Property_getValue (property, &ev);
        if (BONOBO_EX (&ev)) g_error (_("Could not get property 'debug_level'! (%s)"), bonobo_exception_get_text (&ev));
        //FIXME: How do I cast CORBA_any to the debug level???
        if ((result = gp_init (GP_DEBUG_NONE)) != GP_OK) g_error (_("Could not initialize gphoto! (%s)"), gp_result_as_string (result));

        /* Do we already have a prefix stored in our preferences? */
        if (!gconf_client_get_string (client, "/apps/" PACKAGE "/prefix", NULL)) {

                /* Set prefix to HOME by default. */
                gconf_client_set_string (client, "/apps/" PACKAGE "/prefix", g_get_home_dir (), NULL);

                /* Popup a welcome message. */
                g_assert ((glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "welcome_messagebox")));
        }

	/* Free exception */
	CORBA_exception_free (&ev);
}

int main (int argc, char *argv[]) 
{
	GError*			gerror = NULL;
	gint			result;
	GConfClient*		client;
	GtkWidget*		widget;

	/* Use translated strings. */
	bindtextdomain (PACKAGE, GNOME_LOCALEDIR);
	textdomain (PACKAGE);

	/* Init GNOME */
	gnome_init (PACKAGE, VERSION, argc, argv);

	/* Init OAF */
	oaf_init (argc, argv);

	/* Init bonobo */
	g_return_val_if_fail (bonobo_init (CORBA_OBJECT_NIL, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL), 1);

	/* Init glade */
	glade_gnome_init ();

	/* Init gnome-vfs */
	g_return_val_if_fail (gnome_vfs_init (), 1);

	/* Init GConf */
	if (!gconf_init (argc, argv, &gerror)) g_error ("Could not initialize gconf: %s", gerror->message);
	g_return_val_if_fail (client = gconf_client_get_default (), 1);

	load_settings (client);

	/* Create app */
	widget = gnocam_main_new (client);
	gtk_widget_show (widget);

        /* Redirect messages */
	g_log_set_handler (G_LOG_DOMAIN, 
		G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO,
		log_handler, widget);

	/* Start the event loop. */
	bonobo_main ();

	/* Clean up. */
	if ((result = gp_exit ()) != GP_OK) g_error ("Could not exit gphoto2: %s", gp_result_as_string (result));
	gtk_object_unref (GTK_OBJECT (client));

	return (0);
}

