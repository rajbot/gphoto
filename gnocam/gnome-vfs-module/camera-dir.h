#ifndef __CAMERA_DIR_H__
#define __CAMERA_DIR_H__

#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-uri.h>

typedef struct {
} DirHandle;

GnomeVFSResult camera_dir_get (GnomeVFSURI *, DirHandle **);

#endif
