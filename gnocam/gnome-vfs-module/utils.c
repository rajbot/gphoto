#include <stdlib.h>
#include <gtk/gtk.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>
#include <parser.h>
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
	case GP_ERROR_NONCRITICAL:
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

Camera*
camera_new_by_uri (GnomeVFSURI* uri, GConfClient* client, GMutex* client_mutex, GnomeVFSContext* context, GnomeVFSResult* result)
{
        GConfValue*             value = NULL;
        gchar*                  xml;
        gchar*                  id = NULL;
        gchar*                  name= NULL;
        gchar*                  model = NULL;
        gchar*                  port = NULL;
        gchar*                  speed = NULL;
        gint                    i;
        xmlDocPtr               doc;
        xmlNodePtr              node;
        GSList*                 list;
        Camera*                 camera;
	CameraPortInfo		info;
	gchar*			host;

        /* Does gconf know about the camera (host)? */
	*result = GNOME_VFS_ERROR_HOST_NOT_FOUND;
	g_mutex_lock (client_mutex);
	value = gconf_client_get (client, "/apps/GnoCam/cameras", NULL);
	g_mutex_unlock (client_mutex);
	if (!value) return (NULL);
	g_assert (value->type == GCONF_VALUE_LIST);
	g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
	list = gconf_value_get_list (value);
	if (!(host = gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (uri), NULL))) return (NULL);
	for (i = 0; i < g_slist_length (list); i++) {
	        value = g_slist_nth_data (list, i);
	        g_assert (value->type == GCONF_VALUE_STRING);
	        g_assert ((xml = g_strdup (gconf_value_get_string (value))));
	        if (!(doc = xmlParseMemory (xml, strlen (xml)))) continue;
	        g_assert (node = xmlDocGetRootElement (doc));
	        g_assert (id = xmlGetProp (node, "ID"));
	        g_assert (name = xmlGetProp (node, "Name"));
	        g_assert (model = xmlGetProp (node, "Model"));
	        g_assert (port = xmlGetProp (node, "Port"));
	        g_assert (speed = xmlGetProp (node, "Speed"));
	        if (!strcmp (name, host)) break;
	}
	g_free (host);
	if (i == g_slist_length (list)) return (NULL);

        /* Make ready for connection. Beware of 'Directory Browse'.*/
	if (!strcmp ("Directory Browse", model)) strcpy (info.path, "");
	else {
		for (i = 0; i < gp_port_count (); i++) {
			if (gp_port_info (i, &info) != GP_OK) continue;
			if (!strcmp (info.name, port)) break;
		}
		if ((i == gp_port_count ()) || (i < 0)) return (NULL);
		info.speed = atoi (speed);
	}

	/* Create and connect to the camera. */
	*result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	if (gp_camera_new_by_name (&camera, model) != GP_OK) return (NULL);
	if (gnome_vfs_context_check_cancellation (context)) {*result = GNOME_VFS_ERROR_CANCELLED; return (NULL);}
	if (gp_camera_init (camera, &info) != GP_OK) return (NULL);

	*result = GNOME_VFS_OK;
	return (camera);
}

GnomeVFSMethodHandle*
file_handle_new (GnomeVFSURI* uri, GnomeVFSOpenMode mode, GConfClient* client, GMutex* client_mutex, GnomeVFSContext* context, GnomeVFSResult* result)
{
        const gchar*            filename;
        gchar*                  dirname = NULL;
        Camera*                 camera = NULL;
        CameraFile*             file = NULL;
        file_handle_t*          file_handle;
	gint			gp_result;

        /* Do we really have a file? */
        if (!(filename = gnome_vfs_uri_get_basename (uri))) {*result = GNOME_VFS_ERROR_IS_DIRECTORY; return (NULL);}

	/* Connect to the camera. */
	if (!(camera = camera_new_by_uri (uri, client, client_mutex, context, result))) return (NULL);
	if (gnome_vfs_context_check_cancellation (context)) {gp_camera_unref (camera); *result = GNOME_VFS_ERROR_CANCELLED; return (NULL);}

	/* Read or write? */
	if (mode == GNOME_VFS_OPEN_READ) {
	
	        /* Get the file. */
		dirname = gnome_vfs_uri_extract_dirname (uri);
		file = gp_file_new ();
		if ((gp_result = gp_camera_file_get (camera, file, dirname, (gchar*) filename)) != GP_OK) {
			gp_camera_unref (camera);
			*result = GNOME_VFS_RESULT (gp_result);
			return (NULL);
		}
		gp_file_ref (file);
		gp_camera_unref (camera);
		camera = NULL;

	} else if (mode == GNOME_VFS_OPEN_WRITE) {

		/* Create an empty file. */
		file = gp_file_new ();
		file->data = NULL;
		strcpy (file->name, filename);
		
	} else {*result = GNOME_VFS_ERROR_BAD_PARAMETERS; return (NULL);}

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
	if (file_handle->camera) gp_camera_unref (file_handle->camera);
	if (file_handle->folder) g_free (file_handle->folder);
	g_free (file_handle);
	return (GNOME_VFS_OK);
}

GnomeVFSMethodHandle*
directory_handle_new (GnomeVFSURI* uri, GConfClient* client, GMutex* client_mutex, GnomeVFSFileInfoOptions options, GnomeVFSContext* context, GnomeVFSResult* result)
{
	Camera* 		camera;
	directory_handle_t*	directory_handle;
	CameraList		list;
	GSList*			folders = NULL;
	GSList*			files = NULL;
	gint			i;
	gint			gp_result;

	/* Do we really have a directory? */
	if (gnome_vfs_uri_get_basename (uri)) {*result = GNOME_VFS_ERROR_NOT_A_DIRECTORY; return (NULL);}

	/* Connect to the camera. */
	if (!(camera = camera_new_by_uri (uri, client, client_mutex, context, result))) return (NULL);

	/* Get folder list. */
	if ((gp_result = gp_camera_folder_list (camera, &list, gnome_vfs_uri_extract_dirname (uri))) != GP_OK) {
		*result = GNOME_VFS_RESULT (gp_result);
		gp_camera_unref (camera);
		return (NULL);
	}
	if (gnome_vfs_context_check_cancellation (context)) {gp_camera_unref (camera); *result = GNOME_VFS_ERROR_CANCELLED; return (NULL);}
	for (i = 0; i < gp_list_count (&list); i++) folders = g_slist_append (folders, g_strdup (gp_list_entry (&list, i)->name));

	/* Get file list. */
	if ((gp_result = gp_camera_file_list (camera, &list, gnome_vfs_uri_extract_dirname (uri))) != GP_OK) {
		for (i = 0; i < g_slist_length (folders); i++) g_free (g_slist_nth_data (folders, i));
		g_slist_free (folders);
		*result = GNOME_VFS_RESULT (gp_result);
		gp_camera_unref (camera);
		return (NULL);
	}
	for (i = 0; i < gp_list_count (&list); i++) files = g_slist_append (files, g_strdup (gp_list_entry (&list, i)->name));
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


