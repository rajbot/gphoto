#include <config.h>
#include "gnocam-applet.h"

#include <stdlib.h>
#include <string.h>

#include <libgnomeui/gnome-about.h>
#include <libgnomeui/gnome-window-icon.h>

#include <gtk/gtkhbbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhbox.h>

#include <gconf/gconf-client.h>

#include <bonobo/bonobo-exception.h>

#include <panel-applet-gconf.h>

#include "GnoCam.h"

#include "gnocam-prefs.h"

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

#define PARENT_TYPE G_TYPE_OBJECT
static GObjectClass *parent_class;

typedef struct _GnoCamPreferences GnoCamPreferences;
struct _GnoCamPreferences {
	gboolean camera_automatic;
	gboolean connect_automatic;
	gchar *model;
	gchar *port;
};

struct _GnoCamAppletPrivate
{
	GnoCamPreferences prefs;

	GNOME_GnoCam_Camera camera;

	PanelApplet *applet;
};

static void
gnocam_applet_connect (GnoCamApplet *a)
{
	g_return_if_fail (GNOCAM_IS_APPLET (a));

	g_message ("Implement!");
}

static void
gnocam_applet_load_preferences (GnoCamApplet *a)
{
	gchar *key;
	GConfClient *client;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	key = panel_applet_get_preferences_key (a->priv->applet);
	client = gconf_client_get_default ();
	if (key && gconf_client_dir_exists (client, key, NULL)) {
		a->priv->prefs.camera_automatic = panel_applet_gconf_get_bool (
			a->priv->applet, "camera_automatic", NULL);
		a->priv->prefs.connect_automatic = panel_applet_gconf_get_bool (
			a->priv->applet, "connect_automatic", NULL);
		free (a->priv->prefs.model);
		a->priv->prefs.model = panel_applet_gconf_get_string (
			a->priv->applet, "model", NULL);
		free (a->priv->prefs.port);
		a->priv->prefs.port = panel_applet_gconf_get_string (
			a->priv->applet, "port", NULL);
	}
	g_free (key);
	g_object_unref (client);

	if (a->priv->prefs.connect_automatic && 
	    (a->priv->camera == CORBA_OBJECT_NIL))
		gnocam_applet_connect (a);
}

static void
on_close_clicked (GtkButton *button, GtkDialog *dialog)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
gnocam_applet_report_error (GnoCamApplet *a, CORBA_Environment *ev)
{
	GtkWidget *d;
	GtkWidget *w, *hb;

	g_return_if_fail (GNOCAM_IS_APPLET (a));
	g_return_if_fail (ev != NULL);

	d = gtk_dialog_new ();
	gtk_window_set_wmclass (GTK_WINDOW (d), "gnocam-applet",
				"Camera Applet");
	gtk_container_set_border_width (
			GTK_CONTAINER (GTK_DIALOG (d)->vbox), 5);

	w = gtk_button_new_from_stock (GTK_STOCK_OK);
	gtk_widget_show (w);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (d)->action_area), w,
			  FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (w), "clicked",
			  G_CALLBACK (on_close_clicked), d);

	hb = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hb);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (d)->vbox), hb);

	w = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR,
				      GTK_ICON_SIZE_DIALOG);
	gtk_widget_show (w);
	gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 0);

	w = gtk_label_new (bonobo_exception_get_text (ev));
	gtk_label_set_line_wrap (GTK_LABEL (w), TRUE);
	gtk_widget_show (w);
	gtk_box_pack_start (GTK_BOX (hb), w, TRUE, TRUE, 0);

	gtk_widget_show (d);
}

