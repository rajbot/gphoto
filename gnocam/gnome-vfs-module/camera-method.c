#include <libgnome/gnome-defs.h>

#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-method.h>

#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-moniker-util.h>

#include <liboaf/oaf-mainloop.h>

#include <gtk/gtkmain.h>

#include <stdlib.h>
#include <stdio.h>

#define CAM_VFS_DEBUG(x) G_STMT_START {\
	printf ("%s:%d ", __FILE__,__LINE__);\
	printf ("%s() ", __FUNCTION__);\
	printf x;\
	fputc ('\n', stdout);\
	fflush (stdout);\
}G_STMT_END

//Ok, we need to
//#define USE_LOCK
//because ORBit is not thread safe. Michael Meeks answered:
//
//> I thought redirecting all request to a moniker could be the solution.
//> One moniker, several apps. Indeed, this works if I serialize the
//> requests in the module using a global mutex. If I just pass all
//> requests to the moniker, something hangs.
//
//Sadly, you are probably discovering that ORBit-stable ( and bonobo
//) are not thread safe in Gnome 1.4 - this is addressed in Gnome 2.0 with
//ORBit 2.0 [ and some pending bonobo work ]. You can only make CORBA calls
//[ and most bonobo calls ] from a single thread of execution.

#ifdef USE_LOCK
static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
#  define MUTEX_LOCK(mutex) g_static_mutex_lock (&(mutex));
#  define MUTEX_UNLOCK(mutex) g_static_mutex_unlock (&(mutex))
#else
#  define MUTEX_LOCK(mutex)
#  define MUTEX_UNLOCK(mutex)
#endif

static GnomeVFSResult
GNOME_VFS_RESULT (CORBA_Environment *ev)
{
	GnomeVFSResult result;
	const gchar *id;
	
	id = CORBA_exception_id (ev);
	
	/* Bonobo */
	if (!strcmp (id, ex_Bonobo_IOError))
		result = GNOME_VFS_ERROR_IO;
	else if (!strcmp (id, ex_Bonobo_NotSupported)) 
		result = GNOME_VFS_ERROR_NOT_SUPPORTED; 
	else if (!strcmp (id, ex_Bonobo_BadArg)) 
		result = GNOME_VFS_ERROR_BAD_PARAMETERS;

	/* Bonobo/Storage */
	else if (!strcmp (id, ex_Bonobo_Storage_NotFound))
		result = GNOME_VFS_ERROR_NOT_FOUND;
	else if (!strcmp (id, ex_Bonobo_Storage_NameExists))
		result = GNOME_VFS_ERROR_FILE_EXISTS;
	else if (!strcmp (id, ex_Bonobo_Storage_IOError))
		result = GNOME_VFS_ERROR_IO;
	else if (!strcmp (id, ex_Bonobo_Storage_NoPermission))
		result = GNOME_VFS_ERROR_ACCESS_DENIED;
	else if (!strcmp (id, ex_Bonobo_Storage_NotSupported))
		result = GNOME_VFS_ERROR_NOT_SUPPORTED;
	else if (!strcmp (id, ex_Bonobo_Storage_NotStream))
		result = GNOME_VFS_ERROR_NOT_FOUND;
	else if (!strcmp (id, ex_Bonobo_Storage_NotStorage))
		result = GNOME_VFS_ERROR_NOT_A_DIRECTORY;
	else if (!strcmp (id, ex_Bonobo_Storage_NotEmpty))
		result = GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY;

	/* Bonobo/Stream */
	else if (!strcmp (id, ex_Bonobo_Stream_NoPermission))
		result = GNOME_VFS_ERROR_ACCESS_DENIED;
	else if (!strcmp (id, ex_Bonobo_Stream_IOError))
		result = GNOME_VFS_ERROR_IO;

	/* Default */
	else
		result = GNOME_VFS_ERROR_GENERIC;

	return (result);
}

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
	GnomeVFSResult result;
	Bonobo_Stream stream;
	CORBA_Environment ev;
	FileHandle *file_handle;
	gchar *moniker;

	CAM_VFS_DEBUG (("ENTER"));
	
	CORBA_exception_init (&ev);

	/* Get the stream */ 
	CAM_VFS_DEBUG (("Getting stream...")); 
	moniker = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE); 
	stream = bonobo_get_object (moniker, "IDL:Bonobo/Stream:1.0", &ev); 
	g_free (moniker); 
	if (BONOBO_EX (&ev)) {
		g_warning ("Got exception: %s",
			   bonobo_exception_get_text (&ev));
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev); 
		return (result);
	}
	CORBA_exception_free (&ev);

	/* Construct handle */
	file_handle = g_new0 (FileHandle, 1);
	file_handle->stream = stream;
	file_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) file_handle;

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
	
	file_handle = (FileHandle *) handle;
	bonobo_object_release_unref (file_handle->stream, NULL);
	g_free (file_handle);

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
	GnomeVFSResult result;
	FileHandle *file_handle;
	CORBA_Environment ev;
	Bonobo_Stream_iobuf *iobuf;

	file_handle = (FileHandle *) handle;
	
	g_message ("Reading %i bytes...", (int) num_bytes);
	CORBA_exception_init (&ev);
	Bonobo_Stream_read (file_handle->stream, num_bytes, &iobuf, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Got exception: %s", 
			   bonobo_exception_get_text (&ev));
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		return (result);
	}
	CORBA_exception_free (&ev);
	g_message ("... done. I received %i bytes.", (int) iobuf->_length);

	/* Report an error if we are at the end of the file */
	if (!iobuf->_length) {
		CORBA_free (iobuf);
		return (GNOME_VFS_ERROR_EOF);
	}

	memcpy (buffer, iobuf->_buffer, iobuf->_length);
	*bytes_read = iobuf->_length;

	file_handle->pos += iobuf->_length;
	
	CORBA_free (iobuf);

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

	file_handle = (FileHandle *) handle;

	g_warning ("Implement do_write!");

	*bytes_written = num_bytes;

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
	GnomeVFSResult result;
	FileHandle *file_handle;
	Bonobo_Stream_SeekType whence = 0;
	CORBA_Environment ev;

	CAM_VFS_DEBUG (("ENTER"));

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
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		return (result);
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

	file_handle = (FileHandle *) handle;

	*offset_return = file_handle->pos;

	CAM_VFS_DEBUG (("EXIT"));
	
	return GNOME_VFS_OK;
}

