#include <config.h>
#include "GNOME_C.h"

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-ui-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-window.h>
#include <bonobo/bonobo-widget.h>

#include <gtk/gtkvbox.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkhseparator.h>

static void
do_ui_checks (GNOME_C_Camera c, CORBA_Environment *ev)
{
	GtkWidget *win, *p, *vbox, *s;
	Bonobo_Control prefs;
	Bonobo_UIContainer uic;
	Bonobo_Control capture;
	Bonobo_Control preview;

	prefs = GNOME_C_Camera__get_prefs (c, ev);
	if (BONOBO_EX (ev)) {
		g_warning ("Could not get preferences: %s",
			   bonobo_exception_get_text (ev));
	}
	
	win = bonobo_window_new ("Test", "Test");
	uic = BONOBO_OBJREF (bonobo_window_get_ui_container (
						BONOBO_WINDOW (win)));
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
	bonobo_window_set_contents (BONOBO_WINDOW (win), vbox);

	/* Preferences */
	prefs = GNOME_C_Camera__get_prefs (c, ev);
	if (BONOBO_EX (ev)) {
		g_warning ("Could not get preferences: %s",
			   bonobo_exception_get_text (ev)); 
	} else {
		p = bonobo_widget_new_control_from_objref (prefs, uic);
		gtk_widget_show (p);
		gtk_box_pack_start (GTK_BOX (vbox), p, FALSE, FALSE, 0);
		bonobo_object_release_unref (prefs, NULL);
	}

	gtk_widget_show (s = gtk_hseparator_new ());
	gtk_box_pack_start (GTK_BOX (vbox), s, FALSE, FALSE, 0);

	/* Capturing */
	capture = GNOME_C_Camera__get_capture (c, ev);
	if (BONOBO_EX (ev)) {
		g_warning ("Could not get capture: %s",
			   bonobo_exception_get_text (ev));
	} else {
		p = bonobo_widget_new_control_from_objref (capture, uic);
		gtk_widget_show (p);
		gtk_box_pack_start (GTK_BOX (vbox), p, FALSE, FALSE, 0);
		bonobo_object_release_unref (capture, NULL);
	}

	gtk_widget_show (s = gtk_hseparator_new ());
	gtk_box_pack_start (GTK_BOX (vbox), s, FALSE, FALSE, 0);

	/* Preview */
	preview = GNOME_C_Camera__get_preview (c, ev);
	if (BONOBO_EX (ev)) {
		g_warning ("Could not get preview: %s",
			   bonobo_exception_get_text (ev));
		return;
	} else {
		GNOME_C_Preview_start (preview, ev);
		if (BONOBO_EX (ev)) {
			bonobo_object_release_unref (preview, NULL);
			g_warning ("Could not start capturing previews: %s",
				   bonobo_exception_get_text (ev));
			return;
		}
		p = bonobo_widget_new_control_from_objref (preview, uic);
		gtk_widget_show (p);
		gtk_box_pack_start (GTK_BOX (vbox), p, FALSE, FALSE, 0);
		bonobo_object_release_unref (preview, NULL);
	}

	gtk_widget_show (win);
	g_signal_connect (win, "destroy", gtk_main_quit, NULL);

	bonobo_ui_main ();
}

int
main (int argc, char **argv)
{
	GNOME_C_Mngr m;
	CORBA_Environment ev;
	GNOME_C_Mngr_ManufacturerList *ml;
	GNOME_C_IDList *l;
	unsigned int i, j, k;
	CORBA_string s;
	GNOME_C_Camera c;
	GNOME_C_Dir d;
	GNOME_C_File f;

	if (!bonobo_ui_init (PACKAGE, VERSION, &argc, argv))
		g_error ("Failed to initialize Bonobo.");
	bonobo_activate ();

	m = bonobo_get_object ("OAFIID:GNOME_C_Knc", "IDL:GNOME/C/Mngr", NULL);
	if (m == CORBA_OBJECT_NIL)
		g_error ("Could not activate manager.");

	CORBA_exception_init (&ev);

	/* Count supported manufacturers */
	ml = GNOME_C_Mngr__get_devices (m, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (m, NULL);
		g_warning ("Could not get list of manufacturers: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}

	/* List them. */
	g_message ("Listing devices of %i manufacturers...", ml->_length);
	for (i = 0; i < ml->_length; i++) {
	    g_message ("%2i: '%s'", i, ml->_buffer[i].manufacturer);
	    for (j = 0; j < ml->_buffer[i].models._length; j++) {
		g_message ("  %2i: '%s'", j,
			   ml->_buffer[i].models._buffer[j].model);
		for (k = 0;
		     k < ml->_buffer[i].models._buffer[j].ports._length; k++)
			g_message ("    %2i: '%s'", k,
			    ml->_buffer[i].models._buffer[j].ports._buffer[k]);
	    }
	}
	CORBA_free (ml);

	/* Connect to camera */
	c = GNOME_C_Mngr_connect (m, "Hewlett Packard", "PhotoSmart C30",
				  "Serial Port 0", &ev);
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

	do_ui_checks (c, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("UI checks failed: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return 1;
	}

	d = GNOME_C_Camera__get_dir (c, &ev);
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
