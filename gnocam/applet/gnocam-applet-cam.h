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

	void (* changed) (GnocamAppletCam *);
};

GType            gnocam_applet_cam_get_type (void);
GnocamAppletCam *gnocam_applet_cam_new      (guint size);

void         gnocam_applet_cam_set_manufacturer (GnocamAppletCam *,
						 const gchar *);
const gchar *gnocam_applet_cam_get_manufacturer (GnocamAppletCam *);

void gnocam_applet_cam_set_model         (GnocamAppletCam *, const gchar *);
const gchar *gnocam_applet_cam_get_model (GnocamAppletCam *);

void         gnocam_applet_cam_set_port (GnocamAppletCam *, const gchar *);
const gchar *gnocam_applet_cam_get_port (GnocamAppletCam *);

void         gnocam_applet_cam_set_name (GnocamAppletCam *, const gchar *);
const gchar *gnocam_applet_cam_get_name (GnocamAppletCam *);

void     gnocam_applet_cam_set_connect_auto (GnocamAppletCam *, gboolean);
gboolean gnocam_applet_cam_get_connect_auto (GnocamAppletCam *);

void gnocam_applet_cam_set_size (GnocamAppletCam *, guint);

void gnocam_applet_cam_connect    (GnocamAppletCam *);
void gnocam_applet_cam_disconnect (GnocamAppletCam *);

#endif
