#include <gtk/gtkmain.h>
#include <libgnomevfs/gnome-vfs.h>

#define CHECK(r) {GnomeVFSResult result = r; if (result != GNOME_VFS_OK) g_error(gnome_vfs_result_to_string (result));}

static void
dump_info (GnomeVFSFileInfo *info)
{
	if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE)
		g_message (" - type: %i", info->type);
	if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE)
		g_message (" - mime-type: %s", info->mime_type);
	if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
		g_message (" - size: %i", (int) info->size);
}

static void
dump_file (GnomeVFSURI *uri)
{
	GnomeVFSHandle *handle;

	/* Open the URI */
	g_message ("Opening...");
	CHECK (gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ));

	/* Close the URI */
	g_message ("Closing..."); 
	CHECK (gnome_vfs_close (handle));
}

static void
dump_directory (const gchar *text_uri)
{
	GnomeVFSFileInfo *info;
	GList *list;
	guint i;

	/* Load directory list */
	g_message ("Loading directory list...");
	CHECK (gnome_vfs_directory_list_load (&list, text_uri,
			                      GNOME_VFS_FILE_INFO_DEFAULT,
					      NULL));

	/* Dump list */
	g_message ("Dumping contents of '%s'...", text_uri);
	for (i = 0; i < g_list_length (list); i++) {
		info = g_list_nth_data (list, i);
		g_message ("%s:", info->name);
		dump_info (info);
	}

	gnome_vfs_file_info_list_unref (list);
}

int main (int argc, char **argv)
{
	GnomeVFSURI *uri;
	GnomeVFSFileInfo *info;
	gchar *text_uri;

	g_assert (argc == 2);

	gtk_init (&argc, &argv);

	/* Initialize gnome-vfs */
	g_message ("Initializing gnome-vfs...");
	g_return_val_if_fail (gnome_vfs_init (), 1);

	/* Create URI */
	g_message ("Creating URI from '%s'...", argv [1]);
	uri = gnome_vfs_uri_new (argv [1]);
	text_uri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	g_message ("... '%s' created.", text_uri);
	g_free (text_uri);

	/* Get file info */
	g_message ("Getting file info...");
	info = gnome_vfs_file_info_new ();
	CHECK (gnome_vfs_get_file_info_uri (uri, info, 
				      GNOME_VFS_FILE_INFO_FIELDS_TYPE |
				      GNOME_VFS_FILE_INFO_FIELDS_SIZE |
				      GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE));
	dump_info (info);

	/* Dump contents */
	g_message ("Dumping info for '%s'...", info->name);
	switch (info->type) {
	case GNOME_VFS_FILE_TYPE_REGULAR:
		dump_file (uri);
		break;
	case GNOME_VFS_FILE_TYPE_DIRECTORY:
		dump_directory (argv [1]);
		break;
	default:
		break;
	}
	gnome_vfs_file_info_unref (info);

	gnome_vfs_uri_unref (uri);

	g_message ("Voilà!");
	return (0);
}
	
