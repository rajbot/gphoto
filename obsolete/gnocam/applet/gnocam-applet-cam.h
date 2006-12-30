#ifndef __GNOCAM_APPLET_CAM_H__
#define __GNOCAM_APPLET_CAM_H__

#include <gtk/gtkbutton.h>

#define GNOCAM_TYPE_APPLET_CAM (gnocam_applet_cam_get_type ())
#define GNOCAM_APPLET_CAM(o) (GTK_CHECK_CAST((o),GNOCAM_TYPE_APPLET_CAM,GnocamAppletCam))
#define GNOCAM_IS_APPLET_CAM(o) (GTK_CHECK_TYPE((o),GNOCAM_TYPE_APPLET_CAM))

typedef struct _GnocamAppletCam      GnocamAppletCam;
typedef struct _GnocamAppletCamPriv  GnocamAppletCamPriv;
typedef struct _GnocamAppletCamClass GnocamAppletCamClass;

struct _GnocamAppletCam
{
	GtkButton parent;

	GnocamAppletCamPriv *priv;
};

struct _GnocamAppletCamClass
{
	GtkButtonClass parent_class;
};

GType            gnocam_applet_cam_get_type (void);
GnocamAppletCam *gnocam_applet_cam_new      (guint size);

void gnocam_applet_cam_connect    (GnocamAppletCam *);
void gnocam_applet_cam_disconnect (GnocamAppletCam *);

#endif
