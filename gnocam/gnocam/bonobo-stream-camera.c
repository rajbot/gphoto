
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bonobo-stream-camera.h"

#include <bonobo/bonobo-exception.h>
#include <gal/util/e-util.h>

#include "gnocam-util.h"

#define PARENT_TYPE BONOBO_STREAM_TYPE
static BonoboStreamClass *parent_class;

struct _BonoboStreamCameraPrivate {
	Camera*		camera;
	
	gchar*		dirname;
	gchar*		filename;
	gchar*		type;
	gchar*		buffer;

	gint		mode;
	gulong		size;
	gulong		position;
};

static Bonobo_StorageInfo*
camera_get_info (BonoboStream			*s, 
		 const Bonobo_StorageInfoFields	 mask, 
		 CORBA_Environment		*ev)
{
	BonoboStreamCamera *stream;
	Bonobo_StorageInfo *info;

	stream = BONOBO_STREAM_CAMERA (s);

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | 
		     Bonobo_FIELD_SIZE | Bonobo_FIELD_TYPE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL); 
		return CORBA_OBJECT_NIL; 
	}

	info = Bonobo_StorageInfo__alloc ();
	info->size = stream->priv->size;
	info->type = Bonobo_STORAGE_TYPE_REGULAR;
	info->name = CORBA_string_dup (stream->priv->filename);
	info->content_type = CORBA_string_dup (stream->priv->type);

	return (info);
}

static void
camera_write (BonoboStream 		*s, 
	      const Bonobo_Stream_iobuf *buffer, 
	      CORBA_Environment		*ev)
{
	BonoboStreamCamera *stream;
	
	stream = BONOBO_STREAM_CAMERA (s);

	stream->priv->size = stream->priv->position + buffer->_length + 1;
	stream->priv->buffer = g_renew (gchar, stream->priv->buffer,
					stream->priv->size);
	
	memcpy (stream->priv->buffer + stream->priv->position, 
		buffer->_buffer, buffer->_length);
	stream->priv->position += buffer->_length;
}

static void
camera_read (BonoboStream *s, CORBA_long count, Bonobo_Stream_iobuf **buffer,
	     CORBA_Environment *ev)
{
	BonoboStreamCamera *stream;
	CORBA_octet        *data;
	gulong 		    length;

	stream = BONOBO_STREAM_CAMERA (s);
	
	/* Create the buffer. */
	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	data = CORBA_sequence_CORBA_octet_allocbuf (count);
	
	length = MIN (count, stream->priv->size - stream->priv->position);
	memcpy (data, stream->priv->buffer + stream->priv->position, length);
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
		stream->priv->position = stream->priv->size - 1 + (long) offset;
		break;
	case Bonobo_Stream_SEEK_SET:
		stream->priv->position = (long) offset;
		break;
	default:
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_NotSupported, NULL);
		return -1;
	}
	return (stream->priv->position);
}

static void
camera_commit (BonoboStream* s, CORBA_Environment* ev)
{
	CameraFile *file;
	BonoboStreamCamera *stream;
	
	stream = BONOBO_STREAM_CAMERA (s);
	
	file = gp_file_new ();
	CHECK_RESULT (gp_file_append (file, stream->priv->buffer,
				      stream->priv->size), ev);
	CHECK_RESULT (gp_camera_folder_put_file (stream->priv->camera,
						 stream->priv->dirname,
						 file), ev);
	gp_file_unref (file);
}

static void
bonobo_stream_camera_destroy (GtkObject* object)
{
	BonoboStreamCamera* stream;
	
	stream = BONOBO_STREAM_CAMERA (object);

	if (stream->priv->dirname) {
		g_free (stream->priv->dirname);
		stream->priv->dirname = NULL;
	}

	if (stream->priv->filename) {
		g_free (stream->priv->filename);
		stream->priv->filename = NULL;
	}
	
	if (stream->priv->buffer) {
		g_free (stream->priv->buffer);
		stream->priv->buffer = NULL;
	}

	if (stream->priv->camera) {
		gp_camera_unref (stream->priv->camera);
		stream->priv->camera = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);	
}

