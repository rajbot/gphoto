#include <config.h>

#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-generic-factory.h>

#include "gnocam-main.h"

static BonoboObject *
gnocam_factory (BonoboGenericFactory *this, gpointer data)
{
	GnoCamMain *gnocam_main;

	g_message ("Trying to create a new GnoCamMain...");

	gnocam_main = gnocam_main_new ();
	g_assert (gnocam_main);
	
	return (BONOBO_OBJECT (gnocam_main));
}

BONOBO_OAF_FACTORY ("OAFIID:GNOME_GnoCam_Factory", "gnocam", VERSION, gnocam_factory, NULL)
