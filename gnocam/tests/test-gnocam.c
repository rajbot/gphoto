#include <liboaf/liboaf.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-event-source.h>
#include <gtk/gtkmain.h>

#include "GnoCam.h"

#define CHECK(ev) {if (BONOBO_EX (ev)) g_error (bonobo_exception_get_text (ev));}

static Bonobo_EventSource_ListenerId id;
static GNOME_Camera camera;

static void
listener_cb (BonoboListener *listener, gchar *event_name, CORBA_any *any,
	     CORBA_Environment *ev, gpointer data)
{
	GNOME_Camera camera;
	Bonobo_Storage storage;
	Bonobo_Stream stream;
	Bonobo_StorageInfo *info;

	g_message ("Got event: %s", event_name);

	camera = data;

	if (!strcmp (event_name, "GNOME/Camera:CaptureImage:Action")) {
		g_message ("Getting storage...");
		storage = Bonobo_Unknown_queryInterface (camera,
						"IDL:Bonobo/Storage:1.0", ev);
		CHECK (ev);

		g_message ("Getting stream for '%s' (preview only)...",
			   BONOBO_ARG_GET_STRING (any));
		stream = Bonobo_Storage_openStream (storage,
					BONOBO_ARG_GET_STRING (any),
					Bonobo_Storage_READ |
					Bonobo_Storage_COMPRESSED, ev);
		bonobo_object_release_unref (storage, NULL);
		CHECK (ev);
		
		g_message ("Getting info...");
		info = Bonobo_Stream_getInfo (stream,
					      Bonobo_FIELD_CONTENT_TYPE |
					      Bonobo_FIELD_SIZE |
					      Bonobo_FIELD_TYPE, ev);
		g_message ("Unrefing stream...");
		bonobo_object_release_unref (stream, NULL);
		CHECK (ev);

		g_message ("Information about the stream:");
		g_message ("Name: %s", info->name);
		g_message ("Type: %i", (gint) info->type);
		g_message ("Content type: %s", (gchar *) info->content_type);
		g_message ("Size: %i", (gint) info->size);
		CORBA_free (info);

	} else if (!strcmp (event_name, "GNOME/Camera:CaptureImage:Cancel")) {
		g_message ("Capturing cancelled!");
	}

	bonobo_event_source_client_remove_listener (camera, id, NULL);
	bonobo_object_release_unref (camera, NULL);

	gtk_main_quit ();
}

static gboolean
do_idle_tests (gpointer data)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	g_message ("Adding listener...");
	id = bonobo_event_source_client_add_listener (camera, listener_cb, 
				"GNOME/Camera:CaptureImage", &ev, data);
	CHECK (&ev);

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
	GNOME_GnoCam_CameraList *list;
	int i;

	gtk_init (&argc, &argv);

	orb = oaf_init (argc, argv);
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Can not initialize bonobo!");

	CORBA_exception_init (&ev);

	g_message ("Getting GnoCam...");
	gnocam = oaf_activate_from_id ("OAFIID:GNOME_GnoCam", 0, NULL, &ev);
	CHECK (&ev);

	g_message ("Getting list of cameras...");
	list = GNOME_GnoCam_getCameraList (gnocam, &ev);
	CHECK (&ev);

	g_message ("... done. Got the following camera(s):");
	for (i = 0; i < list->_length; i++)
		g_message ("  %2i: '%s'", i, list->_buffer[i]);
	CORBA_free (list);

	g_message ("Getting default Camera...");
	camera = GNOME_GnoCam_getCamera (gnocam, &ev);
	bonobo_object_release_unref (gnocam, NULL);
	CHECK (&ev);

	g_message ("Getting storage...");
	storage = Bonobo_Unknown_queryInterface (camera,
						 "IDL:Bonobo/Storage:1.0", &ev);
	CHECK (&ev)
	bonobo_object_release_unref (storage, NULL);

	gtk_idle_add (do_idle_tests, camera);

	g_message ("Capturing image...");
	GNOME_Camera_captureImage (camera, &ev);
	CHECK (&ev);

	CORBA_exception_free (&ev);

	bonobo_main ();
	
	return (0);
}
