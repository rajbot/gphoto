#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-cancellation.h>
#include <libgnomevfs/gnome-vfs-cancellable-ops.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-method.h>

#include <gphoto-extensions.h>
#include "utils.h"

//#define USE_BONOBO
//#undef G_THREADS_ENABLED

#ifdef G_THREADS_ENABLED
#define MUTEX_NEW()     g_mutex_new ()
#define MUTEX_FREE(a)   g_mutex_free (a)
#define MUTEX_LOCK(a)   if ((a) != NULL) g_mutex_lock (a)
#define MUTEX_UNLOCK(a) if ((a) != NULL) g_mutex_unlock (a)
#else
#define MUTEX_NEW()     NULL
#define MUTEX_FREE(a)
#define MUTEX_LOCK(a)
#define MUTEX_UNLOCK(a)
#endif

static GMutex*	client_mutex = NULL;

/**************/
/* Prototypes */
/**************/

GnomeVFSMethod* vfs_module_init (const gchar* method_name, const gchar* args);
void 		vfs_module_shutdown (GnomeVFSMethod* method);

static GnomeVFSResult do_open (
	GnomeVFSMethod* 		method, 
	GnomeVFSMethodHandle** 		handle, 
	GnomeVFSURI* 			uri, 
	GnomeVFSOpenMode 		mode, 
	GnomeVFSContext* 		context)
{
	GnomeVFSResult	result;

	MUTEX_LOCK(client_mutex);
	CAM_VFS_DEBUG (("entering"));
	
	if ((mode == GNOME_VFS_OPEN_READ) || (mode == GNOME_VFS_OPEN_WRITE)) 
		*handle = file_handle_new (uri, mode, context, &result);

	else result = GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	
	MUTEX_UNLOCK(client_mutex);
	return (result);
}

static GnomeVFSResult do_create ( 
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle**          handle,
        GnomeVFSURI*                    uri,
        GnomeVFSOpenMode                mode,
        gboolean                        exclusive,
        guint                           perm,
        GnomeVFSContext*                context)
{
	return (do_open (method, handle, uri, mode, context));
}

static GnomeVFSResult do_close (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSContext*                context)
{
	file_handle_t*		file_handle;
	GnomeVFSResult		result;
	
	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail ((file_handle->mode == GNOME_VFS_OPEN_WRITE) || (file_handle->mode == GNOME_VFS_OPEN_READ), GNOME_VFS_ERROR_BAD_PARAMETERS);
	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("entering"));
	
	if (file_handle->mode == GNOME_VFS_OPEN_WRITE) {
		result = GNOME_VFS_RESULT (gp_camera_folder_put_file (file_handle->camera, file_handle->folder, file_handle->file));
		file_handle_free (handle);

	} else if (file_handle->mode == GNOME_VFS_OPEN_READ) {
		result = file_handle_free (handle);

	} else result = GNOME_VFS_ERROR_BAD_PARAMETERS;

	CAM_VFS_DEBUG (("exiting"));
	MUTEX_UNLOCK (client_mutex);
	return (result);
}

