
#ifndef _BONOBO_MONIKER_CAMERA_H_
#define _BONOBO_MONIKER_CAMERA_H_

#include <bonobo.h>

BEGIN_GNOME_DECLS

#define BONOBO_MONIKER_CAMERA_TYPE        (bonobo_moniker_http_get_type ())
#define BONOBO_MONIKER_CAMERA(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_CAMERA_TYPE, BonoboMonikerCAMERA))
#define BONOBO_MONIKER_CAMERA_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_CAMERA_TYPE, BonoboMonikerCAMERAClass))
#define BONOBO_IS_MONIKER_CAMERA(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_CAMERA_TYPE))
#define BONOBO_IS_MONIKER_CAMERA_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_CAMERA_TYPE))

typedef struct _BonoboMonikerCAMERA        BonoboMonikerCAMERA;

struct _BonoboMonikerCAMERA {
	BonoboMoniker stream;
};

typedef struct {
	BonoboMonikerClass parent_class;
} BonoboMonikerCAMERAClass;

GtkType        bonobo_moniker_camera_get_type (void);
BonoboMoniker *bonobo_moniker_camera_new      (void);
	
END_GNOME_DECLS

#endif /* _BONOBO_MONIKER_CAMERA_H_ */
