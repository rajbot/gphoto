#include "config.h"

#include <libgnome/gnome-program.h>
#include <libgnomeui/gnome-ui-init.h>

#include "gnocam-applet.h"

static gboolean
factory_callback (PanelApplet *applet, const gchar *iid,
		  gpointer user_data)
{
	GnocamApplet *a;

	a = gnocam_applet_new (applet);

	return TRUE;
}

int
main (int argc, char *argv[])
{
	gnome_program_init ("gnocam-applet", VERSION, LIBGNOMEUI_MODULE,
			    argc, argv, NULL);

	return panel_applet_factory_main ("OAFIID:GNOME_GnocamApplet_Factory",
				PANEL_TYPE_APPLET, factory_callback, NULL);
}
