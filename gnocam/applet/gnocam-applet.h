#ifndef __GNOCAM_APPLET_H__
#define __GNOCAM_APPLET_H__

#include <panel-applet.h>

#define GNOCAM_TYPE_APPLET (gnocam_applet_get_type ())
#define GNOCAM_APPLET(o) (GTK_CHECK_CAST((o),GNOCAM_TYPE_APPLET,GnoCamApplet))
#define GNOCAM_IS_APPLET(o) (GTK_CHECK_TYPE((o),GNOCAM_TYPE_APPLET))

typedef struct _GnoCamApplet        GnoCamApplet;
typedef struct _GnoCamAppletPrivate GnoCamAppletPrivate;
typedef struct _GnoCamAppletClass   GnoCamAppletClass;

struct _GnoCamApplet
{
	PanelApplet parent;

	GnoCamAppletPrivate *priv;
};

struct _GnoCamAppletClass
{
	PanelAppletClass parent_class;
};

GType gnocam_applet_get_type (void);

void  gnocam_applet_create_ui (GnoCamApplet *);

#endif
