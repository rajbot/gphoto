
#ifndef GPHOTO_EXTENSIONS_H
#define GPHOTO_EXTENSIONS_H

#include <gphoto2.h>
#include <bonobo.h>

BEGIN_GNOME_DECLS

gint gp_camera_new_from_gconf (Camera** camera, const gchar* name_or_url);

END_GNOME_DECLS

#endif


	
