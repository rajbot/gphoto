#include <stdlib.h>
#include <gtk/gtk.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>
#include <parser.h>
#include "gphoto-extensions.h"

/********************/
/* Type Definitions */
/********************/

typedef struct {
	CameraFile*	file;
	glong		position;
} file_handle_t;

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
	do_find_directory,
	do_create_symbolic_link};

/*************/
/* Functions */
/*************/

GnomeVFSMethod*
vfs_module_init (const gchar* method_name, const gchar* args)
{
	GError*		gerror;

	gtk_init (NULL, NULL);
	if (!gconf_init (0, NULL, &gerror)) g_warning (_("Could not initialize gconf! (%s)"), gerror->message);
	gp_init (GP_DEBUG_NONE);
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod* method)
{
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
	GConfClient*		client;
	GConfValue*		value;
	gchar*			xml;
	gchar*			id = NULL;
	gchar*			name= NULL;
	gchar*			model = NULL;
	gchar*			port = NULL;
	gchar*			speed = NULL;
	const gchar*		filename;
	gchar*			dirname;
	gint			i;
	xmlDocPtr		doc;
	xmlNodePtr		node;
	GSList*			list;
	Camera*			camera;
	CameraFile*		file;
	file_handle_t*		file_handle;
	
	/* Do we really have a file? */
	if (!(filename = gnome_vfs_uri_get_basename (uri))) return (GNOME_VFS_ERROR_IS_DIRECTORY);
	
	/* Does gconf know about the camera (host)? */
	g_assert (client = gconf_client_get_default ());
	if (!(value = gconf_client_get (client, "/apps/" PACKAGE "/cameras", NULL))) {
		gtk_object_unref (GTK_OBJECT (client));
		return (GNOME_VFS_ERROR_HOST_NOT_FOUND);
	}
	g_assert (value->type == GCONF_VALUE_LIST);
	g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
	list = gconf_value_get_list (value);
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
		if (strcmp (name, gnome_vfs_uri_get_host_name (uri)) == 0) break;
	}
	if (i == g_slist_length (list)) {
		gtk_object_unref (GTK_OBJECT (client));
		return (GNOME_VFS_ERROR_HOST_NOT_FOUND);
	}
	gtk_object_unref (GTK_OBJECT (client));

	/* Connect to the camera (host). */
	if (!(camera = gp_camera_new_by_description (atoi (id), name, model, port, atoi (speed)))) return (GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE);

	/* Get the file. */
	dirname = gnome_vfs_uri_extract_dirname (uri);
	file = gp_file_new ();
	if (gp_camera_file_get_preview (camera, file, dirname, (gchar*) filename) != GP_OK) return (GNOME_VFS_ERROR_GENERIC);
	gp_file_ref (file);
	file_handle = g_new (file_handle_t, 1);
	file_handle->file = file;
	file_handle->position = 0;
	handle = (GnomeVFSMethodHandle) &file_handle;
	
	return (GNOME_VFS_OK);
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
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_close (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSContext*                context)
{
	file_handle_t*		file_handle = NULL;
	
	g_return_val_if_fail ((file_handle = (file_handle_t*) handle), GNOME_VFS_ERROR_INTERNAL);

	gp_file_unref (file_handle->file);
	g_free (file_handle);
	return (GNOME_VFS_OK);
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
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_seek (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
        GnomeVFSSeekPosition            whence,
        GnomeVFSFileOffset              offset,
        GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_tell (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileOffset*             offset)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_truncate_handle (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle, 
        GnomeVFSFileSize                where,  
        GnomeVFSContext*                context)
{
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
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_close_directory (
        GnomeVFSMethod*                 method,
        GnomeVFSMethodHandle*           handle,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_read_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
	GnomeVFSFileInfo*               file_info,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_get_file_info (
        GnomeVFSMethod*                 method,
        GnomeVFSURI*                    uri,
        GnomeVFSFileInfo*               file_info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_get_file_info_from_handle (
	GnomeVFSMethod*                 method,
	GnomeVFSMethodHandle*           handle,
        GnomeVFSFileInfo*               file_info,
        GnomeVFSFileInfoOptions         options,
        GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static gboolean do_is_local (
        GnomeVFSMethod*                 method,
	const GnomeVFSURI*              uri)
{
	/* We don't have local files. */
	return (FALSE);
}

static GnomeVFSResult do_make_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	guint                           perm,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_remove_directory (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_move (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    old_uri,
	GnomeVFSURI*                    new_uri,
	gboolean                        force_replace,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_unlink (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_check_same_fs (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    a,
	GnomeVFSURI*                    b,
	gboolean*                       same_fs_return,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_set_file_info (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	const GnomeVFSFileInfo*         file_info,
	GnomeVFSSetFileInfoMask         mask,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_truncate (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	GnomeVFSFileSize                where,
	GnomeVFSContext*                context)
{
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
	return (GNOME_VFS_ERROR_INTERNAL);
}

static GnomeVFSResult do_create_symbolic_link (
	GnomeVFSMethod*                 method,
	GnomeVFSURI*                    uri,
	const gchar*                    target_reference,
	GnomeVFSContext*                context)
{
	return (GNOME_VFS_ERROR_INTERNAL);
}

