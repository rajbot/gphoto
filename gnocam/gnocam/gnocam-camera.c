#include <gphoto2.h>
#include "gnocam-camera.h"

#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-storage.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-exception.h>
#include <libgnome/gnome-util.h>

#include "gnocam-util.h"
#include "bonobo-storage-camera.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass *parent_class;

struct _GnoCamCameraPrivate
{
	Camera *camera;
	BonoboStorage *storage;
};

static Bonobo_Stream
impl_GNOME_Camera_capturePreview (PortableServer_Servant servant, 
				  CORBA_Environment *ev)
{
	BonoboStream *stream;
	CameraFile *file;
	GnoCamCamera *c;

	g_message ("impl_GNOME_Camera_capturePreview");

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));
	file = gp_file_new ();
	CHECK_RESULT (gp_camera_capture_preview (c->priv->camera, file), ev);
	if (BONOBO_EX (ev)) {
		gp_file_unref (file);
		g_message ("Returning...");
		return (CORBA_OBJECT_NIL);
	}

	stream = bonobo_stream_mem_create (file->data, file->size, TRUE, FALSE);
	gp_file_unref (file);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev));
}

static Bonobo_Stream
impl_GNOME_Camera_captureImage (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	Bonobo_Stream stream;
	GnoCamCamera *c;
	CameraFilePath path;
	gchar *full_path;

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));
	CHECK_RESULT (gp_camera_capture (c->priv->camera,
				GP_OPERATION_CAPTURE_IMAGE, &path), ev);
	if (BONOBO_EX (ev))
		return (NULL);

	full_path = g_concat_dir_and_file (path.folder, path.name);
	stream = Bonobo_Storage_openStream (BONOBO_OBJREF (c->priv->storage),
					    full_path, Bonobo_Storage_READ, ev);
	g_free (full_path);

	return (stream);
}

static void
gnocam_camera_destroy (GtkObject *object)
{
	GnoCamCamera *gnocam_camera;

	gnocam_camera = GNOCAM_CAMERA (object);

	gp_camera_unref (gnocam_camera->priv->camera);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_camera_finalize (GtkObject *object)
{
	GnoCamCamera *gnocam_camera;

	gnocam_camera = GNOCAM_CAMERA (object);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_camera_class_init (GnoCamCameraClass *klass)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);
	POA_GNOME_Camera__epv *epv;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = gnocam_camera_destroy;
	object_class->finalize = gnocam_camera_finalize;

	epv = &klass->epv;
	epv->capturePreview = impl_GNOME_Camera_capturePreview;
	epv->captureImage   = impl_GNOME_Camera_captureImage;
}

static void
gnocam_camera_init (GnoCamCamera *gnocam_camera)
{
	gnocam_camera->priv = g_new0 (GnoCamCameraPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamCamera, GNOME_Camera, PARENT_TYPE, gnocam_camera);

GnoCamCamera *
gnocam_camera_new (Camera *camera, CORBA_Environment *ev)
{
	GnoCamCamera *gnocam_camera;
	BonoboStorage *storage;

	bonobo_return_val_if_fail (camera, NULL, ev);

	g_message ("Creating storage...");
	storage = bonobo_storage_camera_new (camera, "/",
					     Bonobo_Storage_READ |
					     Bonobo_Storage_WRITE, ev);
	if (BONOBO_EX (ev))
		return (NULL);

	gnocam_camera = gtk_type_new (GNOCAM_TYPE_CAMERA);

	gnocam_camera->priv->camera = camera;
	gp_camera_ref (camera);

	gnocam_camera->priv->storage = storage;

	g_message ("Adding interface...");
	bonobo_object_add_interface (BONOBO_OBJECT (gnocam_camera),
				     BONOBO_OBJECT (storage));

	return (gnocam_camera);
}

