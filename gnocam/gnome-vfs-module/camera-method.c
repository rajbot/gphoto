#include <libgnome/gnome-defs.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-method.h>

#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-moniker-util.h>
#include <liboaf/oaf-mainloop.h>

#include <stdlib.h>
#include <stdio.h>

static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

#define CAM_VFS_DEBUG(x) G_STMT_START {\
	printf ("%s:%d ", __FILE__,__LINE__);\
	printf ("%s() ", __FUNCTION__);\
	printf x;\
	fputc ('\n', stdout);\
	fflush (stdout);\
}G_STMT_END

typedef struct {
	Bonobo_Stream stream; 
	guint pos;
} FileHandle;

static GnomeVFSResult do_open (
	GnomeVFSMethod        *method, 
	GnomeVFSMethodHandle **handle, 
	GnomeVFSURI           *uri, 
	GnomeVFSOpenMode       mode, 
	GnomeVFSContext       *context)
{
	Bonobo_Stream stream;
	CORBA_Environment ev;
	FileHandle *file_handle;
	gchar *moniker;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);
	
	CORBA_exception_init (&ev);

	/* Get the stream */ 
	CAM_VFS_DEBUG (("Getting stream...")); 
	moniker = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE); 
	stream = bonobo_get_object (moniker, "IDL:Bonobo/Stream:1.0", &ev); 
	g_free (moniker); 
	if (BONOBO_EX (&ev)) { 
		CORBA_exception_free (&ev); 
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC); 
	}
	CORBA_exception_free (&ev);

	/* Construct handle */
	file_handle = g_new0 (FileHandle, 1);
	file_handle->stream = stream;
	file_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) file_handle;

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_create ( 
        GnomeVFSMethod        *method,
        GnomeVFSMethodHandle **handle,
        GnomeVFSURI           *uri,
        GnomeVFSOpenMode       mode,
        gboolean               exclusive,
        guint                  perm,
        GnomeVFSContext       *context)
{
	return (do_open (method, handle, uri, mode, context));
}

static GnomeVFSResult do_close (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSContext*                context)
{
	FileHandle *file_handle;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);
	
	file_handle = (FileHandle *) handle;
	bonobo_object_release_unref (file_handle->stream, NULL);
	g_free (file_handle);

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));
	
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_read (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        gpointer                        buffer,
        GnomeVFSFileSize                num_bytes,
        GnomeVFSFileSize*               bytes_read,
        GnomeVFSContext*                context)
{
	FileHandle *file_handle;
	CORBA_Environment ev;
	Bonobo_Stream_iobuf *iobuf;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);
	
	file_handle = (FileHandle *) handle;
	
	CORBA_exception_init (&ev);
	Bonobo_Stream_read (file_handle->stream, num_bytes, &iobuf, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC);
	}
	CORBA_exception_free (&ev);

	memcpy (buffer, iobuf->_buffer, iobuf->_length);
	*bytes_read = iobuf->_length;

	file_handle->pos += iobuf->_length;
	
	CORBA_free (iobuf);

	CAM_VFS_DEBUG (("EXIT"));
	g_static_mutex_unlock (&mutex);
	
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_write (
	GnomeVFSMethod       *method,
	GnomeVFSMethodHandle *handle,
	gconstpointer         buffer,
	GnomeVFSFileSize      num_bytes,
	GnomeVFSFileSize     *bytes_written,
	GnomeVFSContext      *context)
{
	FileHandle *file_handle;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);

	file_handle = (FileHandle *) handle;

	g_warning ("Implement do_write!");

	*bytes_written = num_bytes;

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));
	
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_seek (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSSeekPosition            position,
        GnomeVFSFileOffset              offset,
        GnomeVFSContext*                context)
{
	FileHandle *file_handle;
	Bonobo_Stream_SeekType whence = 0;
	CORBA_Environment ev;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);

	file_handle = (FileHandle *) handle;

	switch (position) {
	case GNOME_VFS_SEEK_START:
		whence = Bonobo_Stream_SEEK_SET;
		break;
	case GNOME_VFS_SEEK_CURRENT:
		whence = Bonobo_Stream_SEEK_CUR;
		break;
	case GNOME_VFS_SEEK_END:
		whence = Bonobo_Stream_SEEK_END;
		break;
	}

	CORBA_exception_init (&ev);
	Bonobo_Stream_seek (file_handle->stream, offset, whence, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC);
	}
	CORBA_exception_free (&ev);

	switch (position) {
	case GNOME_VFS_SEEK_START:
		file_handle->pos = offset;
		break;
	case GNOME_VFS_SEEK_CURRENT:
		file_handle->pos += offset;
		break;
	case GNOME_VFS_SEEK_END:
		//FIXME!
		g_assert_not_reached ();
		break;
	}

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_tell (
	GnomeVFSMethod*		method,
	GnomeVFSMethodHandle*	handle,
	GnomeVFSFileOffset*	offset_return)
{
	FileHandle *file_handle;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);

	file_handle = (FileHandle *) handle;

	*offset_return = file_handle->pos;

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));
	
	return GNOME_VFS_OK;
}

