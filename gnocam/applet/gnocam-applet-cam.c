#include <config.h>
#include "gnocam-applet-cam.h"
#include "gnocam/i18n.h"

#include <libgnocam/GNOME_C.h>
#include <libgnocam/gnocam-util.h>

#include <gtk/gtkimage.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkitemfactory.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkhbbox.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo-activation/bonobo-activation-activate.h>
#include <bonobo/bonobo-window.h>
#include <bonobo/bonobo-widget.h>

#include <string.h>

static void gnocam_applet_cam_update (GnocamAppletCam *c);

#define PARENT_TYPE GTK_TYPE_BUTTON
static GtkButtonClass *parent_class;

enum {
	PROP_0,
	PROP_NAME,
	PROP_MANUFACTURER,
	PROP_MODEL,
	PROP_PORT,
	PROP_CONNECT_AUTO,
	PROP_SIZE
};

#if 0
enum {
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};
#endif

struct _GnocamAppletCamPriv
{
	GtkItemFactory *factory;
	GtkImage *image;

	/* Dialogs */
	GtkWidget *prefs, *preview;

	GdkPixbuf *p_connected;
	GdkPixbuf *p_disconnected;

	GNOME_C_Camera camera;

	guint size;

	/* Settings */
	gchar *manuf, *model, *port, *name;
	gboolean connect_auto;
};

static void
gnocam_applet_cam_finalize (GObject *object)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (object);

	g_free (c->priv->manuf);
	g_free (c->priv->port);
	g_free (c->priv->model);
	g_free (c->priv->name);
	g_object_unref (c->priv->factory);
	g_object_unref (c->priv->p_connected);
	g_object_unref (c->priv->p_disconnected);
	g_free (c->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_applet_cam_get_property (GObject *object, guint n, GValue *v,
				GParamSpec *pspec)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (object);

	switch (n) {
	case PROP_NAME:
		g_value_set_string (v, c->priv->name);
		break;
	case PROP_MANUFACTURER:
		g_value_set_string (v, c->priv->manuf);
		break;
	case PROP_MODEL:
		g_value_set_string (v, c->priv->model);
		break;
	case PROP_PORT:
		g_value_set_string (v, c->priv->port);
		break;
	case PROP_SIZE:
		g_value_set_uint (v, c->priv->size);
		break;
	case PROP_CONNECT_AUTO:
		g_value_set_boolean (v, c->priv->connect_auto);
		break;
	default:
		break;
	}
}

static void
gnocam_applet_cam_set_property (GObject *object, guint n, const GValue *v,
				GParamSpec *pspec)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (object);

	switch (n) {
	case PROP_NAME:
		g_free (c->priv->name); 
		c->priv->name = g_strdup (g_value_get_string (v));
		break;
	case PROP_MANUFACTURER:
		gnocam_applet_cam_disconnect (c);
		g_free (c->priv->manuf);
		c->priv->manuf = g_strdup (g_value_get_string (v));
		break;
	case PROP_PORT:
		gnocam_applet_cam_disconnect (c);
		g_free (c->priv->port);
		c->priv->port = g_strdup (g_value_get_string (v));
		break;
	case PROP_MODEL:
		gnocam_applet_cam_disconnect (c);
		g_free (c->priv->model);
		c->priv->model = g_strdup (g_value_get_string (v));
		break;
	case PROP_SIZE:
		c->priv->size = g_value_get_uint (v);
		gnocam_applet_cam_update (c);
		break;
	case PROP_CONNECT_AUTO:
		c->priv->connect_auto = g_value_get_boolean (v);
		break;
	default:
		break;
	}
}

static void
gnocam_applet_cam_init (GTypeInstance *instance, gpointer g_class)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (instance);
	GtkWidget *w;

	c->priv = g_new0 (GnocamAppletCamPriv, 1);
	c->priv->p_disconnected = gdk_pixbuf_new_from_file (
			IMAGEDIR "gnocam-camera-disconnected.png", NULL);
	c->priv->p_connected = gdk_pixbuf_new_from_file (
			IMAGEDIR "gnocam-camera1.png", NULL);

	gtk_button_set_relief (GTK_BUTTON (c), GTK_RELIEF_NONE);

	/* Create the image */
	w = gtk_image_new ();
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (c), w);
	c->priv->image = GTK_IMAGE (w);
}

static gboolean
on_button_press_event (GtkWidget *w, GdkEventButton *event)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (w);

	switch (event->button) {
	case 3:
		gtk_item_factory_popup (c->priv->factory, event->x_root,
			event->y_root, event->button, event->time);
		return TRUE;
	default:
		return FALSE;
	}
}

