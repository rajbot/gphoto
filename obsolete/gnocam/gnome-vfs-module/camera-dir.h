#ifndef __CAMERA_DIR_H__
#define __CAMERA_DIR_H__

#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-file-info.h>

typedef struct _CameraDir CameraDir;

GnomeVFSResult camera_dir_open  (GnomeVFSURI *, CameraDir **);
GnomeVFSResult camera_dir_read  (CameraDir *, GnomeVFSFileInfo *);
GnomeVFSResult camera_dir_close (CameraDir *);

#endif
