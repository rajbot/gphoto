/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * gnome-stream-camera.c: Gnome digital camera stream implementation
 *
 * Author:
 *   Michael Meeks <michael@helixcode.com>
 *
 * Copyright 2000, Helix Code Inc.
 */
#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <errno.h>
#include <bonobo/bonobo-exception.h>
#include "bonobo-stream-camera.h"
#include "utils.h"

static BonoboStreamClass *bonobo_stream_camera_parent_class;

static Bonobo_StorageInfo *
camera_get_info (BonoboStream *stream, const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	printf ("camera_get_info\n");
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
	return CORBA_OBJECT_NIL;
}

static void
camera_set_info (BonoboStream *stream, const Bonobo_StorageInfo *info, const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static void
camera_write (BonoboStream *stream, const Bonobo_Stream_iobuf *buffer, CORBA_Environment *ev)
{
	BonoboStreamCamera *s = BONOBO_STREAM_CAMERA (stream);
	CameraFile *file;
	gchar *folder;

	g_return_if_fail (file = gp_file_new ());
	g_return_if_fail (gp_file_append (file, buffer->_buffer, buffer->_length) == GP_OK);
	g_return_if_fail (folder = gnome_vfs_uri_extract_dirname (s->uri));
	g_return_if_fail (gp_camera_file_put (s->camera, file, folder) == GP_OK);
	g_free (folder);
	g_return_if_fail (gp_file_free (file) == GP_OK);
}

static void
camera_read (BonoboStream *stream, CORBA_long count, Bonobo_Stream_iobuf **buffer, CORBA_Environment *ev)
{
	BonoboStreamCamera *s = BONOBO_STREAM_CAMERA (stream);
	CORBA_octet *data;
	long length;
	
	/* Create the buffer. */
	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	data = CORBA_sequence_CORBA_octet_allocbuf (count);
	
	/* How many bytes? */
	if (s->position + count >= s->size) length = s->size - s->position;
	else length = count;
	
	memcpy (data, s->data + s->position, length);
	s->position += length;
	
	(*buffer)->_buffer = data;
	(*buffer)->_length = length;
}

static CORBA_long
camera_seek (BonoboStream *stream, CORBA_long offset, Bonobo_Stream_SeekType whence, CORBA_Environment *ev)
{
	BonoboStreamCamera *s = BONOBO_STREAM_CAMERA (stream);

	switch (whence) {
	case Bonobo_Stream_SEEK_CUR:
		s->position += (long) offset;
		break;
	case Bonobo_Stream_SEEK_END:
		s->position = s->size - 1 + (long) offset;
		break;
	case Bonobo_Stream_SEEK_SET:
		s->position = (long) offset;
		break;
	default:
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return -1;
	}
	return s->position;
}

static void
camera_truncate (BonoboStream *stream, const CORBA_long new_size, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static void
camera_copy_to (BonoboStream *stream, const CORBA_char *dest, const CORBA_long bytes, CORBA_long *read_bytes, CORBA_long *written_bytes, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static void
camera_commit (BonoboStream *stream, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static void
camera_revert (BonoboStream *stream, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}
	
static void
bonobo_stream_camera_destroy (GtkObject *object)
{
	BonoboStreamCamera *stream = BONOBO_STREAM_CAMERA (object);

	/* Free uri. */
	if (stream->uri) gnome_vfs_uri_unref (stream->uri);
	stream->uri = NULL;

	/* Free camera. */
	if (stream->camera) gp_camera_unref (stream->camera);
	stream->camera = NULL;

	/* Free data. */
	if (stream->data) g_free (stream->data);
	stream->data = NULL;
	
	GTK_OBJECT_CLASS (bonobo_stream_camera_parent_class)->destroy (object);	
}

static void
bonobo_stream_camera_class_init (BonoboStreamCameraClass *klass)
{
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS (klass);
	GtkObjectClass    *object_class = GTK_OBJECT_CLASS (klass);
	
	bonobo_stream_camera_parent_class = gtk_type_class (bonobo_stream_get_type ());

	sclass->get_info = camera_get_info;
	sclass->set_info = camera_set_info;
	sclass->write    = camera_write;
	sclass->read     = camera_read;
	sclass->seek     = camera_seek;
	sclass->truncate = camera_truncate;
	sclass->copy_to  = camera_copy_to;
	sclass->commit   = camera_commit;
	sclass->commit   = camera_revert;

	object_class->destroy = bonobo_stream_camera_destroy;
}

/**
 * bonobo_stream_camera_get_type:
 *
 * Returns the GtkType for the BonoboStreamCamera class.
 */
GtkType
bonobo_stream_camera_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStreamCamera",
			sizeof (BonoboStreamCamera),
			sizeof (BonoboStreamCameraClass),
			(GtkClassInitFunc) bonobo_stream_camera_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_stream_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_stream_camera_open:
 * @path: The path to the file to be opened.
 * @mode: The mode with which the file should be opened.
 *
 * Creates a new BonoboStream object for the filename specified by
 * @path.  
 */
BonoboStream *
bonobo_stream_camera_open (const char *path, gint flags, gint mode, CORBA_Environment *ev)
{
	BonoboStreamCamera *stream;
	Bonobo_Stream corba_stream;
	CameraFile* file;
	gchar* filename;
	gchar* dirname;

	printf ("bonobo_stream_camera_open (%s, %i, %i)\n", path, flags, mode);

	stream = gtk_type_new (bonobo_stream_camera_get_type ());
	if (stream == NULL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return NULL;
	}

	corba_stream = bonobo_stream_corba_object_create (BONOBO_OBJECT (stream));
	if (corba_stream == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (stream));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return NULL;
	}

	stream->uri = gnome_vfs_uri_new (path);
	stream->camera = util_camera_new (stream->uri, ev);
	if (BONOBO_EX (ev)) {
		bonobo_object_unref (BONOBO_OBJECT (stream));
		return NULL;
	}

	/* Get the file. */
	g_assert (file = gp_file_new ());
	g_assert (filename = g_strdup (gnome_vfs_uri_get_basename (stream->uri)));
	g_assert (dirname = gnome_vfs_uri_extract_dirname (stream->uri));
	if (gnome_vfs_uri_get_user_name (stream->uri) && (strcmp (gnome_vfs_uri_get_user_name (stream->uri), "previews") == 0)) {
		if (gp_camera_file_get_preview (stream->camera, file, dirname, filename) != GP_OK) {
			gp_file_unref (file);
			bonobo_object_unref (BONOBO_OBJECT (stream));
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
			return NULL;
		}
	} else {
		if (gp_camera_file_get (stream->camera, file, dirname, filename) != GP_OK) {
			gp_file_unref (file);
			bonobo_object_unref (BONOBO_OBJECT (stream));
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
			return NULL;
		}
	}
	stream->data = file->data;
	stream->size = file->size;
	file->data = NULL;
	gp_file_unref (file);

	return BONOBO_STREAM (bonobo_object_construct (BONOBO_OBJECT (stream), corba_stream));
}
