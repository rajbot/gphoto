#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <gconf/gconf-client.h>

#include "gnocam-shortcut-bar.h"

#define PARENT_TYPE E_TYPE_SHORTCUT_BAR

static GnoCamShortcutBarClass* parent_class = NULL;

struct _GnoCamShortcutBarPrivate {
};

static void
destroy (GtkObject *object)
{
	GnoCamShortcutBar* s;
	
	g_warning ("BEGIN: destroy");
	
	s = GNOCAM_SHORTCUT_BAR (object);
	g_free (s->priv);

	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
class_init (GnoCamShortcutBarClass* klass)
{
	GtkObjectClass *object_class;

	g_warning ("BEGIN: class_init");

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = destroy;
}

static void
init (GnoCamShortcutBar* shortcut_bar)
{
	GnoCamShortcutBarPrivate* priv;

	priv = g_new (GnoCamShortcutBarPrivate, 1);
	shortcut_bar->priv = priv;
}

GtkWidget *
gnocam_shortcut_bar_new (void)
{
	GtkWidget*	new;

	new = gtk_type_new (gnocam_shortcut_bar_get_type ());
	return (new);
}

void
gnocam_shortcut_bar_refresh (GnoCamShortcutBar* bar)
{
	GConfClient*	client;
	EShortcutModel* model;
	gint            i, group;

	client = gconf_client_get_default ();
	g_return_if_fail (client);
	
	model = e_shortcut_model_new ();
	e_shortcut_bar_set_model (E_SHORTCUT_BAR (bar), model);
	
	group = e_shortcut_model_add_group (model, -1, _("Cameras"));
	
        for (i = 0; ; i++) {
                gchar* 		path;

                /* Check each entry in gconf's database. */
                path = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);
		if (gconf_client_dir_exists (client, path, NULL)) {
			gchar*	tmp;
                        gchar*  name;

                        tmp = g_strconcat (path, "/name", NULL);
                        name = gconf_client_get_string (client, tmp, NULL);
                        g_free (tmp);

			tmp = g_strconcat ("camera://", name, "/", NULL);
			e_shortcut_model_add_item (model, group, -1, tmp, name);
			g_free (tmp);

			g_free (path);

		} else {
			g_free (path);
			break;
		}
	}
}

GtkType gnocam_shortcut_bar_get_type ()
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnoCamShortcutBar",
			sizeof (GnoCamShortcutBar),
			sizeof (GnoCamShortcutBarClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};
		type = gtk_type_unique (PARENT_TYPE, &info);
	}
	return type;
}

