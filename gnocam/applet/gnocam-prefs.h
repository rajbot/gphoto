#ifndef __GNOCAM_PREFS_H__
#define __GNOCAM_PREFS_H__

#include <gtk/gtkdialog.h>
#include <bonobo/bonobo-exception.h>

#define GNOCAM_TYPE_PREFS (gnocam_prefs_get_type ())
#define GNOCAM_PREFS(o) (GTK_CHECK_CAST((o),GNOCAM_TYPE_PREFS,GnoCamPrefs))
#define GNOCAM_IS_PREFS(o) (GTK_CHECK_TYPE((o),GNOCAM_TYPE_PREFS))

typedef struct _GnoCamPrefs        GnoCamPrefs;
typedef struct _GnoCamPrefsPrivate GnoCamPrefsPrivate;
typedef struct _GnoCamPrefsClass   GnoCamPrefsClass;

struct _GnoCamPrefs
{
	GtkDialog parent;

	GnoCamPrefsPrivate *priv;
};

struct _GnoCamPrefsClass
{
	GtkDialogClass parent_class;

	/* Signals */
	void (* camera_changed)  (GnoCamPrefs *, gboolean automatic);
	void (* connect_changed) (GnoCamPrefs *, gboolean automatic);
	void (* model_changed)   (GnoCamPrefs *, const gchar *model);
	void (* port_changed)    (GnoCamPrefs *, const gchar *port);
};

GType        gnocam_prefs_get_type (void);
GnoCamPrefs *gnocam_prefs_new      (gboolean camera_automatic,
				    gboolean connect_automatic,
				    const gchar *model, const gchar *port,
				    CORBA_Environment *ev);

#endif /* __GNOCAM_APPLET_H__ */


