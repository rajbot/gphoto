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

#define CHECK_RESULT(result,ev)         G_STMT_START{                                                                           \
        if (result <= 0) {                                                                                                      \
                switch (result) {                                                                                               \
                case GP_OK:                                                                                                     \
                        break;                                                                                                  \
                case GP_ERROR_IO:                                                                                               \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_IOError, NULL);				\
                        break;                                                                                                  \
                case GP_ERROR_DIRECTORY_NOT_FOUND:                                                                              \
                case GP_ERROR_FILE_NOT_FOUND:                                                                                   \
                case GP_ERROR_MODEL_NOT_FOUND:                                                                                  \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotFound, NULL);                       \
                        break;                                                                                                  \
                case GP_ERROR_FILE_EXISTS:                                                                                      \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);                     \
                        break;                                                                                                  \
                case GP_ERROR_NOT_SUPPORTED:                                                                                    \
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);			\
			break;													\
		default:													\
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_IOError, NULL);				\
                        break;                                                                                                  \
                }                                                                                                               \
        }                               }G_STMT_END


static BonoboStreamClass *bonobo_stream_camera_parent_class;

static Bonobo_StorageInfo *
camera_get_info (BonoboStream *stream, const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	BonoboStreamCamera *s = BONOBO_STREAM_CAMERA (stream);
	Bonobo_StorageInfo *info;

	g_return_val_if_fail (s->file != NULL, CORBA_OBJECT_NIL);
	
	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE | Bonobo_FIELD_TYPE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL); 
		return CORBA_OBJECT_NIL; 
	}

	info = Bonobo_StorageInfo__alloc ();
	info->size = s->file->size;
	info->type = Bonobo_STORAGE_TYPE_REGULAR;
	info->name = CORBA_string_dup (s->file->name);
	info->content_type = CORBA_string_dup (s->file->type);

	return (info);
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
	gint i;

	/* Check if we have to append data to the file. */
	i = s->position + buffer->_length - s->file->size;
	if (i > 0) {
		CHECK_RESULT (gp_file_append (s->file, buffer->_buffer, i), ev);
		if (BONOBO_EX (ev)) return;
	}
	memcpy (s->file->data + s->position, buffer->_buffer, buffer->_length);
	s->position += buffer->_length;
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
	if (s->position + count >= s->file->size) length = s->file->size - s->position;
	else length = count;
	
	memcpy (data, s->file->data + s->position, length);
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
		s->position = s->file->size - 1 + (long) offset;
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
	BonoboStreamCamera *s = BONOBO_STREAM_CAMERA (stream);
	gchar *dirname;
	
	/* Send the file to the camera. */
	dirname = gnome_vfs_uri_extract_dirname (s->uri);
	CHECK_RESULT (gp_camera_file_put (s->camera, s->file, dirname), ev);
	g_free (dirname);
}

static void
camera_revert (BonoboStream *stream, CORBA_Environment *ev)
{
	BonoboStreamCamera *s = BONOBO_STREAM_CAMERA (stream);
	
	/* Unref the file and set position to 0. */
	if (s->file) gp_file_unref (s->file);
	s->position = 0;
}
	
static void
bonobo_stream_camera_destroy (GtkObject *object)
{
	BonoboStreamCamera *stream = BONOBO_STREAM_CAMERA (object);

	/* Unref uri, camera, and file. */
	if (stream->uri) gnome_vfs_uri_unref (stream->uri);
	if (stream->camera) gp_camera_unref (stream->camera);
	if (stream->file) gp_file_unref (stream->file);
	
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
	sclass->revert   = camera_revert;

	object_class->destroy = bonobo_stream_camera_destroy;

	/* Make sure gnome-vfs is initialized. */
	if (!gnome_vfs_initialized ()) gnome_vfs_init ();
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
	gchar* dirname;

	g_print ("Creating stream...\n");
	stream = gtk_type_new (bonobo_stream_camera_get_type ());
	if (stream == NULL) {
		g_warning ("Could not create stream!");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return (NULL);
	}

	g_print ("Creating corba stream...\n");
	corba_stream = bonobo_stream_corba_object_create (BONOBO_OBJECT (stream));
	if (corba_stream == CORBA_OBJECT_NIL) {
		g_warning ("Could not create corba stream!");
		bonobo_object_unref (BONOBO_OBJECT (stream));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return (NULL);
	}

	g_print ("Creating uri...\n");
	stream->uri = gnome_vfs_uri_new (path);
	if (stream->uri == NULL) {
		g_warning ("Could not create uri!");
		bonobo_object_unref (BONOBO_OBJECT (stream));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return (NULL);
	}

	/* Connect to the camera. */
	stream->mode = mode;
	g_print ("Creating camera...\n");
	stream->camera = util_camera_new (stream->uri, ev);
	if (BONOBO_EX (ev)) {
		g_warning ("Could not create camera!");
		bonobo_object_unref (BONOBO_OBJECT (stream));
		return NULL;
	}

	/* Get the file. */
	g_print ("Getting file...\n");
	stream->file = gp_file_new ();
	dirname = gnome_vfs_uri_extract_dirname (stream->uri);
#if 0
	if (mode & Bonobo_Storage_COMPRESSED)
#endif
		CHECK_RESULT (gp_camera_file_get_preview (stream->camera, stream->file, dirname, (gchar*) gnome_vfs_uri_get_basename (stream->uri)), ev);
#if 0
	else
		CHECK_RESULT (gp_camera_file_get (stream->camera, stream->file, dirname, (gchar*) gnome_vfs_uri_get_basename (stream->uri)), ev);
#endif
	if (BONOBO_EX (ev)) {
		g_warning ("Could not get file!");
		g_free (dirname);
		bonobo_object_unref (BONOBO_OBJECT (stream));
		return NULL;
	}
	g_free (dirname);

	g_print ("Returning...\n");
	return BONOBO_STREAM (bonobo_object_construct (BONOBO_OBJECT (stream), corba_stream));
}
