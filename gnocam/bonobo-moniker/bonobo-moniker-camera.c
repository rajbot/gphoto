#include <config.h>
#include <bonobo/bonobo-moniker-extender.h>

#include "bonobo-moniker-camera.h"

static Bonobo_Unknown
camera_resolve (BonoboMoniker 		    *moniker, 
		const Bonobo_ResolveOptions *options, 
		const CORBA_char 	    *requested_interface, 
		CORBA_Environment 	    *ev)
{
	CORBA_Environment tmp_ev;
	const gchar *name;

	name = bonobo_moniker_get_name (moniker);
    
	/* Stream? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Stream:1.0")) {
		BonoboStream* stream;
		
		CORBA_exception_init (&tmp_ev);
		
		stream = bonobo_stream_open_full ("camera", name, 
						  Bonobo_Storage_READ |
						  Bonobo_Storage_WRITE, 
						  0644, &tmp_ev);
		if (BONOBO_EX (&tmp_ev)) {
			g_warning ("Could not get stream: %s",
				   bonobo_exception_get_text (&tmp_ev));

			CORBA_exception_free (&tmp_ev);

			stream = bonobo_stream_open_full ("camera", name,
							  Bonobo_Storage_READ,
							  0644, ev);
			if (BONOBO_EX (ev)) {
				g_warning ("Could not get stream: %s", 
					   bonobo_exception_get_text (ev));
				return CORBA_OBJECT_NIL;
			}
		}

		CORBA_exception_free (&tmp_ev);

		return CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev);
	}

	/* Storage? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Storage:1.0")) {
		BonoboStorage* storage;

		g_message ("Opening storage...");
		storage = bonobo_storage_open_full ("camera", name, 
						    Bonobo_Storage_READ | 
						    Bonobo_Storage_WRITE, 
						    0644, ev);
		if (BONOBO_EX (ev)) {
			g_warning ("Could not get storage: %s", 
				   bonobo_exception_get_text (ev));
			return CORBA_OBJECT_NIL;
		}

		return CORBA_Object_duplicate (BONOBO_OBJREF (storage), ev);
	}

	return (bonobo_moniker_use_extender (
		    		"OAFIID:Bonobo_MonikerExtender_stream", 
				moniker, options, requested_interface, ev));
}

static BonoboObject *
bonobo_moniker_camera_factory (BonoboGenericFactory *this, void* data)
{
	return BONOBO_OBJECT (bonobo_moniker_simple_new ("camera:", 
		    					 camera_resolve));
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Moniker_camera_Factory", "camera-moniker", VERSION, bonobo_moniker_camera_factory, NULL)
