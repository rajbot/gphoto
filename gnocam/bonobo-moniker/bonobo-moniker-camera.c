/*
 * bonobo-moniker-camera.c: CAMERA based Moniker
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 *
 * Copyright (c) 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <gphoto2.h>

#include "gnocam-control.h"
#include "bonobo-moniker-camera.h"

static Bonobo_Unknown
camera_resolve (BonoboMoniker *moniker, const Bonobo_ResolveOptions *options, const CORBA_char *requested_interface, CORBA_Environment *ev)
{
	/* Stream? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Stream:1.0")) {
		BonoboStream *stream;

		stream = bonobo_stream_open_full ("camera", bonobo_moniker_get_name (moniker), Bonobo_Storage_READ | Bonobo_Storage_WRITE, 0644, ev);
		if (BONOBO_EX (ev)) stream = bonobo_stream_open_full ("camera", bonobo_moniker_get_name (moniker), Bonobo_Storage_READ, 0644, ev);
		if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;
		g_return_val_if_fail (stream != NULL, CORBA_OBJECT_NIL);

		return CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev);
	}

	/* Storage? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Storage:1.0")) {
		BonoboStorage *storage;

		storage = bonobo_storage_open_full ("camera", bonobo_moniker_get_name (moniker), Bonobo_Storage_READ | Bonobo_Storage_WRITE, 0644, ev);
		if (BONOBO_EX (ev)) storage = bonobo_storage_open_full ("camera", bonobo_moniker_get_name (moniker), Bonobo_Storage_READ, 0644, ev);
		if (BONOBO_EX (ev)) return CORBA_OBJECT_NIL;

		return CORBA_Object_duplicate (BONOBO_OBJREF (storage), ev);
	}

	/* Control? */
	if (!strcmp (requested_interface, "IDL:Bonobo/Control:1.0")) {
		return (CORBA_Object_duplicate (BONOBO_OBJREF (gnocam_control_new (moniker, options)), ev));
	}

	return bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, requested_interface, ev);
}

static BonoboObject *
bonobo_moniker_camera_factory (BonoboGenericFactory *this, void* data)
{
	gint result = gp_init (GP_DEBUG_NONE);
	
	if (result != GP_OK) g_warning (_("Could not initialize gphoto2! (%s)"), gp_result_as_string (result));
	
	return BONOBO_OBJECT (bonobo_moniker_simple_new ("camera:", camera_resolve));
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Moniker_camera_Factory", "camera-moniker", VERSION, bonobo_moniker_camera_factory, NULL)