static GnomeVFSResult do_read (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        gpointer                        buffer,
        GnomeVFSFileSize                num_bytes,
        GnomeVFSFileSize*               bytes_read,
        GnomeVFSContext*                context)
{
	file_handle_t*		file_handle = NULL;

	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (file_handle->mode == GNOME_VFS_OPEN_READ, GNOME_VFS_ERROR_BAD_PARAMETERS);
	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("entering"));

	/* Do we have num_bytes left? */
	CAM_VFS_DEBUG (("num_bytes=%d", (int) num_bytes));
	CAM_VFS_DEBUG (("file_handle->file->size=%d", (int) file_handle->file->size));
	if (file_handle->position + num_bytes >= file_handle->file->size) {
		if ((*bytes_read = file_handle->file->size - file_handle->position) <= 0) {
			CAM_VFS_DEBUG (("returning GNOME_VFS_ERROR_EOF"));
			MUTEX_UNLOCK (client_mutex);
			return (GNOME_VFS_ERROR_EOF);
		}
		memcpy (buffer, file_handle->file->data + file_handle->position, *bytes_read);
	} else {
		memcpy (buffer, file_handle->file->data + file_handle->position, (*bytes_read = num_bytes));
	}
	
	file_handle->position += *bytes_read;
	CAM_VFS_DEBUG (("returning GNOME_VFS_OK"));
	MUTEX_UNLOCK (client_mutex);
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_write (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	gconstpointer                   buffer,
	GnomeVFSFileSize                num_bytes,
	GnomeVFSFileSize*               bytes_written,
	GnomeVFSContext*                context)
{
	file_handle_t* 		file_handle = NULL;
	
	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (file_handle->mode == GNOME_VFS_OPEN_WRITE, GNOME_VFS_ERROR_BAD_PARAMETERS);
	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("entering"));
	
	*bytes_written = 0;
	if (gp_file_append (file_handle->file, (gchar*) buffer, num_bytes) != GP_OK) {
		CAM_VFS_DEBUG (("returning GNOME_VFS_ERROR_GENERIC"));
		MUTEX_UNLOCK (client_mutex);
		return (GNOME_VFS_ERROR_GENERIC);
	}
	*bytes_written = num_bytes;
	CAM_VFS_DEBUG (("returning GNOME_VFS_OK"));
	MUTEX_UNLOCK (client_mutex);
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_seek (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSSeekPosition            position,
        GnomeVFSFileOffset              offset,
        GnomeVFSContext*                context)
{
	file_handle_t*		file_handle = NULL;
	
	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	CAM_VFS_DEBUG (("entering"));

	switch (position) {
	case GNOME_VFS_SEEK_START:
		file_handle->position = (long) offset;
		return (GNOME_VFS_OK);
	case GNOME_VFS_SEEK_CURRENT:
		file_handle->position += (long) offset;
		return (GNOME_VFS_OK);
	case GNOME_VFS_SEEK_END:
		file_handle->position = file_handle->file->size - 1 + (long) offset;
		return (GNOME_VFS_OK);
	default:
		return (GNOME_VFS_ERROR_BAD_PARAMETERS);
	}
}

static GnomeVFSResult do_tell (
	GnomeVFSMethod*		method,
	GnomeVFSMethodHandle*	handle,
	GnomeVFSFileOffset*	offset_return)
{
	file_handle_t*		file_handle = NULL;

	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);

	*offset_return = file_handle->position;
	return GNOME_VFS_OK;
}

#ifdef USE_BONOBO
typedef struct {
	Bonobo_Storage_DirectoryList *list;
	guint pos;
} DirectoryHandle;
#endif

static GnomeVFSResult do_open_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle**          handle,
        GnomeVFSURI*                    uri,
	GnomeVFSFileInfoOptions         options,
        const GnomeVFSDirectoryFilter*  filter,
        GnomeVFSContext*                context)
{
#ifdef USE_BONOBO
	Bonobo_Storage storage;
	Bonobo_Storage_DirectoryList *list;
	DirectoryHandle *directory_handle;
	CORBA_Environment ev;
	gchar *moniker;
#else
	GnomeVFSResult result;
#endif

	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("ENTER"));

#ifdef USE_BONOBO
	CORBA_exception_init (&ev);
	
	/* Get the storage */
	CAM_VFS_DEBUG (("Getting storage..."));
	moniker = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	storage = bonobo_get_object (moniker, "IDL:Bonobo/Storage:1.0", &ev);
	g_free (moniker);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		MUTEX_UNLOCK (client_mutex);
		return (GNOME_VFS_ERROR_GENERIC);
	}

	/* Get the directory list */
	CAM_VFS_DEBUG (("Getting contents..."));
	list = Bonobo_Storage_listContents (storage, "", 
					    Bonobo_FIELD_CONTENT_TYPE | 
					    Bonobo_FIELD_SIZE | 
					    Bonobo_FIELD_TYPE, &ev);
	bonobo_object_release_unref (storage, NULL);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		MUTEX_UNLOCK (client_mutex);
		return (GNOME_VFS_ERROR_GENERIC);
	}
	
	CORBA_exception_free (&ev);

	/* Construct the handle */
	CAM_VFS_DEBUG (("Constructing handle..."));
	directory_handle = g_new0 (DirectoryHandle, 1);
	directory_handle->list = list;
	directory_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) directory_handle;
#else
	*handle = directory_handle_new (uri, options, context, &result);
	if (result != GNOME_VFS_OK) {
		MUTEX_UNLOCK (client_mutex);
		return (result);
	}
#endif

	MUTEX_UNLOCK (client_mutex);
	CAM_VFS_DEBUG (("EXIT"));
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
#ifdef USE_BONOBO
	DirectoryHandle *directory_handle;
#else
	GnomeVFSResult result;
#endif

	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("ENTER"));
	
#ifdef USE_BONOBO
	/* Free the handle */
	directory_handle = (DirectoryHandle *) handle;
	CORBA_free (directory_handle->list);
#else
	result = directory_handle_free (handle);
	if (result != GNOME_VFS_OK) {
		MUTEX_UNLOCK (client_mutex);
		return (result);
	}
