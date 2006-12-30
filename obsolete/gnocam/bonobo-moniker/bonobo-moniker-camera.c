#include <bonobo/bonobo-moniker-extender.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-moniker-simple.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-context.h>

#include "GnoCam.h"

static Bonobo_Unknown
camera_resolve (BonoboMoniker 		    *moniker, 
		const Bonobo_ResolveOptions *options, 
		const CORBA_char 	    *requested_interface, 
		CORBA_Environment 	    *ev)
{
	Bonobo_Storage_OpenMode mode, comp_mode = 0;
	Bonobo_Storage storage;
	Bonobo_Stream stream;
	CORBA_Object gnocam;
	CORBA_Environment tmp_ev;
	const gchar *name;
	gchar *camera_name;
	gint i;
	GNOME_Camera camera;

	/* Start gnocam */
	gnocam = oaf_activate_from_id ("OAFIID:GNOME_GnoCam", 0, NULL, ev);
	if (BONOBO_EX (ev))
		return (NULL);

	name = bonobo_moniker_get_name (moniker);
	if ((strlen (name) > 11) && !strncmp (name, "//previews@", 11))
		comp_mode = Bonobo_Storage_COMPRESSED;

	/* Make sure we are given a camera name. */
	for (i = 0; i < strlen (name); i++)
		if (name [i] == '@') {
			name += i + 1;
			break;
		}
	if ((name [0] == '/') && (name [1] == '/'))
		name += 2;
	for (i = 0; name [i] != 0; i++) 
		if (name [i] == '/') 
			break;
	camera_name = g_strndup (name, i);
	name += i;

	/* Get a camera */
	camera = GNOME_GnoCam_getCameraByName (gnocam, camera_name, ev);
	g_free (camera_name);
	bonobo_object_release_unref (gnocam, NULL);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	/* Get the storage */
	g_message ("Getting storage...");
	storage = Bonobo_Unknown_queryInterface (camera,
						 "IDL:Bonobo/Storage:1.0", ev);
	bonobo_object_release_unref (camera, NULL);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	/* Storage? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Storage:1.0")) {
		Bonobo_Storage sub_storage;

		g_message ("Trying to open storage for '%s'...", name);
		sub_storage = Bonobo_Storage_openStorage (storage, name,
							  Bonobo_Storage_READ |
							  Bonobo_Storage_WRITE,
							  ev);
		bonobo_object_release_unref (storage, NULL);
		if (BONOBO_EX (ev)) {
			g_message ("Failure: %s", 
				   bonobo_exception_get_text (ev));
			return (CORBA_OBJECT_NIL);
		}

		return (sub_storage);
	}

	/* Stream? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Stream:1.0")) {
		CORBA_exception_init (&tmp_ev);
	
		if (getenv ("DEBUG_GNOCAM"))
			g_message ("Trying to get stream (rw) for '%s'...",
				   name);
		mode = Bonobo_Storage_READ | Bonobo_Storage_WRITE | comp_mode;
		stream = Bonobo_Storage_openStream (storage, name,
						    mode, &tmp_ev);
		if (BONOBO_EX (&tmp_ev)) {
			g_warning ("Could not get stream: %s",
				   bonobo_exception_get_text (&tmp_ev));

			CORBA_exception_free (&tmp_ev);

			if (getenv ("DEBUG_GNOCAM"))
				g_message ("Trying to get stream (r)...");
			mode = Bonobo_Storage_READ | comp_mode;
			stream = Bonobo_Storage_openStream (storage, name,
							    mode, ev);
			if (BONOBO_EX (ev)) {
				g_warning ("Could not get stream: %s", 
					   bonobo_exception_get_text (ev));
				return CORBA_OBJECT_NIL;
			}
		}

		CORBA_exception_free (&tmp_ev);

		bonobo_object_release_unref (storage, NULL);
		return (stream);
	}

	bonobo_object_release_unref (storage, NULL);

	return (bonobo_moniker_use_extender (
		    		"OAFIID:Bonobo_MonikerExtender_stream", 
				moniker, options, requested_interface, ev));
}

static BonoboObject *
bonobo_moniker_camera_factory (BonoboGenericFactory *this, gpointer data)
{
	g_message ("Hello!");
	g_message ("This is the bonobo_moniker_camera_factory!");
	return BONOBO_OBJECT (bonobo_moniker_simple_new ("camera:", 
		    					 camera_resolve));
}

BONOBO_OAF_SHLIB_FACTORY ("OAFIID:Bonobo_Moniker_camera_Factory", "camera-moniker", bonobo_moniker_camera_factory, NULL)
