#include <liboaf/liboaf.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>

#include "GnoCam.h"

int main (int argc, char *argv[])
{
	CORBA_ORB orb;
	CORBA_Object gnocam;
	GNOME_Camera camera;
	CORBA_Environment ev;
	Bonobo_Storage storage;
	Bonobo_Stream stream;

	gtk_type_init ();

	orb = oaf_init (argc, argv);
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Can not initialize bonobo!");

	CORBA_exception_init (&ev);

	g_message ("Getting GnoCam...");
	gnocam = oaf_activate_from_id ("OAFIID:GNOME_GnoCam", 0, NULL, &ev);
	if (BONOBO_EX (&ev))
		g_error (bonobo_exception_get_text (&ev));
	g_assert (gnocam);

	g_message ("Getting Camera...");
	camera = GNOME_GnoCam_getCamera (gnocam, "directory", &ev);
	bonobo_object_release_unref (gnocam, NULL);
	if (BONOBO_EX (&ev))
		g_error (bonobo_exception_get_text (&ev));
	g_assert (camera);

	g_message ("Getting storage...");
	storage = Bonobo_Unknown_queryInterface (camera,
						 "IDL:Bonobo/Storage:1.0", &ev);
	bonobo_object_release_unref (camera, NULL);
	if (BONOBO_EX (&ev))
		g_error (bonobo_exception_get_text (&ev));

	g_message ("Getting stream...");
	stream = Bonobo_Storage_openStream (storage, "/home/lutz/Beispiel.png",
					    Bonobo_Storage_READ, &ev);
	bonobo_object_release_unref (storage, NULL);
	if (BONOBO_EX (&ev))
		g_error (bonobo_exception_get_text (&ev));

	bonobo_object_release_unref (stream, NULL);

	CORBA_exception_free (&ev);

	return (0);
}
