
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
	Camera     *camera;
	CameraFile *file;
	
	gchar *dirname;
	gchar *filename;

	gint	mode;
	gulong	position;
};

static Bonobo_StorageInfo*
camera_get_info (BonoboStream			*s, 
		 const Bonobo_StorageInfoFields	 mask, 
		 CORBA_Environment		*ev)
{
	BonoboStreamCamera *stream;
	Bonobo_StorageInfo *info;
	CameraFileInfoStruct fileinfostruct;
	CameraFileInfo       fileinfo;
	const char *data;
	long int size;

	g_message ("Getting info for stream...");

	stream = BONOBO_STREAM_CAMERA (s);

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | 
		     Bonobo_FIELD_SIZE | Bonobo_FIELD_TYPE)) {
		g_message ("... not supported!");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_NotSupported, NULL); 
		return CORBA_OBJECT_NIL; 
	}

	CHECK_RESULT (gp_camera_file_get_info (stream->priv->camera, 
				stream->priv->dirname, stream->priv->filename,
				&fileinfo), ev);
	CHECK_RESULT (gp_file_get_data_and_size (stream->priv->file, &data,
						 &size), ev);
	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;
		

	info = Bonobo_StorageInfo__alloc ();

	if (stream->priv->mode & Bonobo_Storage_COMPRESSED)
		fileinfostruct = fileinfo.preview;
	else
		fileinfostruct = fileinfo.file;

	/* Content type */
	if (fileinfostruct.fields & GP_FILE_INFO_TYPE)
		info->content_type = CORBA_string_dup (fileinfostruct.type);
	else
		info->content_type = CORBA_string_dup (
						"application/octet-stream");

	/* Size */
	if (fileinfostruct.fields & GP_FILE_INFO_SIZE)
		if (fileinfostruct.size != size)
			g_warning ("Size information differs: I have "
				   "%i bytes in memory, gphoto2 "
				   "tells me that the file is %i "
				   "bytes big. I am using my value.",
				   (int) size, (int) info->size);
	info->size = size;

	/* Name and type */
	info->type = Bonobo_STORAGE_TYPE_REGULAR;
	info->name = CORBA_string_dup (stream->priv->filename);

	return (info);
}

static void
camera_write (BonoboStream 		*s, 
	      const Bonobo_Stream_iobuf *buffer, 
	      CORBA_Environment		*ev)
{
	BonoboStreamCamera *stream = BONOBO_STREAM_CAMERA (s);

	CHECK_RESULT (gp_file_append (stream->priv->file, buffer->_buffer, 
				      buffer->_length), ev);
	if (!BONOBO_EX (ev))
		stream->priv->position += buffer->_length;
}

static void
camera_read (BonoboStream *s, CORBA_long count, Bonobo_Stream_iobuf **buffer,
	     CORBA_Environment *ev)
{
	BonoboStreamCamera *stream = BONOBO_STREAM_CAMERA (s);
	const char *data;
	long int size;

	CHECK_RESULT (gp_file_get_data_and_size (stream->priv->file,
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
camera_seek (BonoboStream* s, CORBA_long offset, Bonobo_Stream_SeekType whence, CORBA_Environment* ev)
{
	BonoboStreamCamera* stream = BONOBO_STREAM_CAMERA (s);
	long int size;
	const char *data;

	switch (whence) {
	case Bonobo_Stream_SEEK_CUR:
		stream->priv->position += (long) offset;
		return (stream->priv->position);
	case Bonobo_Stream_SEEK_END:
		CHECK_RESULT (gp_file_get_data_and_size (stream->priv->file,
							 &data, &size), ev);
		if (BONOBO_EX (ev))
			return (-1);
		stream->priv->position = size - 1 + (long) offset;
		return (stream->priv->position);
	case Bonobo_Stream_SEEK_SET:
		stream->priv->position = (long) offset;
		return (stream->priv->position);
	default:
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_NotSupported, NULL);
		return (-1);
	}
}

static void
camera_commit (BonoboStream* s, CORBA_Environment* ev)
{
	BonoboStreamCamera *stream = BONOBO_STREAM_CAMERA (s);
	
	CHECK_RESULT (gp_camera_folder_put_file (stream->priv->camera,
						 stream->priv->dirname,
						 stream->priv->file), ev);
}

static void
bonobo_stream_camera_destroy (GtkObject* object)
{
	BonoboStreamCamera* stream;
	
	stream = BONOBO_STREAM_CAMERA (object);

	g_message ("Destroying BonoboStreamCamera...");

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
	CameraFile         *file = NULL;
	CameraFileInfo info;
	int result;

	g_message ("Creating new BonoboStreamCamera...");

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

        /* Does the requested file exist? */
        if (flags & Bonobo_Storage_FAILIFEXIST) {

		result = gp_camera_file_get_info (camera, dirname, filename,
						  &info);
		if (result == GP_OK)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					ex_Bonobo_Storage_NameExists, NULL);
		else if (result != GP_ERROR_FILE_NOT_FOUND)
			CHECK_RESULT (result, ev);
		if (BONOBO_EX (ev))
			return (NULL);
        }

        /* Get the file. */
	CHECK_RESULT (gp_file_new (&file), ev);
        if (flags & Bonobo_Storage_READ) {
		if (flags & Bonobo_Storage_COMPRESSED) {
			CHECK_RESULT (gp_camera_file_get (camera, dirname,
				filename, GP_FILE_TYPE_PREVIEW, file), ev);
		} else {
			CHECK_RESULT (gp_camera_file_get (camera, dirname,
				filename, GP_FILE_TYPE_NORMAL, file), ev);
		}
	}
	if (BONOBO_EX (ev)) {
		gp_file_unref (file);
		return (NULL);
	}

	new = gtk_type_new (BONOBO_STREAM_CAMERA_TYPE);
	new->priv->dirname = g_strdup (dirname); 
	new->priv->filename = g_strdup (filename); 
	new->priv->mode = flags; 
	new->priv->camera = camera;
	gp_camera_ref (camera);
	new->priv->file = file;

	corba_new = bonobo_stream_corba_object_create (BONOBO_OBJECT (new));
	object = bonobo_object_construct (BONOBO_OBJECT (new), corba_new);
	return (BONOBO_STREAM (object));
}

E_MAKE_TYPE (bonobo_stream_camera, "BonoboStreamCamera", BonoboStreamCamera, bonobo_stream_camera_class_init, bonobo_stream_camera_init, PARENT_TYPE)

