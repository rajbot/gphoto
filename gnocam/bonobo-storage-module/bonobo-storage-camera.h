/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_STORAGE_CAMERA_H_
#define _BONOBO_STORAGE_CAMERA_H_

#include <bonobo/bonobo-storage.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gphoto2.h>

BEGIN_GNOME_DECLS

#define BONOBO_STORAGE_CAMERA_TYPE        (bonobo_storage_camera_get_type ())
#define BONOBO_STORAGE_CAMERA(o)          (GTK_CHECK_CAST ((o), BONOBO_STORAGE_CAMERA_TYPE, BonoboStorageCamera))
#define BONOBO_STORAGE_CAMERA_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STORAGE_CAMERA_TYPE, BonoboStorageCameraClass))
#define BONOBO_IS_STORAGE_CAMERA(o)       (GTK_CHECK_TYPE ((o), BONOBO_STORAGE_CAMERA_TYPE))
#define BONOBO_IS_STORAGE_CAMERA_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_CAMERA_TYPE))

typedef struct {
	BonoboStorage storage;

	GnomeVFSURI *uri;

	Camera *camera;
} BonoboStorageCamera;

typedef struct {
	BonoboStorageClass parent_class;
} BonoboStorageCameraClass;

GtkType         bonobo_storage_camera_get_type     (void);

END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_CAMERA_H_ */
