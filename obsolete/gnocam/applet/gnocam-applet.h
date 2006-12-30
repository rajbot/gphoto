#ifndef __GNOCAM_APPLET_H__
#define __GNOCAM_APPLET_H__

#include <panel-applet.h>

#define GNOCAM_TYPE_APPLET (gnocam_applet_get_type ())
#define GNOCAM_APPLET(o) (GTK_CHECK_CAST((o),GNOCAM_TYPE_APPLET,GnocamApplet))
#define GNOCAM_IS_APPLET(o) (GTK_CHECK_TYPE((o),GNOCAM_TYPE_APPLET))

typedef struct _GnocamApplet        GnocamApplet;
typedef struct _GnocamAppletPrivate GnocamAppletPrivate;
typedef struct _GnocamAppletClass   GnocamAppletClass;

struct _GnocamApplet
{
	GObject parent;

	GnocamAppletPrivate *priv;
};

struct _GnocamAppletClass
{
	GObjectClass parent_class;
};

GType         gnocam_applet_get_type (void);
GnocamApplet *gnocam_applet_new      (PanelApplet *applet);

#endif
