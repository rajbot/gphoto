
#include "../bonobo-storage-module/bonobo-storage-camera.h"

#include <bonobo.h>

#include "bonobo-extensions.h"

BonoboStorage*
bonobo_storage_open_full_with_data (const char* driver, const char* path, gint flags, gint mode, CORBA_Environment* ev, gpointer user_data)
{
	BonoboStorageCamera*	new;
	Camera*			camera;

	camera = (Camera*) user_data;

        /* Get the real path (i.e. without "camera://camera_name") */
        if (!strncmp (path, "camera:", 7)) path += 7;
        for (path += 2; *path != 0; path++) if (*path == '/') break;

        /* Create the storage. */
        new = bonobo_storage_camera_new (camera, path, flags, ev);
        gp_camera_unref (camera);
        if (BONOBO_EX (ev)) return (NULL);
        g_return_val_if_fail (new, NULL);

        return (BONOBO_STORAGE (new));
}


