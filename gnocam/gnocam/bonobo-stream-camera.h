
#ifndef _BONOBO_STREAM_CAMERA_H_
#define _BONOBO_STREAM_CAMERA_H_

#include <gphoto2.h>
#include <bonobo/bonobo-stream.h>

BEGIN_GNOME_DECLS

#define BONOBO_STREAM_CAMERA_TYPE        (bonobo_stream_camera_get_type ())
#define BONOBO_STREAM_CAMERA(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_CAMERA_TYPE, BonoboStreamCamera))
#define BONOBO_STREAM_CAMERA_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_CAMERA_TYPE, BonoboStreamCameraClass))
#define BONOBO_IS_STREAM_CAMERA(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_CAMERA_TYPE))
#define BONOBO_IS_STREAM_CAMERA_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_CAMERA_TYPE))

typedef struct _BonoboStreamCamera		BonoboStreamCamera;
typedef struct _BonoboStreamCameraPrivate 	BonoboStreamCameraPrivate;
typedef struct _BonoboStreamCameraClass		BonoboStreamCameraClass;

struct _BonoboStreamCamera {
	BonoboStream 		   stream;
	BonoboStreamCameraPrivate *priv;
};

struct _BonoboStreamCameraClass {
	BonoboStreamClass parent_class;
};

GtkType       bonobo_stream_camera_get_type (void);
BonoboStream *bonobo_stream_camera_new   (Camera *camera, 
					  const gchar *dirname, 
					  const gchar *filename, 
					  gint flags, 
					  CORBA_Environment *ev);

END_GNOME_DECLS

#endif /* _BONOBO_STREAM_CAMERA_H_ */
