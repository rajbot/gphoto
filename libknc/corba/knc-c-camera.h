#ifndef __KNC_C_CAMERA_H__
#define __KNC_C_CAMERA_H__

#include <bonobo/bonobo-object.h>
#include <GNOME_C.h>
#include <gphoto2-port.h>

G_BEGIN_DECLS

#define KNC_C_TYPE_CAMERA (knc_c_camera_get_type ())
#define KNC_C_CAMERA(o) (G_TYPE_CHECK_INSTANCE_CAST((o),KNC_C_TYPE_CAMERA,KncCCamera))
#define KNC_C_CAMERA_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k),KNC_C_TYPE_CAMERA,KncCCameraClass))
#define KNC_C_IS_CAMERA(o) (G_TYPE_CHECK_INSTANCE_TYPE((o),KNC_C_TYPE_CAMERA))

typedef struct _KncCCamera      KncCCamera;
typedef struct _KncCCameraClass KncCCameraClass;
typedef struct _KncCCameraPriv  KncCCameraPriv;

struct _KncCCamera {
	BonoboObject parent;

	KncCCameraPriv *priv;
};

struct _KncCCameraClass {
	BonoboObjectClass parent_class;

	POA_GNOME_C_Camera__epv epv;
};

GType       knc_c_camera_get_type (void);
KncCCamera *knc_c_camera_new      (const char *, const char *, GPPort *,
				   CORBA_Environment *);

G_END_DECLS

#endif
