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
				ex_GNOME_GnoCam_Camera_NotSupported, NULL);\
			break;\
		case GP_ERROR_IO:\
		default:\
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,\
				ex_GNOME_GnoCam_Camera_IOError, NULL);\
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
	CR (gp_camera_get_manual (c->camera, &text, NULL), ev);
	if (BONOBO_EX (ev))
		return (NULL);

	return (g_strdup (text.text));
}

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

#if 0
static void
on_dialog_destroy (GtkObject *object, gpointer data)
{
	bonobo_object_idle_unref (BONOBO_OBJECT (data));
}
#endif

#if 0
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
#endif

#if 0
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

	CR (gp_camera_capture (c->camera,
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
#endif

#if 0
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
#endif

#if 0
static void
impl_GNOME_Camera_captureImage (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	GnoCamCamera *c;
	GnomeDialog *capture;

	c = GNOCAM_CAMERA (bonobo_object_from_servant (servant));

	g_message ("Creating GnoCamCapture...");
	capture = gnocam_capture_new (c->camera, ev);
	if (BONOBO_EX (ev))
		return;

	gtk_widget_show (GTK_WIDGET (capture));
	gtk_signal_connect (GTK_OBJECT (capture), "clicked",
			    GTK_SIGNAL_FUNC (on_capture_clicked), c);
	bonobo_object_ref (BONOBO_OBJECT (c));
	gtk_signal_connect (GTK_OBJECT (capture), "close",
			    GTK_SIGNAL_FUNC (on_capture_close), c);
}
#endif

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
	POA_GNOME_GnoCam_Camera__epv *epv;

	parent_class = g_type_class_peek_parent (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gnocam_camera_finalize;

	epv = &klass->epv;
	epv->getInfo           = impl_GNOME_Camera_getInfo;
}

static void
gnocam_camera_init (GnoCamCamera *gnocam_camera)
{
	gnocam_camera->priv = g_new0 (GnoCamCameraPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (GnoCamCamera, GNOME_GnoCam_Camera, BONOBO_TYPE_OBJECT,
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

