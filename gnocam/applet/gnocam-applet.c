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
#include <bonobo/bonobo-window.h>
#include <bonobo/bonobo-control-frame.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-util.h>

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

	GNOME_Camera camera;

	PanelApplet *applet;

	GtkWidget *image;
};

static void
gnocam_applet_update (GnoCamApplet *a)
{
	GdkPixbuf *p;
	BonoboUIComponent *c;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	g_message ("Updating applet...");

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
listener_cb (BonoboListener *listener, const char *event_name,
	     const CORBA_any *any, CORBA_Environment *ev, gpointer user_data)
{
	GnoCamApplet *a = GNOCAM_APPLET (user_data);

	g_message ("Got event '%s'.", event_name);

	if (CORBA_TypeCode_equal (any->_type, TC_GNOME_GnoCam_ErrorCode, ev)) {
		g_message ("Error!");
	} else if (CORBA_TypeCode_equal (any->_type, TC_GNOME_Camera, ev)) {
		a->priv->camera = any->_value;
	} else {
		g_message ("Unknown type.");
	}

	gnocam_applet_update (a);

	bonobo_object_unref (listener);
}

static void
gnocam_applet_connect (GnoCamApplet *a)
{
	CORBA_Environment ev;
	Bonobo_Unknown o;
	BonoboListener *l;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	g_message ("Connecting...");

	CORBA_exception_init (&ev);

	o = bonobo_get_object ("OAFIID:GNOME_GnoCam", "Bonobo/Unknown", &ev);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	/* Terminate any existing connection. */
	gnocam_applet_disconnect (a);

	l = bonobo_listener_new (listener_cb, a);
	GNOME_GnoCam_getCameraByModel (o, BONOBO_OBJREF (l),
		a->priv->prefs.model ? a->priv->prefs.model : "", &ev);
	bonobo_object_release_unref (o, NULL);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

	CORBA_exception_free (&ev);
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
verb_FileExit_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	gtk_widget_destroy (GTK_WIDGET (user_data));
}

static BonoboUIVerb verb_list[] = {
	BONOBO_UI_VERB ("FileExit", verb_FileExit_cb),
	BONOBO_UI_VERB_END
};

static void
capture_cb (BonoboListener *listener, const char *event_name,
	    const CORBA_any *any, CORBA_Environment *ev, gpointer user_data)
{
	Bonobo_Storage st;
	Bonobo_Stream s;
	Bonobo_PersistStream p;
	Bonobo_Unknown o;
	gchar *r;
	Bonobo_StorageInfo *info;
	Bonobo_Control c;
	GtkWidget *w, *i;
	BonoboControlFrame *f;
	BonoboUIComponent *cmp;
	GnoCamApplet *a = GNOCAM_APPLET (user_data);

	g_message ("Got event '%s'.", event_name);

	if (CORBA_TypeCode_equal (any->_type, TC_GNOME_Camera_ErrorCode, ev)) {
		g_message ("Error!");
		return;
	}
	if (!CORBA_TypeCode_equal (any->_type, TC_CORBA_string, ev)) {
		g_message ("Unknown type!");
		return;
	}

	st = Bonobo_Unknown_queryInterface (a->priv->camera,
					    "IDL:Bonobo/Storage:1.0", ev);
	if (BONOBO_EX (ev)) {
		gnocam_applet_report_error (a, ev);
		CORBA_free (any->_value);
		return;
	}

	s = Bonobo_Storage_openStream (st, any->_value,
				       Bonobo_Storage_READ, ev);
	bonobo_object_release_unref (st, NULL);
	CORBA_free (any->_value);
	if (BONOBO_EX (ev)) {
		gnocam_applet_report_error (a, ev);
		return;
	}

	info = Bonobo_Stream_getInfo (s, Bonobo_FIELD_CONTENT_TYPE, ev);
	if (BONOBO_EX (ev)) {
		gnocam_applet_report_error (a, ev);
		bonobo_object_release_unref (s, NULL);
	}

	g_message ("Looking for an application that can display '%s'...",
		   info->content_type);
	r = g_strdup_printf (
		"repo_ids.has_all(['IDL:Bonobo/PersistStream:1.0', "
		"                  'IDL:Bonobo/Control:1.0']) AND "
		"bonobo:supported_mime_types.has('%s')",
			     info->content_type);
	o = bonobo_activation_activate (r, NULL, 0, NULL, ev);
	g_free (r);
	if (BONOBO_EX (ev)) {
		CORBA_free (info);
		bonobo_object_release_unref (s, NULL);
		gnocam_applet_report_error (a, ev);
		return;
	}

	if (o == CORBA_OBJECT_NIL) {
		g_warning ("No viewer available.");
		bonobo_object_release_unref (s, NULL);
		CORBA_exception_free (ev);
		return;
	}

	g_message ("Got object. Asking for PersistStream...");
	p = Bonobo_Unknown_queryInterface (o,
					"IDL:Bonobo/PersistStream:1.0", ev);
	if (BONOBO_EX (ev)) {
		CORBA_free (info);
		bonobo_object_release_unref (s, NULL);
		bonobo_object_release_unref (o, NULL);
		gnocam_applet_report_error (a, ev);
		return;
	}

	g_message ("Loading stream...");
	Bonobo_PersistStream_load (p, s, info->content_type, ev);
	bonobo_object_release_unref (s, NULL);
	bonobo_object_release_unref (p, NULL);
	CORBA_free (info);
	if (BONOBO_EX (ev)) {
		gnocam_applet_report_error (a, ev);
		bonobo_object_release_unref (o, NULL);
		return;
	}

	g_message ("Asking for control...");
	c = Bonobo_Unknown_queryInterface (o, "IDL:Bonobo/Control:1.0", ev);
	bonobo_object_release_unref (o, NULL);
	if (BONOBO_EX (ev)) {
		gnocam_applet_report_error (a, ev);
		return;
	}

	g_message ("Creating BonoboWindow...");
	w = bonobo_window_new ("GnoCam", "GnoCam");
	cmp = bonobo_ui_component_new ("GnoCam");
	bonobo_ui_component_set_container (cmp, BONOBO_OBJREF (
		bonobo_window_get_ui_container (BONOBO_WINDOW (w))), NULL);
	bonobo_ui_util_set_ui (cmp, UIDIR, "gnocam-viewer-ui.xml",
			       "GnoCam", NULL);
	bonobo_ui_component_add_verb_list_with_data (cmp, verb_list, w);

	g_message ("Creating control frame...");
	f = bonobo_control_frame_new (BONOBO_OBJREF (
		bonobo_window_get_ui_container (BONOBO_WINDOW (w))));
	bonobo_control_frame_set_autoactivate (f, FALSE);
	bonobo_control_frame_bind_to_control (f, c, ev);
	bonobo_control_frame_control_activate (f);
	bonobo_object_release_unref (c, NULL);
	if (BONOBO_EX (ev)) {
		gtk_widget_destroy (w);
		bonobo_object_unref (f);
		gnocam_applet_report_error (a, ev);
		return;
	}

	/* Show! */
	i = bonobo_control_frame_get_widget (f);
	gtk_widget_show (i);
	bonobo_window_set_contents (BONOBO_WINDOW (w), i);
	gtk_widget_show (w);
}

static void
gnocam_applet_capture_cb (BonoboUIComponent *uic, GnoCamApplet *a,
			  const char *verbname)
{
	CORBA_Environment ev;
	BonoboListener *l;

	CORBA_exception_init (&ev);

	l = bonobo_listener_new (capture_cb, a);
	g_message ("Capturing image...");
	GNOME_Camera_captureImage (a->priv->camera, BONOBO_OBJREF (l), &ev);
	if (BONOBO_EX (&ev)) {
		gnocam_applet_report_error (a, &ev);
		CORBA_exception_free (&ev);
		return;
	}

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