typedef struct {
	Bonobo_Storage storage;
	Bonobo_Storage_DirectoryList *list;
	guint pos;
} DirectoryHandle;

static GnomeVFSResult do_open_directory (
        GnomeVFSMethod                *method,
        GnomeVFSMethodHandle         **handle,
        GnomeVFSURI                   *uri,
	GnomeVFSFileInfoOptions        options,
        const GnomeVFSDirectoryFilter *filter,
        GnomeVFSContext               *context)
{
	Bonobo_Storage storage;
	Bonobo_Storage_DirectoryList *list;
	DirectoryHandle *directory_handle;
	CORBA_Environment ev;
	gchar *moniker;

	CAM_VFS_DEBUG (("ENTER..."));
	g_static_mutex_lock (&mutex);
	CAM_VFS_DEBUG (("... done."));

	CORBA_exception_init (&ev);
	
	/* Get the storage */
	CAM_VFS_DEBUG (("Getting storage..."));
	moniker = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	storage = bonobo_get_object (moniker, "IDL:Bonobo/Storage:1.0", &ev);
	CAM_VFS_DEBUG (("... done."));
	g_free (moniker);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC);
	}

	/* Get the directory list */
	CAM_VFS_DEBUG (("Getting contents..."));
	list = Bonobo_Storage_listContents (storage, "", 
					    Bonobo_FIELD_CONTENT_TYPE | 
					    Bonobo_FIELD_SIZE | 
					    Bonobo_FIELD_TYPE, &ev);
	CAM_VFS_DEBUG (("... done."));
	if (BONOBO_EX (&ev)) {
		bonobo_object_release_unref (storage, NULL);
		CORBA_exception_free (&ev);
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC);
	}
	
	CORBA_exception_free (&ev);

	/* Construct the handle */
	CAM_VFS_DEBUG (("Constructing handle..."));
	directory_handle = g_new0 (DirectoryHandle, 1);
	directory_handle->storage = storage;
	directory_handle->list = list;
	directory_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) directory_handle;

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
	DirectoryHandle *directory_handle;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);
	
	/* Free the handle */
	directory_handle = (DirectoryHandle *) handle;
	bonobo_object_release_unref (directory_handle->storage, NULL);
	CORBA_free (directory_handle->list);

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*		info,
	GnomeVFSContext*                context)
{
	DirectoryHandle *dh;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);

	dh = (DirectoryHandle *) handle;
	if (dh->list->_length <= dh->pos) {
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_EOF)"));
		return (GNOME_VFS_ERROR_EOF);
	}
	
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);

	/* Tell gnome-vfs which fields will be valid */
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE |
			     GNOME_VFS_FILE_INFO_FIELDS_TYPE |
			     GNOME_VFS_FILE_INFO_FIELDS_SIZE |
			     GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	/* Fill the fields */
	info->name      = g_strdup (dh->list->_buffer [dh->pos].name);
	info->mime_type = g_strdup (dh->list->_buffer [dh->pos].content_type);
	info->size      = dh->list->_buffer [dh->pos].size;

	/* Directory or folder? */
	if (dh->list->_buffer [dh->pos].type == Bonobo_STORAGE_TYPE_REGULAR)
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
	else 
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;

	dh->pos++;

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));

	return (GNOME_VFS_OK);
}

static void
get_info_from_stream (Bonobo_Stream stream, GnomeVFSFileInfo *info, 
		      CORBA_Environment *ev)
{
	Bonobo_StorageInfo *storage_info;

	storage_info = Bonobo_Stream_getInfo (stream, 
			                      Bonobo_FIELD_CONTENT_TYPE | 
					      Bonobo_FIELD_SIZE | 
					      Bonobo_FIELD_TYPE, ev);
	if (BONOBO_EX (ev))
		return;

	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);

	/* Tell gnome-vfs which fields will be valid */
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE | 
			     GNOME_VFS_FILE_INFO_FIELDS_SIZE | 
			     GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	/* Fill the fields */
	info->name = g_strdup (storage_info->name);
	info->mime_type = g_strdup (storage_info->content_type);
	info->size = storage_info->size;

	if (storage_info->type == Bonobo_STORAGE_TYPE_REGULAR)
		info->type = GNOME_VFS_FILE_TYPE_REGULAR; 
	else 
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
}
	

