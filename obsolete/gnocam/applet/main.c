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

PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_GnocamApplet_Factory",
	PANEL_TYPE_APPLET, "gnocam-applet", "0", factory_callback, NULL);
