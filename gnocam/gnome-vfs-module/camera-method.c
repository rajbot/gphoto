#include <config.h>
#include <gphoto2.h>
#include <libgnome/gnome-defs.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <gconf/gconf-client.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

static GnomeVFSResult
GNOME_VFS_RESULT (int result)
{
	if (result >= 0)
		return (GNOME_VFS_OK);

	switch (result) {
	case GP_ERROR_BAD_PARAMETERS:
		return GNOME_VFS_ERROR_BAD_PARAMETERS;
	case GP_ERROR_IO:
		return GNOME_VFS_ERROR_IO;
	case GP_ERROR_CORRUPTED_DATA:
		return GNOME_VFS_ERROR_CORRUPTED_DATA;
	case GP_ERROR_FILE_EXISTS:
		return GNOME_VFS_ERROR_FILE_EXISTS;
	case GP_ERROR_NO_MEMORY:
		return GNOME_VFS_ERROR_NO_MEMORY;
	case GP_ERROR_MODEL_NOT_FOUND:
		return GNOME_VFS_ERROR_HOST_NOT_FOUND;
	case GP_ERROR_NOT_SUPPORTED:
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	case GP_ERROR_FILE_NOT_FOUND:
	case GP_ERROR_DIRECTORY_NOT_FOUND:
		return GNOME_VFS_ERROR_NOT_FOUND;
	case GP_ERROR_DIRECTORY_EXISTS:
		return GNOME_VFS_ERROR_FILE_EXISTS;
	default:
		return GNOME_VFS_ERROR_GENERIC;
	}
}

typedef struct {
	gchar  *name;
	Camera *camera;
} CamerasEntry;

static GSList *cameras = NULL;
static GConfClient *client = NULL;
static CameraAbilitiesList *al = NULL;
static GPPortInfoList *il = NULL;

G_LOCK_DEFINE_STATIC (cameras);

static void get_info_from_camera_info (CameraFileInfo *cfi, gboolean preview,
		                       GnomeVFSFileInfo *info);


