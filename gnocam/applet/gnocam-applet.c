#include "config.h"
#include "gnocam-applet.h"
#include "gnocam-applet-cam.h"
#include "gnocam/i18n.h"

#include <libgnocam/GNOME_C.h>

#include <stdlib.h>
#include <string.h>

#include <libgnomeui/gnome-about.h>
#include <libgnomeui/gnome-window-icon.h>

#include <gtk/gtkhbbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkeventbox.h>

#include <gconf/gconf-client.h>

#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-window.h>
#include <bonobo/bonobo-control-frame.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-util.h>

#include <panel-applet-gconf.h>

#define PARENT_TYPE G_TYPE_OBJECT
static GObjectClass *parent_class;

#if 0
static void gnocam_applet_report_error (GnocamApplet *, CORBA_Environment *);
static void gnocam_applet_disconnect   (GnocamApplet *);
#endif

struct _GnocamAppletPrivate
{
	PanelApplet *applet;

	GtkBox *box;
	GtkWidget *image;
	GList *cameras;
};

static void
gnocam_applet_update (GnocamApplet *a)
{
#if 0
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
#endif
}

static void
on_changed (GnocamAppletCam *c, GnocamApplet *a)
{
	gchar *key = panel_applet_get_preferences_key (a->priv->applet);
	guint i = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (c), "number"));
	gchar *s;
	const gchar *t;
	
	/* New camera? */
	if (!i) i = g_list_length (a->priv->cameras) + 1;
	s = g_strdup_printf ("%s/%i", key, i);
	
	t = gnocam_applet_cam_get_manufacturer (c);
	if (t) panel_applet_gconf_set_string (a->priv->applet,
					      "manufacturer", t, NULL);
	t = gnocam_applet_cam_get_model (c);
	if (t) panel_applet_gconf_set_string (a->priv->applet,
					      "model", t, NULL);
	t = gnocam_applet_cam_get_port (c);
	if (t) panel_applet_gconf_set_string (a->priv->applet, "port", t, NULL);
	panel_applet_gconf_set_bool (a->priv->applet, "connect_auto", 
		gnocam_applet_cam_get_connect_auto (c), NULL);

	g_free (key);
	g_free (s);
}

static void
gnocam_applet_load_preferences (GnocamApplet *a)
{
	gchar *key;
	GConfClient *client;
	gchar *s;
	guint i;
	GnocamAppletCam *c;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	key = panel_applet_get_preferences_key (a->priv->applet);
	client = gconf_client_get_default ();
	if (key && gconf_client_dir_exists (client, key, NULL)) {
	    for (i = 1; ; i++) {
		s = g_strdup_printf ("%s/%i", key, i);
		if (gconf_client_dir_exists (client, s, NULL)) {
		    c = gnocam_applet_cam_new (
				    panel_applet_get_size (a->priv->applet));
		    g_signal_connect (c, "changed",
				      G_CALLBACK (on_changed), a);
		    a->priv->cameras = g_list_append (a->priv->cameras, c);
		    g_object_set_data (G_OBJECT (c), "number",
				       GINT_TO_POINTER (i));
		    gnocam_applet_cam_set_manufacturer (c,
			panel_applet_gconf_get_string (a->priv->applet,
						       "manufacturer", NULL));
		    gnocam_applet_cam_set_model (c,
			panel_applet_gconf_get_string (a->priv->applet,
						       "model", NULL));
		    gnocam_applet_cam_set_port (c,
			panel_applet_gconf_get_string (a->priv->applet,
						       "port", NULL));
		    gnocam_applet_cam_set_connect_auto (c,
			panel_applet_gconf_get_bool (a->priv->applet,
						     "connect_auto", NULL));
		    gtk_box_pack_start (a->priv->box, GTK_WIDGET (c),
				        FALSE, FALSE, 0);
		}
		g_free (s);
	    }
	}
	g_free (key);
	g_object_unref (client);
}

#if 0
static void
gnocam_applet_report_error (GnocamApplet *a, CORBA_Environment *ev)
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
#endif

