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
	case GP_ERROR_NO_CAMERA_FOUND:
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

static GMutex *client_mutex;
static GMutex *cameras_mutex;

static GnomeVFSResult
get_camera (GnomeVFSURI *uri, Camera **camera)
{
	CamerasEntry *entry;
	const gchar *host;
	GSList *list, *sl;
	guint i;
	GnomeVFSResult result;

	g_return_val_if_fail (uri, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (camera, GNOME_VFS_ERROR_BAD_PARAMETERS);

	*camera = NULL;

	host = gnome_vfs_uri_get_host_name (uri);

	/* Could be that this camera is currently active */
	g_mutex_lock (cameras_mutex);
	for (sl = cameras; sl; sl = sl->next) {
		entry = sl->data;
		if (!strcmp (entry->name, host)) {
			*camera = entry->camera;
			gp_camera_ref (entry->camera);
			g_mutex_unlock (cameras_mutex);
			return (GNOME_VFS_OK);
		}
	}

	/* Ok, this camera isn't active. */
	gp_camera_new (camera);
	g_mutex_lock (client_mutex);
	list = gconf_client_get_list (client, "/apps/" PACKAGE "/cameras",
				      GCONF_VALUE_STRING, NULL);
printf ("Our database has %i entries.\n", g_slist_length (list));
	if (!host) {
		if (!gconf_client_get_bool (client,
					   "/apps/" PACKAGE "/autodetect",
					   NULL) &&
		    (g_slist_length (list) == 3)) {
			strcpy ((*camera)->model, g_slist_nth_data (list, 1));
			strcpy ((*camera)->port->name,
				g_slist_nth_data (list, 2));
		}
	} else 
		for (i = 0; i < g_slist_length (list); i += 3)
			if (!strcmp (g_slist_nth_data (list, i), host)) {
				strcpy ((*camera)->model,
					g_slist_nth_data (list, i + 1));
				strcpy ((*camera)->port->name,
					g_slist_nth_data (list, i + 2));
printf ("Found %s in database!\n", host);
			}
	g_mutex_unlock (client_mutex);

	result = GNOME_VFS_RESULT (gp_camera_init (*camera));
	if (result != GNOME_VFS_OK) {
		gp_camera_unref (*camera);
		g_mutex_unlock (cameras_mutex);
printf ("Could not initialize camera!\n");
		return (result);
	}

	/* Cache the result */
	entry = g_new0 (CamerasEntry, 1);
	entry->camera = *camera;
	entry->name = g_strdup (host);
	gp_camera_ref (*camera);
	cameras = g_slist_append (cameras, entry);

	g_mutex_unlock (cameras_mutex);

	return (GNOME_VFS_OK);
}

static void
unref_camera (Camera *camera)
{
	CamerasEntry *entry;
	GSList *sl;

	g_mutex_lock (cameras_mutex);
	gp_camera_unref (camera);

	/* We don't want to keep USB cameras in cache */
	if (camera->port->type & GP_PORT_USB)
		for (sl = cameras; sl; sl = sl->next) {
			entry = sl->data;
			if ((camera == entry->camera) &&
			    (camera->ref_count == 1)) {
				cameras = g_slist_remove_link (cameras, sl);
				gp_camera_unref (entry->camera);
				g_free (entry);
			}
		}
	g_mutex_unlock (cameras_mutex);
}

typedef struct {
	Camera *camera;
	GMutex *camera_mutex;
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

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK)
		return (result);

	dirname = gnome_vfs_uri_extract_dirname (uri);
	filename = gnome_vfs_uri_get_basename (uri);
	file = gp_file_new ();
	g_mutex_lock (cameras_mutex);
	result = GNOME_VFS_RESULT (gp_camera_file_get_file (camera, dirname,
							    filename, file));
	g_mutex_unlock (cameras_mutex);
	if (result != GNOME_VFS_OK) {
		gp_file_unref (file);
		unref_camera (camera);
		g_free (dirname);
		return (result);
	}

	/* Construct handle */
	file_handle = g_new0 (FileHandle, 1);
	g_mutex_lock (cameras_mutex);
	file_handle->camera = camera;
	g_mutex_unlock (cameras_mutex);
	file_handle->file = file;
	file_handle->dirname = dirname;
	file_handle->create = FALSE;
	file_handle->preview = (gnome_vfs_uri_get_user_name (uri) != NULL);
	*handle = (GnomeVFSMethodHandle *) file_handle;

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
	FileHandle *file_handle;
	GnomeVFSResult result;

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK)
		return (result);

	if (!(camera->abilities->folder_operations &
					GP_FOLDER_OPERATION_PUT_FILE)) {
		unref_camera (camera);
		return (GNOME_VFS_ERROR_NOT_SUPPORTED);
	}

	/* Construct the file handle */
	file_handle = g_new0 (FileHandle, 1);
	g_mutex_lock (cameras_mutex);
	file_handle->camera = camera;
	g_mutex_unlock (cameras_mutex);
	file_handle->file = gp_file_new ();
	file_handle->create = TRUE;
	file_handle->dirname = gnome_vfs_uri_extract_dirname (uri);
	file_handle->preview = (gnome_vfs_uri_get_user_name (uri) != NULL);
	*handle = (GnomeVFSMethodHandle *) file_handle;

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_close (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSContext*                context)
{
	FileHandle *fh = (FileHandle *) handle;
	GnomeVFSResult result = GNOME_VFS_OK;

	if (fh->create) {
		g_mutex_lock (fh->camera_mutex);
		result = GNOME_VFS_RESULT (gp_camera_folder_put_file (
				fh->camera, fh->dirname, fh->file));
		g_mutex_unlock (fh->camera_mutex);
	}

	unref_camera (fh->camera);
	gp_file_unref (fh->file);
	g_free (fh->dirname);
	g_free (fh);

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
	FileHandle *file_handle;

	file_handle = (FileHandle *) handle;

	*bytes_read = MIN (file_handle->file->size - file_handle->pos,
			  num_bytes);
	if (!*bytes_read)
		return (GNOME_VFS_ERROR_EOF);

	memcpy (buffer, file_handle->file->data + file_handle->pos,*bytes_read);
	file_handle->pos += *bytes_read;
	
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

	file_handle = (FileHandle *) handle;

	result = GNOME_VFS_RESULT (gp_file_append (file_handle->file,
						   (char*) buffer, num_bytes));
	*bytes_written = num_bytes;

	return (result);
}