typedef struct {
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
	GnomeVFSResult result;
	Bonobo_Storage storage;
	Bonobo_Storage_DirectoryList *list;
	DirectoryHandle *directory_handle;
	CORBA_Environment ev;
	gchar *moniker;

	CAM_VFS_DEBUG (("ENTER"));
	MUTEX_LOCK (mutex);

	CORBA_exception_init (&ev);
	
	/* Get the storage */
	moniker = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	CAM_VFS_DEBUG (("Getting storage for %s...", moniker));
	storage = bonobo_get_object (moniker, "IDL:Bonobo/Storage:1.0", &ev);
	CAM_VFS_DEBUG (("... done."));
	g_free (moniker);
	if (BONOBO_EX (&ev)) {
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		MUTEX_UNLOCK (mutex);
		return (result);
	}

	/* Get the directory list */
	CAM_VFS_DEBUG (("Getting contents..."));
	list = Bonobo_Storage_listContents (storage, "", 
					    Bonobo_FIELD_CONTENT_TYPE | 
					    Bonobo_FIELD_SIZE | 
					    Bonobo_FIELD_TYPE, &ev);
	CAM_VFS_DEBUG (("... done."));
	bonobo_object_release_unref (storage, NULL);
	if (BONOBO_EX (&ev)) {
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		MUTEX_UNLOCK (mutex);
		return (result);
	}
	
	CORBA_exception_free (&ev);

	/* Construct the handle */
	CAM_VFS_DEBUG (("Constructing handle..."));
	directory_handle = g_new0 (DirectoryHandle, 1);
	directory_handle->list = list;
	directory_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) directory_handle;

	MUTEX_UNLOCK (mutex);
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
	
	/* Free the handle */
	directory_handle = (DirectoryHandle *) handle;
	CORBA_free (directory_handle->list);

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

	dh = (DirectoryHandle *) handle;
	if (dh->list->_length <= dh->pos) {
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

	CAM_VFS_DEBUG (("EXIT"));

	return (GNOME_VFS_OK);
}

