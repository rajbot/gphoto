#include <liboaf/liboaf.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-listener.h>
#include <gtk/gtkmain.h>

#include "GnoCam.h"

#define CHECK(ev) {if (BONOBO_EX (&ev)) g_error (bonobo_exception_get_text (&ev));}

static Bonobo_EventSource_ListenerId id;
static GNOME_Camera camera;

static void
listener_cb (BonoboListener *listener, gchar *event_name, CORBA_any *any,
	     CORBA_Environment *ev, gpointer data)
{
	g_message ("Got event: %s", event_name);

	if (!strcmp (event_name, "GNOME/Camera:CaptureImage:Action")) {
		g_message ("An image has been captured and is available "
			   "at '%s'.", (gchar *) any->_value);
	} else if (!strcmp (event_name, "GNOME/Camera:CaptureImage:Cancel")) {
		g_message ("Capturing of image has been cancelled!");
	}

	gtk_main_quit ();
}

static gboolean
do_idle_tests (gpointer data)
{
	Bonobo_EventSource event_source;
	BonoboListener *listener;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	g_message ("Getting event source...");
	event_source = Bonobo_Unknown_queryInterface (camera,
			"IDL:Bonobo/EventSource:1.0", &ev);
	CHECK (ev);

	g_message ("Adding listener...");
	listener = bonobo_listener_new (listener_cb, NULL);
	id = Bonobo_EventSource_addListenerWithMask (event_source,
			BONOBO_OBJREF (listener),
			"GNOME/Camera:CaptureImage", &ev);
	bonobo_object_unref (BONOBO_OBJECT (listener));
	bonobo_object_release_unref (event_source, NULL);
	CHECK (ev);

	g_message ("Capturing image...");
	GNOME_Camera_captureImage (camera, &ev);
	CHECK (ev);

	CORBA_exception_free (&ev);

	g_message ("Back again in the idle loop.");
	return (FALSE);
}

int
main (int argc, char *argv[])
{
	CORBA_ORB orb;
	CORBA_Object gnocam;
	CORBA_Environment ev;
	Bonobo_Storage storage;
	Bonobo_EventSource event_source;

	gtk_init (&argc, &argv);

	orb = oaf_init (argc, argv);
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Can not initialize bonobo!");

	CORBA_exception_init (&ev);

	g_message ("Getting GnoCam...");
	gnocam = oaf_activate_from_id ("OAFIID:GNOME_GnoCam", 0, NULL, &ev);
	CHECK (ev);

	g_message ("Getting default Camera...");
	camera = GNOME_GnoCam_getCamera (gnocam, &ev);
	bonobo_object_release_unref (gnocam, NULL);
	CHECK (ev);

	g_message ("Getting storage...");
	storage = Bonobo_Unknown_queryInterface (camera,
						 "IDL:Bonobo/Storage:1.0", &ev);
	CHECK (ev)
	bonobo_object_release_unref (storage, NULL);

	gtk_idle_add (do_idle_tests, NULL);

	bonobo_main ();
	
	g_message ("Getting event source...");
	event_source = Bonobo_Unknown_queryInterface (camera,
			"IDL:Bonobo/EventSource:1.0", &ev);
	bonobo_object_release_unref (camera, NULL);
	CHECK (ev);

	g_message ("Removing listener...");
	Bonobo_EventSource_removeListener (event_source, id, &ev);
	bonobo_object_release_unref (event_source, NULL);
	CHECK (ev);

	CORBA_exception_free (&ev);

	return (0);
}
