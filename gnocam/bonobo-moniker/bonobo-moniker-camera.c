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

#include "bonobo-moniker-camera.h"

static Bonobo_Unknown
camera_resolve (BonoboMoniker *moniker,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char *requested_interface,
	      CORBA_Environment *ev)
{
	const char *url = bonobo_moniker_get_name (moniker);
	char *real_url;

	g_warning ("Going to resolve the camera now");

	/* because resolving the moniker drops the "camera:" */
	real_url = g_strconcat ("camera:", url, NULL);

	if (strcmp (requested_interface, "IDL:Bonobo/Control:1.0") == 0) {
		Bonobo_Unknown object;
		BonoboStream *stream;
		BonoboStream *stream_destination;
		Bonobo_Stream_iobuf* buffer;

		/* Download the file. */
		g_print ("Downloading...\n");
		if (!(stream = bonobo_stream_open_full ("camera", real_url, Bonobo_Storage_READ | Bonobo_Storage_COMPRESSED, 0644, ev))) {
			g_warning ("Could not open stream!");
			g_free (real_url);
			return CORBA_OBJECT_NIL;
		}

		g_print ("Saving...\n");
		if (!(stream_destination = bonobo_stream_open_full ("fs", "/tmp/x.jpg", Bonobo_Storage_WRITE | Bonobo_Storage_CREATE, 0664, ev))) {
			g_warning ("Could not open destination stream!");
			g_free (real_url);
			bonobo_object_unref (BONOBO_OBJECT (stream));
			return CORBA_OBJECT_NIL;
		}
		Bonobo_Stream_read (bonobo_stream_corba_object_create (BONOBO_OBJECT (stream)), 4000000, &buffer, ev);
		g_return_val_if_fail (ev->_major == CORBA_NO_EXCEPTION, CORBA_OBJECT_NIL);
		Bonobo_Stream_write (bonobo_stream_corba_object_create (BONOBO_OBJECT (stream_destination)), buffer, ev);
		g_return_val_if_fail (ev->_major == CORBA_NO_EXCEPTION, CORBA_OBJECT_NIL);
		bonobo_object_unref (BONOBO_OBJECT (stream_destination));
		bonobo_object_unref (BONOBO_OBJECT (stream));

		g_print ("Getting object...\n");
		object = bonobo_get_object ("file:/tmp/x.jpg", "IDL:Bonobo/Control:1.0", ev);
		g_return_val_if_fail (ev->_major == CORBA_NO_EXCEPTION, CORBA_OBJECT_NIL);
			
		if  (object == CORBA_OBJECT_NIL) {
			g_warning ("Can't find object satisfying requirements");
			CORBA_exception_set  (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_Moniker_InterfaceNotFound, NULL);
			return CORBA_OBJECT_NIL;
		}

		g_print ("Returning...\n");
		return bonobo_moniker_util_qi_return (
			object, requested_interface, ev);
	}
	else if (strcmp (requested_interface, "IDL:Bonobo/Stream:1.0") == 0) {
		BonoboStream *stream;

		g_print ("REQUESTED INTERFACE: IDL:Bonobo/Stream:1.0\n");

		stream = bonobo_stream_open_full (
			"camera", real_url, Bonobo_Storage_READ, 0644, ev);

		if (!stream) {
			g_warning ("Failed to open stream '%s'", real_url);
			g_free (real_url);
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_Moniker_InterfaceNotFound, NULL);

			return CORBA_OBJECT_NIL;
		}

		g_print ("GOT REQUESTED INTERFACE\n");

		g_free (real_url);
		return CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev);
	}

	return CORBA_OBJECT_NIL;
}

static BonoboObject *
bonobo_moniker_camera_factory (BonoboGenericFactory *this, void *closure)
{
	return BONOBO_OBJECT (bonobo_moniker_simple_new (
		"camera:", camera_resolve));
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Moniker_camera_Factory",
		    "camera-moniker", VERSION,
		    bonobo_moniker_camera_factory,
		    NULL)
