/* bonobo-stream-camera.c
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
#include "config.h"
#include "bonobo-stream-camera.h"

#include <string.h>

#include <bonobo/bonobo-exception.h>

#include "gnocam-util.h"

#define PARENT_TYPE BONOBO_TYPE_OBJECT
static BonoboObjectClass *parent_class;

struct _BonoboStreamCameraPrivate {
	Camera     *camera;
	CameraFile *file;
	
	gchar *dirname;
	gchar *filename;

	gint	mode;
	gulong	position;
};

static Bonobo_StorageInfo*
camera_get_info_impl (PortableServer_Servant servant,
		      const Bonobo_StorageInfoFields mask,
		      CORBA_Environment *ev)
{
	BonoboStreamCamera *stream;
	Bonobo_StorageInfo *info;
	CameraFileInfo       fi;
	const char *data;
	long int size;

	g_message ("Getting info for stream...");

	stream = BONOBO_STREAM_CAMERA (bonobo_object (servant));

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | 
		     Bonobo_FIELD_SIZE | Bonobo_FIELD_TYPE)) {
		g_message ("... not supported!");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_NotSupported, NULL); 
		return CORBA_OBJECT_NIL; 
	}

	CR (gp_camera_file_get_info (stream->priv->camera, 
				stream->priv->dirname, stream->priv->filename,
				&fi, NULL), ev);
	CR (gp_file_get_data_and_size (stream->priv->file, &data,
						 &size), ev);
	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;
		

	info = Bonobo_StorageInfo__alloc ();

	/* Content type */
	if (stream->priv->mode & Bonobo_Storage_COMPRESSED) { 
		if (fi.preview.fields & GP_FILE_INFO_TYPE)
			info->content_type = CORBA_string_dup (fi.preview.type);
		else
			info->content_type = CORBA_string_dup (
						"application/octet-stream");
	} else {
		if (fi.file.fields & GP_FILE_INFO_TYPE)
			info->content_type = CORBA_string_dup (fi.file.type);
		else
			info->content_type = CORBA_string_dup (
						"application/octet-stream");
	}

	/* Size */
	if (stream->priv->mode & Bonobo_Storage_COMPRESSED) {
		if (fi.preview.fields & GP_FILE_INFO_SIZE) {
			if (fi.preview.size != size)
				g_warning ("Size information differs: I have "
					"%i bytes in memory, gphoto2 "
					"tells me that the file is %i "
					"bytes big. I am using my value.",
					(int) size, (int) fi.preview.size);
			info->size = size;
		} else {
			if (fi.file.size != size)
				g_warning ("Size information differs: I have "
					"%i bytes in memory, gphoto2 "
					"tells me that the file is %i "
					"bytes big. I am using my value.",
					(int) size, (int) fi.file.size);
			info->size = size;
		}
	}

	/* Name and type */
	info->type = Bonobo_STORAGE_TYPE_REGULAR;
	info->name = CORBA_string_dup (stream->priv->filename);

	return (info);
}

static void
camera_write_impl (PortableServer_Servant servant,
	      const Bonobo_Stream_iobuf *buffer, CORBA_Environment *ev)
{
	BonoboStreamCamera *stream;

	stream = BONOBO_STREAM_CAMERA (bonobo_object (servant));

	CR (gp_file_append (stream->priv->file, buffer->_buffer, 
				      buffer->_length), ev);
	if (!BONOBO_EX (ev))
		stream->priv->position += buffer->_length;
}

static void
camera_read_impl (PortableServer_Servant servant, CORBA_long count,
		  Bonobo_Stream_iobuf **buffer, CORBA_Environment *ev)
{
	BonoboStreamCamera *stream;
	const char *data;
	long int size;

	stream = BONOBO_STREAM_CAMERA (bonobo_object (servant));

	CR (gp_file_get_data_and_size (stream->priv->file,
						 &data, &size), ev);
	if (BONOBO_EX (ev))
		return;

	/* Create the buffer. */
	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	(*buffer)->_buffer = CORBA_sequence_CORBA_octet_allocbuf (count);
	(*buffer)->_length = MIN (count, size - stream->priv->position);

	memcpy ((*buffer)->_buffer, data + stream->priv->position,
		(*buffer)->_length);

	stream->priv->position += (*buffer)->_length;
}

