
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bonobo-stream-camera.h"

#include <bonobo/bonobo-exception.h>
#include <gal/util/e-util.h>

#include <gphoto-extensions.h>

#define PARENT_TYPE BONOBO_STREAM_TYPE
static BonoboStreamClass *bonobo_stream_camera_parent_class;

struct _BonoboStreamCameraPrivate {
	Camera*		camera;
	CameraFile*	file;
	
	gchar*		dirname;
	gchar*		filename;

	gint		mode;
	gulong		position;
};

static Bonobo_StorageInfo*
camera_get_info (BonoboStream* s, const Bonobo_StorageInfoFields mask, CORBA_Environment* ev)
{
	BonoboStreamCamera*	stream;
	Bonobo_StorageInfo*	info;

	stream = BONOBO_STREAM_CAMERA (s);

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE | Bonobo_FIELD_TYPE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL); 
		return CORBA_OBJECT_NIL; 
	}

	info = Bonobo_StorageInfo__alloc ();
	info->size = stream->priv->file->size;
	info->type = Bonobo_STORAGE_TYPE_REGULAR;
	info->name = CORBA_string_dup (stream->priv->filename);
	info->content_type = CORBA_string_dup (stream->priv->file->type);

	return (info);
}

static void
camera_write (BonoboStream* s, const Bonobo_Stream_iobuf* buffer, CORBA_Environment* ev)
{
	BonoboStreamCamera* 	stream;
	gint			i;
	
	stream = BONOBO_STREAM_CAMERA (s);

	/* Check if we have to append data to the file. */
	i = stream->priv->position + buffer->_length - stream->priv->file->size;
	if (i > 0) {
		CHECK_RESULT (gp_file_append (stream->priv->file, buffer->_buffer, i), ev);
		if (BONOBO_EX (ev)) return;
	}
	memcpy (stream->priv->file->data + stream->priv->position, buffer->_buffer, buffer->_length);
	stream->priv->position += buffer->_length;
}

static void
camera_read (BonoboStream *s, CORBA_long count, Bonobo_Stream_iobuf **buffer, CORBA_Environment *ev)
{
	BonoboStreamCamera*	stream;
	CORBA_octet*		data;
	gulong 			length;

	stream = BONOBO_STREAM_CAMERA (s);
	
	/* Create the buffer. */
	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	data = CORBA_sequence_CORBA_octet_allocbuf (count);
	
	/* How many bytes? */
	if (stream->priv->position + count >= stream->priv->file->size) length = stream->priv->file->size - stream->priv->position;
	else length = count;

	memcpy (data, stream->priv->file->data + stream->priv->position, length);
	stream->priv->position += length;
	
	(*buffer)->_buffer = data;
	(*buffer)->_length = length;
}

static CORBA_long
camera_seek (BonoboStream* s, CORBA_long offset, Bonobo_Stream_SeekType whence, CORBA_Environment* ev)
{
	BonoboStreamCamera* stream; 
	
	stream = BONOBO_STREAM_CAMERA (s);

	switch (whence) {
	case Bonobo_Stream_SEEK_CUR:
		stream->priv->position += (long) offset;
		break;
	case Bonobo_Stream_SEEK_END:
		stream->priv->position = stream->priv->file->size - 1 + (long) offset;
		break;
	case Bonobo_Stream_SEEK_SET:
		stream->priv->position = (long) offset;
		break;
	default:
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return -1;
	}
	return (stream->priv->position);
}

static void
camera_commit (BonoboStream* s, CORBA_Environment* ev)
{
	BonoboStreamCamera *stream;
	
	stream = BONOBO_STREAM_CAMERA (s);
	
	CHECK_RESULT (gp_camera_folder_put_file (stream->priv->camera, stream->priv->dirname, stream->priv->file), ev);
}

static void
bonobo_stream_camera_destroy (GtkObject* object)
{
	BonoboStreamCamera* stream;
	
	stream = BONOBO_STREAM_CAMERA (object);

	g_free (stream->priv->dirname);
	g_free (stream->priv->filename);
	gp_camera_unref (stream->priv->camera);
	gp_file_unref (stream->priv->file);
	g_free (stream->priv);
	
	GTK_OBJECT_CLASS (bonobo_stream_camera_parent_class)->destroy (object);	
}

