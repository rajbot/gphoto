/* gnocam-camera.c
 *
 * Copyright (C) 2002 Lutz Müller <lutz@users.sourceforge.net>
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
#include <config.h>
#include "gnocam-camera.h"

#include <stdlib.h>
#include <string.h>

#include <gphoto2.h>

#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-exception.h>

#include "bonobo-storage-camera.h"

#define PARENT_TYPE BONOBO_TYPE_OBJECT
static BonoboObjectClass *parent_class;

struct _GnoCamCameraPrivate
{
	BonoboEventSource *event_source;
};

#define CR(result,ev) G_STMT_START{\
	gint r = result;\
	if (r < 0) {\
		switch (r) {\
		case GP_ERROR_NOT_SUPPORTED:\
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,\
				ex_GNOME_Camera_NotSupported, NULL);\
			break;\
		case GP_ERROR_IO:\
		default:\
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,\
				ex_GNOME_Camera_IOError, NULL);\
			break;\
		}\
	}\
}G_STMT_END

#if 0
static CORBA_char *
impl_GNOME_Camera_getInfo (PortableServer_Servant servant, 
		           CORBA_Environment *ev)
{
	GnoCamCamera *c;
	CameraText text;

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));
	CR (gp_camera_get_manual (c->camera, &text, NULL), ev);
	if (BONOBO_EX (ev))
		return (NULL);

	return (g_strdup (text.text));
}
#endif

#if 0
static Bonobo_Stream
impl_GNOME_Camera_capturePreview (PortableServer_Servant servant, 
				  CORBA_Environment *ev)
{
	BonoboStream *stream;
	CameraFile *file;
	GnoCamCamera *c;

	g_message ("impl_GNOME_Camera_capturePreview");

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));
	CR (gp_file_new (&file), ev);
	CR (gp_camera_capture_preview (c->camera, file), ev);
	if (BONOBO_EX (ev)) {
		gp_file_unref (file);
		g_message ("Returning...");
		return (CORBA_OBJECT_NIL);
	}

	stream = bonobo_stream_mem_create (file->data, file->size, TRUE, FALSE);
	gp_file_unref (file);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev));
}
#endif

static CORBA_char *
impl_GNOME_Camera_captureImage (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	GnoCamCamera *c;
	CameraFilePath path;

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));

	memset (&path, 0, sizeof (CameraFilePath));
	CR (gp_camera_capture (c->camera, GP_CAPTURE_IMAGE, &path, NULL), ev);
	if (BONOBO_EX (ev))
		return NULL;

	return g_build_path ("/", path.folder, path.name);
}

static void
gnocam_camera_finalize (GObject *object)
{
	GnoCamCamera *gc = GNOCAM_CAMERA (object);

	if (gc->priv) {
		if (gc->camera) {
			gp_camera_unref (gc->camera);
			gc->camera = NULL;
		}
		g_free (gc->priv);
		gc->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_camera_class_init (GnoCamCameraClass *klass)
{
	GObjectClass *object_class;
	POA_GNOME_Camera__epv *epv;

	parent_class = g_type_class_peek_parent (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gnocam_camera_finalize;

	epv = &klass->epv;
	epv->captureImage      = impl_GNOME_Camera_captureImage;
}

static void
gnocam_camera_init (GnoCamCamera *gnocam_camera)
{
	gnocam_camera->priv = g_new0 (GnoCamCameraPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (GnoCamCamera, GNOME_Camera, BONOBO_TYPE_OBJECT,
		       gnocam_camera);

GnoCamCamera *
gnocam_camera_new (Camera *camera, CORBA_Environment *ev)
{
	GnoCamCamera *gc;
	BonoboStorageCamera *storage;

	bonobo_return_val_if_fail (camera, NULL, ev);

	g_message ("Creating storage...");
	storage = bonobo_storage_camera_new (camera, "/",
					     Bonobo_Storage_READ |
					     Bonobo_Storage_WRITE, ev);
	if (BONOBO_EX (ev))
		return (NULL);

	gc = g_object_new (GNOCAM_TYPE_CAMERA, NULL);

	gc->camera = camera;
	gp_camera_ref (camera);

	g_message ("Adding interfaces...");
	bonobo_object_add_interface (BONOBO_OBJECT (gc),
				     BONOBO_OBJECT (storage));

	gc->priv->event_source = bonobo_event_source_new ();
	bonobo_object_add_interface (BONOBO_OBJECT (gc),
				     BONOBO_OBJECT (gc->priv->event_source));

	return (gc);
}

