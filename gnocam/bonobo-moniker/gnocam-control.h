
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
};

struct _GnoCamControlClass {
	BonoboControlClass parent_class;
};

GtkType gnocam_control_get_type		(void);
void	gnocam_control_complete 	(GnoCamControl* control, BonoboMoniker* moniker);
Camera*	gnocam_control_get_camera 	(GnoCamControl* control);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GNOCAM__CONTROL_H_ */
