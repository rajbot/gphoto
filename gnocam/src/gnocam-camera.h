
#ifndef _GNOCAM_CAMERA_H_
#define _GNOCAM_CAMERA_H_

#include <GnoCam.h>
#include <bonobo.h>

BEGIN_GNOME_DECLS

#define GNOCAM_CAMERA_TYPE           (gnocam_camera_get_type ())
#define GNOCAM_CAMERA(o)             (GTK_CHECK_CAST ((o), GNOCAM_CAMERA_TYPE, GnoCamCamera))
#define GNOCAM_CAMERA_CLASS(k)       (GTK_CHECK_CLASS_CAST((k), GNOCAM_CAMERA_TYPE, GnoCamCameraClass))
#define GNOCAM_IS_CAMERA(o)          (GTK_CHECK_TYPE ((o), GNOCAM_CAMERA_TYPE))
#define GNOCAM_IS_CAMERA_CLASS(k)    (GTK_CHECK_CLASS_TYPE ((k), GNOCAM_CAMERA_TYPE))

typedef struct _GnoCamCamera		GnoCamCamera;
typedef struct _GnoCamCameraPrivate	GnoCamCameraPrivate;
typedef struct _GnoCamCameraClass    	GnoCamCameraClass;

enum _GnoCamCameraStorageViewMode {
	GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN,
	GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT,
	GNOCAM_CAMERA_STORAGE_VIEW_MODE_STICKY
};
typedef enum _GnoCamCameraStorageViewMode	GnoCamCameraStorageViewMode;
			

struct _GnoCamCamera {
	BonoboXObject           parent;

	GnoCamCameraPrivate*	priv;
};

struct _GnoCamCameraClass {
	BonoboXObjectClass              parent_class;

	POA_GNOME_GnoCam_camera__epv	epv;
};

GtkType 	gnocam_camera_get_type			(void);
GnoCamCamera*	gnocam_camera_new			(const gchar* url, Bonobo_UIContainer container);

void 		gnocam_camera_set_storage_view_mode 	(GnoCamCamera* camera, GnoCamCameraStorageViewMode mode);
GtkWidget*	gnocam_camera_get_widget 		(GnoCamCamera* camera);

END_GNOME_DECLS

#endif /* _GNOCAM__CAMERA_H_ */
