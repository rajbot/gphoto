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

#include <bonobo/bonobo-moniker-util.h>
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

static void gnocam_applet_report_error (GnoCamApplet *, CORBA_Environment *);
static void gnocam_applet_disconnect   (GnoCamApplet *);

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

	GtkWidget *image;
};

static void
gnocam_applet_update (GnoCamApplet *a)
{
	GdkPixbuf *p;
	BonoboUIComponent *c;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	p = gdk_pixbuf_new_from_file (a->priv->camera != CORBA_OBJECT_NIL ?
			IMAGEDIR "gnocam-camera1.png" : 
			IMAGEDIR "gnocam-camera-disconnected.png", NULL);
	if (!p)
		return;
	gdk_pixbuf_scale_simple (p, panel_applet_get_size (a->priv->applet),
				    panel_applet_get_size (a->priv->applet),
				    GDK_INTERP_NEAREST);
	gtk_image_set_from_pixbuf (GTK_IMAGE (a->priv->image), p);
	gdk_pixbuf_unref (p);

	/* Update the menu */
	c = panel_applet_get_popup_component (a->priv->applet);
	bonobo_ui_component_set_prop (c, "/commands/Connect",
		"hidden", (a->priv->camera != CORBA_OBJECT_NIL) ? "1" : "0",
		NULL);
	bonobo_ui_component_set_prop (c, "/commands/Disconnect",
		"hidden", (a->priv->camera != CORBA_OBJECT_NIL) ? "0": "1",
		NULL);
	bonobo_ui_component_set_prop (c, "/commands/Capture",
		"sensitive", (a->priv->camera != CORBA_OBJECT_NIL) ? "1" : "0",
		NULL);
}

static void
gnocam_applet_connect (GnoCamApplet *a)
{
	CORBA_Environment ev;
	Bonobo_Unknown o;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	CORBA_exception_init (&ev);

	o = bonobo_get_object ("OAFIID:GNOME_GnoCam", "Bonobo/Unknown", &ev);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	/* Terminate any existing connection. */
	gnocam_applet_disconnect (a);

	if (a->priv->prefs.camera_automatic)
		a->priv->camera = GNOME_GnoCam_Factory_getCamera (o, &ev);
	else if (!a->priv->prefs.port || !strcmp (a->priv->prefs.port, ""))
		a->priv->camera = GNOME_GnoCam_Factory_getCameraByModel (o, 
			a->priv->prefs.model ? a->priv->prefs.model : "", &ev);
	else
		a->priv->camera = GNOME_GnoCam_Factory_getCameraByModelAndPort (
			o, a->priv->prefs.model ? a->priv->prefs.model : "",
			   a->priv->prefs.port, &ev);
	bonobo_object_release_unref (o, NULL);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	CORBA_exception_free (&ev);

	gnocam_applet_update (a);
}