static void
gnocam_applet_finalize (GObject *object)
{
	GnoCamApplet *a = GNOCAM_APPLET (object);

	g_free (a->priv->prefs.model);
	g_free (a->priv->prefs.port);
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
on_prefs_camera_changed (GnoCamPrefs *prefs, gboolean automatic,
			 GnoCamApplet *a)
{
	panel_applet_gconf_set_bool (a->priv->applet, "camera_automatic",
				     automatic, NULL);
	gnocam_applet_load_preferences (a);
}

static void
on_prefs_connect_changed (GnoCamPrefs *prefs, gboolean automatic,
			  GnoCamApplet *a)
{
	panel_applet_gconf_set_bool (a->priv->applet, "connect_automatic",
				     automatic, NULL);
	gnocam_applet_load_preferences (a);
}

static void
on_prefs_model_changed (GnoCamPrefs *prefs, const gchar *model, GnoCamApplet *a)
{
	panel_applet_gconf_set_string (a->priv->applet, "model", model, NULL);
	gnocam_applet_load_preferences (a);
}

static void
on_prefs_port_changed (GnoCamPrefs *prefs, const gchar *port, GnoCamApplet *a)
{
	panel_applet_gconf_set_string (a->priv->applet, "port", port, NULL);
	gnocam_applet_load_preferences (a);
}

static void
gnocam_applet_properties_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			     const char verbname)
{
	static GnoCamPrefs *prefs = NULL;
	CORBA_Environment ev;

	if (prefs) {
		gtk_window_present (GTK_WINDOW (prefs));
		return;
	}

	/* Create the dialog. */
	CORBA_exception_init (&ev);
	prefs = gnocam_prefs_new (a->priv->prefs.camera_automatic,
				  a->priv->prefs.connect_automatic,
				  a->priv->prefs.model, a->priv->prefs.port,
				  &ev);
	if (BONOBO_EX (&ev) || !prefs) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);

	/* Set up the dialog. */
	gtk_window_set_wmclass (GTK_WINDOW (prefs), "gnocam-applet",
				"Camera Applet");
	gnome_window_icon_set_from_file (GTK_WINDOW (prefs),
					 IMAGEDIR "gnocam-camera1.png");
	g_signal_connect (prefs, "destroy", G_CALLBACK (gtk_widget_destroyed),
			  &prefs);
	g_signal_connect (prefs, "camera_changed",
			  G_CALLBACK (on_prefs_camera_changed), a);
	g_signal_connect (prefs, "connect_changed",
			  G_CALLBACK (on_prefs_connect_changed), a);
	g_signal_connect (prefs, "model_changed",
			  G_CALLBACK (on_prefs_model_changed), a);
	g_signal_connect (prefs, "port_changed",
			  G_CALLBACK (on_prefs_port_changed), a);

	/* Show the dialog. */
	gtk_widget_show (GTK_WIDGET (prefs));
}

static void
gnocam_applet_about_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			const char *verbname)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf;
	GError *e = NULL;

	static const gchar *authors[] = {
		"Lutz Mueller <lutz@users.sourceforge.net>",
		NULL
	};
	const gchar *documenters[] = {NULL};
	const gchar *translator_credits = _("translator_credits");

	/* Do we already have the widget? */
	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	/* Create the widget. */
	pixbuf = gdk_pixbuf_new_from_file (IMAGEDIR "gnocam-camera1.png", &e);
	about = gnome_about_new (
		_("Camera Applet"), VERSION,
		_("Copyright (c) 2002 Lutz Mueller"),
		_("Access your digital camera."),
		authors, documenters,
		(strcmp (translator_credits, "translator_credits") ?
	 				translator_credits : NULL), pixbuf);
	if (pixbuf)
		g_object_unref (pixbuf);

	/* Set up the widget. */
	gtk_window_set_wmclass (GTK_WINDOW (about), "gnocam-applet", 
				"Camera Applet");
	gnome_window_icon_set_from_file (GTK_WINDOW (about),
					 IMAGEDIR "gnocam-camera1.png");
	g_signal_connect (about, "destroy", G_CALLBACK (gtk_widget_destroyed),
			  &about);

	/* Show the widget. */
	gtk_widget_show (about);
}

static const BonoboUIVerb gnocam_applet_menu_verbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Props", gnocam_applet_properties_cb),
	BONOBO_UI_UNSAFE_VERB ("About", gnocam_applet_about_cb),
	BONOBO_UI_VERB_END
};

GnoCamApplet *
gnocam_applet_new (PanelApplet *applet)
{
	GnoCamApplet *a;
	GtkWidget *w;

	g_return_val_if_fail (PANEL_IS_APPLET (applet), NULL);

	a = g_object_new (GNOCAM_TYPE_APPLET, NULL);
	a->priv->applet = applet;

	/* Setup menu. */
	panel_applet_setup_menu_from_file (applet, UIDIR,
		"GNOME_GnoCamApplet.xml", NULL, gnocam_applet_menu_verbs, a);

	/* Setup widget. */
	w = gtk_image_new_from_file (IMAGEDIR "gnocam-camera1.png");
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (a->priv->applet), w);
	gtk_widget_show (GTK_WIDGET (a->priv->applet));

	gnocam_applet_load_preferences (a);

	return (a);
}

static void
gnocam_applet_init (GTypeInstance *instance, gpointer g_class)
{
        GnoCamApplet *a = GNOCAM_APPLET (instance);

	a->priv = g_new0 (GnoCamAppletPrivate, 1);
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
