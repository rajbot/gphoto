
#ifndef _BONOBO_STORAGE_CAMERA_H_
#define _BONOBO_STORAGE_CAMERA_H_

#include <gphoto2.h>
#include <bonobo/bonobo-storage.h>

BEGIN_GNOME_DECLS

#define BONOBO_STORAGE_CAMERA_TYPE        (bonobo_storage_camera_get_type ())
#define BONOBO_STORAGE_CAMERA(o)          (GTK_CHECK_CAST ((o), BONOBO_STORAGE_CAMERA_TYPE, BonoboStorageCamera))
#define BONOBO_STORAGE_CAMERA_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STORAGE_CAMERA_TYPE, BonoboStorageCameraClass))
#define BONOBO_IS_STORAGE_CAMERA(o)       (GTK_CHECK_TYPE ((o), BONOBO_STORAGE_CAMERA_TYPE))
#define BONOBO_IS_STORAGE_CAMERA_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_CAMERA_TYPE))

typedef struct _BonoboStorageCamera		BonoboStorageCamera;
typedef struct _BonoboStorageCameraPrivate	BonoboStorageCameraPrivate;
typedef struct _BonoboStorageCameraClass	BonoboStorageCameraClass;

struct _BonoboStorageCamera {
	BonoboStorage 		    parent;
	BonoboStorageCameraPrivate *priv;
};

struct _BonoboStorageCameraClass {
	BonoboStorageClass 		parent_class;
};

GtkType        bonobo_storage_camera_get_type (void);
BonoboStorage *bonobo_storage_camera_new      (Camera *camera,
					       const gchar *path,
					       Bonobo_Storage_OpenMode mode,
					       CORBA_Environment* ev);

END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_CAMERA_H_ */
