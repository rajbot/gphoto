#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>

#include "gnocam-main.h"

#define WELCOME 			\
"It seems that you are running\n"	\
"GnoCam for the first time.\n"		\
"\n"					\
"GnoCam is a small but powerful\n"	\
"front-end to gphoto. Welcome to\n"	\
"the wonderful world of GPhoto!\n"	\
"Before you do anything else, you\n"	\
"should probably first open the\n"	\
"preferences dialog and add a\n"	\
"camera.\n"				\
"\n"					\
"Enjoy!"

static void
log_handler (const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
{
	GtkWindow*	window;
	
	window = GTK_WINDOW (user_data);

	if (log_level == G_LOG_LEVEL_CRITICAL) gnome_error_dialog_parented (message, window);
	else if (log_level == G_LOG_LEVEL_WARNING) gnome_warning_dialog_parented (message, window);
	else if ((log_level == G_LOG_LEVEL_INFO) || (log_level == G_LOG_LEVEL_MESSAGE)) gnome_ok_dialog_parented (message, window);
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

	/* Init GConf */
	if (!gconf_init (argc, argv, &gerror)) g_error ("Could not initialize gconf: %s", gerror->message);
	g_return_val_if_fail (client = gconf_client_get_default (), 1);

	/* Init GPhoto2 */
	if ((result = gp_init (GP_DEBUG_NONE)) != GP_OK) g_error (_("Could not initialize gphoto! (%s)"), gp_result_as_string (result));

	/* Create app */
	widget = gnocam_main_new (client);
	gtk_widget_show (widget);

	/* Redirect messages */
	g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO, log_handler, widget);
	
	/* Is this a first time run? */
	if (!gconf_client_get_string (client, "/apps/" PACKAGE "/prefix", NULL)) {
		
		/* Set prefix to HOME by default. */
		gconf_client_set_string (client, "/apps/" PACKAGE "/prefix", g_get_home_dir (), NULL);

		/* Set view mode to preview by default */
		gconf_client_set_bool (client, "/apps/" PACKAGE "/preview", TRUE, NULL);

		/* Popup a welcome message. */
		g_message (WELCOME);
	}

	/* Start the event loop. */
	bonobo_main ();

	/* Clean up. */
	if ((result = gp_exit ()) != GP_OK) g_error ("Could not exit gphoto2: %s", gp_result_as_string (result));
	gtk_object_unref (GTK_OBJECT (client));

	return (0);
}

