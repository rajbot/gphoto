
#ifndef _GNOCAM_CAPTURE_H_
#define _GNOCAM_CAPGURE_H_

#include <gphoto2.h>
#include <bonobo.h>
#include <gconf/gconf-client.h>

BEGIN_GNOME_DECLS

#define GNOCAM_TYPE_CAPTURE		(gnocam_capture_get_type ())
#define GNOCAM_CAPTURE(obj)		(GTK_CHECK_CAST ((obj), GNOCAM_TYPE_CAPTURE, GnoCamCapture))
#define GNOCAM_CAPGURE_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOCAM_TYPE_CAPTURE, GnoCamCaptureClass))
#define GNOCAM_IS_CAPTURE(obj)		(GTK_CHECK_TYPE ((obj), GNOCAM_TYPE_CAPTURE))
#define GNOCAM_IS_CAPTURE_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GNOCAM_TYPE_CAPTURE))

typedef struct _GnoCamCapture		GnoCamCapture;
typedef struct _GnoCamCapturePrivate	GnoCamCapturePrivate;
typedef struct _GnoCamCaptureClass	GnoCamCaptureClass;

struct _GnoCamCapture {
	BonoboWindow		parent;
	GnoCamCapturePrivate*	priv;
};

struct _GnoCamCaptureClass {
	BonoboWindowClass	parent_class;
};

GtkType		gnocam_capture_get_type 	(void);
GtkWidget*	gnocam_capture_new 		(Camera* camera, GConfClient* client, GtkWindow* window);

END_GNOME_DECLS

#endif /* _GNOCAM_CAPTURE_H_ */