static void
bonobo_stream_camera_class_init (BonoboStreamCameraClass* klass)
{
	BonoboStreamClass*	sclass;
	GtkObjectClass*		object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = bonobo_stream_camera_destroy;
	
	sclass = BONOBO_STREAM_CLASS (klass);
	sclass->get_info = camera_get_info;
	sclass->set_info = NULL;
	sclass->write    = camera_write;
	sclass->read     = camera_read;
	sclass->seek     = camera_seek;
	sclass->truncate = NULL;
	sclass->copy_to  = NULL;
	sclass->commit   = camera_commit;
	sclass->revert   = NULL;

	bonobo_stream_camera_parent_class = gtk_type_class (PARENT_TYPE);
}

static void
bonobo_stream_camera_init (BonoboStreamCamera* stream)
{
	stream->priv = g_new (BonoboStreamCameraPrivate, 1);
}

BonoboStream*
bonobo_stream_camera_open (const gchar* path, gint flags, gint mode, CORBA_Environment* ev)
{
	Camera*			camera = NULL;
	BonoboStreamCamera*	new;
	gchar*			dirname;
	gchar*			filename;

	/* Create camera. */
	CHECK_RESULT (gp_camera_new_from_gconf (&camera, path), ev);
	if (BONOBO_EX (ev)) return (NULL);

	if (!strncmp (path, "camera:", 7)) path += 7;
	for (path += 2; *path != 0; path++) if (*path == '/') break;

	filename = g_basename (path);
	if (!strcmp (path + 1, filename)) dirname = g_strdup ("/");
	else dirname = g_dirname (path);
	new = bonobo_stream_camera_new (camera, dirname, filename, mode, ev);
	g_free (dirname);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (NULL);
	}

	return (BONOBO_STREAM (new));
}

BonoboStreamCamera*
bonobo_stream_camera_new (Camera* camera, const gchar* dirname, const gchar* filename, gint mode, CORBA_Environment* ev)
{
	BonoboStreamCamera*	new;

        /* Reject some unsupported open modes. */
	if (mode & Bonobo_Storage_TRANSACTED) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
	}

        /* Does the camera support upload? */
        if ((mode & (Bonobo_Storage_WRITE | Bonobo_Storage_CREATE)) && (!(camera->abilities->folder_operations & GP_FOLDER_OPERATION_PUT_FILE))) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
        }

	new = gtk_type_new (BONOBO_STREAM_CAMERA_TYPE);
	new->priv->dirname = g_strdup (dirname);
	new->priv->filename = g_strdup (filename);
	new->priv->mode = mode;
	new->priv->position = 0;
	new->priv->camera = camera;
	new->priv->file = gp_file_new ();
	gp_camera_ref (camera);

        /* Does the requested file exist? */
        if (mode & Bonobo_Storage_FAILIFEXIST) {
		gint		i;
		CameraList	list;

                CHECK_RESULT (gp_camera_folder_list_files (new->priv->camera, new->priv->dirname, &list), ev);
                if (BONOBO_EX (ev)) {
			bonobo_object_unref (BONOBO_OBJECT (new));
			return (NULL);
		}
                for (i = 0; i < gp_list_count (&list); i++) {
                        if (strcmp ((gp_list_entry (&list, i))->name, new->priv->filename) == 0) {
                                CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);
				bonobo_object_unref (BONOBO_OBJECT (new));
                                return (NULL);
                        }
                }
        }

        /* Get the file. */
        if (mode & Bonobo_Storage_READ) {
		if (mode & Bonobo_Storage_COMPRESSED) {
			CHECK_RESULT (gp_camera_file_get_preview (new->priv->camera, new->priv->dirname, new->priv->filename, new->priv->file), ev);
		} else {
			CHECK_RESULT (gp_camera_file_get_file (new->priv->camera, new->priv->dirname, new->priv->filename, new->priv->file), ev);
		}
		if (BONOBO_EX (ev)) {
			bonobo_object_unref (BONOBO_OBJECT (new));
			return (NULL);
		}
		g_return_val_if_fail (new->priv->file, NULL);
		if (strcmp (new->priv->file->name, new->priv->filename)) 
			g_warning ("Filenames differ: filename is '%s', file->name is '%s'!", new->priv->filename, new->priv->file->name);
	}

	return (BONOBO_STREAM_CAMERA (bonobo_object_construct (BONOBO_OBJECT (new), bonobo_stream_corba_object_create (BONOBO_OBJECT (new)))));
}

E_MAKE_TYPE (bonobo_stream_camera, "BonoboStreamCamera", BonoboStreamCamera, bonobo_stream_camera_class_init, bonobo_stream_camera_init, PARENT_TYPE)