static GnomeVFSResult do_seek (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSSeekPosition            position,
        GnomeVFSFileOffset              offset,
        GnomeVFSContext*                context)
{
	FileHandle *file_handle;

	file_handle = (FileHandle *) handle;

	switch (position) {
	case GNOME_VFS_SEEK_START:
		file_handle->pos = MIN (file_handle->file->size, offset);
		break;
	case GNOME_VFS_SEEK_CURRENT:
		file_handle->pos = MIN (file_handle->pos + offset,
					file_handle->file->size);
		break;
	case GNOME_VFS_SEEK_END:
		file_handle->pos = MAX (file_handle->file->size - 1 - offset,0);
		break;
	}

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_tell (
	GnomeVFSMethod*		method,
	GnomeVFSMethodHandle*	handle,
	GnomeVFSFileOffset*	offset)
{
	FileHandle *file_handle;

	file_handle = (FileHandle *) handle;

	*offset = file_handle->pos;

	return GNOME_VFS_OK;
}

typedef struct {
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

printf ("do_open_directory\n");

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK)
		return (result);

	path = gnome_vfs_uri_get_path (uri);
	if (!*path)
		path = "/";

	/* Get the list of files */
	files = gp_list_new ();
	g_mutex_lock (cameras_mutex);
	result = GNOME_VFS_RESULT (gp_camera_folder_list_files (camera, path,
								files));
	g_mutex_unlock (cameras_mutex);
	if (result != GNOME_VFS_OK) {
		gp_list_free (files);
		unref_camera (camera);
		return (result);
	}

	/* Get the list of directories */
	dirs = gp_list_new ();
	g_mutex_lock (cameras_mutex);
	result = GNOME_VFS_RESULT (gp_camera_folder_list_folders (camera, path,
								  dirs));
	g_mutex_unlock (cameras_mutex);
	unref_camera (camera);
	if (result != GNOME_VFS_OK) {
		gp_list_free (files);
		gp_list_free (dirs);
		return (result);
	}

	/* Construct the handle */
	directory_handle = g_new0 (DirectoryHandle, 1);
	directory_handle->files = files;
	directory_handle->dirs = dirs;
	directory_handle->pos = 0;
	*handle = (GnomeVFSMethodHandle *) directory_handle;

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
	DirectoryHandle *directory_handle;

	/* Free the handle */
	directory_handle = (DirectoryHandle *) handle;
	gp_list_free (directory_handle->files);
	gp_list_free (directory_handle->dirs);
	g_free (directory_handle);

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*		info,
	GnomeVFSContext*                context)
{
	CameraListEntry *entry;
	DirectoryHandle *dh;

	dh = (DirectoryHandle *) handle;

	if (dh->pos >= dh->dirs->count + dh->files->count)
		return (GNOME_VFS_ERROR_EOF);

	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);

	/* Tell gnome-vfs which fields will be valid */
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE |
			     GNOME_VFS_FILE_INFO_FIELDS_TYPE;

	/* Fill the fields */
	if (dh->pos < dh->dirs->count) {
		entry = gp_list_entry (dh->dirs, dh->pos);
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
	} else {
		entry = gp_list_entry (dh->files, dh->pos - dh->dirs->count);
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
	}
	info->name = g_strdup (entry->name);

	dh->pos++;

	return (GNOME_VFS_OK);
}

