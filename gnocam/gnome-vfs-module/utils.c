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

GnomeVFSMethodHandle*
file_handle_new (GnomeVFSURI* uri, GnomeVFSOpenMode mode, GnomeVFSContext* context, GnomeVFSResult* result)
{
        const gchar		*filename;
	const gchar		*host;
        gchar			*dirname = NULL;
        Camera			*camera = NULL;
        CameraFile		*file = NULL;
        file_handle_t		*file_handle;
	gchar			*url;

	filename = gnome_vfs_uri_get_basename (uri);
	host = gnome_vfs_uri_get_host_name (uri);
	
        /* Do we really have a file? */
        if (!filename) {
	    	*result = GNOME_VFS_ERROR_IS_DIRECTORY; 
		return (NULL);
	}

	url = gnome_vfs_unescape_string (host, NULL);
	if (!url) {
	    	*result = GNOME_VFS_ERROR_HOST_NOT_FOUND;
		return (NULL);
	}

	/* Connect to the camera. */
	*result = GNOME_VFS_RESULT (gp_camera_new_from_gconf (&camera, url));
	g_free (url);
	if (*result != GNOME_VFS_OK)
		return (NULL);
	
	/* Cancelled? */
	if (gnome_vfs_context_check_cancellation (context)) {
	    	gp_camera_unref (camera); 
		*result = GNOME_VFS_ERROR_CANCELLED; 
	    	return (NULL);
	}

	file = gp_file_new ();
	dirname = gnome_vfs_uri_extract_dirname (uri);

	/* Read or write? */
	if (mode == GNOME_VFS_OPEN_READ) {
	
		/* Preview? */
		if (gnome_vfs_uri_get_user_name (uri) && (strcmp (gnome_vfs_uri_get_user_name (uri), "previews") == 0)) {
			CAM_VFS_DEBUG (("preview mode"));
			CAM_VFS_DEBUG (("dirname=%s", dirname));
			CAM_VFS_DEBUG (("filename=%s", filename));
			*result = GNOME_VFS_RESULT (gp_camera_file_get_preview (camera, dirname, (gchar*) filename, file));
		} else {
			CAM_VFS_DEBUG (("fullsize mode"));
			CAM_VFS_DEBUG (("dirname=%s", dirname));
			CAM_VFS_DEBUG (("filename=%s", filename));
			*result = GNOME_VFS_RESULT (gp_camera_file_get_file (camera, dirname, (gchar*) filename, file));
		}

		/* Everything's ok? */
		if (*result != GNOME_VFS_OK) {
		    	gp_camera_unref (camera); 
			gp_file_unref (file); 
			return (NULL); 
		}
		
	} else if (mode == GNOME_VFS_OPEN_WRITE) {

		/* Keep the filename. */
		strcpy (file->name, filename);
		
	} else {
	    	*result = GNOME_VFS_ERROR_BAD_PARAMETERS; 
		gp_file_unref (file); 
		gp_camera_unref (camera); 
		return (NULL);
	}

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
directory_handle_new (GnomeVFSURI* uri, GnomeVFSFileInfoOptions options, 
		      GnomeVFSContext* context, GnomeVFSResult* result)
{
	Camera			*camera = NULL;
	directory_handle_t	*directory_handle;
	CameraList		 camera_list;
	GSList			*folders = NULL;
	GSList			*files = NULL;
	gint			 i;
	const gchar		*path;
	const gchar		*host;
	gchar			*url;
	gchar			*name;

	path = gnome_vfs_uri_get_path (uri);
	host = gnome_vfs_uri_get_host_name (uri);
	
	CAM_VFS_DEBUG (("uri=%s",uri->text));
	CAM_VFS_DEBUG (("basename(uri)=%s",gnome_vfs_uri_get_basename(uri)));
	CAM_VFS_DEBUG (("dirname(uri)=%s",gnome_vfs_uri_extract_dirname(uri)));
	CAM_VFS_DEBUG (("path(uri)=%s", path));

	url = gnome_vfs_unescape_string (host, NULL);
	if (!url) {
		*result = GNOME_VFS_ERROR_HOST_NOT_FOUND;
		CAM_VFS_DEBUG (("returns GNOME_VFS_ERROR_HOST_NOT_FOUND"));
        	return (NULL);
	}

	/* Connect to the camera. */
	*result = GNOME_VFS_RESULT (gp_camera_new_from_gconf (&camera, url));
        if (*result != GNOME_VFS_OK) {
                g_free (url);
		CAM_VFS_DEBUG (("gp_camera_new_from_gconf() failed"));
                return (NULL);
        }
	g_free (url);

	/* Cancelled? */
        if (gnome_vfs_context_check_cancellation (context)) {
		gp_camera_unref (camera); 
		*result = GNOME_VFS_ERROR_CANCELLED; 
		CAM_VFS_DEBUG (("returns GNOME_VFS_ERROR_CANCELED"));
		return (NULL);
	}

	/* Get folder list. If an error occurs, skip this. It can well be */
	/* that the camera does not support folders.			  */
	*result = GNOME_VFS_RESULT (gp_camera_folder_list_folders (
		    				camera, path, &camera_list));
	if (*result == GNOME_VFS_OK) {
	    	for (i = 0; i < gp_list_count (&camera_list); i++) {
		    	name = g_strdup (gp_list_entry (&camera_list, i)->name);
			folders = g_slist_append (folders, name);
		}
	}

	/* Cancelled? */
	if (gnome_vfs_context_check_cancellation (context)) {
		gp_camera_unref (camera); 
		*result = GNOME_VFS_ERROR_CANCELLED; 
		CAM_VFS_DEBUG (("returns GNOME_VFS_ERROR_CANCELED"));
		return (NULL);
	}

	/* Get file list. */
	*result = GNOME_VFS_RESULT (gp_camera_folder_list_files (camera, path,
		    						 &camera_list));
	if (*result != GNOME_VFS_OK) {
		for (i = 0; i < g_slist_length (folders); i++) 
			g_free (g_slist_nth_data (folders, i));
		g_slist_free (folders);
		gp_camera_unref (camera);
		CAM_VFS_DEBUG (("gp_camera_folder_list_list_files() failed"));
		return (NULL);
	}
	for (i = 0; i < gp_list_count (&camera_list); i++) {
	    	name = g_strdup (gp_list_entry (&camera_list, i)->name);
		files = g_slist_append (files, name);
	}

	/* Create the directory handle. */
	directory_handle = g_new (directory_handle_t, 1);
	directory_handle->folders = folders;
	directory_handle->files = files;
	directory_handle->options = options;
	directory_handle->position = 0;
	directory_handle->camera = camera;
	directory_handle->folder = g_strdup (path);

	*result = GNOME_VFS_OK;
	CAM_VFS_DEBUG (("exiting"));
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
	g_free (directory_handle->folder);
	gp_camera_unref (directory_handle->camera);
	g_free (directory_handle);
	return (GNOME_VFS_OK);
}