#endif

	MUTEX_UNLOCK (client_mutex);
	CAM_VFS_DEBUG (("EXIT"));
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*		info,
	GnomeVFSContext*                context)
{
#ifdef USE_BONOBO
	DirectoryHandle *dh;
#else
	directory_handle_t*	h;
#endif

	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("ENTER"));

#ifdef USE_BONOBO
	dh = (DirectoryHandle *) handle;
	if (dh->list->_length <= dh->pos) {
		MUTEX_UNLOCK (client_mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_EOF)"));
		return (GNOME_VFS_ERROR_EOF);
	}
#else
	h = (directory_handle_t*) handle;
	if (h->position >= g_slist_length (h->folders) + 
						g_slist_length (h->files)) {
		h->position = -1; 
		MUTEX_UNLOCK (client_mutex);
		CAM_VFS_DEBUG (("EXIT (GNOME_VFS_ERROR_EOF)"));
		return (GNOME_VFS_ERROR_EOF); 
	} 
							      
#endif
	
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);

#ifdef USE_BONOBO

	/* Tell gnome-vfs which fields will be valid */
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE |
			     GNOME_VFS_FILE_INFO_FIELDS_TYPE |
			     GNOME_VFS_FILE_INFO_FIELDS_SIZE |
			     GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	/* Fill the fields */
	info->name      = g_strdup (dh->list->_buffer [dh->pos].name);
	info->mime_type = g_strdup (dh->list->_buffer [dh->pos].content_type);
	info->size      = dh->list->_buffer [dh->pos].size;

	CAM_VFS_DEBUG (("name: %s", info->name));
	CAM_VFS_DEBUG (("mime_type: %s", info->mime_type));
	CAM_VFS_DEBUG (("size: %i", (int) info->size));

	/* Directory or folder? */
	if (dh->list->_buffer [dh->pos].type == Bonobo_STORAGE_TYPE_REGULAR)
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
	else 
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;

	dh->pos++;

#else
	if (h->position < g_slist_length (h->folders)) {

		/* Folder */
		info->name = g_strdup (g_slist_nth_data (h->folders, 
			    				 h->position));
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		info->mime_type = g_strdup ("x-directory/normal");
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		CAM_VFS_DEBUG (("%s is a folder",info->name));
	} else if (h->position < g_slist_length (h->folders) + 
				      g_slist_length (h->files)) {
		GnomeVFSResult	 result;
		const gchar 	*file;

		/* File */
		file = g_slist_nth_data (h->files,
			h->position - g_slist_length (h->folders));
		result = gp_camera_file_get_vfs_info (h->camera, 
						      h->folder, file, 
						      info, h->preview);
		if (result != GNOME_VFS_OK) {
		    	
		    	/* We always have to return GNOME_VFS_OK... */
			info->name = g_strdup (file);

			/* Type */
			info->type = GNOME_VFS_FILE_TYPE_REGULAR;
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		}
		CAM_VFS_DEBUG (("%s is a file",info->name));
	}
	
	h->position++;
	CAM_VFS_DEBUG ((" -> (%i) %s", h->position - 1, info->name));
