#include <gphoto2.h>
#include "gnocam-camera.h"

#include <gtk/gtksignal.h>
#include <gtk/gtkmain.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-storage.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-event-source.h>
#include <libgnome/gnome-util.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-dialog-util.h>

#include "gnocam-capture.h"
#include "bonobo-storage-camera.h"
#include "GnoCam.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboXObjectClass *parent_class;

struct _GnoCamCameraPrivate
{
	Camera *camera;
	
	BonoboEventSource *event_source;
};

#define CHECK_RESULT(result,ev) G_STMT_START{\
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

static CORBA_char *
impl_GNOME_Camera_getInfo (PortableServer_Servant servant, 
		           CORBA_Environment *ev)
{
	GnoCamCamera *c;
	CameraText text;

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));
	CHECK_RESULT (gp_camera_get_manual (c->priv->camera, &text), ev);
	if (BONOBO_EX (ev))
		return (NULL);

	return (g_strdup (text.text));
}

static Bonobo_Stream
impl_GNOME_Camera_capturePreview (PortableServer_Servant servant, 
				  CORBA_Environment *ev)
{
	BonoboStream *stream;
	CameraFile *file;
	GnoCamCamera *c;

	g_message ("impl_GNOME_Camera_capturePreview");

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));
	CHECK_RESULT (gp_file_new (&file), ev);
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

static void
on_dialog_destroy (GtkObject *object, gpointer data)
{
	bonobo_object_idle_unref (BONOBO_OBJECT (data));
}

static gboolean
on_capture_close (GnomeDialog *dialog, gpointer data)
{
	GnoCamCamera *c;

	g_return_val_if_fail (GNOCAM_IS_CAMERA (data), FALSE);
	c = GNOCAM_CAMERA (data);

	g_message ("on_capture_close");

	bonobo_object_unref (BONOBO_OBJECT (c));

	return (FALSE);
}

static gboolean 
do_capture (gpointer data)
{
	GnoCamCamera *c;
	CORBA_Environment ev;
	BonoboArg *arg;
	gchar *txt;
	CameraFilePath path;

	g_return_val_if_fail (GNOCAM_IS_CAMERA (data), FALSE);
	c = GNOCAM_CAMERA (data);

	CORBA_exception_init (&ev);

	CHECK_RESULT (gp_camera_capture (c->priv->camera,
				GP_OPERATION_CAPTURE_IMAGE, &path), &ev);
	if (BONOBO_EX (&ev)) {
		txt = g_strdup_printf (_("Could not capture image: %s"),
				       bonobo_exception_get_text (&ev));
		gnome_error_dialog (txt);
		g_free (txt);
	} else {
		arg = bonobo_arg_new (BONOBO_ARG_STRING);
		txt = g_concat_dir_and_file (path.folder, path.name);
		g_message ("The picture is in '%s'.", txt);
		BONOBO_ARG_SET_STRING (arg, txt);
		bonobo_event_source_notify_listeners_full (
					c->priv->event_source,
					"GNOME/Camera", "CaptureImage",
					"Action", arg, NULL);
		bonobo_arg_release (arg);
	}
	
	CORBA_exception_free (&ev);

	bonobo_object_unref (BONOBO_OBJECT (c));

	return (FALSE);
}

static void
on_capture_clicked (GnomeDialog *dialog, gint button_number, gpointer data)
{
	GnoCamCamera *c;
	BonoboArg *arg;

	g_return_if_fail (GNOCAM_IS_CAMERA (data));
	c = GNOCAM_CAMERA (data);

	g_message ("on_capture_clicked");

	switch (button_number) {
	case 1: /* Cancel */
		arg = bonobo_arg_new (BONOBO_ARG_STRING);
		BONOBO_ARG_SET_STRING (arg, "");
		bonobo_event_source_notify_listeners_full (
						c->priv->event_source, 
						"GNOME/Camera",
						"CaptureImage",
						"Cancel", arg, NULL);
		bonobo_arg_release (arg);
		break;
	case 0: /* Ok */
		bonobo_object_ref (BONOBO_OBJECT (c));
		gtk_idle_add (do_capture, c);
		break;
	default:
		g_warning ("Unhandled button: %i", button_number);
		break;
	}
}

static void
impl_GNOME_Camera_captureImage (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	GnoCamCamera *c;
	GnomeDialog *capture;

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));

	g_message ("Creating GnoCamCapture...");
	capture = gnocam_capture_new (c->priv->camera, ev);
	if (BONOBO_EX (ev))
		return;

	gtk_widget_show (GTK_WIDGET (capture));
	gtk_signal_connect (GTK_OBJECT (capture), "clicked",
			    GTK_SIGNAL_FUNC (on_capture_clicked), c);
	bonobo_object_ref (BONOBO_OBJECT (c));
	gtk_signal_connect (GTK_OBJECT (capture), "close",
			    GTK_SIGNAL_FUNC (on_capture_close), c);
}

static void
gnocam_camera_destroy (GtkObject *object)
{
	GnoCamCamera *gnocam_camera;

	gnocam_camera = GNOCAM_CAMERA (object);

	g_message ("Destroying GnoCamCamera...");

	gp_camera_unref (gnocam_camera->priv->camera);
	gnocam_camera->priv->camera = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_camera_finalize (GtkObject *object)
{
	GnoCamCamera *gnocam_camera;

	gnocam_camera = GNOCAM_CAMERA (object);

	g_free (gnocam_camera->priv);

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
	epv->capturePreview    = impl_GNOME_Camera_capturePreview;
	epv->captureImage      = impl_GNOME_Camera_captureImage;
	epv->getInfo           = impl_GNOME_Camera_getInfo;
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
	GnoCamCamera *gc;
	BonoboStorage *storage;

	bonobo_return_val_if_fail (camera, NULL, ev);

	g_message ("Creating storage...");
	storage = bonobo_storage_camera_new (camera, "/",
					     Bonobo_Storage_READ |
					     Bonobo_Storage_WRITE, ev);
	if (BONOBO_EX (ev))
		return (NULL);

	gc = gtk_type_new (GNOCAM_TYPE_CAMERA);

	gc->priv->camera = camera;
	gp_camera_ref (camera);

	g_message ("Adding interfaces...");
	bonobo_object_add_interface (BONOBO_OBJECT (gc),
				     BONOBO_OBJECT (storage));

	gc->priv->event_source = bonobo_event_source_new ();
	bonobo_object_add_interface (BONOBO_OBJECT (gc),
				     BONOBO_OBJECT (gc->priv->event_source));

	return (gc);
}

