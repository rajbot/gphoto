
#ifndef _GNOCAM_CAMERA_H_
#define _GNOCAM_CAMERA_H_

#include <GnoCam.h>
#include <bonobo.h>
#include <gconf/gconf-client.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAMERA		(gnocam_camera_get_type ())
#define GNOCAM_CAMERA(o)             	(GTK_CHECK_CAST ((o), GNOCAM_TYPE_CAMERA, GnoCamCamera))
#define GNOCAM_CAMERA_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), GNOCAM_TYPE_CAMERA, GnoCamCameraClass))
#define GNOCAM_IS_CAMERA(o)		(GTK_CHECK_TYPE ((o), GNOCAM_TYPE_CAMERA))
#define GNOCAM_IS_CAMERA_CLASS(k)	(GTK_CHECK_CLASS_TYPE ((k), GNOCAM_TYPE_CAMERA))

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
	GtkVBox			parent;

	GnoCamCameraPrivate*	priv;
};

struct _GnoCamCameraClass {
	GtkVBoxClass		parent_class;
};

GtkType 	gnocam_camera_get_type			(void);
GtkWidget*	gnocam_camera_new			(const gchar* url, Bonobo_UIContainer container, GtkWindow* window, GConfClient* client, CORBA_Environment* ev);

void 		gnocam_camera_set_storage_view_mode 	(GnoCamCamera* camera, GnoCamCameraStorageViewMode mode);

void		gnocam_camera_show_menu			(GnoCamCamera* camera);
void		gnocam_camera_hide_menu			(GnoCamCamera* camera);

END_GNOME_DECLS

#endif /* _GNOCAM__CAMERA_H_ */
