#ifndef __GNOCAM_CAMERA_SELECTOR_H__
#define __GNOCAM_CAMERA_SELECTOR_H__

#include <libgnomeui/gnome-dialog.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAMERA_SELECTOR             (gnocam_camera_selector_get_type ())
#define GNOCAM_CAMERA_SELECTOR(obj)		GTK_CHECK_CAST(obj, gnocam_camera_selector_get_type (), GnoCamCameraSelector)
#define GNOCAM_CAMERA_SELECTOR_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gnocam_camera_selector_get_type (), GnoCamCameraSelectorClass)
#define GNOCAM_CAMERA_IS_SELECTOR(obj)		GTK_CHECK_TYPE (obj, gnocam_camera_selector_get_type ())

typedef struct _GnoCamCameraSelector        GnoCamCameraSelector;
typedef struct _GnoCamCameraSelectorPrivate GnoCamCameraSelectorPrivate;
typedef struct _GnoCamCameraSelectorClass   GnoCamCameraSelectorClass;

struct _GnoCamCameraSelector {
	GnomeDialog dialog;

	GnoCamCameraSelectorPrivate *priv;
};

struct _GnoCamCameraSelectorClass {
	GnomeDialogClass parent_class;
};

GtkType	     gnocam_camera_selector_get_type (void);

GnomeDialog *gnocam_camera_selector_new (void);

const gchar *gnocam_camera_selector_get_name (GnoCamCameraSelector *);

END_GNOME_DECLS

#endif /* __GNOCAM_CAMERA_SELECTOR_H__ */

