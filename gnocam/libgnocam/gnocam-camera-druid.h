
#ifndef __GNOCAM_CAMERA_DRUID_H__
#define __GNOCAM_CAMERA_DRUID_H__

#include <gnome.h>
#include <gconf/gconf-client.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAMERA_DRUID             	(gnocam_camera_druid_get_type ())
#define GNOCAM_CAMERA_DRUID(obj)             	GTK_CHECK_CAST (obj, gnocam_camera_druid_get_type (), GnoCamCameraDruid)
#define GNOCAM_CAMERA_DRUID_CLASS(klass)     	GTK_CHECK_CLASS_CAST (klass, gnocam_camera_druid_get_type (), GnoCamCameraDruidClass)
#define GNOCAM_IS_CAMERA_DRUID(obj)          	GTK_CHECK_TYPE (obj, gnocam_camera_druid_get_type ())
#define GNOCAM_IS_CAMERA_DRUID_CLASS(klass)  	GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_CAMERA_DRUID)

typedef struct _GnoCamCameraDruid               GnoCamCameraDruid;
typedef struct _GnoCamCameraDruidPrivate        GnoCamCameraDruidPrivate;
typedef struct _GnoCamCameraDruidClass          GnoCamCameraDruidClass;

struct _GnoCamCameraDruid
{
	GtkWindow			parent;

	GnoCamCameraDruidPrivate*	priv;
};

struct _GnoCamCameraDruidClass
{
	GtkWindowClass			parent_class;
};

GtkType		gnocam_camera_druid_get_type	(void);
GtkWidget*	gnocam_camera_druid_new	(GConfClient* client, GtkWindow* window);

END_GNOME_DECLS

#endif /* __GNOCAM_CAMERA_DRUID_H__ */