static void
get_info_from_storage_info (Bonobo_StorageInfo *storage_info, 
			    GnomeVFSFileInfo *info)
{
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

static void
get_info_from_storage (Bonobo_Storage storage, const gchar *name, 
		       GnomeVFSFileInfo *info, CORBA_Environment *ev)
{
	Bonobo_StorageInfo *storage_info;

	storage_info = Bonobo_Storage_getInfo (storage, name, 
					       Bonobo_FIELD_CONTENT_TYPE | 
					       Bonobo_FIELD_SIZE | 
					       Bonobo_FIELD_TYPE, ev);
	
	if (BONOBO_EX (ev))
		return;

	get_info_from_storage_info (storage_info, info);

        CORBA_free (storage_info);
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

	get_info_from_storage_info (storage_info, info);

	CORBA_free (storage_info);
}

static gchar *
get_basename (GnomeVFSURI *uri)
{
	gchar *text_uri;
	gchar *basename;
	GnomeVFSURI *tmp_uri;

	/* The problem: Trailing slashes. If there is a slash at the end 
	 * (for example in "file://home/lutz/",
	 * gnome_vfs_uri_basename will just return NULL, but 
	 * gnome_vfs_uri_get_parent will return "file://home". Then, bad 
	 * things will happen. Therefore, remove the trailing slash and 
	 * rejoice. */
	text_uri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	if (text_uri [strlen (text_uri) - 1] == '/')
		text_uri [strlen (text_uri) - 1] = '\0';

	tmp_uri = gnome_vfs_uri_new (text_uri);
	g_free (text_uri);

	basename = g_strdup (gnome_vfs_uri_get_basename (tmp_uri));
	gnome_vfs_uri_unref (tmp_uri);

	return (basename);
}

static GnomeVFSResult do_get_file_info (
        GnomeVFSMethod*                 method,
        GnomeVFSURI*                    uri,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	GnomeVFSResult result;
	gchar *moniker;
	gchar *basename;
	CORBA_Environment ev;
	Bonobo_Storage storage;
	GnomeVFSURI *tmp_uri;

	CAM_VFS_DEBUG (("ENTER"));
	MUTEX_LOCK (mutex);

	CORBA_exception_init (&ev);

	if (gnome_vfs_uri_has_parent (uri)) {
		basename = get_basename (uri);
		tmp_uri = gnome_vfs_uri_get_parent (uri);
		moniker = gnome_vfs_uri_to_string (tmp_uri, 
						   GNOME_VFS_URI_HIDE_NONE);
		gnome_vfs_uri_unref (tmp_uri);
	} else {
		basename = g_strdup (".");
		moniker = gnome_vfs_uri_to_string (uri, 
						   GNOME_VFS_URI_HIDE_NONE);
	}

	CAM_VFS_DEBUG (("Trying to get storage for %s...", moniker)); 
	usleep (300);
	storage = bonobo_get_object (moniker, "IDL:Bonobo/Storage:1.0", &ev); 
	CAM_VFS_DEBUG (("... done.")); 
	g_free (moniker);
	if (BONOBO_EX (&ev)) {
		g_warning ("Got exception: %s",
			   bonobo_exception_get_text (&ev));
		g_free (basename);
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		MUTEX_UNLOCK (mutex);
		return (result);
	}

	CAM_VFS_DEBUG (("Getting info for %s from storage...", basename));
	get_info_from_storage (storage, basename, info, &ev);
	CAM_VFS_DEBUG (("... done."));
	bonobo_object_release_unref (storage, NULL);
	g_free (basename);
	if (BONOBO_EX (&ev)) {
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		MUTEX_UNLOCK (mutex);
		return (result);
	}

	CORBA_exception_free (&ev);

	CAM_VFS_DEBUG (("mime-type: %s", info->mime_type));
	CAM_VFS_DEBUG (("size: %i", (int) info->size));
	CAM_VFS_DEBUG (("type: %i", info->type));
	
	MUTEX_UNLOCK (mutex);
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
	GnomeVFSResult result;
	CORBA_Environment ev;
	FileHandle *file_handle;

	CAM_VFS_DEBUG (("ENTER"));

	CORBA_exception_init (&ev);

	file_handle = (FileHandle *) handle;

	get_info_from_stream (file_handle->stream, info, &ev);
	if (BONOBO_EX (&ev)) {
		result = GNOME_VFS_RESULT (&ev);
		CORBA_exception_free (&ev);
		return (result);
	}

	CORBA_exception_free (&ev);
	
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

	/* Initialize GTK */
	if (!gtk_init_check (0, NULL))
		g_error ("Cannot init GTK");

	/* Initialize ORBit */
	if (oaf_is_initialized ())
		orb = oaf_orb_get ();
	else
		orb = oaf_init (0, NULL);
	if (!orb)
		g_error ("Cannot init oaf");

	/* Initialize bonobo */
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("Cannot init Bonobo!");

	/* Activate the Bonobo POA manager */
	if (!bonobo_activate ())
		g_error ("Cannot activate Bonobo POA manager!");

	CAM_VFS_DEBUG (("EXIT"));

        return (&method);
}

void vfs_module_shutdown (GnomeVFSMethod *method);

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
        return;
}


