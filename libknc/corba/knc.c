#include <config.h>
#include "knc-c-mngr.h"

#include <libbonobo.h>

static BonoboObject *
knc_factory (BonoboGenericFactory *this_factory,
	     const char *iid, gpointer user_data)
{
	return g_object_new (KNC_C_TYPE_MNGR, NULL);
}

BONOBO_ACTIVATION_FACTORY ("OAFIID:GNOME_C_Knc_Factory",
	"Konica camera factory", "1.0", knc_factory, NULL);