static void
get_info_from_camera_info (CameraFileInfo *cfi, gboolean preview,
			   GnomeVFSFileInfo *info)
{
	CameraFileInfoStruct cfis;

	info->name = g_strdup (cfi->file.name);
	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE;
	info->type = GNOME_VFS_FILE_TYPE_REGULAR;

	if (preview)
		cfis = cfi->preview;
	else
		cfis = cfi->file;

	if (cfis.fields & GP_FILE_INFO_SIZE) {
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
		info->size = cfis.size;
	}

	if (cfis.fields & GP_FILE_INFO_TYPE) {
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		info->mime_type = g_strdup (cfis.type);
	}

	if (cfis.fields & GP_FILE_INFO_PERMISSIONS) {
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS;
		info->permissions = 0;
		if (cfis.permissions & GP_FILE_PERM_READ)
			info->permissions |= GNOME_VFS_PERM_USER_READ |
					     GNOME_VFS_PERM_GROUP_READ |
					     GNOME_VFS_PERM_OTHER_READ;
		if (cfis.permissions & GP_FILE_PERM_DELETE)
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
	CameraList list;
	const gchar *path, *basename;
	guint i;

printf ("do_get_file_info (%s)\n", gnome_vfs_uri_get_path (uri));

	/* Is this root? */
	if (!gnome_vfs_uri_has_parent (uri)) {
		info->name = g_strdup ("/");
		info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
				     GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->mime_type = g_strdup ("x-directory/normal");
		return (GNOME_VFS_OK);
	}

	result = get_camera (uri, &camera);
	if (result != GNOME_VFS_OK)
		return (result);

	basename = gnome_vfs_uri_get_basename (uri);
	parent = gnome_vfs_uri_get_parent (uri);
	path = gnome_vfs_uri_get_path (parent);

	g_mutex_lock (cameras_mutex);
	result = GNOME_VFS_RESULT (gp_camera_folder_list_folders (camera, path,
								  &list));
	g_mutex_unlock (cameras_mutex);
	if (result != GNOME_VFS_OK) {
		gnome_vfs_uri_unref (parent);
		unref_camera (camera);
		return (result);
	}

	for (i = 0; i < list.count; i++)
		if (!strcmp (gp_list_entry (&list, i)->name, basename)) {
			info->name = g_strdup (basename);
			info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE |
					GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
			info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
			info->mime_type = g_strdup ("x-directory/normal");
			gnome_vfs_uri_unref (parent);
			unref_camera (camera);
printf ("Found it!\n");
			return (GNOME_VFS_OK);
		}
	
	g_mutex_lock (cameras_mutex);
	result = GNOME_VFS_RESULT (gp_camera_folder_list_files (camera, path,
								&list));
	g_mutex_unlock (cameras_mutex);
	if (result != GNOME_VFS_OK) {
		gnome_vfs_uri_unref (parent);
		unref_camera (camera);
		return (result);
	}

	for (i = 0; i < list.count; i++)
		if (!strcmp (gp_list_entry (&list, i)->name, basename)) {
			g_mutex_lock (cameras_mutex);
			result = GNOME_VFS_RESULT (gp_camera_file_get_info (
					camera, path, basename, &camera_info));
			g_mutex_unlock (cameras_mutex);
			unref_camera (camera);
			if (result != GNOME_VFS_OK)
				return (result);
			get_info_from_camera_info (&camera_info, FALSE, info);
			gnome_vfs_uri_unref (parent);
printf ("Found it!\n");
			return (GNOME_VFS_OK);
		}

	gnome_vfs_uri_unref (parent);
	unref_camera (camera);
	
	return (GNOME_VFS_ERROR_NOT_FOUND);
}

static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
        GnomeVFSFileInfo*               info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	CameraFileInfo camera_info;
	GnomeVFSResult result;
	FileHandle *fh;

	fh = (FileHandle *) handle;

	g_mutex_lock (cameras_mutex);
	result = GNOME_VFS_RESULT (gp_camera_file_get_info (fh->camera,
							    fh->dirname,
							    fh->file->name,
							    &camera_info));
	g_mutex_unlock (cameras_mutex);
	if (result != GNOME_VFS_OK)
		return (result);

	get_info_from_camera_info (&camera_info, fh->preview, info);

	return (GNOME_VFS_OK);
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

	if (!gconf_is_initialized ())
		gconf_init (argc, argv, NULL);
	client = gconf_client_get_default ();

	if (g_thread_supported ()) {
		cameras_mutex = g_mutex_new ();
		client_mutex = g_mutex_new ();
	}

	/* Initialize gphoto */
	gp_init (GP_DEBUG_NONE);

        return (&method);
}

void vfs_module_shutdown (GnomeVFSMethod *method);

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	CamerasEntry *entry;
	GSList *sl;

	/* Unref all cameras */
	g_mutex_free (cameras_mutex);
	cameras_mutex = NULL;
	for (sl = cameras; sl; sl = sl->next) {
		entry = sl->data;
		gp_camera_unref (entry->camera);
		g_free (entry->name);
	}
	g_slist_free (cameras);
	cameras = NULL;

	/* Exit gphoto */
	gp_exit ();

	/* Unref client */
	g_mutex_free (client_mutex);
	client_mutex = NULL;
	gtk_object_unref (GTK_OBJECT (client));
	client = NULL;
}


