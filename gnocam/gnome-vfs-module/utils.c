#include <config.h>
#include <gphoto2.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>

#include <gphoto-extensions.h>

#include "utils.h"

/*************/
/* Functions */
/*************/

GnomeVFSResult
GNOME_VFS_RESULT (int result)
{
	switch (result) {
	case GP_OK:
		return (GNOME_VFS_OK);
	case GP_ERROR:
		return (GNOME_VFS_ERROR_GENERIC);
	case GP_ERROR_BAD_PARAMETERS:
		return (GNOME_VFS_ERROR_BAD_PARAMETERS);
	case GP_ERROR_IO:
		return (GNOME_VFS_ERROR_IO);
	case GP_ERROR_CORRUPTED_DATA:
		return (GNOME_VFS_ERROR_CORRUPTED_DATA);
	case GP_ERROR_FILE_EXISTS:
		return (GNOME_VFS_ERROR_FILE_EXISTS);
	case GP_ERROR_NO_MEMORY:
		return (GNOME_VFS_ERROR_NO_MEMORY);
	case GP_ERROR_MODEL_NOT_FOUND:
		return (GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS);
	case GP_ERROR_NOT_SUPPORTED:
		return (GNOME_VFS_ERROR_NOT_SUPPORTED);
	case GP_ERROR_DIRECTORY_NOT_FOUND:
		return (GNOME_VFS_ERROR_NOT_FOUND);
	case GP_ERROR_FILE_NOT_FOUND:
		return (GNOME_VFS_ERROR_NOT_FOUND);
	default:
		return (GNOME_VFS_ERROR_GENERIC);
	}
}

GnomeVFSMethodHandle*
file_handle_new (GnomeVFSURI* uri, GnomeVFSOpenMode mode, GnomeVFSContext* context, GnomeVFSResult* result)
{
        const gchar*            filename;
        gchar*                  dirname = NULL;
        Camera*                 camera = NULL;
        CameraFile*             file = NULL;
        file_handle_t*          file_handle;

        /* Do we really have a file? */
        if (!(filename = gnome_vfs_uri_get_basename (uri))) {*result = GNOME_VFS_ERROR_IS_DIRECTORY; return (NULL);}

	/* Connect to the camera. */
	{
		gchar* url = gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (uri), NULL);
		
		if (!url) {
			*result = GNOME_VFS_ERROR_HOST_NOT_FOUND; 
			return (NULL);
		}
		if ((*result = GNOME_VFS_RESULT (gp_camera_new_from_gconf (&camera, url))) != GNOME_VFS_OK) {
			g_free (url);
			return (NULL);
		}
		g_free (url);
	}
	
	if (gnome_vfs_context_check_cancellation (context)) {gp_camera_unref (camera); *result = GNOME_VFS_ERROR_CANCELLED; return (NULL);}

	file = gp_file_new ();
	dirname = gnome_vfs_uri_extract_dirname (uri);

	/* Read or write? */
	if (mode == GNOME_VFS_OPEN_READ) {
	
		/* Preview? */
		if (gnome_vfs_uri_get_user_name (uri) && (strcmp (gnome_vfs_uri_get_user_name (uri), "previews") == 0)) {
			g_print ("PREVIEW\n");
			*result = GNOME_VFS_RESULT (gp_camera_file_get_preview (camera, file, dirname, (gchar*) filename));
		} else {
			g_print ("NORMAL\n");
			*result = GNOME_VFS_RESULT (gp_camera_file_get (camera, file, dirname, (gchar*) filename));
		}

		/* Everything's ok? */
		if (*result != GNOME_VFS_OK) {gp_camera_unref (camera); gp_file_unref (file); return (NULL); }
		
	} else if (mode == GNOME_VFS_OPEN_WRITE) {

		/* Keep the filename. */
		strcpy (file->name, filename);
		
	} else {*result = GNOME_VFS_ERROR_BAD_PARAMETERS; gp_file_unref (file); gp_camera_unref (camera); return (NULL);}

	/* Create the file handle. */
	file_handle = g_new (file_handle_t, 1);
	file_handle->uri = gnome_vfs_uri_ref (uri);
	file_handle->mode = mode;
	file_handle->file = file;
	file_handle->camera = camera;
	file_handle->folder = dirname;
	file_handle->position = 0;
	
	return ((GnomeVFSMethodHandle*) file_handle);
}

