#include <stdlib.h>
#include <gtk/gtk.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>
#include <parser.h>
#include "gphoto-extensions.h"
#include "utils.h"

/**************/
/* Prototypes */
/**************/

GnomeVFSMethod* vfs_module_init (const gchar* method_name, const gchar* args);
void 		vfs_module_shutdown (GnomeVFSMethod* method);

static GnomeVFSResult do_open (
	GnomeVFSMethod* 		method, 
	GnomeVFSMethodHandle** 		handle, 
	GnomeVFSURI* 			uri, 
	GnomeVFSOpenMode 		mode, 
	GnomeVFSContext* 		context);
static GnomeVFSResult do_create	(
	GnomeVFSMethod* 		method, 
	GnomeVFSMethodHandle** 		handle, 
	GnomeVFSURI* 			uri, 
	GnomeVFSOpenMode 		mode, 
	gboolean 			exclusive, 
	guint 				perm, 
	GnomeVFSContext* 		context);
static GnomeVFSResult do_close (
	GnomeVFSMethod* 		method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSContext*		context);
static GnomeVFSResult do_read (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	gpointer			buffer,
	GnomeVFSFileSize		num_bytes,
	GnomeVFSFileSize*		bytes_read,
	GnomeVFSContext*		context);
static GnomeVFSResult do_write (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	gconstpointer			buffer,
	GnomeVFSFileSize		num_bytes,
	GnomeVFSFileSize*		bytes_written,
	GnomeVFSContext*		context);
static GnomeVFSResult do_seek (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSSeekPosition		whence,
	GnomeVFSFileOffset		offset,
	GnomeVFSContext*		context);
static GnomeVFSResult do_tell (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle* 		handle,
	GnomeVFSFileOffset*		offset);
static GnomeVFSResult do_truncate_handle (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSFileSize                where,
        GnomeVFSContext*                context);
static GnomeVFSResult do_open_directory (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle**		handle,
	GnomeVFSURI*			uri,
	GnomeVFSFileInfoOptions		options,
	const GnomeVFSDirectoryFilter*	filter,
	GnomeVFSContext*		context);
static GnomeVFSResult do_close_directory (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSContext*		context);
static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSFileInfo*		file_info,
	GnomeVFSContext*		context);
static GnomeVFSResult do_get_file_info (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	GnomeVFSFileInfo*		file_info,
	GnomeVFSFileInfoOptions		options,
	GnomeVFSContext*		context);
static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod*			method,
	GnomeVFSMethodHandle*		handle,
	GnomeVFSFileInfo*		file_info,
	GnomeVFSFileInfoOptions		options,
	GnomeVFSContext*		context);
static gboolean do_is_local (
	GnomeVFSMethod*			method,
	const GnomeVFSURI*		uri);
static GnomeVFSResult do_make_directory (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	guint				perm,
	GnomeVFSContext*		context);
static GnomeVFSResult do_remove_directory (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	GnomeVFSContext*		context);
static GnomeVFSResult do_move (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			old_uri,
	GnomeVFSURI*			new_uri,
	gboolean			force_replace,
	GnomeVFSContext*		context);
static GnomeVFSResult do_unlink (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	GnomeVFSContext*		context);
static GnomeVFSResult do_check_same_fs (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			a,
	GnomeVFSURI*			b,
	gboolean*			same_fs_return,
	GnomeVFSContext*		context);
static GnomeVFSResult do_set_file_info (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	const GnomeVFSFileInfo*		file_info,
	GnomeVFSSetFileInfoMask		mask,
	GnomeVFSContext*		context);
static GnomeVFSResult do_truncate (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	GnomeVFSFileSize		where,
	GnomeVFSContext*		context);
static GnomeVFSResult do_find_directory (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			near_uri,
	GnomeVFSFindDirectoryKind	kind,
	GnomeVFSURI**			result_uri,
	gboolean			create_if_needed,
	gboolean			find_if_needed,
	guint				permissions,
	GnomeVFSContext*		context);
static GnomeVFSResult do_create_symbolic_link (
	GnomeVFSMethod*			method,
	GnomeVFSURI*			uri,
	const gchar*			target_reference,
	GnomeVFSContext*		context);

/********************/
/* Static Variables */
/********************/

static GnomeVFSMethod method = {
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	do_unlink,
	do_check_same_fs,
	do_set_file_info,
	do_truncate,
	NULL, //do_find_directory,
	do_create_symbolic_link};

/*************/
/* Functions */
/*************/

GnomeVFSMethod*
vfs_module_init (const gchar* method_name, const gchar* args)
{
	if (!gconf_is_initialized ()) gconf_init (0, NULL, NULL);
	gp_init (GP_DEBUG_NONE);
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod* method)
{
	g_print ("vfs_module_shutdown\n");
	gp_exit ();
	return;
}

