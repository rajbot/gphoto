#ifndef __GNOCAM_CACHE_H__
#define __GNOCAM_CACHE_H__

#include <gnocam-camera.h>

#define GNOCAM_TYPE_CACHE (gnocam_cache_get_type ())
#define GNOCAM_CACHE(o) (G_TYPE_CHECK_INSTANCE_CAST((o),GNOCAM_TYPE_CACHE,GnoCamCache))
#define GNOCAM_IS_CACHE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),GNOCAM_TYPE_CACHE))

typedef struct _GnoCamCache        GnoCamCache;
typedef struct _GnoCamCachePrivate GnoCamCachePrivate;
typedef struct _GnoCamCacheClass   GnoCamCacheClass;

struct _GnoCamCache {
	GObject parent;

	GnoCamCachePrivate *priv;
};

struct _GnoCamCacheClass {
	GObjectClass parent_class;
};

GType        gnocam_cache_get_type (void);
GnoCamCache *gnocam_cache_new      (void);

void          gnocam_cache_add    (GnoCamCache *, GnoCamCamera *);
GnoCamCamera *gnocam_cache_lookup (GnoCamCache *, const gchar *model,
						  const gchar *port);

#endif
