
#ifndef _GNOCAM_CONTROL_H_
#define _GNOCAM_CONTROL_H_

#include <gphoto2.h>
#include <bonobo.h>

BEGIN_GNOME_DECLS

#define GNOCAM_CONTROL_TYPE           (gnocam_control_get_type ())
#define GNOCAM_CONTROL(o)             (GTK_CHECK_CAST ((o), GNOCAM_CONTROL_TYPE, GnoCamControl))
#define GNOCAM_CONTROL_CLASS(k)       (GTK_CHECK_CLASS_CAST((k), GNOCAM_CONTROL_TYPE, GnoCamControlClass))
#define GNOCAM_IS_CONTROL(o)          (GTK_CHECK_TYPE ((o), GNOCAM_CONTROL_TYPE))
#define GNOCAM_IS_CONTROL_CLASS(k)    (GTK_CHECK_CLASS_TYPE ((k), GNOCAM_CONTROL_TYPE))

typedef struct _GnoCamControl		GnoCamControl;
typedef struct _GnoCamControlPrivate	GnoCamControlPrivate;
typedef struct _GnoCamControlClass    	GnoCamControlClass;

enum _GnoCamControlStorageViewMode {
	GNOCAM_CONTROL_STORAGE_VIEW_MODE_HIDDEN,
	GNOCAM_CONTROL_STORAGE_VIEW_MODE_TRANSIENT,
	GNOCAM_CONTROL_STORAGE_VIEW_MODE_STICKY
};
typedef enum _GnoCamControlStorageViewMode	GnoCamControlStorageViewMode;
			

struct _GnoCamControl {
	BonoboControl 		control;

	GnoCamControlPrivate*	priv;
};

struct _GnoCamControlClass {
	BonoboControlClass parent_class;
};

GtkType 	gnocam_control_get_type		(void);
GnoCamControl*	gnocam_control_new		(BonoboMoniker* moniker, CORBA_Environment* ev);

Camera*		gnocam_control_get_camera 		(GnoCamControl* control);
void 		gnocam_control_set_storage_view_mode 	(GnoCamControl* control, GnoCamControlStorageViewMode mode);


END_GNOME_DECLS

#endif /* _GNOCAM__CONTROL_H_ */