static void
gnocam_applet_cam_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize     = gnocam_applet_cam_finalize;
	gobject_class->set_property = gnocam_applet_cam_set_property;
	gobject_class->get_property = gnocam_applet_cam_get_property;

	g_object_class_install_property (gobject_class, PROP_NAME,
		g_param_spec_string ("name", _("Name"),
			_("Name"), _("Camera"), G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_MODEL,
		g_param_spec_string ("model", _("Model"),
			_("Model"), _("Unspecified"), G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_MANUFACTURER,
		g_param_spec_string ("manufacturer", _("Manufacturer"),
			_("Manufacturer"), _("Unspecified"),
			G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_PORT,
		g_param_spec_string ("port", _("Port"), _("Port"),
			_("Unspecified"), G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_CONNECT_AUTO,
		g_param_spec_boolean ("connect_auto",
			_("Connect automatically"), _("Connect automatically"),
			TRUE, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_SIZE,
		g_param_spec_uint ("size", _("Size"), _("Size"), 1, 256, 24,
			G_PARAM_READWRITE));

	parent_class = g_type_class_peek_parent (g_class);
}

GType
gnocam_applet_cam_get_type (void)
{
	static GType t = 0;

	if (!t) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size    = sizeof (GnocamAppletCamClass);
		ti.class_init    = gnocam_applet_cam_class_init;
		ti.instance_size = sizeof (GnocamAppletCam);
		ti.instance_init = gnocam_applet_cam_init;

		t = g_type_register_static (PARENT_TYPE, "GnocamAppletCam",
					    &ti, 0);
	}

	return t;
}

static void
action_connect (gpointer callback_data, guint callback_action, GtkWidget *w)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (callback_data);

	if (GTK_CHECK_MENU_ITEM (w)->active &&
	    (c->priv->camera == CORBA_OBJECT_NIL))
		gnocam_applet_cam_connect (c);
	else if (!GTK_CHECK_MENU_ITEM (w)->active &&
		 (c->priv->camera != CORBA_OBJECT_NIL))
		gnocam_applet_cam_disconnect (c);
}

static void
gnocam_applet_cam_update (GnocamAppletCam *c)
{
	GdkPixbuf *p;
	GtkWidget *w;

	/* Update image */
	p = gdk_pixbuf_scale_simple (
		c->priv->camera ? c->priv->p_connected :
				  c->priv->p_disconnected,
		c->priv->size, c->priv->size, GDK_INTERP_NEAREST);
	gtk_image_set_from_pixbuf (c->priv->image, p);
	gdk_pixbuf_unref (p);

	/* Update popup */
	w = gtk_item_factory_get_widget (c->priv->factory, "/Connect");
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), 
				      c->priv->camera != CORBA_OBJECT_NIL);
	w = gtk_item_factory_get_widget (c->priv->factory, "/Settings");
	gtk_widget_set_sensitive (w, c->priv->camera != CORBA_OBJECT_NIL);
	w = gtk_item_factory_get_widget (c->priv->factory, "/Preview");
	gtk_widget_set_sensitive (w, c->priv->camera != CORBA_OBJECT_NIL);
}

void
gnocam_applet_cam_disconnect (GnocamAppletCam *c)
{
	if (c->priv->camera != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (c->priv->camera, NULL);
	c->priv->camera = CORBA_OBJECT_NIL;
	gnocam_applet_cam_update (c);
}

void
gnocam_applet_cam_connect (GnocamAppletCam *c)
{
	CORBA_Environment ev;

	gnocam_applet_cam_disconnect (c);

	CORBA_exception_init (&ev);
	c->priv->camera = gnocam_util_get_camera (c->priv->manuf,
		c->priv->model, c->priv->port, &ev);
	if (BONOBO_EX (&ev)) {
		if (c->priv->camera) {
			//bonobo_object_release_unref (c->priv->camera, NULL);
			c->priv->camera = CORBA_OBJECT_NIL;
		}
		g_warning ("Could not get camera!");
	}
	CORBA_exception_free (&ev);
	gnocam_applet_cam_update (c);
}

static void
on_close_clicked (GtkButton *button, GtkWidget *win)
{
	gtk_widget_destroy (GTK_WIDGET (win));
}

static void
on_prefs_destroy (GtkObject *o, GnocamAppletCam *c)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));

	c->priv->prefs = NULL;
}

static void
on_preview_destroy (GtkObject *o, GnocamAppletCam *c)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));

	c->priv->preview = NULL;
}

