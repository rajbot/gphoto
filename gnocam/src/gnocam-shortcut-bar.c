#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <gconf/gconf-client.h>
#include <gal/util/e-util.h>

#include "gnocam-shortcut-bar.h"

#define PARENT_TYPE E_TYPE_SHORTCUT_BAR
static GnoCamShortcutBarClass* parent_class = NULL;

struct _GnoCamShortcutBarPrivate {
	GConfClient*	client;
	guint		cnxn;

	EShortcutModel*	model;

	gint		group;
};

/************************/
/* GConf - notification */
/************************/

static void
on_cameras_changed (GConfClient *client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GnoCamShortcutBar*	bar;
        gint                    i, item;
        GSList*                 list;

        g_return_if_fail (GNOCAM_IS_SHORTCUT_BAR (user_data));
	bar = GNOCAM_SHORTCUT_BAR (user_data);
        list = gconf_client_get_list (bar->priv->client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);

        /* Delete deleted cameras */
	for (item = 0; item < e_shortcut_model_get_num_items (bar->priv->model, bar->priv->group); ) {
                gchar*  name = NULL;
		gchar*	url = NULL;

		e_shortcut_model_get_item_info (bar->priv->model, bar->priv->group, item, &url, &name);
		g_free (url);
                for (i = 0; i < g_slist_length (list); i++) {
                        if (!strcmp (g_slist_nth_data (list, i), name)) break;
                }
		g_free (name);
                if (i == g_slist_length (list)) e_shortcut_model_remove_item (bar->priv->model, bar->priv->group, item);
                else item++;
        }

        /* Append the new cameras */
        for (i = 0; i < g_slist_length (list); i += 3) {
                for (item = 0; item < e_shortcut_model_get_num_items (bar->priv->model, bar->priv->group); item++) {
                        gchar* name = NULL;
			gchar* url = NULL;

                        e_shortcut_model_get_item_info (bar->priv->model, bar->priv->group, item, &url, &name);
			g_free (url);
			if (!strcmp (g_slist_nth_data (list, i), name)) {
				g_free (name); 
				break;
			}
			g_free (name);
                }
                if (item == e_shortcut_model_get_num_items (bar->priv->model, bar->priv->group)) {
			gchar*	url;

			url = g_strconcat ("camera://", g_slist_nth_data (list, i), "/", NULL);
			e_shortcut_model_add_item (bar->priv->model, bar->priv->group, -1, url, g_slist_nth_data (list, i));
			g_free (url);
                }
        }

        /* Free the list */
        for (i = 0; i < g_slist_length (list); i++) g_free (g_slist_nth_data (list, i));
        g_slist_free (list);
}

/***************************/
/* E-Shortcut-Bar specific */
/***************************/

static void
gnocam_shortcut_bar_destroy (GtkObject *object)
{
	GnoCamShortcutBar* bar;
	
	bar = GNOCAM_SHORTCUT_BAR (object);

	gconf_client_notify_remove (bar->priv->client, bar->priv->cnxn);
	gtk_object_unref (GTK_OBJECT (bar->priv->client));
	
	g_free (bar->priv);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_shortcut_bar_class_init (GnoCamShortcutBarClass* klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_shortcut_bar_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_shortcut_bar_init (GnoCamShortcutBar* shortcut_bar)
{
	GnoCamShortcutBarPrivate* priv;

	priv = g_new (GnoCamShortcutBarPrivate, 1);
	shortcut_bar->priv = priv;
}

GtkWidget *
gnocam_shortcut_bar_new (void)
{
	GnoCamShortcutBar*	new;

	new = gtk_type_new (gnocam_shortcut_bar_get_type ());
	new->priv->client = gconf_client_get_default ();
	new->priv->cnxn = gconf_client_notify_add (new->priv->client, "/apps/" PACKAGE "/cameras", on_cameras_changed, new, NULL, NULL);

	/* Create the group for the cameras */
	new->priv->model = e_shortcut_model_new ();
        e_shortcut_bar_set_model (E_SHORTCUT_BAR (new), new->priv->model);
	new->priv->group = e_shortcut_model_add_group (new->priv->model, -1, _("Cameras"));

	/* Add the cameras */
	on_cameras_changed (new->priv->client, 0, NULL, new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_shortcut_bar, "GnoCamShortcutBar", GnoCamShortcutBar, gnocam_shortcut_bar_class_init, gnocam_shortcut_bar_init, PARENT_TYPE)

