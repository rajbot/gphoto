#ifndef __GNOCAM_UTIL_H__
#define __GNOCAM_UTIL_H__

#include <libgnocam/GNOME_C.h>

GNOME_C_Camera gnocam_util_get_camera (const gchar *, const gchar *,
				       const gchar *, CORBA_Environment *);

#endif
