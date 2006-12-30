/* bonobo-storage-camera.h
 *
 * Copyright © 2002 Lutz Müller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _BONOBO_STORAGE_CAMERA_H_
#define _BONOBO_STORAGE_CAMERA_H_

#include <gphoto2/gphoto2-camera.h>
#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS

#define BONOBO_TYPE_STORAGE_CAMERA        (bonobo_storage_camera_get_type ())
#define BONOBO_STORAGE_CAMERA(o)          (G_TYPE_CHECK_INSTANCE_CAST((o),BONOBO_TYPE_STORAGE_CAMERA,BonoboStorageCamera))
#define BONOBO_STORAGE_CAMERA_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k),BONOBO_TYPE_STORAGE_CAMERA,BonoboStorageCameraClass))
#define BONOBO_IS_STORAGE_CAMERA(o)       (G_TYPE_CHECK_INSTANCE_TYPE((o),BONOBO_TYPE_STORAGE_CAMERA))
#define BONOBO_IS_STORAGE_CAMERA_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k),BONOBO_TYPE_STORAGE_CAMERA))

typedef struct _BonoboStorageCamera		BonoboStorageCamera;
typedef struct _BonoboStorageCameraPrivate	BonoboStorageCameraPrivate;
typedef struct _BonoboStorageCameraClass	BonoboStorageCameraClass;

struct _BonoboStorageCamera {
	BonoboObject parent;
	BonoboStorageCameraPrivate *priv;
};

struct _BonoboStorageCameraClass {
	BonoboObjectClass parent_class;

	POA_Bonobo_Storage__epv epv;
};

GType                bonobo_storage_camera_get_type (void);
BonoboStorageCamera *bonobo_storage_camera_new      (Camera *,
				const gchar *path,
				Bonobo_Storage_OpenMode mode,
				CORBA_Environment* ev);

G_END_DECLS

#endif /* _BONOBO_STORAGE_CAMERA_H_ */