static void
gnocam_applet_disconnect (GnoCamApplet *a)
{
	if (a->priv->camera != CORBA_OBJECT_NIL) {
		bonobo_object_release_unref (a->priv->camera, NULL);
		a->priv->camera = CORBA_OBJECT_NIL;
	}

	gnocam_applet_update (a);
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

	g_message ("Loaded preferences: ");
	g_message ("  Automatic connection: %i",
		   a->priv->prefs.connect_automatic);
	g_message ("  Automatic detection of camera model and port: %o",
		   a->priv->prefs.camera_automatic);
	g_message ("  Model: '%s'", a->priv->prefs.model);
	g_message ("  Port: '%s'", a->priv->prefs.port);

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
gnocam_applet_capture_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			  const char *verbname)
{
	CORBA_Environment ev;
	CORBA_char *path;
	Bonobo_Storage st;
	Bonobo_Stream s, dest;
	Bonobo_Unknown o;
	gchar *r;
	Bonobo_StorageInfo *info;
	Bonobo_Stream_iobuf *iobuf = NULL;

	CORBA_exception_init (&ev);

	path = GNOME_GnoCam_Camera_captureImage (a->priv->camera, &ev);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	st = Bonobo_Unknown_queryInterface (a->priv->camera,
					    "IDL:Bonobo/Storage:1.0", &ev);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		CORBA_free (path);
		return;
	}

	s = Bonobo_Storage_openStream (st, path, Bonobo_Storage_READ, &ev);
	bonobo_object_release_unref (st, NULL);
	CORBA_free (path);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	info = Bonobo_Stream_getInfo (s, Bonobo_FIELD_CONTENT_TYPE |
					 Bonobo_FIELD_SIZE, &ev);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		bonobo_object_release_unref (s, NULL);
		CORBA_exception_free (&ev);
	}

	r = g_strdup_printf ("repo_ids.has('IDL:Bonobo/Stream:1.0') AND "
			     "bonobo:supported_mime_types.has('%s')",
			     info->content_type);
	o = bonobo_activation_activate (r, NULL, 0, NULL, &ev);
	g_free (r);
	if (BONOBO_EX (&ev)) {
		CORBA_free (info);
		bonobo_object_release_unref (s, NULL);
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	dest = Bonobo_Unknown_queryInterface (o, "IDL:Bonobo/Stream:1.0", &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_free (info);
		bonobo_object_release_unref (s, NULL);
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}
	Bonobo_Stream_read (s, info->size, &iobuf, &ev);
	CORBA_free (info);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (s, NULL);
		bonobo_object_release_unref (dest, NULL);
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}
	Bonobo_Stream_write (s, iobuf, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (s, NULL);
		bonobo_object_release_unref (dest, NULL);
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_free (iobuf);

	bonobo_object_release_unref (dest, NULL);
	bonobo_object_release_unref (s, NULL);

	CORBA_exception_free (&ev);
}

static void
gnocam_applet_properties_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			     const char *verbname)
{
	static GnoCamPrefs *prefs = NULL;
	CORBA_Environment ev;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

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
gnocam_applet_connect_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			  const char *verbname)
{
	g_return_if_fail (GNOCAM_IS_APPLET (a));

	gnocam_applet_connect (a);
}

static void
gnocam_applet_disconnect_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			     const char *verbname)
{
	g_return_if_fail (GNOCAM_IS_APPLET (a));

	gnocam_applet_disconnect (a);
}

static void
gnocam_applet_about_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			const char *verbname)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf;
	GError *e = NULL;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

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

static void
on_change_size (PanelApplet *applet, gint size, GnoCamApplet *a)
{
	gnocam_applet_update (a);
};

static const BonoboUIVerb gnocam_applet_menu_verbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Connect", gnocam_applet_connect_cb),
	BONOBO_UI_UNSAFE_VERB ("Disconnect", gnocam_applet_disconnect_cb),
	BONOBO_UI_UNSAFE_VERB ("Capture", gnocam_applet_capture_cb),
	BONOBO_UI_UNSAFE_VERB ("Props", gnocam_applet_properties_cb),
	BONOBO_UI_UNSAFE_VERB ("About", gnocam_applet_about_cb),
	BONOBO_UI_VERB_END
};

GnoCamApplet *
gnocam_applet_new (PanelApplet *applet)
{
	GnoCamApplet *a;

	g_return_val_if_fail (PANEL_IS_APPLET (applet), NULL);

	a = g_object_new (GNOCAM_TYPE_APPLET, NULL);
	a->priv->applet = applet;

	/* Setup menu. */
	panel_applet_setup_menu_from_file (applet, UIDIR,
		"GNOME_GnoCamApplet.xml", NULL, gnocam_applet_menu_verbs, a);

	/* Setup widget. */
	a->priv->image= gtk_image_new ();
	gtk_widget_show (a->priv->image);
	gtk_container_add (GTK_CONTAINER (a->priv->applet), a->priv->image);
	gnocam_applet_update (a);

	/* Setup the applet. */
	gtk_widget_show (GTK_WIDGET (a->priv->applet));
	g_signal_connect (a->priv->applet, "change_size",
			  G_CALLBACK (on_change_size), a);

	gnocam_applet_update (a);
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
