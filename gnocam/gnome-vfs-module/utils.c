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

Camera*
camera_new_by_uri (GnomeVFSURI* uri, GConfClient* client, GMutex* client_mutex, GnomeVFSResult* result)
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

        /* Connect to the camera (host). Beware of 'Directory Browse'.*/
	*result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
	if (!strcmp ("Directory Browse", model)) strcpy (info.path, "");
	else {
		for (i = 0; i < gp_port_count (); i++) {
			if (gp_port_info (i, &info) != GP_OK) continue;
			if (!strcmp (info.name, port)) break;
		}
		if ((i == gp_port_count ()) || (i < 0)) return (NULL);
		info.speed = atoi (speed);
	}
	if (gp_camera_new_by_name (&camera, model) != GP_OK) return (NULL);
	if (gp_camera_init (camera, &info) != GP_OK) return (NULL);

	*result = GNOME_VFS_OK;
	return (camera);
}

GnomeVFSMethodHandle*
file_handle_new (GnomeVFSURI* uri, GConfClient* client, GMutex* client_mutex, GnomeVFSResult* result)
{
        const gchar*            filename;
        gchar*                  dirname;
        Camera*                 camera;
        CameraFile*             file;
	CameraList		list;
        file_handle_t*          file_handle;
	gint			i;

        /* Do we really have a file? */
        if (!(filename = gnome_vfs_uri_get_basename (uri))) {
		*result = GNOME_VFS_ERROR_IS_DIRECTORY;
		return (NULL);
	}

	/* Connect to the camera. */
	if (!(camera = camera_new_by_uri (uri, client, client_mutex, result))) return (NULL);

	/* Check if we've got the file. */
	if (gp_camera_file_list (camera, &list, gnome_vfs_uri_extract_dirname (uri)) != GP_OK) {
		*result = GNOME_VFS_ERROR_GENERIC;
		return (NULL);
	}
	for (i = 0; i < gp_list_count (&list); i++) if (!strcmp (filename, gp_list_entry (&list, i)->name)) break;
	if (i == gp_list_count (&list)) {
		*result = GNOME_VFS_ERROR_NOT_FOUND;
		return (NULL);
	}

        /* Get the file. */
	dirname = gnome_vfs_uri_extract_dirname (uri);
	g_print ("  getting file '%s' from directory '%s'...\n", filename, dirname);
	file = gp_file_new ();
	if (gp_camera_file_get (camera, file, dirname, (gchar*) filename) != GP_OK) {
		*result = GNOME_VFS_ERROR_GENERIC;
		return (NULL);
	}
	gp_file_ref (file);

	/* Create the file handle. */
	file_handle = g_new (file_handle_t, 1);
	file_handle->file = file;
	file_handle->position = 0;

	*result = GNOME_VFS_OK;
	return ((GnomeVFSMethodHandle*) file_handle);
}

GnomeVFSResult
file_handle_free (GnomeVFSMethodHandle* handle)
{
	file_handle_t* 	file_handle;
	
	g_return_val_if_fail (handle, GNOME_VFS_ERROR_INTERNAL);

	file_handle = (file_handle_t*) handle;
	gp_file_unref (file_handle->file);
	g_free (file_handle);
	return (GNOME_VFS_OK);
}

GnomeVFSMethodHandle*
directory_handle_new (GnomeVFSURI* uri, GConfClient* client, GMutex* client_mutex, GnomeVFSFileInfoOptions options, GnomeVFSResult* result)
{
	Camera* 		camera;
	directory_handle_t*	directory_handle;
	CameraList		list;
	GSList*			folders = NULL;
	GSList*			files = NULL;
	gint			i;

	/* Do we really have a directory? */
	if (gnome_vfs_uri_get_basename (uri)) {
		*result = GNOME_VFS_ERROR_NOT_A_DIRECTORY;
		return (NULL);
	}

	/* Connect to the camera. */
	if (!(camera = camera_new_by_uri (uri, client, client_mutex, result))) return (NULL);

	/* Get folder list. */
	g_print ("  looking for folders in %s\n", gnome_vfs_uri_extract_dirname (uri));
	if ((gp_camera_folder_list (camera, &list, gnome_vfs_uri_extract_dirname (uri))) != GP_OK) {
		g_print ("Could not get folder list!");
		*result = GNOME_VFS_ERROR_GENERIC;
		gp_camera_unref (camera);
		return (NULL);
	}
	for (i = 0; i < gp_list_count (&list); i++) folders = g_slist_append (folders, g_strdup (gp_list_entry (&list, i)->name));

	/* Get file list. */
	g_print ("  looking for files in %s\n", gnome_vfs_uri_extract_dirname (uri));
	if ((gp_camera_file_list (camera, &list, gnome_vfs_uri_extract_dirname (uri))) != GP_OK) {
		g_print ("Could not get file list!");
		for (i = 0; i < g_slist_length (folders); i++) g_free (g_slist_nth_data (folders, i));
		g_slist_free (folders);
		*result = GNOME_VFS_ERROR_GENERIC;
		gp_camera_unref (camera);
		return (NULL);
	}
	for (i = 0; i < gp_list_count (&list); i++) files = g_slist_append (files, g_strdup (gp_list_entry (&list, i)->name));

	/* Create the directory handle. */
	directory_handle = g_new (directory_handle_t, 1);
	directory_handle->folders = folders;
	directory_handle->files = files;
	directory_handle->options = options;
	directory_handle->position = 0;

	*result = GNOME_VFS_OK;
	gp_camera_unref (camera);
	return ((GnomeVFSMethodHandle*) directory_handle);
}

GnomeVFSResult
directory_handle_free (GnomeVFSMethodHandle* handle)
{
	directory_handle_t* 	directory_handle;
	gint			i;
	
	g_return_val_if_fail (handle, GNOME_VFS_ERROR_INTERNAL);

	directory_handle = (directory_handle_t*) handle;
	for (i = 0; i < g_slist_length (directory_handle->folders); i++) g_free (g_slist_nth_data (directory_handle->folders, i));
	g_slist_free (directory_handle->folders);
	for (i = 0; i < g_slist_length (directory_handle->files); i++) g_free (g_slist_nth_data (directory_handle->files, i));
	g_slist_free (directory_handle->files);
	g_free (directory_handle);
	return (GNOME_VFS_OK);
}


