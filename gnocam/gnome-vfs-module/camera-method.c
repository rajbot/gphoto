#include "config.h"
#include "GNOME_C.h"
#include "camera-dir.h"
#include "camera-file.h"
#include "camera-utils.h"

#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-method.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bonobo/bonobo-main.h>

static GnomeVFSResult
do_open (GnomeVFSMethod *method, GnomeVFSMethodHandle **handle, 
	 GnomeVFSURI *uri, GnomeVFSOpenMode mode, GnomeVFSContext *context)
{
	FileHandle *fh;
	GnomeVFSResult r;

	r = camera_file_get (uri, &fh);
	if (r != GNOME_VFS_OK) return r;

	return (GNOME_VFS_OK);
}

#if 0
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
	file_handle->preview = (camera_uri_get_user_name (uri) != NULL);
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
#endif

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method, GnomeVFSMethodHandle **handle,
	GnomeVFSURI *uri, GnomeVFSFileInfoOptions options,
	GnomeVFSContext *context)
{
	DirHandle *dh;
	GnomeVFSResult r;

	r = camera_dir_get (uri, &dh);
	if (r != GNOME_VFS_OK) return r;

	return (GNOME_VFS_OK);
}

#if 0
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

printf ("ENTER: do_get_file_info (%s)\n", camera_uri_get_path (uri));

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

	parent = camera_uri_get_parent (uri);
	path = camera_uri_get_path (parent);

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
#endif

static gboolean do_is_local (GnomeVFSMethod *method, const GnomeVFSURI *uri)
{
	return FALSE;
}

static GnomeVFSResult do_check_same_fs (
	GnomeVFSMethod *method, GnomeVFSURI *a, GnomeVFSURI *b,
	gboolean *same_fs_return, GnomeVFSContext *context)
{
	gchar *manuf1, *manuf2, *model1, *model2, *port1, *port2;

	manuf1 = camera_uri_get_manuf (a);
	manuf2 = camera_uri_get_manuf (b);
	model1 = camera_uri_get_model (a);
	model2 = camera_uri_get_model (b);
	port1  = camera_uri_get_port (a);
	port2  = camera_uri_get_port (b);
	if (manuf1 && manuf2 && model1 && model2 && port1 && port2)
		*same_fs_return = !strcmp (manuf1, manuf2) &&
				  !strcmp (model1, model2) &&
				  !strcmp (port1, port2);
	g_free (model1); g_free (model2);
	g_free (manuf1); g_free (manuf2);
	g_free (port1); g_free (port2);
	return GNOME_VFS_OK;
}

static GnomeVFSMethod method = {
        sizeof (GnomeVFSMethod),
        do_open,
        NULL, //do_create,
        NULL, //do_close,
        NULL, //do_read,
        NULL, //do_write,
        NULL, //do_seek,
        NULL, //do_tell,
        NULL,                           /* do_truncate_handle           */
        do_open_directory,
        NULL, //do_close_directory,
        NULL, //do_read_directory,
        NULL, //do_get_file_info,
        NULL, //do_get_file_info_from_handle,
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
	/* Initialize bonobo */
	if (!bonobo_is_initialized () && !bonobo_init (NULL, NULL))
		return NULL;

        return (&method);
}

void vfs_module_shutdown (GnomeVFSMethod *method);

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
}
