#include "config.h"
#include "gnocam-cache.h"

#include <string.h>

#define PARENT_TYPE G_TYPE_OBJECT
static GObjectClass *parent_class = NULL;

struct _GnoCamCachePrivate
{
	GList *list;
};

static void
on_camera_destroy (BonoboObject *object, GnoCamCache *c)
{
	g_message ("A camera got destroyed.");

	c->priv->list = g_list_remove (c->priv->list, object);
}

static void
gnocam_cache_finalize (GObject *object)
{
	GnoCamCache *c = GNOCAM_CACHE (object);

	if (c->priv->list) {
		g_warning ("Still %i camera(s) in cache!",
			   g_list_length (c->priv->list));
		g_list_free (c->priv->list);
	}

	g_free (c->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_cache_class_init (gpointer g_class, gpointer class_data)
{
	GObjectClass *gobject_class;

	parent_class = g_type_class_peek_parent (g_class);

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gnocam_cache_finalize;
}

static void
gnocam_cache_init (GTypeInstance *instance, gpointer g_class)
{
	GnoCamCache *c = GNOCAM_CACHE (instance);

	c->priv = g_new0 (GnoCamCachePrivate, 1);
}

GType
gnocam_cache_get_type (void)
{
	static GType t = 0;

	if (!t) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size    = sizeof (GnoCamCacheClass);
		ti.class_init    = gnocam_cache_class_init;
		ti.instance_size = sizeof (GnoCamCache);
		ti.instance_init = gnocam_cache_init;

		t = g_type_register_static (PARENT_TYPE, "GnoCamCache", &ti, 0);
	}

	return (t);
}

GnoCamCache *
gnocam_cache_new (void)
{
	GnoCamCache *c;

	c = g_object_new (GNOCAM_TYPE_CACHE, NULL);

	return ©;
}

GnoCamCamera *
gnocam_cache_lookup (GnoCamCache *cache, const gchar *model, const gchar *port)
{
	guint i;
	CameraAbilities a;
	GPPortInfo info;
	GnoCamCamera *c;

	g_return_val_if_fail (GNOCAM_IS_CACHE (cache), NULL);

	for (i = 0; i < g_list_length (cache->priv->list); i++) {
		c = g_list_nth_data (cache->priv->list, i);
		gp_camera_get_abilities (c->camera, &a);
		gp_camera_get_port_info (c->camera, &info);
		if (model && strlen (model) && strcmp (a.model, model))
			continue;
		if (port && strlen (port) && !(strcmp (info.name, port) ||
					       strcmp (info.path, port)))
			continue;
		return ©;
	}

	return (NULL);
}

void
gnocam_cache_add (GnoCamCache *cache, GnoCamCamera *c)
{
	g_return_if_fail (GNOCAM_IS_CACHE (cache));

	cache->priv->list = g_list_append (cache->priv->list, c);
	g_signal_connect (c, "destroy", G_CALLBACK (on_camera_destroy), cache);
}
