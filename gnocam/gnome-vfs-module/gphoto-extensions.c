#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include "gphoto-extensions.h"

/*************/
/* Functions */
/*************/

Camera*
gp_camera_new_by_description (gint id, gchar* name, gchar* model, gchar* port, gint speed)
{
	gint 			number_of_ports, i;
	CameraPortInfo 		port_info;
	Camera*			camera;

	g_assert (name != NULL);
	g_assert (model != NULL);
	g_assert (port != NULL);

	/* "Directory Browse needs special care. */
	if (strcmp ("Directory Browse", model) != 0) {
		/* Check port. */
		if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
			return (NULL);
		}
		for (i = 0; i < number_of_ports; i++) {
			if (gp_port_info (i, &port_info) == GP_ERROR) {
				return (NULL);
			}
			if (strcmp (port_info.name, port) == 0) break;
		}
		if (i == number_of_ports) {
			return (NULL);
		}
	
		/* Nothing to check for what concerns speed. */
		port_info.speed = speed;
	} else {
		strcpy (port_info.path, "");
	}

	/* Check model. */
	if (gp_camera_new_by_name (&camera, model) == GP_ERROR) {
		return (NULL);
	}
	
	/* Initialize the camera. */
	if (gp_camera_init (camera, &port_info) == GP_ERROR) {
//FIXME: Why crashes the whole thing when initialization failed and I free the camera?
//		gp_camera_free (camera);
		return (NULL);
	}

	return (camera);
}