static void
gnocam_applet_finalize (GObject *object)
{
	GnocamApplet *a = GNOCAM_APPLET (object);

	g_list_free (a->priv->cameras);
	g_free (a->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
on_change_orient (PanelApplet *applet, PanelAppletOrient orient,
		  GnocamApplet *a)
{
	GtkWidget *b = NULL, *w;
	GList *list;
	guint i;

	switch (orient) {
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
		b = gtk_hbox_new (FALSE, 0);
		break;
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		b = gtk_vbox_new (FALSE, 0);
		break;
	default:
		return;
	}
	gtk_widget_show (b);

	/* Move all widgets over */
	list = gtk_container_get_children (GTK_CONTAINER (a->priv->box));
	for (i = 0; i < g_list_length (list); i++) {
		w = g_list_nth_data (list, i);
		g_object_ref (w);
		gtk_container_remove (GTK_CONTAINER (a->priv->box), w);
		gtk_box_pack_start (GTK_BOX (b), w, FALSE, FALSE, 0);
		g_object_unref (w);
	}
	g_list_free (list);
	gtk_container_remove (GTK_CONTAINER (a->priv->applet),
			      GTK_WIDGET (a->priv->box));
	gtk_container_add (GTK_CONTAINER (a->priv->applet), b);
	a->priv->box = GTK_BOX (b);
}

static void
on_change_size (PanelApplet *applet, guint size, GnocamApplet *a)
{
	g_message ("change_size");
}

static void
on_change_background (PanelApplet *applet, PanelAppletBackgroundType type,
		      GdkColor *color, GdkPixmap *frame, GnocamApplet *a)
{
	g_message ("change_background");
}

static void
on_move_focus_out_of_applet (PanelApplet *frame, GtkDirectionType direction,
			     GnocamApplet *a)
{
	g_message ("move_focus_out_of_applet");
}

static void
gnocam_applet_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_applet_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

#if 0
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
	GnocamApplet *a = GNOCAM_APPLET (user_data);

	g_message ("Got event '%s'.", event_name);

	if (CORBA_TypeCode_equal (any->_type, TC_GNOME_Camera_ErrorCode, ev)) {
		g_message ("Error!");
		return;
	}
	if (!CORBA_TypeCode_equal (any->_type, TC_CORBA_string, ev)) {
		g_message ("Unknown type!");
		return;
	}

	g_message ("The captured image is available at '%s'.",
		   *((const gchar **) any->_value));

	st = Bonobo_Unknown_queryInterface (a->priv->camera,
					    "IDL:Bonobo/Storage:1.0", ev);
	if (BONOBO_EX (ev)) {
		gnocam_applet_report_error (a, ev);
		return;
	}

	s = Bonobo_Storage_openStream (st, *((const gchar **) any->_value),
				       Bonobo_Storage_READ, ev);
	bonobo_object_release_unref (st, NULL);
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
	w = bonobo_window_new ("Gnocam", "Gnocam");
	cmp = bonobo_ui_component_new ("Gnocam");
	bonobo_ui_component_set_container (cmp, BONOBO_OBJREF (
		bonobo_window_get_ui_container (BONOBO_WINDOW (w))), NULL);
	bonobo_ui_util_set_ui (cmp, UIDIR, "gnocam-viewer-ui.xml",
			       "Gnocam", NULL);
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
#endif

#if 0
static void
gnocam_applet_capture_cb (BonoboUIComponent *uic, GnocamApplet *a,
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
#endif

static void
gnocam_applet_add_cb (BonoboUIComponent *uic, GnocamApplet *a,
		      const char *verbname)
{
	GnocamAppletCam *c;
	
	c = gnocam_applet_cam_new (panel_applet_get_size (a->priv->applet));
	g_signal_connect (c, "changed", G_CALLBACK (on_changed), a);
	a->priv->cameras = g_list_append (a->priv->cameras, c);
	gtk_widget_show (GTK_WIDGET (c));
	gtk_box_pack_start (a->priv->box, GTK_WIDGET (c), FALSE, FALSE, 0);
}

static void
gnocam_applet_about_cb (BonoboUIComponent *uic, GnocamApplet *a,
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
		_("Copyright © 2002 Lutz Mueller"),
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
	BONOBO_UI_UNSAFE_VERB ("Add", gnocam_applet_add_cb),
	BONOBO_UI_UNSAFE_VERB ("About", gnocam_applet_about_cb),
	BONOBO_UI_VERB_END
};

GnocamApplet *
gnocam_applet_new (PanelApplet *applet)
{
	GnocamApplet *a;
	GtkWidget *w;

	g_return_val_if_fail (PANEL_IS_APPLET (applet), NULL);

	a = g_object_new (GNOCAM_TYPE_APPLET, NULL);
	a->priv->applet = applet;

	/* Setup menu. */
	panel_applet_setup_menu_from_file (applet, UIDIR,
		"GNOME_GnocamApplet.xml", NULL, gnocam_applet_menu_verbs, a);

	/* Setup widget. */
	switch (panel_applet_get_orient (applet)) {
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		w = gtk_vbox_new (FALSE, 0);
		break;
	default:
		w = gtk_hbox_new (FALSE, 0);
		break;
	}
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (a->priv->applet), w);
	a->priv->box = GTK_BOX (w);
	a->priv->image= gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD,
						  GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (a->priv->image);
	gtk_box_pack_start (GTK_BOX (w), a->priv->image, FALSE, FALSE, 0);
	gnocam_applet_update (a);

	/* Setup the applet. */
	g_signal_connect (applet, "change_orient",
			  G_CALLBACK (on_change_orient), a);
	g_signal_connect (applet, "change_size",
			  G_CALLBACK (on_change_size), a);
	g_signal_connect (applet, "change_background",
			  G_CALLBACK (on_change_background), a);
	g_signal_connect (applet, "move_focus_out_of_applet",
			  G_CALLBACK (on_move_focus_out_of_applet), a);

	gtk_widget_show (GTK_WIDGET (a->priv->applet));

	gnocam_applet_update (a);
	gnocam_applet_load_preferences (a);

	return (a);
}

static void
gnocam_applet_init (GTypeInstance *instance, gpointer g_class)
{
        GnocamApplet *a = GNOCAM_APPLET (instance);

	a->priv = g_new0 (GnocamAppletPrivate, 1);
}

GType
gnocam_applet_get_type (void)
{
        static GType type = 0;

        if (!type) {
                GTypeInfo ti;

                memset (&ti, 0, sizeof (GTypeInfo));
                ti.class_size    = sizeof (GnocamAppletClass);
                ti.class_init    = gnocam_applet_class_init;
                ti.instance_size = sizeof (GnocamApplet);
                ti.instance_init = gnocam_applet_init;

                type = g_type_register_static (PARENT_TYPE, "GnocamApplet",
                                               &ti, 0);
        }

        return (type);
}