static void
bonobo_stream_camera_finalize (GtkObject *object)
{
	BonoboStreamCamera *stream;

	stream = BONOBO_STREAM_CAMERA (object);

	g_free (stream->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bonobo_stream_camera_class_init (BonoboStreamCameraClass* klass)
{
	BonoboStreamClass*	sclass;
	GtkObjectClass*		object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = bonobo_stream_camera_destroy;
	object_class->finalize = bonobo_stream_camera_finalize;
	
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

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
bonobo_stream_camera_init (BonoboStreamCamera* stream)
{
	stream->priv = g_new0 (BonoboStreamCameraPrivate, 1);
}

BonoboStream*
bonobo_stream_camera_new (Camera            *camera, 
			  const gchar       *dirname, 
			  const gchar       *filename, 
			  gint 	             flags, 
			  CORBA_Environment *ev)
{
    	BonoboObject	   *object;
	BonoboStreamCamera *new;
	Bonobo_Stream	    corba_new;
	CameraList          list;
	CameraFile         *file;
	gint i;

        /* Reject some unsupported flags. */ 
	if (flags & Bonobo_Storage_TRANSACTED) { 
	    	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL); 
		return (NULL); 
	}

	/* Does the camera support upload? */
        if ((flags & (Bonobo_Storage_WRITE | Bonobo_Storage_CREATE)) && 
	    (!(camera->abilities->folder_operations & 
	       GP_FOLDER_OPERATION_PUT_FILE))) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
        }

	/* You cannot do a gp_camera_file_get_file() if you have not 
	 * performed a gp_camera_folder_list_files() previously to populate 
	 * the filesystem struct of the driver. 
	 */
	CHECK_RESULT (gp_camera_folder_list_files (camera, dirname, &list), ev);
	if (BONOBO_EX (ev)) {
		g_warning ("Couldn't get list of files: %s",
			   bonobo_exception_get_text (ev));
		return (NULL);
	}

        /* Does the requested file exist? */
        if (flags & Bonobo_Storage_FAILIFEXIST) {
                for (i = 0; i < gp_list_count (&list); i++) {
                        if (!strcmp ((gp_list_entry (&list, i))->name, 
				     filename)) {
                                CORBA_exception_set (
					ev, CORBA_USER_EXCEPTION, 
					ex_Bonobo_Storage_NameExists, NULL);
                                return (NULL);
                        }
                }
        }

        /* Get the file. */
	file = gp_file_new ();
        if (flags & Bonobo_Storage_READ) {
		if (flags & Bonobo_Storage_COMPRESSED) {
			CHECK_RESULT (gp_camera_file_get_preview (camera, 
						dirname, filename, file), ev);
		} else {
			CHECK_RESULT (gp_camera_file_get_file (camera, 
						dirname, filename, file), ev);
		}
		if (BONOBO_EX (ev)) {
			gp_file_unref (file);
			return (NULL);
		}
		if (strcmp (file->name, filename)) 
			g_warning ("Filenames differ: filename is '%s', 
				   file->name is '%s'!", filename, file->name);
	}

	new = gtk_type_new (BONOBO_STREAM_CAMERA_TYPE);
	new->priv->dirname = g_strdup (dirname); 
	new->priv->filename = g_strdup (filename); 
	new->priv->mode = flags; 
	new->priv->camera = camera; 
	gp_camera_ref (camera);
	
	new->priv->buffer = file->data;
	new->priv->size = file->size;
	file->data = NULL;
	gp_file_unref (file);

	corba_new = bonobo_stream_corba_object_create (BONOBO_OBJECT (new));
	object = bonobo_object_construct (BONOBO_OBJECT (new), corba_new);
	return (BONOBO_STREAM (object));
}

E_MAKE_TYPE (bonobo_stream_camera, "BonoboStreamCamera", BonoboStreamCamera, bonobo_stream_camera_class_init, bonobo_stream_camera_init, PARENT_TYPE)

