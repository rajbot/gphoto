#include <config.h>
#include "gnocam-applet-cam.h"
#include "gnocam/i18n.h"
#include "GNOME_C.h"

#include <libgnocam/gnocam-chooser.h>
#include <libgnocam/gnocam-util.h>

#include <gtk/gtkimage.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkitemfactory.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkcheckmenuitem.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo-activation/bonobo-activation-activate.h>

#include <string.h>

#define PARENT_TYPE GTK_TYPE_BUTTON
static GtkButtonClass *parent_class;

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct _GnocamAppletCamPriv
{
	GtkItemFactory *factory;
	GtkImage *image;

	GdkPixbuf *p_connected;
	GdkPixbuf *p_disconnected;

	GNOME_C_Camera camera;

	guint size;

	/* Settings */
	gchar *manuf, *model, *port;
	gboolean connect_auto;
};

static void
gnocam_applet_cam_finalize (GObject *object)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (object);

	g_free (c->priv->manuf);
	g_free (c->priv->port);
	g_free (c->priv->model);
	g_object_unref (c->priv->factory);
	g_object_unref (c->priv->p_connected);
	g_object_unref (c->priv->p_disconnected);
	g_free (c->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
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
	gobject_class->finalize = gnocam_applet_cam_finalize;

	parent_class = g_type_class_peek_parent (g_class);

	signals[CHANGED] = g_signal_new ("changed",
		G_TYPE_FROM_CLASS (g_class), G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (GnocamAppletCamClass, changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
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

	if (GTK_CHECK_MENU_ITEM (w)->active)
		gnocam_applet_cam_connect (c);
	else
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
	g_signal_handlers_block_by_func (w, action_connect, c);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (w), 
				      c->priv->camera != CORBA_OBJECT_NIL);
	g_signal_handlers_unblock_by_func (w, action_connect, c);
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
	if (BONOBO_EX (&ev))
		g_warning ("Could not get camera!");
	CORBA_exception_free (&ev);
	
	gnocam_applet_cam_update (c);
}

static void
on_changed (GnocamChooser *chooser, GnocamAppletCam *c)
{
	gboolean b = (c->priv->camera != CORBA_OBJECT_NIL);

	if (b) gnocam_applet_cam_disconnect (c);

	g_free (c->priv->manuf);
	g_free (c->priv->model);
	g_free (c->priv->port);

	c->priv->manuf = gnocam_chooser_get_manufacturer (chooser);
	c->priv->model = gnocam_chooser_get_model (chooser);
	c->priv->port  = gnocam_chooser_get_port (chooser);
	c->priv->connect_auto = gnocam_chooser_get_connect_auto (chooser);

	if (b) gnocam_applet_cam_connect (c);

	g_signal_emit (c, signals[CHANGED], 0);
}

static void
action_select_camera (gpointer callback_data, guint callback_action, 
		      GtkWidget *w)
{
	GnocamAppletCam *c = GNOCAM_APPLET_CAM (callback_data);
	GnocamChooser *chooser = gnocam_chooser_new ();

	gnocam_chooser_set_manufacturer (chooser, c->priv->manuf);
	gnocam_chooser_set_model (chooser, c->priv->model);
	gnocam_chooser_set_port (chooser, c->priv->port);
	gnocam_chooser_set_connect_auto (chooser, c->priv->connect_auto);
	g_signal_connect (chooser, "changed", G_CALLBACK (on_changed), c);
	gtk_widget_show (GTK_WIDGET (chooser));
}

static GtkItemFactoryEntry popup[] =
{
	{"/_Connect", NULL, action_connect, 0, "<CheckItem>"},
	{"/_Select camera", NULL, action_select_camera, 0, "<Item>"},
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

void
gnocam_applet_cam_set_manufacturer (GnocamAppletCam *c, const gchar *m)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));
	if (c->priv->camera != CORBA_OBJECT_NIL)
		gnocam_applet_cam_disconnect (c);
	g_free (c->priv->manuf);
	c->priv->manuf = g_strdup (m);
}

void
gnocam_applet_cam_set_model (GnocamAppletCam *c, const gchar *m)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));
	if (c->priv->camera != CORBA_OBJECT_NIL)
		gnocam_applet_cam_disconnect (c);
	c->priv->model = g_strdup (m);
}

void
gnocam_applet_cam_set_port (GnocamAppletCam *c, const gchar *m)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));
	if (c->priv->camera != CORBA_OBJECT_NIL)
		gnocam_applet_cam_disconnect (c);
	g_free (c->priv->port);
	c->priv->port = g_strdup (m);
}

void
gnocam_applet_cam_set_connect_auto (GnocamAppletCam *c, gboolean connect_auto)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));
	c->priv->connect_auto = connect_auto;
}

void
gnocam_applet_cam_set_size (GnocamAppletCam *c, guint size)
{
	g_return_if_fail (GNOCAM_IS_APPLET_CAM (c));

	c->priv->size = size;
	gnocam_applet_cam_update (c);
}

const gchar *
gnocam_applet_cam_get_manufacturer (GnocamAppletCam *c)
{
	g_return_val_if_fail (GNOCAM_IS_APPLET_CAM (c), NULL);
	return c->priv->manuf;
}

const gchar *
gnocam_applet_cam_get_model (GnocamAppletCam *c)
{
	g_return_val_if_fail (GNOCAM_IS_APPLET_CAM (c), NULL);
	return c->priv->model;
}

const gchar *
gnocam_applet_cam_get_port (GnocamAppletCam *c)
{
	g_return_val_if_fail (GNOCAM_IS_APPLET_CAM (c), NULL);
	return c->priv->port;
}

gboolean
gnocam_applet_cam_get_connect_auto (GnocamAppletCam *c)
{
	g_return_val_if_fail (GNOCAM_IS_APPLET_CAM (c), FALSE);
	return c->priv->connect_auto;
}
