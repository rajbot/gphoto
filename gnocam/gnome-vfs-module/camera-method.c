#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>
#include <gnome.h>
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

	if ((mode == GNOME_VFS_OPEN_READ) || (mode == GNOME_VFS_OPEN_WRITE)) 
		*handle = file_handle_new (uri, mode, context, &result);

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
	GnomeVFSResult		result;
	
	g_return_val_if_fail (file_handle = (file_handle_t*) handle, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail ((file_handle->mode == GNOME_VFS_OPEN_WRITE) || (file_handle->mode == GNOME_VFS_OPEN_READ), GNOME_VFS_ERROR_BAD_PARAMETERS);
	
	if (file_handle->mode == GNOME_VFS_OPEN_WRITE) {
		result = GNOME_VFS_RESULT (gp_camera_file_put (file_handle->camera, file_handle->file, file_handle->folder));
		file_handle_free (handle);

	} else if (file_handle->mode == GNOME_VFS_OPEN_READ) {
		result = file_handle_free (handle);

	} else result = GNOME_VFS_ERROR_BAD_PARAMETERS;

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

static GnomeVFSResult do_open_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle**          handle,
        GnomeVFSURI*                    uri,
	GnomeVFSFileInfoOptions         options,
        const GnomeVFSDirectoryFilter*  filter,
        GnomeVFSContext*                context)
{
	GnomeVFSResult		result;

	*handle = directory_handle_new (uri, options, context, &result);
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
		return (GNOME_VFS_ERROR_EOF);
	}
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);
	
	directory_handle->position++;
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
	CameraList	camera_list;
	Camera*		camera = NULL;
	gint		i;

        /* Connect to the camera. */
        {
                gchar* url = gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (uri), NULL);

                if (!url) {
                        return (GNOME_VFS_ERROR_HOST_NOT_FOUND);
                }
                if ((result = GNOME_VFS_RESULT (gp_camera_new_from_gconf (&camera, url))) != GNOME_VFS_OK) {
                        g_free (url);
                        return (result);
                }
                g_free (url);
        }

	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;
	filename = (gchar*) gnome_vfs_uri_get_basename (uri);
	dirname = gnome_vfs_uri_extract_dirname (uri);
	if (filename) {

		/* Check if existent. */
		if (gp_camera_file_list (camera, &camera_list, dirname) != GP_OK) {
			gp_camera_unref (camera);
			return (GNOME_VFS_ERROR_GENERIC);
		}
		gp_camera_unref (camera);
		for (i = 0; i < gp_list_count (&camera_list); i++) if (!strcmp (filename, gp_list_entry (&camera_list, i)->name)) break;
		if (i == gp_list_count (&camera_list)) return (GNOME_VFS_ERROR_NOT_FOUND);
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
			if (gp_camera_folder_list (camera, &camera_list, dirname) != GP_OK) {
				gp_camera_unref (camera);
				return (GNOME_VFS_ERROR_GENERIC);
			}
			gp_camera_unref (camera);
			for (i = 0; i < gp_list_count (&camera_list); i++) if (!strcmp (dirname, gp_list_entry (&camera_list, i)->name)) break;
			if (i == gp_list_count (&camera_list)) return (GNOME_VFS_ERROR_NOT_FOUND);
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
        gtk_init (0, NULL);
        gp_init (GP_DEBUG_NONE);
        gp_frontend_register (NULL, NULL, NULL, NULL, NULL);

        return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod* method)
{
        gp_exit ();

        return;
}


