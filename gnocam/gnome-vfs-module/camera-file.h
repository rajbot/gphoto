#ifndef __CAMERA_FILE_H__
#define __CAMERA_FILE_H__

#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-uri.h>

typedef struct {
} FileHandle;

GnomeVFSResult camera_file_get (GnomeVFSURI *, FileHandle **);

#endif
