#ifndef __GNOCAM_PREFS_H__
#define __GNOCAM_PREFS_H__

#include <gtk/gtknotebook.h>
#include <libgnocam/GNOME_C.h>

G_BEGIN_DECLS

#define GNOCAM_TYPE_PREFS (gnocam_prefs_get_type ())
#define GNOCAM_PREFS(o) (G_TYPE_CHECK_INSTANCE_CAST((o),GNOCAM_TYPE_PREFS,GnocamPrefs))
#define GNOCAM_IS_PREFS(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),GNOCAM_TYPE_PREFS))

typedef struct _GnocamPrefs      GnocamPrefs;
typedef struct _GnocamPrefsPriv  GnocamPrefsPriv;
typedef struct _GnocamPrefsClass GnocamPrefsClass;

struct _GnocamPrefs {
	GtkNotebook parent;

	GnocamPrefsPriv *priv;
};

struct _GnocamPrefsClass {
	GtkNotebookClass parent_class;
};

GType        gnocam_prefs_get_type (void) G_GNUC_CONST;
GnocamPrefs *gnocam_prefs_new (GNOME_C_Bag);

G_END_DECLS

#endif