#endif

	MUTEX_UNLOCK (client_mutex);
	CAM_VFS_DEBUG (("EXIT"));
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_get_file_info (
        GnomeVFSMethod*                 method,
        GnomeVFSURI*                    uri,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	const gchar	*filename = NULL;
	gchar		*dirname = NULL;
	GnomeVFSResult	 result;
	CameraList	 camera_list;
	Camera		*camera = NULL;
	gint		 i;
	gboolean	 file;
	gboolean	 preview;
	const gchar	*escaped_host;
	gchar		*host;
	gchar		*tmp;

	MUTEX_LOCK (client_mutex);
	CAM_VFS_DEBUG (("ENTER"));

	tmp = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	CAM_VFS_DEBUG (("uri: %s", tmp));
	g_free (tmp);
	
	escaped_host = gnome_vfs_uri_get_host_name (uri);
	if (!escaped_host) {
	    	CAM_VFS_DEBUG (("returning GNOME_VFS_ERROR_HOST_NOT_FOUND"));
		MUTEX_UNLOCK (client_mutex);
		return (GNOME_VFS_ERROR_HOST_NOT_FOUND);
	}
	host = gnome_vfs_unescape_string (escaped_host, NULL);
	CAM_VFS_DEBUG (("host: %s", host));

	filename = gnome_vfs_uri_get_basename (uri);
	CAM_VFS_DEBUG (("filename: %s", filename));

	dirname = gnome_vfs_uri_extract_dirname (uri);
	CAM_VFS_DEBUG (("dirname: %s", dirname));

	preview = (gnome_vfs_uri_get_user_name (uri) && 
		   !strcmp (gnome_vfs_uri_get_user_name (uri), "previews"));
	CAM_VFS_DEBUG (("preview: %i", preview));
	
	/* Connect to the camera */
	result = GNOME_VFS_RESULT (gp_camera_new_from_gconf (&camera, host));
	g_free (host);
	if (result != GNOME_VFS_OK) {
		CAM_VFS_DEBUG (("Could not connect to camera!"));
		MUTEX_UNLOCK (client_mutex);
                return (result);
        }

	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;

	/* File or folder? */
	file = FALSE;
	if (filename) {

	    	/* Get the list of files */
	    	CAM_VFS_DEBUG (("  Getting list of files..."));
		result = GNOME_VFS_RESULT (gp_camera_folder_list_files (
	        			camera, dirname, &camera_list));
		if (result != GNOME_VFS_OK) {
			gp_camera_unref (camera);
			CAM_VFS_DEBUG (("returning GNOME_VFS_ERROR_GENERIC"));
			MUTEX_UNLOCK (client_mutex);
			return (result);
		}

		/* Is the file in there? */
		for (i = 0; i < gp_list_count (&camera_list); i++) 
			CAM_VFS_DEBUG (("camera_list[%d]->name=%s",
			    i,gp_list_entry (&camera_list, i)->name));
		for (i = 0; i < gp_list_count (&camera_list); i++)
			if (!strcmp (filename, 
				     gp_list_entry (&camera_list, i)->name))
				break;
		if (i != gp_list_count (&camera_list))
		    	file = TRUE;
	}

	/* If folder, do we have this folder (only if non-root)? */
	if (!file && strcmp (dirname, "/")) {

	    	/* Get the list of folders */
	    	CAM_VFS_DEBUG (("Getting list of folders..."));
	    	result = GNOME_VFS_RESULT (gp_camera_folder_list_folders (
						camera, dirname, &camera_list));
		if (result != GNOME_VFS_OK) {
			gp_camera_unref (camera);
			CAM_VFS_DEBUG (("returning GNOME_VFS_ERROR_GENERIC"));
			MUTEX_UNLOCK (client_mutex);
			return (result);
		}

		/* Is the folder in there? */
		for (i = 0; i < gp_list_count (&camera_list); i++) 
			CAM_VFS_DEBUG (("camera_list[%d]->name=%s",
				i,gp_list_entry (&camera_list, i)->name));
		for (i = 0; i < gp_list_count (&camera_list); i++) 
			if (!strcmp (filename, 
				gp_list_entry (&camera_list, i)->name)) 
				break;
		if (i == gp_list_count (&camera_list)) {
			gp_camera_unref (camera);
			CAM_VFS_DEBUG (("returning GNOME_VFS_ERROR_NOT_FOUND"));
			MUTEX_UNLOCK (client_mutex);
			return (GNOME_VFS_ERROR_NOT_FOUND);
		}
	}

	if (file) {
	    	CAM_VFS_DEBUG (("  Getting file information..."));
		result = gp_camera_file_get_vfs_info (camera, dirname, 
						      filename, info, preview);
		if (result != GNOME_VFS_OK) {
		    	gp_camera_unref (camera);
			MUTEX_UNLOCK (client_mutex);
		    	return (result);
		}
	} else {
		info->name = g_strdup (dirname);
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("x-directory/normal");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	}
	gp_camera_unref (camera);

	info->flags = GNOME_VFS_FILE_FLAGS_NONE;
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);

	MUTEX_UNLOCK (client_mutex);
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
	return (do_get_file_info (method, ((file_handle_t*) handle)->uri, info, options, context));
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
	*same_fs_return = !strcmp (gnome_vfs_uri_get_host_name (a), gnome_vfs_uri_get_host_name (b));
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

GnomeVFSMethod*
vfs_module_init (const gchar* method_name, const gchar* args)
{
#ifdef USE_BONOBO
	CORBA_ORB orb;
#endif
	
	CAM_VFS_DEBUG (("ENTER"));

#ifdef USE_BONOBO
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
#else

        gtk_init (0, NULL);
#endif

        gp_init (GP_DEBUG_NONE);
        gp_frontend_register (NULL, NULL, NULL, NULL, NULL);
	client_mutex = MUTEX_NEW();

        return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod* method)
{
        gp_exit ();
	MUTEX_FREE(client_mutex);

        return;
}


