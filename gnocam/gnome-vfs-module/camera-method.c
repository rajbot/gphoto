#include <stdlib.h>
#include <gtk/gtk.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>
#include <parser.h>
#include "utils.h"

/********************/
/* Static variables */
/********************/

static GConfClient* 	client = NULL;
static GMutex*		client_mutex = NULL;

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
	GnomeVFSContext* 		context);
static GnomeVFSResult do_create	(
	GnomeVFSMethod* 		method, 
	GnomeVFSMethodHandle** 		handle, 
	GnomeVFSURI* 			uri, 
	GnomeVFSOpenMode 		mode, 
	gboolean 			exclusive, 
	guint 				perm, 
	GnomeVFSContext* 		context);
static GnomeVFSResult do_close (
	GnomeVFSMethod* 		method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSContext*		context);
static GnomeVFSResult do_read (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	gpointer			buffer,
	GnomeVFSFileSize		num_bytes,
	GnomeVFSFileSize*		bytes_read,
	GnomeVFSContext*		context);
static GnomeVFSResult do_write (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	gconstpointer			buffer,
	GnomeVFSFileSize		num_bytes,
	GnomeVFSFileSize*		bytes_written,
	GnomeVFSContext*		context);
static GnomeVFSResult do_seek (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSSeekPosition		whence,
	GnomeVFSFileOffset		offset,
	GnomeVFSContext*		context);
static GnomeVFSResult do_open_directory (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle**		handle,
	GnomeVFSURI*			uri,
	GnomeVFSFileInfoOptions		options,
	const GnomeVFSDirectoryFilter*	filter,
	GnomeVFSContext*		context);
static GnomeVFSResult do_close_directory (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSContext*		context);
static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSFileInfo*		file_info,
	GnomeVFSContext*		context);
static GnomeVFSResult do_get_file_info (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	GnomeVFSFileInfo*		file_info,
	GnomeVFSFileInfoOptions		options,
	GnomeVFSContext*		context);
static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSFileInfo*		file_info,
	GnomeVFSFileInfoOptions		options,
	GnomeVFSContext*		context);
static gboolean do_is_local (
	GnomeVFSMethod*			method,
	const GnomeVFSURI*		uri);
static GnomeVFSResult do_make_directory (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	guint				perm,
	GnomeVFSContext*		context);
static GnomeVFSResult do_remove_directory (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	GnomeVFSContext*		context);
static GnomeVFSResult do_move (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			old_uri,
	GnomeVFSURI*			new_uri,
	gboolean			force_replace,
	GnomeVFSContext*		context);
static GnomeVFSResult do_check_same_fs (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			a,
	GnomeVFSURI*			b,
	gboolean*			same_fs_return,
	GnomeVFSContext*		context);
static GnomeVFSResult do_set_file_info (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	const GnomeVFSFileInfo*		file_info,
	GnomeVFSSetFileInfoMask		mask,
	GnomeVFSContext*		context);

/********************/
/* Static Variables */
/********************/

static GnomeVFSMethod method = {
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	NULL, 				/* do_tell 			*/
	NULL, 				/* do_truncate_handle		*/
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	NULL, 				/* do_unlink			*/
	do_check_same_fs,
	do_set_file_info,
	NULL, 				/* do_truncate			*/
	NULL,				/* do_find_directory 		*/
	NULL};				/* do_create_symbolic_link	*/

/*************/
/* Functions */
/*************/

int gp_frontend_message (Camera* camera, char *message)
{
	g_print ("MESSAGE: %s\n", message);
	return (GP_OK);
}

GnomeVFSMethod*
vfs_module_init (const gchar* method_name, const gchar* args)
{
	gchar*	argv[] = {"dummy"};
	
	/* GConf */
	if (!gconf_is_initialized ()) gconf_init (1, argv, NULL);
	client = gconf_client_get_default ();
	gtk_object_ref (GTK_OBJECT (client));
	gtk_object_sink (GTK_OBJECT (client));
#ifdef G_THREADS_ENABLED
	if (g_thread_supported ()) client_mutex = g_mutex_new ();
	else client_mutex = NULL;
#endif
	
	/* GPhoto */
	gp_init (GP_DEBUG_NONE);
	gp_frontend_register (NULL, NULL, gp_frontend_message, NULL, NULL);
	
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod* method)
{
	/* GConf */
	gtk_object_destroy (GTK_OBJECT (client));
	gtk_object_unref (GTK_OBJECT (client));
	client = NULL;
#ifdef G_THREADS_ENABLED
	if (g_thread_supported ()) g_mutex_free (client_mutex);
#endif

	/* GPhoto */
	gp_exit ();
	
	return;
}

