#include <config.h>
#include "gnocam-applet.h"

#include <stdlib.h>
#include <string.h>

#include <libgnomeui/gnome-about.h>
#include <libgnomeui/gnome-window-icon.h>

#include <gtk/gtkhbbox.h>
#include <gtk/gtkbutton.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define PARENT_TYPE PANEL_TYPE_APPLET
static PanelAppletClass *parent_class;

static void
gnocam_applet_finalize (GObject *object)
{
	GnoCamApplet *a = GNOCAM_APPLET (object);

	g_free (a->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_applet_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_applet_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gnocam_applet_properties_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			     const char verbname)
{
	g_message ("Implement!");
}

static void
gnocam_applet_about_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			const char *verbname)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf;
	GError *e = NULL;

	static const gchar *authors[] = {
		"Lutz Müller <lutz@users.sourceforge.net>",
		NULL
	};
	const gchar *documenters[] = {NULL};
	const gchar *translator_credits = _("translator_credits");

	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "gnocam-camera1.png", &e);
	about = gnome_about_new (
		_("Camera Applet"), VERSION,
		_("Copyright (c) 2002 Lutz Müller"),
		_("Access your digital camera."),
		authors, documenters,
		(strcmp (translator_credits, "translator_credits") ?
	 				translator_credits : NULL), pixbuf);
	if (pixbuf)
		g_object_unref (pixbuf);
	gtk_window_set_wmclass (GTK_WINDOW (about), "gnocam-applet", 
				"Camera Applet");
	g_signal_connect (about, "destroy", G_CALLBACK (gtk_widget_destroyed),
			  &about);
	gtk_widget_show (about);
}

static const BonoboUIVerb gnocam_applet_menu_verbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Props", gnocam_applet_properties_cb),
	BONOBO_UI_UNSAFE_VERB ("About", gnocam_applet_about_cb),
	BONOBO_UI_VERB_END
};

void
gnocam_applet_create_ui (GnoCamApplet *a)
{
	g_return_if_fail (GNOCAM_IS_APPLET (a));

	GtkWidget *bbox, *button;

	panel_applet_setup_menu_from_file (PANEL_APPLET (a), UIDIR,
		"GNOME_GnoCamApplet.xml", NULL, gnocam_applet_menu_verbs, a);

	gnome_window_icon_set_default_from_file (IMAGEDIR "gnocam-camera1.png");

	bbox = gtk_hbutton_box_new ();
	gtk_widget_show (bbox);
	gtk_container_add (GTK_CONTAINER (a), bbox);

	button = gtk_button_new_with_label (_("Capture"));
	gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (bbox), button);

	gtk_widget_show (GTK_WIDGET (a));
}

static void
gnocam_applet_init (GTypeInstance *instance, gpointer g_class)
{
        GnoCamApplet *a = GNOCAM_APPLET (instance);

	a = NULL;
}

GType
gnocam_applet_get_type (void)
{
        static GType type = 0;

        if (!type) {
                GTypeInfo ti;

                memset (&ti, 0, sizeof (GTypeInfo));
                ti.class_size    = sizeof (GnoCamAppletClass);
                ti.class_init    = gnocam_applet_class_init;
                ti.instance_size = sizeof (GnoCamApplet);
                ti.instance_init = gnocam_applet_init;

                type = g_type_register_static (PARENT_TYPE, "GnoCamApplet",
                                               &ti, 0);
        }

        return (type);
}

#if 0
#include <gtk/gtksignal.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkpixmap.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnome/gnome-exec.h>
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
on_capture_image_clicked (GtkButton *button)
{
	gtk_idle_add (do_capture, NULL);
}

static void
on_show_contents_clicked (GtkButton *button)
{
	int argc = 2;
	char * const argv [] = {"eog", "camera:/"};
	
	if (gnome_execute_async (NULL, argc, argv) == -1)
		g_warning ("Could not start EOG!");
}

int
main (int argc, char **argv)
{
	GtkWidget *applet, *image, *toolbar;
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

	toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL,
				   GTK_TOOLBAR_ICONS);
	gtk_widget_show (toolbar);
	applet_widget_add (APPLET_WIDGET (applet), toolbar);

	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "/gnocam-small.xpm");
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 1);
	gdk_pixbuf_unref (pixbuf); 
	image = gtk_pixmap_new (pixmap, bitmap);
	gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
				    GTK_TOOLBAR_CHILD_BUTTON, NULL,
				    _("Capture image"), _("Capture image"),
				    NULL, image,
				    GTK_SIGNAL_FUNC (on_capture_image_clicked),
				    NULL);
	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "/gnocam-folder.png");
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 1);
	gdk_pixbuf_unref (pixbuf);
	image = gtk_pixmap_new (pixmap, bitmap); 
	gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
				    GTK_TOOLBAR_CHILD_BUTTON, NULL,
				    _("Show contents"), _("Show contents"),
				    NULL, image,
				    GTK_SIGNAL_FUNC (on_show_contents_clicked),
				    NULL);

	gtk_widget_show (applet);

	applet_widget_gtk_main ();

	return (0);
}

#endif