static GnomeVFSResult
get_camera (GnomeVFSURI *uri, Camera **camera)
{
	CamerasEntry *entry;
	CameraAbilities abilities;
	const gchar *host;
	GSList *list, *sl;
	guint i;
	GnomeVFSResult result;
	int m, p;
	GPPortInfo info;

	g_return_val_if_fail (uri, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (camera, GNOME_VFS_ERROR_BAD_PARAMETERS);

	*camera = NULL;

	host = gnome_vfs_uri_get_host_name (uri);

printf ("Is '%s' cached?\n", host);
	/* Could be that this camera is currently active */
	for (sl = cameras; sl; sl = sl->next) {
		entry = sl->data;
		if (!strcmp (entry->name, host)) {
			*camera = entry->camera;
			gp_camera_ref (entry->camera);
			G_UNLOCK (cameras);
printf ("Found cached camera!\n");
			return (GNOME_VFS_OK);
		}
	}

printf ("No. Camera isn't in cache.\n");
	/* Ok, this camera isn't active. */
	gp_camera_new (camera);
	list = gconf_client_get_list (client, "/apps/" PACKAGE "/cameras",
				      GCONF_VALUE_STRING, NULL);
	if (!host) {
		if (!gconf_client_get_bool (client,
					   "/apps/" PACKAGE "/autodetect",
					   NULL) &&
		    (g_slist_length (list) == 3)) {

			/* Model */
			m = gp_abilities_list_lookup_model (al,
						g_slist_nth_data (list, 1));
			gp_abilities_list_get_abilities (al, m, &abilities);
			gp_camera_set_abilities (*camera, abilities);
			
			/* Port */
			p = gp_port_info_list_lookup_name (il,
						g_slist_nth_data (list, 2));
			gp_port_info_list_get_info (il, p, &info);
			gp_camera_set_port_info (*camera, info);
		}
	} else 
		for (i = 0; i < g_slist_length (list); i += 3)
			if (!strcmp (g_slist_nth_data (list, i), host)) {

				/* Model */
				m = gp_abilities_list_lookup_model (al,
					g_slist_nth_data (list, i + 1));
				gp_abilities_list_get_abilities (al, m,
								&abilities);
				gp_camera_set_abilities (*camera, abilities);

				/* Port */
				p = gp_port_info_list_lookup_name (il,
					g_slist_nth_data (list, i + 2));
				gp_port_info_list_get_info (il, p, &info);
				gp_camera_set_port_info (*camera, info);
printf ("Found %s in database!\n", host);
			}

	result = GNOME_VFS_RESULT (gp_camera_init (*camera, NULL));
	if (result != GNOME_VFS_OK) {
		gp_camera_unref (*camera);
printf ("Could not initialize camera!\n");
		return (result);
	}

	/* Cache the result */
	entry = g_new0 (CamerasEntry, 1);
	entry->camera = *camera;
	entry->name = g_strdup (host);
	gp_camera_ref (*camera);
	cameras = g_slist_append (cameras, entry);

	return (GNOME_VFS_OK);
}

static void
unref_camera (Camera *camera)
{
#if 0
	CamerasEntry *entry;
	GSList *sl;
#endif

	gp_camera_exit (camera, NULL);
	gp_camera_unref (camera);

#if 0
	/* We don't want to keep USB cameras in cache */
	if (camera->port->type & GP_PORT_USB)
		for (sl = cameras; sl; sl = sl->next) {
			entry = sl->data;
			if ((camera == entry->camera) &&
			    (camera->ref_count == 1)) {
				cameras = g_slist_remove_link (cameras, sl);
				gp_camera_unref (entry->camera);
				g_free (entry);
printf ("I just destroyed a camera.\n");
				break;
			}
		}
#endif
}

typedef struct {
	Camera *camera;
	CameraFile *file;
	guint pos;
	gchar *dirname;
	gboolean create;
	gboolean preview;
} FileHandle;

static GnomeVFSResult do_open (
	GnomeVFSMethod        *method, 
	GnomeVFSMethodHandle **handle, 
	GnomeVFSURI           *uri, 
	GnomeVFSOpenMode       mode, 
	GnomeVFSContext       *context)
{
	Camera *camera;
	CameraFile *file;
	GnomeVFSResult result;
	FileHandle *file_handle;
	const gchar *filename;
	gchar *dirname;

printf ("ENTER: do_open\n");

	G_LOCK (cameras);

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK) {
		G_UNLOCK (cameras);
		return (result);
	}

	result = GNOME_VFS_RESULT (gp_file_new (&file));
	if (result != GNOME_VFS_OK) {
		unref_camera (camera);
		G_UNLOCK (cameras);
		return (result);
	}

	dirname = gnome_vfs_uri_extract_dirname (uri);
	filename = gnome_vfs_uri_get_basename (uri);
	if (gnome_vfs_uri_get_user_name (uri) != NULL)
		result = GNOME_VFS_RESULT (gp_camera_file_get (camera, dirname,
					filename, GP_FILE_TYPE_PREVIEW, file,
					NULL));
	else
		result = GNOME_VFS_RESULT (gp_camera_file_get (camera, dirname,
					filename, GP_FILE_TYPE_NORMAL, file,
					NULL));
	if (result != GNOME_VFS_OK) {
		gp_file_unref (file);
		unref_camera (camera);
		g_free (dirname);
		G_UNLOCK (cameras);
		return (result);
	}

	/* Construct handle */
	file_handle = g_new0 (FileHandle, 1);
	file_handle->camera = camera;
	file_handle->file = file;
	file_handle->dirname = dirname;
	file_handle->create = FALSE;
	file_handle->preview = (gnome_vfs_uri_get_user_name (uri) != NULL);
	*handle = (GnomeVFSMethodHandle *) file_handle;

	G_UNLOCK (cameras);
printf ("EXIT: do_open\n");

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
	Camera *camera;
	CameraAbilities a;
	FileHandle *file_handle;
	GnomeVFSResult result;

	G_LOCK (cameras);

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK) {
		G_UNLOCK (cameras);
		return (result);
	}

	gp_camera_get_abilities (camera, &a);
	if (!(a.folder_operations & GP_FOLDER_OPERATION_PUT_FILE)) {
		unref_camera (camera);
		G_UNLOCK (cameras);
		return (GNOME_VFS_ERROR_NOT_SUPPORTED);
	}

	/* Construct the file handle */
	file_handle = g_new0 (FileHandle, 1);
	file_handle->camera = camera;
	gp_file_new (&(file_handle->file));
	file_handle->create = TRUE;
	file_handle->dirname = gnome_vfs_uri_extract_dirname (uri);
	file_handle->preview = (gnome_vfs_uri_get_user_name (uri) != NULL);
	*handle = (GnomeVFSMethodHandle *) file_handle;

	G_UNLOCK (cameras);

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_close (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSContext*                context)
{
	FileHandle *fh = (FileHandle *) handle;
	GnomeVFSResult result = GNOME_VFS_OK;

	G_LOCK (cameras);

	if (fh->create)
		result = GNOME_VFS_RESULT (gp_camera_folder_put_file (
				fh->camera, fh->dirname, fh->file, NULL));

	unref_camera (fh->camera);
	gp_file_unref (fh->file);
	g_free (fh->dirname);
	g_free (fh);

	G_UNLOCK (cameras);

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
	FileHandle *fh = (FileHandle *) handle;
	const char *data;
	long int size;
	GnomeVFSResult result;

	G_LOCK (cameras);
	result = gp_file_get_data_and_size (fh->file, &data, &size);
	if (result != GNOME_VFS_OK) {
		G_UNLOCK (cameras);
		return (result);
	}

	*bytes_read = MIN (size - fh->pos, num_bytes);
	if (!*bytes_read) {
		G_UNLOCK (cameras);
		return (GNOME_VFS_ERROR_EOF);
	}

	memcpy (buffer, data + fh->pos, *bytes_read);
	fh->pos += *bytes_read;

	G_UNLOCK (cameras);

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
	GnomeVFSResult result;

	G_LOCK (cameras);

	file_handle = (FileHandle *) handle;

	result = GNOME_VFS_RESULT (gp_file_append (file_handle->file,
						   (char*) buffer, num_bytes));
	*bytes_written = num_bytes;

	G_UNLOCK (cameras);

	return (result);
}

