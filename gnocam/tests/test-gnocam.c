#include <liboaf/liboaf.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-event-source.h>
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
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	g_message ("Adding listener...");
	id = bonobo_event_source_client_add_listener (camera, listener_cb, 
				"GNOME/Camera:CaptureImage", &ev, NULL);
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

	g_message ("Capturing image...");
	GNOME_Camera_captureImage (camera, &ev);
	CHECK (ev);

	bonobo_main ();
	
	g_message ("Removing listener...");
	bonobo_event_source_client_remove_listener (camera, id, &ev);
	bonobo_object_release_unref (camera, NULL);
	CHECK (ev);

	CORBA_exception_free (&ev);

	return (0);
}
