#include <config.h>

#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-directory.h>

#define URI1 "camera:Hewlett Packard:PhotoSmart C20:Serial Port 0:/"
#define URI2 "camera:Hewlett Packard:PhotoSmart C200:Serial Port 0:/"

int
main (int argc, char **argv)
{
	GnomeVFSURI *u1, *u2;
	GnomeVFSResult r;
	gboolean b;
	GnomeVFSDirectoryHandle *dh = NULL;
	GnomeVFSFileInfo *fi;
	GnomeVFSHandle *fh = NULL;
	gchar *s, buf;

	/* Initialize libgnomevfs */
	if (!gnome_vfs_init ()) g_error ("Could not initialize gnome-vfs!");

	u1 = gnome_vfs_uri_new (URI1);
	if (!u1) g_error ("Could not create URI for '%s'!", URI1);
	u2 = gnome_vfs_uri_new (URI2);
	if (!u2) g_error ("Could not create URI for '%s'!", URI2);

	/* Check if local. */
	g_message ("'%s' is %slocal.", URI1,
		   gnome_vfs_uri_is_local (u1) ? "" : "not ");
	g_message ("'%s' is %slocal.", URI2,
		   gnome_vfs_uri_is_local (u2) ? "" : "not ");

	r = gnome_vfs_check_same_fs (URI1, URI2, &b);
	if (r != GNOME_VFS_OK)
		g_error ("Could not check if on same "
			"filesystem: %s", gnome_vfs_result_to_string (r));
	g_message ("'%s' and '%s' are %son the same filesystem.", URI1, URI2,
		   b ? "" : "not ");

	g_message ("Reading root directory...");
	r = gnome_vfs_directory_open (&dh, URI1, GNOME_VFS_FILE_INFO_DEFAULT);
	if (r != GNOME_VFS_OK)
		g_error ("Could not read root directory: %s",
			 gnome_vfs_result_to_string (r));
	fi = gnome_vfs_file_info_new ();
	while (1) {
		r = gnome_vfs_directory_read_next (dh, fi);
		if (r == GNOME_VFS_ERROR_EOF)
			break;
		if (r != GNOME_VFS_OK)
			g_error ("Could not read next entry: %s",
				 gnome_vfs_result_to_string (r));
		g_message (" - '%s'", fi->name);
		if (fi->type == GNOME_VFS_FILE_TYPE_REGULAR) {
			s = g_strdup_printf ("%s%s", URI1, fi->name);
			r = gnome_vfs_open (&fh, s, GNOME_VFS_OPEN_READ);
			if (r != GNOME_VFS_OK)
				g_error ("Could not open '%s': %s",
					 s, gnome_vfs_result_to_string (r));
			g_free (s);
			r = gnome_vfs_read (fh, &buf, 1, NULL);
			if (r != GNOME_VFS_OK)
				g_error ("Could not read: %s",
					 gnome_vfs_result_to_string (r));
			gnome_vfs_close (fh);
		}
	}
	gnome_vfs_file_info_unref (fi);
	gnome_vfs_directory_close (dh);

	return 0;
}
