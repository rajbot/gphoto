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

struct _GnocamAppletPrivate
{
	PanelApplet *applet;

	GtkBox *box;
	GtkWidget *image;

	GHashTable *cameras;
};

static GnocamAppletCam *
gnocam_applet_add_cam (GnocamApplet *a, const gchar *id)
{
	GnocamAppletCam *c;

	g_return_val_if_fail (GNOCAM_IS_APPLET (a), NULL);
	g_return_val_if_fail (id != NULL, NULL);

	c = gnocam_applet_cam_new (panel_applet_get_size (a->priv->applet));
	gtk_widget_show (GTK_WIDGET (c));
	gtk_box_pack_start (a->priv->box, GTK_WIDGET (c), FALSE, FALSE, 0);
	g_hash_table_insert (a->priv->cameras, g_strdup (id), c);

	return c;
}

static void
gnocam_applet_load_preferences (GnocamApplet *a)
{
	GConfClient *client;
	gchar *s, *k, *key;
	guint i;
	GnocamAppletCam *c;
	GSList *l;

	g_return_if_fail (GNOCAM_IS_APPLET (a));

	client = gconf_client_get_default ();
	l = gconf_client_all_dirs (client, "/desktop/gnome/cameras", NULL);
	for (i = 0; i < g_slist_length (l); i++) {

		/* Create the widget */
		c = gnocam_applet_add_cam (a, g_slist_nth_data (l, i));

		/* Load the preferences into the widget. */
		key = g_slist_nth_data (l, i);
		k = g_strdup_printf ("%s/name", key);
		s = gconf_client_get_string (client, k, NULL);
		gnocam_applet_cam_set_name (c, s);
		g_free (s); g_free (k);
		k = g_strdup_printf ("%s/manufacturer", key);
		s = gconf_client_get_string (client, k, NULL);
		gnocam_applet_cam_set_manufacturer (c, s);
		g_free (s); g_free (k);
		k = g_strdup_printf ("%s/model", key);
		s = gconf_client_get_string (client, k, NULL);
		gnocam_applet_cam_set_model (c, s);
		g_free (s); g_free (k);
		k = g_strdup_printf ("%s/port", key);
		s = gconf_client_get_string (client, k, NULL);
		gnocam_applet_cam_set_port (c, s);
		g_free (s); g_free (k);
		k = g_strdup_printf ("%s/connect_auto", key);
		gnocam_applet_cam_set_connect_auto (c,
				gconf_client_get_bool (client, k, NULL));
	}
	g_object_unref (client);
}

static void
gnocam_applet_finalize (GObject *object)
{
	GnocamApplet *a = GNOCAM_APPLET (object);

	g_hash_table_destroy (a->priv->cameras);
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
foreach_func (gpointer key, gpointer value, gpointer user_data)
{
	guint *size = (guint *) user_data;

	g_return_if_fail (GNOCAM_IS_APPLET_CAM (value));
	g_return_if_fail (size != NULL);

	gnocam_applet_cam_set_size (GNOCAM_APPLET_CAM (value), *size);
}

static void
on_change_size (PanelApplet *applet, guint size, GnocamApplet *a)
{
	g_return_if_fail (GNOCAM_IS_APPLET (a));

	g_hash_table_foreach (a->priv->cameras, foreach_func, &size);
}

static void
on_change_background (PanelApplet *applet, PanelAppletBackgroundType type,
		      GdkColor *color, GdkPixmap *frame, GnocamApplet *a)
{
	/* Nothing here yet */
}

static void
on_move_focus_out_of_applet (PanelApplet *frame, GtkDirectionType direction,
			     GnocamApplet *a)
{
	/* Nothing here yet */
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
gnocam_applet_prefs_cb (BonoboUIComponent *uic, GnocamApplet *a,
			const char *verbname)
{
	GError *e = NULL;
	gchar *argv[] = {"gnocam-capplet", NULL};

	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		       NULL, NULL, NULL, &e);
	if (e) {
		g_warning ("Could not start capplet: %s", e->message);
		g_error_free (e);
	}
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
	BONOBO_UI_UNSAFE_VERB ("Preferences", gnocam_applet_prefs_cb),
	BONOBO_UI_UNSAFE_VERB ("About", gnocam_applet_about_cb),
	BONOBO_UI_VERB_END
};

static void
notify_func (GConfClient *client, guint cnxn_id, GConfEntry *entry,
	     gpointer user_data)
{
	GnocamApplet *a = GNOCAM_APPLET (user_data);
	GnocamAppletCam *c;
	const gchar *b = g_basename (entry->key);
	gchar *id;

	if (!strcmp (entry->key, "/desktop/gnome/cameras")) return;

	g_message ("Key: '%s'", entry->key);
	if (!entry->value) {
		c = g_hash_table_lookup (a->priv->cameras, entry->key);
		if (c) {
			gtk_object_destroy (GTK_OBJECT (c));
			g_hash_table_remove (a->priv->cameras, entry->key);
		}
	} else {
		if (strcmp (b, "name") && strcmp (b, "manufacturer") &&
		    strcmp (b, "model") && strcmp (b, "port")) return;
		
		id = g_path_get_dirname (entry->key);
		c = g_hash_table_lookup (a->priv->cameras, id);
		if (!c) c = gnocam_applet_add_cam (a, id);
		if (!strcmp (b, "name"))
			gnocam_applet_cam_set_name (c,
				gconf_value_get_string (entry->value));
		if (!strcmp (b, "model"))
			gnocam_applet_cam_set_model (c,
				gconf_value_get_string (entry->value));
		if (!strcmp (b, "manufacturer"))
			gnocam_applet_cam_set_manufacturer (c,
				gconf_value_get_string (entry->value));
		if (!strcmp (b, "port"))
			gnocam_applet_cam_set_port (c,
				gconf_value_get_string (entry->value));
		g_free (id);
	}
}

GnocamApplet *
gnocam_applet_new (PanelApplet *applet)
{
	GnocamApplet *a;
	GtkWidget *w;
	GConfClient *client;

	g_return_val_if_fail (PANEL_IS_APPLET (applet), NULL);

	a = g_object_new (GNOCAM_TYPE_APPLET, NULL);
	a->priv->applet = applet;

	client = gconf_client_get_default ();
	gconf_client_notify_add (client, "/desktop/gnome/cameras", 
				 notify_func, a, NULL, NULL);
	g_object_unref (G_OBJECT (client));

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

	gnocam_applet_load_preferences (a);

	return (a);
}

static void
gnocam_applet_init (GTypeInstance *instance, gpointer g_class)
{
        GnocamApplet *a = GNOCAM_APPLET (instance);

	a->priv = g_new0 (GnocamAppletPrivate, 1);
	a->priv->cameras = g_hash_table_new (g_str_hash, g_str_equal);
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