static GnomeVFSResult do_get_file_info (
        GnomeVFSMethod*                 method,
        GnomeVFSURI*                    uri,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	gchar *moniker;
	CORBA_Environment ev;
	Bonobo_Stream stream;
	Bonobo_Storage storage;

	CAM_VFS_DEBUG (("ENTER..."));
	g_static_mutex_lock (&mutex);
	CAM_VFS_DEBUG (("... done."));

	CORBA_exception_init (&ev);

	/* Is this a file? */
	moniker = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	CAM_VFS_DEBUG (("Trying to get stream for %s...", moniker));
	stream = bonobo_get_object (moniker, "IDL:Bonobo/Stream:1.0", &ev);
	CAM_VFS_DEBUG (("... done."));
	if (BONOBO_EX (&ev)) {

		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);

		/* Is this a directory? */
		CAM_VFS_DEBUG (("Trying to get storage for %s...", moniker));
		storage = bonobo_get_object (moniker, 
					     "IDL:Bonobo/Storage:1.0", &ev);
		CAM_VFS_DEBUG (("... done."));
		g_free (moniker);
		if (BONOBO_EX (&ev)) {
			g_static_mutex_unlock (&mutex);
			CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_NOT_FOUND)"));
			return (GNOME_VFS_ERROR_NOT_FOUND);
		}

		bonobo_object_release_unref (storage, NULL);
		CORBA_exception_free (&ev);

		info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
				     GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

		info->name = gnome_vfs_uri_extract_dirname (uri);
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY; 
		info->mime_type = g_strdup ("x-directory/normal");

		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT"));

		return (GNOME_VFS_OK);
	}
	g_free (moniker);

	get_info_from_stream (stream, info, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC);
	}
	
	CORBA_exception_free (&ev);

	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));
	
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	CORBA_Environment ev;
	FileHandle *file_handle;

	CAM_VFS_DEBUG (("ENTER"));
	g_static_mutex_lock (&mutex);

	CORBA_exception_init (&ev);

	file_handle = (FileHandle *) handle;

	get_info_from_stream (file_handle->stream, info, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		g_static_mutex_unlock (&mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_GENERIC)"));
		return (GNOME_VFS_ERROR_GENERIC);
	}

	CORBA_exception_free (&ev);
	
	g_static_mutex_unlock (&mutex);
	CAM_VFS_DEBUG (("EXIT"));
	
	return (GNOME_VFS_OK);
}

static gboolean do_is_local (
        GnomeVFSMethod*                 method,
	const GnomeVFSURI*              uri)
{
	return (TRUE);
}

static GnomeVFSResult do_check_same_fs (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    a,
	GnomeVFSURI*                    b,
	gboolean*                       same_fs_return,
	GnomeVFSContext*                context)
{
	*same_fs_return = !strcmp (gnome_vfs_uri_get_host_name (a), 
			           gnome_vfs_uri_get_host_name (b));
	return (GNOME_VFS_OK);
}

static GnomeVFSMethod method = {
        sizeof (GnomeVFSMethod),
        do_open,
        do_create,
        do_close,
        do_read,
        do_write,
        do_seek,
        do_tell,
        NULL,                           /* do_truncate_handle           */
        do_open_directory,
        do_close_directory,
        do_read_directory,
        do_get_file_info,
        do_get_file_info_from_handle,
        do_is_local,
        NULL,                           /* do_make_directory */
        NULL,                           /* do_remove_directory          */
        NULL,                           /* do_move */
        NULL,                           /* do_unlink                    */
        do_check_same_fs,
        NULL,                           /* do_set_file_info             */
        NULL,                           /* do_truncate                  */
        NULL,                           /* do_find_directory            */
        NULL				/* do_create_symbolic_link      */
};

GnomeVFSMethod *vfs_module_init (const gchar *method_name, const gchar *args);
	
GnomeVFSMethod *
vfs_module_init (const gchar *method_name, const gchar *args)
{
	CORBA_ORB orb;
	
	CAM_VFS_DEBUG (("ENTER"));

	/* Initialize ORBit */
	if (oaf_is_initialized ())
		orb = oaf_orb_get ();
	else
		orb = oaf_init (0, NULL);
	if (!orb)
		g_error ("Cannot init oaf");

	/* Initialize bonobo */
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Cannot init bonobo");

	CAM_VFS_DEBUG (("EXIT"));

        return (&method);
}

void vfs_module_shutdown (GnomeVFSMethod *method);

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
        return;
}


