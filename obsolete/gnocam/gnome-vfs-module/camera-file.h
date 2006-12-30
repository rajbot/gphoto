#ifndef __CAMERA_FILE_H__
#define __CAMERA_FILE_H__

#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-file-size.h>

typedef struct _CameraFile CameraFile;

GnomeVFSResult camera_file_open  (GnomeVFSURI *, CameraFile **);
GnomeVFSResult camera_file_read  (CameraFile *, gpointer, GnomeVFSFileSize,
							  GnomeVFSFileSize *);
GnomeVFSResult camera_file_close (CameraFile *);

#endif