static GnomeVFSResult do_seek (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSSeekPosition            position,
        GnomeVFSFileOffset              offset,
        GnomeVFSContext*                context)
{
	FileHandle *file_handle = (FileHandle *) handle;
	const char *data;
	long int size;

	G_LOCK (cameras);

	gp_file_get_data_and_size (file_handle->file, &data, &size);
	switch (position) {
	case GNOME_VFS_SEEK_START:
		file_handle->pos = MIN (size, offset);
		break;
	case GNOME_VFS_SEEK_CURRENT:
		file_handle->pos = MIN (file_handle->pos + offset, size);
		break;
	case GNOME_VFS_SEEK_END:
		file_handle->pos = MAX (size - 1 - offset,0);
		break;
	}

	G_LOCK (cameras);

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_tell (
	GnomeVFSMethod*		method,
	GnomeVFSMethodHandle*	handle,
	GnomeVFSFileOffset*	offset)
{
	FileHandle *file_handle = (FileHandle *) handle;

	G_LOCK (cameras);

	*offset = file_handle->pos;

	G_UNLOCK (cameras);

	return GNOME_VFS_OK;
}

typedef struct {
	Camera *camera;
	gchar *dirname;
	CameraList *dirs;
	CameraList *files;
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
	Camera *camera;
	GnomeVFSResult result;
	DirectoryHandle *directory_handle;
	const gchar *path;
	CameraList *files;
	CameraList *dirs;

printf ("ENTER: do_open_directory\n");

	G_LOCK (cameras);

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK) {
		G_UNLOCK (cameras);
		return (result);
	}

	path = gnome_vfs_uri_get_path (uri);
	if (!*path)
		path = "/";

	/* Get the list of files */
printf ("Getting list of files...\n");
	result = GNOME_VFS_RESULT (gp_list_new (&files));
	if (result != GNOME_VFS_OK) {
		unref_camera (camera);
		G_UNLOCK (cameras);
		return (result);
	}
	result = GNOME_VFS_RESULT (gp_camera_folder_list_files (camera, path,
								files, NULL));
	if (result != GNOME_VFS_OK) {
		gp_list_free (files);
		unref_camera (camera);
		G_UNLOCK (cameras);
		return (result);
	}

	/* Get the list of directories */
printf ("Getting list of directories...\n");
	result = GNOME_VFS_RESULT (gp_list_new (&dirs));
	if (result != GNOME_VFS_OK) {
		gp_list_free (files);
		unref_camera (camera);
		G_UNLOCK (cameras);
		return (result);
	}
	result = GNOME_VFS_RESULT (gp_camera_folder_list_folders (camera, path,
								  dirs, NULL));
	if (result != GNOME_VFS_OK) {
		unref_camera (camera);
		gp_list_free (files);
		gp_list_free (dirs);
		G_UNLOCK (cameras);
		return (result);
	}

	/* Construct the handle */
	directory_handle = g_new0 (DirectoryHandle, 1);
	directory_handle->dirname = g_strdup (path);
	directory_handle->camera = camera;
	directory_handle->files = files;
	directory_handle->dirs = dirs;
	directory_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) directory_handle;

	G_UNLOCK (cameras);

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
	DirectoryHandle *directory_handle = (DirectoryHandle *) handle;

	G_LOCK (cameras);

	/* Free the handle */
	unref_camera (directory_handle->camera);
	gp_list_free (directory_handle->files);
	gp_list_free (directory_handle->dirs);
	g_free (directory_handle->dirname);
	g_free (directory_handle);

	G_UNLOCK (cameras);

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*		info,
	GnomeVFSContext*                context)
{
	DirectoryHandle *dh = (DirectoryHandle *) handle;
	CameraFileInfo camera_info;
	GnomeVFSResult result;
	const char *name = NULL;
	int dirs_count, files_count;

printf ("ENTER: do_read_directory\n");

	G_LOCK (cameras);

	dirs_count = gp_list_count (dh->dirs);
	if (dirs_count < 0) {
		G_UNLOCK (cameras);
		return (GNOME_VFS_RESULT (dirs_count));
	}

	files_count = gp_list_count (dh->files);
	if (files_count < 0) {
		G_UNLOCK (cameras);
		return (GNOME_VFS_RESULT (files_count));
	}

	if (dh->pos >= dirs_count + files_count) {
		G_UNLOCK (cameras);
		return (GNOME_VFS_ERROR_EOF);
	}

	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);

	/* Tell gnome-vfs which fields will be valid */
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE |
			     GNOME_VFS_FILE_INFO_FIELDS_TYPE;

	/* Fill the fields */
	if (dh->pos < dirs_count) {
		gp_list_get_name (dh->dirs, dh->pos, &name);
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		info->mime_type = g_strdup ("x-directory/normal");
	} else {
		gp_list_get_name (dh->files, dh->pos - dirs_count, &name);
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
printf ("Trying to get file info for '%s'...\n", name);
		result = GNOME_VFS_RESULT (gp_camera_file_get_info (
			dh->camera, dh->dirname, name, &camera_info, NULL));
printf ("... done.\n");
		if (result == GNOME_VFS_OK)
			get_info_from_camera_info (&camera_info, FALSE, info);
	}
	info->name = g_strdup (name);

	dh->pos++;

	G_UNLOCK (cameras);

	return (GNOME_VFS_OK);
}

