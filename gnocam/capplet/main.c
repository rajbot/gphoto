#include "config.h"
#include "gnocam-capplet.h"
#include "i18n.h"

#include <gconf/gconf-client.h>
#include <gtk/gtkmain.h>
#include <libgnomeui/gnome-ui-init.h>

int
main (int argc, char** argv)
{
	GtkWidget *capplet;
	GConfClient *client;
	
	/* Internationalization */
	bindtextdomain (PACKAGE, GNOME_LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	gnome_program_init ("camera-capplet", VERSION, LIBGNOMEUI_MODULE, argc,
			    argv, NULL);

	/* Get the gconf-client */
	client = gconf_client_get_default ();
	gconf_client_add_dir (client, "/desktop/gnome/cameras",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);

	/* Create the capplet */
	capplet = gnocam_capplet_new (client);
	gtk_widget_show (capplet);
	
	gtk_main ();

	/* Clean up */
	g_object_unref (client);

	return 0;
}

