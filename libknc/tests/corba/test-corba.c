#include <config.h>
#include "GNOME_C.h"

#include <libbonobo.h>

int
main (int argc, char **argv)
{
	GNOME_C_Mngr m;
	CORBA_Environment ev;
	GNOME_C_Mngr_DeviceList *dl;
	GNOME_C_IDList *l;
	unsigned int i, j;
	CORBA_string s;
	GNOME_C_Camera c;
	GNOME_C_Dir d;
	GNOME_C_File f;

	if (!bonobo_init (&argc, argv))
		g_error ("Failed to initialize Bonobo.");
	bonobo_activate ();

	m = bonobo_get_object ("OAFIID:GNOME_C_Knc", "IDL:GNOME/C/Mngr", NULL);
	if (m == CORBA_OBJECT_NIL)
		g_error ("Could not activate manager.");

	CORBA_exception_init (&ev);

	/* Count supported camera models */
	dl = GNOME_C_Mngr_get_devices (m, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (m, NULL);
		g_warning ("Could not get list of devices: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}

	/* List them. */
	g_message ("Listing %i supported devices...", dl->_length);
	for (i = 0; i < dl->_length; i++) {
		g_message ("%2i: '%s', '%s', %i port(s) supported", i,
			   dl->_buffer[i].manufacturer,
			   dl->_buffer[i].model, dl->_buffer[i].ports._length);
		for (j = 0; j < dl->_buffer[i].ports._length; j++) {
			s = GNOME_C_Mngr_get_port_name (m,
				dl->_buffer[i].ports._buffer[j], &ev);
			if (BONOBO_EX (&ev)) {
				CORBA_free (dl);
				bonobo_object_release_unref (m, NULL);
				g_warning ("Could not get name of port %i: %s",
					j + 1, bonobo_exception_get_text (&ev));
				CORBA_exception_free (&ev);
				return 1;
			}
			g_message ("      %2i: '%s'", j, s);
			CORBA_free (s);
		}
	}
	CORBA_free (dl);

	/* Connect to camera */
	c = GNOME_C_Mngr_connect_to_device_at_port (m, 0, 0, &ev);
	bonobo_object_release_unref (m, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not connect to camera: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}

	s = GNOME_C_Camera__get_model (c, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (c, NULL);
		g_warning ("Could not get model of camera: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}
	g_message ("Now connected to model '%s'.", s);
	CORBA_free (s);

	d = GNOME_C_Camera_get_dir (c, &ev);
	bonobo_object_release_unref (c, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get root directory: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}

	l = GNOME_C_Dir_get_files (d, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (d, NULL);
		g_warning ("Could not count files in root directory: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}
	g_message ("Found %i file(s) in the root directory.", l->_length);
	if (!l->_length) {
		bonobo_object_release_unref (d, NULL);
		CORBA_free (l);
		return 0;
	}

	f = GNOME_C_Dir_get_file (d, l->_buffer[0], &ev);
	CORBA_free (l);
	bonobo_object_release_unref (d, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get first file of root directory: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}

	l = GNOME_C_File_get_ifs (f, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (f, NULL);
		g_warning ("Could not count interfaces: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}
	g_message ("The file has %i interface(s).", l->_length);

	CORBA_exception_free (&ev);

	bonobo_object_release_unref (f, NULL);

	return 0;
}