static void
get_info_from_camera_info (CameraFileInfo *cfi, gboolean preview,
			   GnomeVFSFileInfo *info)
{
	info->name = g_strdup (cfi->file.name);
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE;
	info->type = GNOME_VFS_FILE_TYPE_REGULAR;

	if (preview) {
		if (cfi->preview.fields & GP_FILE_INFO_SIZE) {
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
			info->size = cfi->preview.size;
		}
		if (cfi->preview.fields & GP_FILE_INFO_TYPE) {
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
			info->mime_type = g_strdup (cfi->preview.type);
		}
	} else {
		if (cfi->file.fields & GP_FILE_INFO_SIZE) {
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
			info->size = cfi->file.size;
		}
		if (cfi->file.fields & GP_FILE_INFO_TYPE) {
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
			info->mime_type = g_strdup (cfi->file.type);
		}
	}

	if (cfi->file.fields & GP_FILE_INFO_PERMISSIONS) {
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;
		info->permissions = 0;
		if (cfi->file.permissions & GP_FILE_PERM_READ)
			info->permissions |= GNOME_VFS_PERM_USER_READ |
					     GNOME_VFS_PERM_GROUP_READ |
					     GNOME_VFS_PERM_OTHER_READ;
		if (cfi->file.permissions & GP_FILE_PERM_DELETE)
			info->permissions |= GNOME_VFS_PERM_USER_WRITE |
					     GNOME_VFS_PERM_GROUP_WRITE |
					     GNOME_VFS_PERM_OTHER_WRITE;
	}
}