static GnomeVFSResult do_open (
	GnomeVFSMethod* 		method, 
	GnomeVFSMethodHandle** 		handle, 
	GnomeVFSURI* 			uri, 
	GnomeVFSOpenMode 		mode, 
	GnomeVFSContext* 		context)
{
	GnomeVFSResult	result;
	
	g_print ("do_open\n");
	
	*handle = file_handle_new (uri, &result);
	
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
	g_print ("do_create\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_close (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSContext*                context)
{
	g_print ("do_close\n");

	return (file_handle_free (handle));
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

	g_print ("do_read\n");
	g_return_val_if_fail ((file_handle = (file_handle_t*) handle), GNOME_VFS_ERROR_INTERNAL);

	/* 'Read' the num_bytes data. */
	for (*bytes_read = 0; *bytes_read < num_bytes; (*bytes_read)++) {
		if (file_handle->position + *bytes_read >= file_handle->file->size) return GNOME_VFS_ERROR_EOF;
		((gchar*) buffer) [*bytes_read] = file_handle->file->data [file_handle->position + *bytes_read];
		file_handle->position++;
	}
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
	g_print ("do_write\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_seek (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSSeekPosition            whence,
        GnomeVFSFileOffset              offset,
        GnomeVFSContext*                context)
{
	g_print ("do_seek\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_tell (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileOffset*             offset)
{
	g_print ("do_tell\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_truncate_handle (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle, 
        GnomeVFSFileSize                where,  
        GnomeVFSContext*                context)
{
	g_print ("do_truncate_handle\n");
	return (GNOME_VFS_ERROR_INTERNAL);
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
	
	g_print ("do_open_directory (%s)\n", gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE));
	
	*handle = directory_handle_new (uri, options, &result);
	return (result);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
	g_print ("do_close_directory\n");

	return (directory_handle_free (handle));
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*		info,
	GnomeVFSContext*                context)
{
	directory_handle_t*	directory_handle;
	
	g_print ("CAMERA: do_read_directory\n");

	directory_handle = (directory_handle_t*) handle;
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
			info->mime_type = g_strdup ("image/png");
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
	GnomeVFSURI*	parent;
	
	g_print ("CAMERA: do_get_file_info (%s)\n", gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE));

	info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_NONE;
	if (gnome_vfs_uri_get_basename (uri)) {
		
		/* File */
		info->name = g_strdup (gnome_vfs_uri_get_basename (uri));
		info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("image/png");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	} else {
		
		/* Directory */
		if (!(parent = gnome_vfs_uri_get_parent (uri))) info->name = g_strdup ("/");
		else {
			info->name = g_strdup (gnome_vfs_uri_extract_dirname (uri));
			gnome_vfs_uri_unref (parent);
		}
		info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE) {
			info->mime_type = g_strdup ("x-directory/normal");
			info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
	}
	GNOME_VFS_FILE_INFO_SET_LOCAL (info, FALSE);	

	return (GNOME_VFS_OK);
}

static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
        GnomeVFSFileInfo*               file_info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	g_print ("do_get_file_info_from_handle\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static gboolean do_is_local (
        GnomeVFSMethod*                 method,
	const GnomeVFSURI*              uri)
{
	g_print ("do_is_local\n");
	return (FALSE);
}

static GnomeVFSResult do_make_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	guint                           perm,
	GnomeVFSContext*                context)
{
	g_print ("do_make_directory\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_remove_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSContext*                context)
{
	g_print ("do_remove_directory\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_move (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    old_uri,
	GnomeVFSURI*                    new_uri,
	gboolean                        force_replace,
	GnomeVFSContext*                context)
{
	g_print ("do_move\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_unlink (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSContext*                context)
{
	g_print ("do_unlink\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_check_same_fs (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    a,
	GnomeVFSURI*                    b,
	gboolean*                       same_fs_return,
	GnomeVFSContext*                context)
{
	g_print ("do_check_same_fs\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_set_file_info (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	const GnomeVFSFileInfo*         file_info,
	GnomeVFSSetFileInfoMask         mask,
	GnomeVFSContext*                context)
{
	g_print ("do_set_file_info\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_truncate (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSFileSize                where,
	GnomeVFSContext*                context)
{
	g_print ("do_truncate\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_find_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    near_uri,
	GnomeVFSFindDirectoryKind       kind,
	GnomeVFSURI**                   result_uri,
	gboolean                        create_if_needed,
	gboolean                        find_if_needed,
	guint                           permissions,
	GnomeVFSContext*                context)
{
	g_print ("do_find_directory\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_create_symbolic_link (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	const gchar*                    target_reference,
	GnomeVFSContext*                context)
{
	g_print ("do_create_symbolic_link\n");
	return (GNOME_VFS_ERROR_INTERNAL);
}

