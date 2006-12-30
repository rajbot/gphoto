
#ifndef _GNOCAM_CAMERA_H_
#define _GNOCAM_CAMERA_H_

#include <gphoto2.h>

#include <GnoCam.h>

#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS

#define GNOCAM_TYPE_CAMERA	  (gnocam_camera_get_type ())
#define GNOCAM_CAMERA(o)          (G_TYPE_CHECK_INSTANCE_CAST((o),GNOCAM_TYPE_CAMERA,GnoCamCamera))
#define GNOCAM_CAMERA_CLASS(k)	  (G_TYPE_CHECK_CLASS_CAST((k),GNOCAM_TYPE_CAMERA,GnoCamCameraClass))
#define GNOCAM_IS_CAMERA(o)	  (G_TYPE_CHECK_INSTANCE_TYPE((o),GNOCAM_TYPE_CAMERA))
#define GNOCAM_IS_CAMERA_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k),GNOCAM_TYPE_CAMERA))

typedef struct _GnoCamCamera		GnoCamCamera;
typedef struct _GnoCamCameraPrivate	GnoCamCameraPrivate;
typedef struct _GnoCamCameraClass    	GnoCamCameraClass;

struct _GnoCamCamera {
	BonoboObject parent;

	Camera *camera;

	GnoCamCameraPrivate *priv;
};

struct _GnoCamCameraClass {
	BonoboObjectClass parent_class;

	POA_GNOME_Camera__epv epv;
};

GType         gnocam_camera_get_type (void);
GnoCamCamera *gnocam_camera_new	     (Camera *camera, CORBA_Environment *ev);

G_END_DECLS

#endif /* _GNOCAM__CAMERA_H_ */