static GnomeVFSResult do_get_file_info (
        GnomeVFSMethod*                 method,
        GnomeVFSURI*                    uri,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	GnomeVFSResult result;
	GnomeVFSURI *parent;
	Camera *camera;
	CameraFileInfo camera_info;
	CameraList *list;
	const gchar *path, *name;
	char *basename;
	guint i;
	int count;

printf ("ENTER: do_get_file_info (%s)\n", gnome_vfs_uri_get_path (uri));

	G_LOCK (cameras);

	/* Is this root? */
	if (!gnome_vfs_uri_has_parent (uri)) {
		info->name = g_strdup ("/");
		info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
				     GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->mime_type = g_strdup ("x-directory/normal");
		G_UNLOCK (cameras);
		return (GNOME_VFS_OK);
	}

printf ("Getting camera (do_get_file_info)...\n");
	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK) {
		G_UNLOCK (cameras);
		return (result);
	}

	result = GNOME_VFS_RESULT (gp_list_new (&list));
	if (result != GNOME_VFS_OK) {
		unref_camera (camera);
		G_UNLOCK (cameras);
		return (result);
	}

	/*
	 * We cannot use get_basename here because of potential trailing
	 * slashes.
	 */
	basename = gnome_vfs_uri_extract_short_name (uri);

	parent = gnome_vfs_uri_get_parent (uri);
	path = gnome_vfs_uri_get_path (parent);

	result = GNOME_VFS_RESULT (gp_camera_folder_list_folders (camera, path,
								  list, NULL));
	if (result != GNOME_VFS_OK) {
		gnome_vfs_uri_unref (parent);
		unref_camera (camera);
		gp_list_free (list);
		g_free (basename);
		G_UNLOCK (cameras);
		return (result);
	}

	count = gp_list_count (list);
	if (count < 0) {
		gnome_vfs_uri_unref (parent);
		unref_camera (camera);
		gp_list_free (list);
		g_free (basename);
		G_UNLOCK (cameras);
		return (GNOME_VFS_RESULT (count));
	}

	for (i = 0; i < count; i++) {
		result = GNOME_VFS_RESULT (gp_list_get_name (list, i, &name));
		if (result != GNOME_VFS_OK) {
			gnome_vfs_uri_unref (parent);
			unref_camera (camera);
			gp_list_free (list);
			g_free (basename);
			G_UNLOCK (cameras);
			return (result);
		}
		if (!strcmp (name, basename)) {
			info->name = g_strdup (basename);
			info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
					GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
			info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
			info->mime_type = g_strdup ("x-directory/normal");
			gnome_vfs_uri_unref (parent);
			unref_camera (camera);
			gp_list_free (list);
			g_free (basename);
			G_UNLOCK (cameras);
printf ("Found folder!\n");
			return (GNOME_VFS_OK);
		}
	}

	result = GNOME_VFS_RESULT (gp_camera_folder_list_files (camera, path,
								list, NULL));
	if (result != GNOME_VFS_OK) {
		gnome_vfs_uri_unref (parent);
		unref_camera (camera);
		gp_list_free (list);
		g_free (basename);
		G_UNLOCK (cameras);
		return (result);
	}

	count = gp_list_count (list);
	if (count < 0) {
		gnome_vfs_uri_unref (parent);
		unref_camera (camera);
		gp_list_free (list);
		g_free (basename);
		G_UNLOCK (cameras);
		return (GNOME_VFS_RESULT (count));
	} 

	for (i = 0; i < count; i++) {
		result = GNOME_VFS_RESULT (gp_list_get_name (list, i, &name));
		if (result != GNOME_VFS_OK) {
			gnome_vfs_uri_unref (parent);
			unref_camera (camera);
			gp_list_free (list);
			g_free (basename);
			G_UNLOCK (cameras);
			return (result);
		}
		if (!strcmp (name, basename)) {
			result = GNOME_VFS_RESULT (gp_camera_file_get_info (
					camera, path, basename, &camera_info,
					NULL));
			g_free (basename);
			unref_camera (camera);
			gp_list_free (list);
			if (result != GNOME_VFS_OK) {
				G_UNLOCK (cameras);
				return (result);
			}
			get_info_from_camera_info (&camera_info, FALSE, info);
			gnome_vfs_uri_unref (parent);
			G_UNLOCK (cameras);
printf ("Found file!\n");
			return (GNOME_VFS_OK);
		}
	}

	gnome_vfs_uri_unref (parent);
	g_free (basename);
	unref_camera (camera);
	gp_list_free (list);

	G_UNLOCK (cameras);

	return (GNOME_VFS_ERROR_NOT_FOUND);
}

