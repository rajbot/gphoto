/* bonobo-stream-camera.h
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

#ifndef _BONOBO_STREAM_CAMERA_H_
#define _BONOBO_STREAM_CAMERA_H_

#include <gphoto2/gphoto2-camera.h>
#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS

#define BONOBO_TYPE_STREAM_CAMERA        (bonobo_stream_camera_get_type ())
#define BONOBO_STREAM_CAMERA(o)          (G_TYPE_CHECK_INSTANCE_CAST((o),BONOBO_TYPE_STREAM_CAMERA,BonoboStreamCamera))
#define BONOBO_STREAM_CAMERA_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k),BONOBO_TYPE_STREAM_CAMERA,BonoboStreamCameraClass))
#define BONOBO_IS_STREAM_CAMERA(o)       (G_TYPE_CHECK_INSTANCE_TYPE((o),BONOBO_TYPE_STREAM_CAMERA))
#define BONOBO_IS_STREAM_CAMERA_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k),BONOBO_TYPE_STREAM_CAMERA))

typedef struct _BonoboStreamCamera             BonoboStreamCamera;
typedef struct _BonoboStreamCameraPrivate      BonoboStreamCameraPrivate;
typedef struct _BonoboStreamCameraClass        BonoboStreamCameraClass;

struct _BonoboStreamCamera {
        BonoboObject parent;
        BonoboStreamCameraPrivate *priv;
};

struct _BonoboStreamCameraClass {
        BonoboObjectClass parent_class;

	POA_Bonobo_Stream__epv epv;
};

GType               bonobo_stream_camera_get_type (void);
BonoboStreamCamera *bonobo_stream_camera_new   (Camera *camera, 
				const gchar *dirname, const gchar *filename, 
				gint flags, CORBA_Environment *ev);

G_END_DECLS

#endif /* _BONOBO_STREAM_CAMERA_H_ */
