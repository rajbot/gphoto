#include <config.h>
#include <gphoto2.h>
#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-storage-plugin.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>

#include "gphoto-extensions.h"

#define GNOCAM_EXT_DEBUG 1

#if GNOCAM_EXT_DEBUG
#define CAM_EXT_DEBUG_PRINT(x)				\
G_STMT_START {                                          \
        printf ("%s:%04d ", __FILE__,__LINE__);		\
        printf ("%s() ", __FUNCTION__);			\
        printf x;					\
        fputc ('\n', stdout);				\
        fflush (stdout);				\
}G_STMT_END
#define CAM_EXT_DEBUG(x) CAM_EXT_DEBUG_PRINT(x)
#else
#define CAM_EXT_DEBUG(x)
#endif


GnomeVFSResult
gp_result_as_gnome_vfs_result (gint result)
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

gint
gp_camera_new_from_gconf (Camera** camera, const gchar* name_or_url)
{
	static GConfClient *client = NULL;
        gint 		    i, result;
	gchar		   *name;
	gchar		   *model = NULL;
	gchar		   *port = NULL;
	gchar		   *tmp;
	static GSList	   *list = NULL;
	static GHashTable  *hash_table = NULL;

	g_return_val_if_fail (camera, GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (name_or_url [0] != '/', GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (name_or_url [1] != '/', GP_ERROR_BAD_PARAMETERS);

	CAM_EXT_DEBUG (("ENTER"));

	/* Create the hash table if necessary */
	if (!hash_table)
		hash_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* Make sure GConf is initialized. */
	if (!gconf_is_initialized ()) {
		GError* gerror = NULL;
		gchar*  argv[1] = {"Whatever"};

		CAM_EXT_DEBUG (("  Initializing gconf..."));
		g_return_val_if_fail (gconf_init (1, argv, &gerror), GP_ERROR);
	}

	/* Get the default client if necessary */
	if (!client) {
	    	CAM_EXT_DEBUG (("  Getting default client..."));
	    	client = gconf_client_get_default ();
	}
	
	/* Make sure we are given a camera name. */
	if (!strncmp (name_or_url, "camera:", 7)) name_or_url += 7;
	if (name_or_url [0] == '/') {
		name_or_url += 2;
		for (i = 0; name_or_url [i] != 0; i++) 
		    	if (name_or_url [i] == '/') 
			    	break;
		name = g_strndup (name_or_url, i);
	} else {
		name = g_strdup (name_or_url);
	}

	/* Get the list of configured cameras */
	CAM_EXT_DEBUG (("  Getting list of configured cameras..."));
	list = gconf_client_get_list (client, 
				      "/apps/" PACKAGE "/cameras",
				      GCONF_VALUE_STRING, NULL);
	
        /* What model is associated to the name? */
	for (i = 0; i < g_slist_length (list); i += 3) {
		if (!strcmp (g_slist_nth_data (list, i), name)) {
			model = g_strdup (g_slist_nth_data (list, i + 1));
			port = g_strdup (g_slist_nth_data (list, i + 2));
			break;
		}
	}
	g_free (name);

	/* Free the list */
//FIXME: Don't free the list?
//	for (i = 0; i < g_slist_length (list); i++) 
//	    	g_free (g_slist_nth_data (list, i));
//	g_slist_free (list);

	/* If we don't have a model at this point, exit. */
	if (!model) {
		g_warning ("GConf is unable to find a camera for '%s'!", 
			   name_or_url);
		return (GP_ERROR);
	}
	g_return_val_if_fail (model, GP_ERROR);
	g_return_val_if_fail (port, GP_ERROR);

	/* Do we already have this camera? */
	CAM_EXT_DEBUG (("Looking for model '%s' on port '%s'...", model, port));
	tmp = g_strconcat (model, port, NULL);
	*camera = g_hash_table_lookup (hash_table, tmp);
	g_free (tmp);
	if (*camera) {

	    	/* Camera found! Ref it so it doesn't get lost... */
	    	CAM_EXT_DEBUG (("Found camera!"));
	    	gp_camera_ref (*camera);

	} else {

		/* Create a new camera */
	        CAM_EXT_DEBUG (("Creating new camera..."));
		if ((result = gp_camera_new (camera)) != GP_OK) {
			g_free (model);
			g_free (port);
			return (result);
		}
	    	strcpy ((*camera)->model, model);
		strcpy ((*camera)->port->name, port);
		(*camera)->port->speed = 0;

		/* Connect to the camera */
		CAM_EXT_DEBUG (("Initializing camera..."));
		if ((result = gp_camera_init (*camera)) != GP_OK) {
		    	g_free (model);
			g_free (port);
		    	gp_camera_unref (*camera);
			*camera = NULL;
			CAM_EXT_DEBUG (("gp_camera_init() failed"));
			return (result);
		}

		/* We need to reference the camera once because we 	*/
		/* won't get informed when gphoto2 frees the camera.	*/
		/* Hence, we keep our own reference - and the camera 	*/
		/* will never be freed. We wouldn't have to do this if	*/
		/* gphoto2 used GObjects...				*/
		gp_camera_ref (*camera);
		g_hash_table_insert (hash_table, 
				     g_strconcat (model, port, NULL), 
				     *camera);
	}

	g_free (model);
	g_free (port);

	CAM_EXT_DEBUG (("EXIT"));
	return (GP_OK);
}

GnomeVFSResult
gp_camera_file_get_vfs_info (Camera             *camera, 
			     const gchar        *folder, 
			     const gchar        *file, 
			     GnomeVFSFileInfo   *info,
			     gboolean		 preview)
{
	GnomeVFSResult 		result;
	CameraFileInfo 		file_info;
	CameraFileInfoStruct 	file_info_struct;

	g_return_val_if_fail (info, GNOME_VFS_ERROR_BAD_PARAMETERS);

	result = GNOME_VFS_RESULT (gp_camera_file_get_info (camera, folder,
		    					    file, &file_info));
	if (result != GNOME_VFS_OK)
	    	return (result);

	/* Name */
	info->name = g_strdup (file);

	/* Type */
	info->type = GNOME_VFS_FILE_TYPE_REGULAR; 
	info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;

	/* Information on preview or file? */
	if (preview)
	    	file_info_struct = file_info.preview;
	else
	    	file_info_struct = file_info.file;

	/* Mime type */
	if (file_info_struct.fields && GP_FILE_INFO_TYPE) {
		info->mime_type = g_strdup (file_info_struct.type);
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
	} else {
	    	const gchar *mime_type;

		mime_type = gnome_vfs_mime_type_from_name (file);
	    	info->mime_type = g_strdup (mime_type);
	}

	/* Permissions */
	if (file_info_struct.fields && GP_FILE_INFO_PERMISSIONS) { 
	    	info->permissions = 0; 
		if (file_info_struct.permissions & GP_FILE_PERM_READ)
		    	info->permissions = GNOME_VFS_PERM_USER_READ | 
			    		    GNOME_VFS_PERM_GROUP_READ | 
					    GNOME_VFS_PERM_OTHER_READ; 
		if (file_info_struct.permissions & GP_FILE_PERM_DELETE)
		    	info->permissions |= GNOME_VFS_PERM_USER_WRITE | 
			    		     GNOME_VFS_PERM_GROUP_WRITE |
					     GNOME_VFS_PERM_OTHER_WRITE;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;
	}

	/* Size */
	if (file_info_struct.fields && GP_FILE_INFO_SIZE) {
	    	info->size = file_info_struct.size;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
	}

	return (GNOME_VFS_OK);
}



