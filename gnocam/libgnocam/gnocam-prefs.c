#include <config.h>
#include "gnocam-prefs.h"

#include <bonobo/bonobo-object.h>

struct _GnocamPrefsPriv {
	GNOME_C_Bag bag;
};

#define PARENT_TYPE GTK_TYPE_NOTEBOOK
static GtkNotebookClass *parent_class;

static void
gnocam_prefs_finalize (GObject *o)
{
	GnocamPrefs *p = GNOCAM_PREFS (o);

	bonobo_object_release_unref (p->priv->bag, NULL);
	g_free (p->priv);

	G_OBJECT_CLASS (parent_class)->finalize (o);
}

static void
gnocam_prefs_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_prefs_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gnocam_prefs_init (GTypeInstance *instance, gpointer g_class)
{
	GnocamPrefs *p = GNOCAM_PREFS (instance);

	p->priv = g_new0 (GnocamPrefsPriv, 1);
}

GType
gnocam_prefs_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static const GTypeInfo ti = {
			sizeof (GnocamPrefsClass), NULL, NULL,
			gnocam_prefs_class_init, NULL, NULL,
			sizeof (GnocamPrefs), 0, gnocam_prefs_init
		};
		t = g_type_register_static (PARENT_TYPE, "GnocamPrefs", &ti, 0);
	}
	return t;
}

GnocamPrefs *
gnocam_prefs_new (GNOME_C_Bag bag)
{
	GnocamPrefs *p = g_object_new (GNOCAM_TYPE_PREFS, NULL);

	p->priv->bag = bonobo_object_dup_ref (bag, NULL);

	return p;
}