GnomeVFSResult
file_handle_free (GnomeVFSMethodHandle* handle)
{
	file_handle_t* 	file_handle;
	
	if (!handle) return (GNOME_VFS_ERROR_BAD_PARAMETERS);

	file_handle = (file_handle_t*) handle;
	gp_file_unref (file_handle->file);
	gnome_vfs_uri_unref (file_handle->uri);
	gp_camera_unref (file_handle->camera);
	g_free (file_handle->folder);
	g_free (file_handle);
	return (GNOME_VFS_OK);
}

GnomeVFSMethodHandle*
directory_handle_new (GnomeVFSURI* uri, GnomeVFSFileInfoOptions options, GnomeVFSContext* context, GnomeVFSResult* result)
{
	Camera* 		camera = NULL;
	directory_handle_t*	directory_handle;
	CameraList		camera_list;
	GSList*			folders = NULL;
	GSList*			files = NULL;
	gint			i;

	/* Do we really have a directory? */
	if (gnome_vfs_uri_get_basename (uri)) {*result = GNOME_VFS_ERROR_NOT_A_DIRECTORY; return (NULL);}

        /* Connect to the camera. */
	{
		gchar* url = gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (uri), NULL);

		if (!url) {
	                *result = GNOME_VFS_ERROR_HOST_NOT_FOUND;
        	        return (NULL);
		}
	        if ((*result = GNOME_VFS_RESULT (gp_camera_new_from_gconf (&camera, url))) != GNOME_VFS_OK) {
	                g_free (url);
	                return (NULL);
	        }
		g_free (url);
	}

        if (gnome_vfs_context_check_cancellation (context)) {gp_camera_unref (camera); *result = GNOME_VFS_ERROR_CANCELLED; return (NULL);}

	/* Get folder list. */
	if ((*result = GNOME_VFS_RESULT (gp_camera_folder_list (camera, &camera_list, gnome_vfs_uri_extract_dirname (uri)))) != GNOME_VFS_OK) {
		gp_camera_unref (camera);
		return (NULL);
	}
	if (gnome_vfs_context_check_cancellation (context)) {gp_camera_unref (camera); *result = GNOME_VFS_ERROR_CANCELLED; return (NULL);}
	for (i = 0; i < gp_list_count (&camera_list); i++) folders = g_slist_append (folders, g_strdup (gp_list_entry (&camera_list, i)->name));

	/* Get file list. */
	if ((*result = GNOME_VFS_RESULT (gp_camera_file_list (camera, &camera_list, gnome_vfs_uri_extract_dirname (uri)))) != GNOME_VFS_OK) {
		for (i = 0; i < g_slist_length (folders); i++) g_free (g_slist_nth_data (folders, i));
		g_slist_free (folders);
		gp_camera_unref (camera);
		return (NULL);
	}
	for (i = 0; i < gp_list_count (&camera_list); i++) files = g_slist_append (files, g_strdup (gp_list_entry (&camera_list, i)->name));
	gp_camera_unref (camera);

	/* Create the directory handle. */
	directory_handle = g_new (directory_handle_t, 1);
	directory_handle->folders = folders;
	directory_handle->files = files;
	directory_handle->options = options;
	directory_handle->position = 0;

	*result = GNOME_VFS_OK;
	return ((GnomeVFSMethodHandle*) directory_handle);
}

GnomeVFSResult
directory_handle_free (GnomeVFSMethodHandle* handle)
{
	directory_handle_t* 	directory_handle;
	gint			i;
	
	if (!handle) return (GNOME_VFS_ERROR_BAD_PARAMETERS);

	directory_handle = (directory_handle_t*) handle;
	for (i = 0; i < g_slist_length (directory_handle->folders); i++) g_free (g_slist_nth_data (directory_handle->folders, i));
	g_slist_free (directory_handle->folders);
	for (i = 0; i < g_slist_length (directory_handle->files); i++) g_free (g_slist_nth_data (directory_handle->files, i));
	g_slist_free (directory_handle->files);
	g_free (directory_handle);
	return (GNOME_VFS_OK);
}