static GnomeVFSResult do_open (
	GnomeVFSMethod* 		method, 
	GnomeVFSMethodHandle** 		handle, 
	GnomeVFSURI* 			uri, 
	GnomeVFSOpenMode 		mode, 
	GnomeVFSContext* 		context)
{
	GnomeVFSResult	result;
	
	if ((mode == GNOME_VFS_OPEN_READ) || (mode == GNOME_VFS_OPEN_WRITE)) *handle = file_handle_new (uri, mode, client, client_mutex, context, &result);
	else result = GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	
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
	
	g_print ("CAMERA: do_close\n");

	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail ((file_handle->mode == GNOME_VFS_OPEN_WRITE) || (file_handle->mode == GNOME_VFS_OPEN_READ), GNOME_VFS_ERROR_BAD_PARAMETERS);
	
	if (file_handle->mode == GNOME_VFS_OPEN_WRITE) {
		if (gp_camera_file_put (file_handle->camera, file_handle->file, file_handle->folder) != GP_OK) {
			file_handle_free (handle);
			return (GNOME_VFS_ERROR_GENERIC);
		}
	}
	
	return (file_handle_free (handle));
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

	/* Do we have num_bytes left? */
	if (file_handle->position + num_bytes >= file_handle->file->size) {
		if ((*bytes_read = file_handle->file->size - file_handle->position) <= 0) return (GNOME_VFS_ERROR_EOF);
		memcpy (buffer, file_handle->file->data + file_handle->position, *bytes_read);
	} else {
		memcpy (buffer, file_handle->file->data + file_handle->position, (*bytes_read = num_bytes));
	}
	
	file_handle->position += *bytes_read;
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
	
	*bytes_written = 0;
	if (gp_file_append (file_handle->file, (gchar*) buffer, num_bytes) != GP_OK) return (GNOME_VFS_ERROR_GENERIC);
	*bytes_written = num_bytes;
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

static GnomeVFSResult do_open_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle**          handle,
        GnomeVFSURI*                    uri,
	GnomeVFSFileInfoOptions         options,
        const GnomeVFSDirectoryFilter*  filter,
        GnomeVFSContext*                context)
{
	GnomeVFSResult		result;

	g_print ("CAMERA: do_open_directory\n");
	
	*handle = directory_handle_new (uri, client, client_mutex, options, context, &result);
	return (result);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
	g_return_val_if_fail (handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	
	return (directory_handle_free (handle));
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*		info,
	GnomeVFSContext*                context)
{
	directory_handle_t*	directory_handle;
	
	g_return_val_if_fail (directory_handle = (directory_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;
	if (directory_handle->position < g_slist_length (directory_handle->folders)) {

		/* Folder */
		info->name = g_strdup (g_slist_nth_data (directory_handle->folders, directory_handle->position));
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (directory_handle->options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("x-directory/normal");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	} else if (directory_handle->position < g_slist_length (directory_handle->folders) + g_slist_length (directory_handle->files)) {

		/* File */
		info->name = g_strdup (g_slist_nth_data (directory_handle->files, directory_handle->position - g_slist_length (directory_handle->folders)));
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (directory_handle->options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("image/jpeg");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	} else {
		directory_handle->position = -1;
		g_print (" -> EOF\n");
		return (GNOME_VFS_ERROR_EOF);
	}
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);
	
	directory_handle->position++;
	g_print (" -> (%i) %s\n", directory_handle->position - 1, info->name);
	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_get_file_info (
        GnomeVFSMethod*                 method,
        GnomeVFSURI*                    uri,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	gchar*		filename = NULL;
	gchar*		dirname = NULL;
	GnomeVFSResult	result;
	CameraList	list;
	Camera*		camera;
	gint		i;
	
	/* Connect to the camera. */
	if (!(camera = camera_new_by_uri (uri, client, client_mutex, context, &result))) return (result);

	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;
	filename = (gchar*) gnome_vfs_uri_get_basename (uri);
	dirname = gnome_vfs_uri_extract_dirname (uri);
	if (filename) {

		/* Check if existent. */
		if (gp_camera_file_list (camera, &list, dirname) != GP_OK) {
			gp_camera_unref (camera);
			return (GNOME_VFS_ERROR_GENERIC);
		}
		gp_camera_unref (camera);
		for (i = 0; i < gp_list_count (&list); i++) if (!strcmp (filename, gp_list_entry (&list, i)->name)) break;
		if (i == gp_list_count (&list)) return (GNOME_VFS_ERROR_NOT_FOUND);
		info->name = g_strdup (filename);
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("image/png");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	} else {

		/* Check if existent (only non-root). */
		if (strcmp (dirname, "/")) {
			if (gp_camera_folder_list (camera, &list, dirname) != GP_OK) {
				gp_camera_unref (camera);
				return (GNOME_VFS_ERROR_GENERIC);
			}
			gp_camera_unref (camera);
			for (i = 0; i < gp_list_count (&list); i++) if (!strcmp (dirname, gp_list_entry (&list, i)->name)) break;
			if (i == gp_list_count (&list)) return (GNOME_VFS_ERROR_NOT_FOUND);
		}

		info->name = g_strdup (dirname);
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("x-directory/normal");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	}
	info->flags = GNOME_VFS_FILE_FLAGS_NONE;
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);	

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
	/* 'Directory Browse' is something different. */
	return (!strcmp (gnome_vfs_uri_get_host_name (uri), "Directory Browse"));
}

static GnomeVFSResult do_make_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	guint                           perm,
	GnomeVFSContext*                context)
{
	g_print ("do_make_directory\n");
	return (GNOME_VFS_ERROR_NOT_SUPPORTED);
}

static GnomeVFSResult do_remove_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSContext*                context)
{
	g_print ("do_remove_directory\n");
	return (GNOME_VFS_ERROR_NOT_SUPPORTED);
}

static GnomeVFSResult do_move (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    old_uri,
	GnomeVFSURI*                    new_uri,
	gboolean                        force_replace,
	GnomeVFSContext*                context)
{
	g_print ("do_move\n");
	return (GNOME_VFS_ERROR_NOT_SUPPORTED);
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

static GnomeVFSResult do_set_file_info (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	const GnomeVFSFileInfo*         file_info,
	GnomeVFSSetFileInfoMask         mask,
	GnomeVFSContext*                context)
{
	g_print ("do_set_file_info\n");
	return (GNOME_VFS_ERROR_NOT_SUPPORTED);
}


