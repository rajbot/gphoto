#include <config.h>
#include "knc-c-mngr.h"

#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-ui-main.h>
#include <bonobo/bonobo-main.h>

static BonoboObject *
knc_factory (BonoboGenericFactory *this_factory,
	     const char *iid, gpointer user_data)
{
	if (!bonobo_ui_is_initialized () &&
	    !bonobo_ui_init (NULL, NULL, NULL, NULL))
		g_warning ("Could not initialize libbonoboui!");
	return g_object_new (KNC_C_TYPE_MNGR, NULL);
}

int
main (int argc, char **argv)
{
	if (!bonobo_ui_init (PACKAGE, VERSION, &argc, argv))
		g_error ("Could not initialize libbonoboui!");

	return bonobo_generic_factory_main ("OAFIID:GNOME_C_Knc_Factory",
		knc_factory, NULL);
}