static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod          *method,
	GnomeVFSMethodHandle    *handle,
        GnomeVFSFileInfo        *info,
        GnomeVFSFileInfoOptions  options,
        GnomeVFSContext         *context)
{
	CameraFileInfo camera_info;
	GnomeVFSResult result;
	FileHandle *fh = (FileHandle *) handle;
	const char *name;

	G_LOCK (cameras);

	gp_file_get_name (fh->file, &name);
	result = GNOME_VFS_RESULT (gp_camera_file_get_info (fh->camera,
							    fh->dirname,
							    name,
							    &camera_info,
							    NULL));
	if (result != GNOME_VFS_OK) {
		G_UNLOCK (cameras);
		return (result);
	}

	get_info_from_camera_info (&camera_info, fh->preview, info);

	G_UNLOCK (cameras);

	return (GNOME_VFS_OK);
}

static gboolean do_is_local (
        GnomeVFSMethod*                 method,
	const GnomeVFSURI*              uri)
{
	return (FALSE);
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
	char *argv[] = {"dummy"};
	int argc = 1;
	gchar **gtk_argv;

	gtk_argv = g_new0 (char*, 1);
	*gtk_argv = g_strdup ("dummy");
	gtk_init_check (&argc, &gtk_argv);
	g_free (gtk_argv);

	if (!gconf_is_initialized ())
		gconf_init (argc, argv, NULL);
	client = gconf_client_get_default ();

	gp_abilities_list_new (&al);
	gp_abilities_list_load (al, NULL);

	gp_port_info_list_new (&il);
	gp_port_info_list_load (il);

        return (&method);
}

void vfs_module_shutdown (GnomeVFSMethod *method);

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	CamerasEntry *entry;
	GSList *sl;

	/* Unref all cameras */
	for (sl = cameras; sl; sl = sl->next) {
		entry = sl->data;
		gp_camera_unref (entry->camera);
		g_free (entry->name);
	}
	g_slist_free (cameras);
	cameras = NULL;

	gp_abilities_list_free (al);
	al = NULL;

	gp_port_info_list_free (il);
	il = NULL;

	/* Unref client */
	g_object_unref (G_OBJECT (client));
	client = NULL;
}


