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
        printf ("%s:%d ", __FILE__,__LINE__);		\
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
	static GConfClient*	client = NULL;
        gint 			i, result;
	gchar*			name;
	GSList*			list;

	g_return_val_if_fail (camera, 							GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (!((name_or_url [0] == '/') && (name_or_url [1] != '/')), 	GP_ERROR_BAD_PARAMETERS);
	CAM_EXT_DEBUG (("entering"));

	if ((result = gp_camera_new (camera)) != GP_OK) {
		CAM_EXT_DEBUG (("gp_camera_new() failed"));
		return (result);
	}

	/* Make sure GConf is initialized. */
	if (!gconf_is_initialized ()) {
		GError* gerror = NULL;
		gchar*  argv[1] = {"Whatever"};
		g_return_val_if_fail (gconf_init (1, argv, &gerror), GP_ERROR);
	}
	if (!client) client = gconf_client_get_default ();
	
	/* Make sure we are given a camera name. */
	if (!strncmp (name_or_url, "camera:", 7)) name_or_url += 7;
	if (name_or_url [0] == '/') {
		name_or_url += 2;
		for (i = 0; name_or_url [i] != 0; i++) if (name_or_url [i] == '/') break;
		name = g_strndup (name_or_url, i);
	} else {
		name = g_strdup (name_or_url);
	}
	
        /* Does GConf know about the camera? */
	list = gconf_client_get_list (client, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, NULL);
	
	for (i = 0; i < g_slist_length (list); i += 3) {
		if (!strcmp (g_slist_nth_data (list, i), name)) {
			strcpy ((*camera)->model, g_slist_nth_data (list, i + 1));
			strcpy ((*camera)->port->name, g_slist_nth_data (list, i + 2));
			(*camera)->port->speed = 0;
			break;
		}
	}
	g_free (name);

	if (i == g_slist_length (list)) {
		g_warning ("GConf is unable to find a camera for '%s'!", name_or_url);
		gp_camera_unref (*camera);
		*camera = NULL;
		return (GP_ERROR);
	}

	/* Free the list */
	for (i = 0; i < g_slist_length (list); i++) g_free (g_slist_nth_data (list, i));
	g_slist_free (list);

	/* Connect to the camera */
	if ((result = gp_camera_init (*camera)) != GP_OK) {
		gp_camera_unref (*camera);
		*camera = NULL;
		CAM_EXT_DEBUG (("gp_camera_init() failed"));
	}

	return (result);
}

GnomeVFSResult
gp_camera_file_get_vfs_info (Camera             *camera, 
			     const gchar        *folder, 
			     const gchar        *file, 
			     GnomeVFSFileInfo   *info)
{
	GnomeVFSResult result;
	CameraFileInfo file_info;

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

	/* Mime type */
	if (file_info.file.fields && GP_FILE_INFO_TYPE) {
		info->mime_type = g_strdup (file_info.file.type);
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
	}

	/* Permissions */
	if (file_info.file.fields && GP_FILE_INFO_PERMISSIONS) { 
	    	info->permissions = 0; 
		if (file_info.file.permissions & GP_FILE_PERM_READ)
		    	info->permissions = GNOME_VFS_PERM_USER_READ | 
			    		    GNOME_VFS_PERM_GROUP_READ | 
					    GNOME_VFS_PERM_OTHER_READ; 
		if (file_info.file.permissions & GP_FILE_PERM_DELETE)
		    	info->permissions |= GNOME_VFS_PERM_USER_WRITE | 
			    		     GNOME_VFS_PERM_GROUP_WRITE |
					     GNOME_VFS_PERM_OTHER_WRITE;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;
	}

	/* Size */
	if (file_info.file.fields && GP_FILE_INFO_SIZE) {
	    	info->size = file_info.file.size;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
	}

	return (GNOME_VFS_OK);
}



