#ifndef __CAMERA_UTILS_H__
#define __CAMERA_UTILS_H__

#include <libgnomevfs/gnome-vfs-uri.h>

gchar *camera_uri_get_manuf (GnomeVFSURI *);
gchar *camera_uri_get_model (GnomeVFSURI *);
gchar *camera_uri_get_port  (GnomeVFSURI *);
GList *camera_uri_get_dirs  (GnomeVFSURI *);

#endif
