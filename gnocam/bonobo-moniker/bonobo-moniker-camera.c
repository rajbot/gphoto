#include <config.h>
#include <gphoto2.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-moniker-simple.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-context.h>
#include <stdlib.h>

#include "bonobo-moniker-camera.h"

#include "bonobo-storage-camera.h"
#include "bonobo-stream-camera.h"

static Bonobo_Unknown
camera_resolve (BonoboMoniker 		    *moniker, 
		const Bonobo_ResolveOptions *options, 
		const CORBA_char 	    *requested_interface, 
		CORBA_Environment 	    *ev)
{
	Bonobo_Storage_OpenMode mode, comp_mode = 0;
	CORBA_Environment tmp_ev;
	const gchar *name;

	name = bonobo_moniker_get_name (moniker);
	if ((strlen (name) > 11) && !strncmp (name, "//previews@", 11))
		comp_mode = Bonobo_Storage_COMPRESSED;
    
	/* Stream? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Stream:1.0")) {
		BonoboStream* stream;
		
		CORBA_exception_init (&tmp_ev);
	
		if (getenv ("DEBUG_GNOCAM"))
			g_message ("Trying to get stream (rw)...");
		mode = Bonobo_Storage_READ | Bonobo_Storage_WRITE | comp_mode;
		stream = bonobo_stream_camera_open (name, mode, 0644, &tmp_ev);
		if (BONOBO_EX (&tmp_ev)) {
			g_warning ("Could not get stream: %s",
				   bonobo_exception_get_text (&tmp_ev));

			CORBA_exception_free (&tmp_ev);

			if (getenv ("DEBUG_GNOCAM"))
				g_message ("Trying to get stream (r)...");
			mode = Bonobo_Storage_READ | comp_mode;
			stream = bonobo_stream_camera_open (name, mode,
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
		gchar *tmp;

		if (getenv ("DEBUG_GNOCAM"))
			g_message ("Trying to get storage for '%s'...", name);

		mode = Bonobo_Storage_READ | Bonobo_Storage_WRITE;
		tmp = g_strconcat ("camera:", name, NULL);
		storage = bonobo_storage_camera_open (tmp, mode, 0644, ev);
		g_free (tmp);
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
bonobo_moniker_camera_factory (BonoboGenericFactory *this, gpointer data)
{
	/* Initialize gphoto */
	gp_init (GP_DEBUG_NONE);

	return BONOBO_OBJECT (bonobo_moniker_simple_new ("camera:", 
		    					 camera_resolve));
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Moniker_camera_Factory", "camera-moniker", VERSION, bonobo_moniker_camera_factory, NULL)