static void
action_preview (gpointer callback_data, guint callback_action, GtkWidget *w)
{
	CORBA_Environment ev;
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (callback_data);
	GtkWidget *b, *vbox, *widget;
	Bonobo_Control control;

	if (c->priv->preview) {
		gtk_window_present (GTK_WINDOW (c->priv->preview));
		return;
	}

	CORBA_exception_init (&ev);
	control = GNOME_C_Camera__get_preview (c->priv->camera, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get preview: %s",
				bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	GNOME_C_Preview_start (control, &ev);
	CORBA_exception_free (&ev);
	c->priv->preview = bonobo_window_new ("Preview", _("Preview"));
	g_signal_connect (c->priv->preview, "destroy",
			G_CALLBACK (on_preview_destroy), c);
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 5));
	bonobo_window_set_contents (BONOBO_WINDOW (c->priv->preview), vbox);

	gtk_widget_show (widget = bonobo_widget_new_control_from_objref (
		control, BONOBO_OBJREF (bonobo_window_get_ui_container (
				BONOBO_WINDOW (c->priv->preview)))));
	gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	/* Add the button */
	gtk_widget_show (widget = gtk_hbutton_box_new ());
	gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_END);
	gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, TRUE, 0);
	b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (b);
	gtk_box_pack_end (GTK_BOX (widget), b, FALSE, FALSE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_close_clicked),
			c->priv->preview);
	gtk_widget_grab_focus (b);

	gtk_widget_show (widget = gtk_hseparator_new ());
	gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, TRUE, 0);

	/* Show the dialog */
	gtk_widget_show (c->priv->preview);
}

static void
action_settings (gpointer callback_data, guint callback_action, GtkWidget *w)
{
	CORBA_Environment ev;
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (callback_data);
	GtkWidget *b, *vbox, *widget;
	Bonobo_Control control;

	if (c->priv->prefs) {
		gtk_window_present (GTK_WINDOW (c->priv->prefs));
		return;
	}

	CORBA_exception_init (&ev);
	control = GNOME_C_Camera__get_prefs (c->priv->camera, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not get preferences: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);
	c->priv->prefs = bonobo_window_new ("Preferences", _("Preferences"));
	g_signal_connect (c->priv->prefs, "destroy",
		G_CALLBACK (on_prefs_destroy), c);
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 5));
	bonobo_window_set_contents (BONOBO_WINDOW (c->priv->prefs), vbox);
	
	gtk_widget_show (widget = bonobo_widget_new_control_from_objref (
		control, BONOBO_OBJREF (bonobo_window_get_ui_container (
					BONOBO_WINDOW (c->priv->prefs)))));
	gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	/* Add the button */
	gtk_widget_show (widget = gtk_hbutton_box_new ());
	gtk_button_box_set_layout (GTK_BUTTON_BOX (widget), GTK_BUTTONBOX_END);
	gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, TRUE, 0);
	b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (b);
	gtk_box_pack_end (GTK_BOX (widget), b, FALSE, FALSE, 0);
	g_signal_connect (b, "clicked", G_CALLBACK (on_close_clicked),
			  c->priv->prefs);
	gtk_widget_grab_focus (b);

	gtk_widget_show (widget = gtk_hseparator_new ());
	gtk_box_pack_end (GTK_BOX (vbox), widget, FALSE, TRUE, 0);

	/* Show the dialog */
	gtk_widget_show (c->priv->prefs);
}

static GtkItemFactoryEntry popup[] =
{
	{"/_Connect", NULL, action_connect, 0, "<CheckItem>"},
	{"/_Settings", NULL, action_settings, 0, "<StockItem>",
						GTK_STOCK_PROPERTIES},
	{"/_Preview", NULL, action_preview, 0, "<Item>"}
};

GnocamAppletCam *
gnocam_applet_cam_new (guint size)
{
	GnocamAppletCam *c = g_object_new (GNOCAM_TYPE_APPLET_CAM, NULL);
	GtkAccelGroup *ag = gtk_accel_group_new ();

	c->priv->factory = gtk_item_factory_new (GTK_TYPE_MENU, "<popup>", ag);
	gtk_item_factory_create_items (c->priv->factory, G_N_ELEMENTS (popup),
				       popup, c);
	g_object_ref (c->priv->factory);
	gtk_object_sink (GTK_OBJECT (c->priv->factory));

	g_signal_connect (c, "button_press_event",
			  G_CALLBACK (on_button_press_event), NULL);

	c->priv->size = size;
	gnocam_applet_cam_update (c);

	return c;
}