static CORBA_long
camera_seek_impl (PortableServer_Servant servant, CORBA_long offset, 
		  Bonobo_Stream_SeekType whence, CORBA_Environment *ev)
{
	BonoboStreamCamera* stream;
	long int size;
	const char *data;

	stream = BONOBO_STREAM_CAMERA (bonobo_object (servant));

	switch (whence) {
	case Bonobo_Stream_SeekCur:
		stream->priv->position += (long) offset;
		return (stream->priv->position);
	case Bonobo_Stream_SeekEnd:
		CR (gp_file_get_data_and_size (stream->priv->file,
							 &data, &size), ev);
		if (BONOBO_EX (ev))
			return (-1);
		stream->priv->position = size - 1 + (long) offset;
		return (stream->priv->position);
	case Bonobo_Stream_SeekSet:
		stream->priv->position = (long) offset;
		return (stream->priv->position);
	default:
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_NotSupported, NULL);
		return (-1);
	}
}

static void
camera_commit_impl (PortableServer_Servant servant, CORBA_Environment* ev)
{
	BonoboStreamCamera *stream;
	
	stream = BONOBO_STREAM_CAMERA (bonobo_object (servant));

	CR (gp_camera_folder_put_file (stream->priv->camera,
			stream->priv->dirname, stream->priv->file, NULL), ev);
}

static void
bonobo_stream_camera_finalize (GObject *object)
{
	BonoboStreamCamera *stream;

	stream = BONOBO_STREAM_CAMERA (object);

	if (stream->priv) {
		if (stream->priv->dirname) {
			g_free (stream->priv->dirname);
			stream->priv->dirname = NULL;
		}

		if (stream->priv->filename) {
			g_free (stream->priv->filename);
			stream->priv->filename = NULL;
		}

		if (stream->priv->file) {
			gp_file_unref (stream->priv->file);
			stream->priv->file = NULL;
		}

		if (stream->priv->camera) {
			gp_camera_unref (stream->priv->camera);
			stream->priv->camera = NULL;
		}

		g_free (stream->priv);
		stream->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bonobo_stream_camera_class_init (BonoboStreamCameraClass* klass)
{
	GObjectClass *object_class;
	POA_Bonobo_Stream__epv *epv = &klass->epv;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = bonobo_stream_camera_finalize;
	
	epv->getInfo = camera_get_info_impl;
	epv->write   = camera_write_impl;
	epv->read    = camera_read_impl;
	epv->seek    = camera_seek_impl;
	epv->commit  = camera_commit_impl;

	parent_class = g_type_class_peek_parent (klass);
}

static void
bonobo_stream_camera_init (BonoboStreamCamera* stream)
{
	stream->priv = g_new0 (BonoboStreamCameraPrivate, 1);
}

BonoboStreamCamera*
bonobo_stream_camera_new (Camera *camera, const gchar *dirname, 
		          const gchar *fname, gint flags, 
			  CORBA_Environment *ev)
{
	BonoboStreamCamera *new;
	CameraFile         *file = NULL;
	CameraFileInfo info;
	int result;
	CameraAbilities a;

	g_message ("Creating new BonoboStreamCamera...");

        /* Reject some unsupported flags. */ 
	if (flags & Bonobo_Storage_TRANSACTED) { 
	    	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL); 
		return (NULL); 
	}

	/* Does the camera support upload? */
	gp_camera_get_abilities (camera, &a);
        if ((flags & (Bonobo_Storage_WRITE | Bonobo_Storage_CREATE)) && 
	    (!(a.folder_operations & GP_FOLDER_OPERATION_PUT_FILE))) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
        }

        /* Does the requested file exist? */
        if (flags & Bonobo_Storage_FAILIFEXIST) {

		result = gp_camera_file_get_info (camera, dirname, fname,
						  &info, NULL);
		if (result == GP_OK)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					ex_Bonobo_Storage_NameExists, NULL);
		else if (result != GP_ERROR_FILE_NOT_FOUND)
			CR (result, ev);
		if (BONOBO_EX (ev))
			return (NULL);
        }

        /* Get the file. */
	CR (gp_file_new (&file), ev);
        if (flags & Bonobo_Storage_READ) {
		if (flags & Bonobo_Storage_COMPRESSED) {
			CR (gp_camera_file_get (camera, dirname,
				fname, GP_FILE_TYPE_PREVIEW, file, NULL), ev);
		} else {
			CR (gp_camera_file_get (camera, dirname,
				fname, GP_FILE_TYPE_NORMAL, file, NULL), ev);
		}
	}
	if (BONOBO_EX (ev)) {
		gp_file_unref (file);
		return (NULL);
	}

	new = g_object_new (BONOBO_TYPE_STREAM_CAMERA, NULL);
	new->priv->dirname = g_strdup (dirname); 
	new->priv->filename = g_strdup (fname); 
	new->priv->mode = flags; 
	new->priv->camera = camera;
	gp_camera_ref (camera);
	new->priv->file = file;

	return (new);
}

BONOBO_TYPE_FUNC_FULL (BonoboStreamCamera,Bonobo_Stream,PARENT_TYPE,bonobo_stream_camera)
