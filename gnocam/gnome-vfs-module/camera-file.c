#include <config.h>
#include "camera-file.h"

#include <libgnocam/GNOME_C.h>

struct _CameraFile {
};

GnomeVFSResult
camera_file_open (GnomeVFSURI *uri, CameraFile **fh)
{
	*fh = g_new0 (CameraFile, 1);

	return GNOME_VFS_OK;
}

GnomeVFSResult
camera_file_read (CameraFile *fh, gpointer b, GnomeVFSFileSize num_bytes,
					      GnomeVFSFileSize *bytes_read)
{
	return GNOME_VFS_OK;
}

GnomeVFSResult
camera_file_close (CameraFile *fh)
{
	g_free (fh);

	return GNOME_VFS_OK;
}
