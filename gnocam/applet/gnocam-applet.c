#include <config.h>
#include <applet-widget.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkvbbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkpixmap.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <liboaf/liboaf.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-exception.h>

#include "GnoCam.h"

#if 0
static void
listener_cb (BonoboListener *listener, gchar *event_name, CORBA_any *any,
	     CORBA_Environment *ev, gpointer data)
{
	GNOME_Camera camera;
	gchar *msg;

	camera = data;

	if (!strcmp (event_name, "GNOME/Camera:CaptureImage:Action")) {
		msg = g_strdup_printf (_("Image is available at '%s'."),
				       BONOBO_ARG_GET_STRING (any));
		gnome_ok_dialog (msg);
		g_free (msg);
	}

	bonobo_object_release_unref (camera, NULL);
}
#endif

static gboolean
do_capture (gpointer data)
{
	CORBA_Environment ev;
	CORBA_Object gnocam;
	GNOME_Camera camera;

	CORBA_exception_init (&ev);
	gnocam = oaf_activate_from_id ("OAFIID:GNOME_GnoCam", 0, NULL, &ev);
	if (BONOBO_EX (&ev)) {
		gnome_error_dialog (_("Could not start GnoCam!"));
		CORBA_exception_free (&ev);
		return (FALSE);
	}

	camera = GNOME_GnoCam_getCamera (gnocam, &ev);
	bonobo_object_release_unref (gnocam, NULL);
	if (BONOBO_EX (&ev)) {
		gnome_error_dialog (_("Could not get camera!"));
		CORBA_exception_free (&ev);
		return (FALSE);
	}

#if 0
	bonobo_event_source_client_add_listener (camera, listener_cb, 
				"GNOME/Camera:CaptureImage", &ev, camera); 
	if (BONOBO_EX (&ev)) {
		gnome_error_dialog (_("Could not get event source!"));
		CORBA_exception_free (&ev);
		bonobo_object_release_unref (camera, NULL);
		return (FALSE);
	}
#endif

	GNOME_Camera_captureImage (camera, &ev);
	if (BONOBO_EX (&ev)) {
		gnome_error_dialog (_("Could not capture image!"));
		bonobo_object_release_unref (camera, NULL);
		CORBA_exception_free (&ev);
		return (FALSE);
	}

	return (FALSE);
}

static void
on_button_clicked (GtkButton *button)
{
	gtk_idle_add (do_capture, NULL);
}

int
main (int argc, char **argv)
{
	GtkWidget *applet, *bbox, *button, *image;
	GdkPixbuf *pixbuf;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	CORBA_ORB orb;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("gnocam_capplet", VERSION, argc, argv,
			    NULL, 0, NULL);

	orb = oaf_init (argc, argv);
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Can not initialize bonobo!");

	applet = applet_widget_new ("gnocam_capplet");
	if (!applet)
		g_error ("Cannot create GnoCam applet!");

	bbox = gtk_vbutton_box_new ();
	gtk_widget_show (bbox);
	applet_widget_add (APPLET_WIDGET (applet), bbox);

	button = gtk_button_new ();
	gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_button_clicked), NULL);

	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "/gnocam-small.xpm");
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 1);
	gdk_pixbuf_unref (pixbuf);

	image = gtk_pixmap_new (pixmap, bitmap);
	gtk_widget_show (image);
	gtk_container_add (GTK_CONTAINER (button), image);

	gtk_widget_show (applet);

	applet_widget_gtk_main ();

	return (0);
}
