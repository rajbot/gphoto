
#ifndef _GNOCAM_CONTROL_H_
#define _GNOCAM_CONTROL_H_

#include <gphoto2.h>
#include <bonobo.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */
 
#define GNOCAM_CONTROL_TYPE           (gnocam_control_get_type ())
#define GNOCAM_CONTROL(o)             (GTK_CHECK_CAST ((o), GNOCAM_CONTROL_TYPE, GnoCamControl))
#define GNOCAM_CONTROL_CLASS(k)       (GTK_CHECK_CLASS_CAST((k), GNOCAM_CONTROL_TYPE, GnoCamControlClass))

#define GNOCAM_IS_CONTROL(o)          (GTK_CHECK_TYPE ((o), GNOCAM_CONTROL_TYPE))
#define GNOCAM_IS_CONTROL_CLASS(k)    (GTK_CHECK_CLASS_TYPE ((k), GNOCAM_CONTROL_TYPE))

typedef struct _GnoCamControl		GnoCamControl;
typedef struct _GnoCamControlPrivate	GnoCamControlPrivate;
typedef struct _GnoCamControlClass    	GnoCamControlClass;

struct _GnoCamControl {
	BonoboControl 		control;

	GnoCamControlPrivate*	priv;

	Camera* 		camera;

	CameraWidget*		config_camera;
	CameraWidget*		config_folder;
	CameraWidget*		config_file;

	gchar*			path;
};

struct _GnoCamControlClass {
	BonoboControlClass parent_class;
};

GtkType         gnocam_control_get_type	(void);
GnoCamControl*	gnocam_control_new	(BonoboMoniker *moniker, const Bonobo_ResolveOptions *options, CORBA_Environment* ev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GNOCAM__CONTROL_H_ */
