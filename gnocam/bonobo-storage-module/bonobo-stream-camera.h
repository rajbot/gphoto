/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_STREAM_CAMERA_H_
#define _BONOBO_STREAM_CAMERA_H_

#include <bonobo/bonobo-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gphoto2.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboStreamCamera BonoboStreamCamera;

#define BONOBO_STREAM_CAMERA_TYPE        (bonobo_stream_camera_get_type ())
#define BONOBO_STREAM_CAMERA(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_CAMERA_TYPE, BonoboStreamCamera))
#define BONOBO_STREAM_CAMERA_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_CAMERA_TYPE, BonoboStreamCameraClass))
#define BONOBO_IS_STREAM_CAMERA(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_CAMERA_TYPE))
#define BONOBO_IS_STREAM_CAMERA_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_CAMERA_TYPE))

typedef struct _BonoboStreamCameraPrivate BonoboStreamCameraPrivate;

struct _BonoboStreamCamera {
	BonoboStream stream;

	GnomeVFSURI* uri;

	Camera *camera;

	gchar *data;
	long size;
	long position;
};

typedef struct {
	BonoboStreamClass parent_class;
} BonoboStreamCameraClass;

GtkType       bonobo_stream_camera_get_type (void);
BonoboStream *bonobo_stream_camera_open     (const char *path,
					     gint flags, gint mode,
					     CORBA_Environment *ev);
	
END_GNOME_DECLS

#endif /* _BONOBO_STREAM_CAMERA_H_ */
