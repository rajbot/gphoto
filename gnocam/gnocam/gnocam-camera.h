
#ifndef _GNOCAM_CAMERA_H_
#define _GNOCAM_CAMERA_H_

#include <gphoto2.h>
#include "GnoCam.h"
#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAMERA		(gnocam_camera_get_type ())
#define GNOCAM_CAMERA(o)             	(GTK_CHECK_CAST ((o), GNOCAM_TYPE_CAMERA, GnoCamCamera))
#define GNOCAM_CAMERA_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), GNOCAM_TYPE_CAMERA, GnoCamCameraClass))
#define GNOCAM_IS_CAMERA(o)		(GTK_CHECK_TYPE ((o), GNOCAM_TYPE_CAMERA))
#define GNOCAM_IS_CAMERA_CLASS(k)	(GTK_CHECK_CLASS_TYPE ((k), GNOCAM_TYPE_CAMERA))

typedef struct _GnoCamCamera		GnoCamCamera;
typedef struct _GnoCamCameraPrivate	GnoCamCameraPrivate;
typedef struct _GnoCamCameraClass    	GnoCamCameraClass;

struct _GnoCamCamera {
	BonoboXObject base;

	GnoCamCameraPrivate *priv;
};

struct _GnoCamCameraClass {
	BonoboXObjectClass parent_class;

	POA_GNOME_Camera__epv epv;
};

GtkType       gnocam_camera_get_type (void);
GnoCamCamera* gnocam_camera_new	     (Camera *camera, CORBA_Environment *ev);

END_GNOME_DECLS

#endif /* _GNOCAM__CAMERA_H_ */
