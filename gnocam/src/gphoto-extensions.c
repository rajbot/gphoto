#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gnome.h>
#include <gphoto2.h>
#include "gphoto-extensions.h"


Camera *
gp_camera_new_by_description (gchar *model, gchar *port, gchar *speed)
{
	gboolean found;
	gint number_of_ports, i;
	CameraPortInfo port_info;
	Camera *camera;

	/* Check port. */
	if (port == NULL) {
		gnome_error_dialog (_("You have to indicate a port!"));
		return (NULL);
	}
	found = FALSE;
	if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
		gnome_error_dialog (_("Could not get number of ports!"));
		return (NULL);
	}
	for (i = 0; i < number_of_ports; i++) {
		if (gp_port_info (i, &port_info) == GP_ERROR) {
			gnome_error_dialog (_("Could not get port information!"));
			return (NULL);
		}
		if (strcmp (port_info.name, port) == 0) {
			found = TRUE;
			break;
		}
	}
	if (!found) {
		gnome_error_dialog (_("Could not find port!"));
		return (NULL);
	}

	/* Check speed. */
	//FIXME: Check the speed...
	if (speed == NULL) port_info.speed = 0;
	else port_info.speed = atoi (speed);

	/* Check model. */
	if (model == NULL) {
		gnome_error_dialog (_("You have to indicate a model!"));
		return (NULL);
	}
	if (gp_camera_new_by_name (&camera, model) == GP_ERROR) {
		gnome_error_dialog (_("Could not set camera model!"));
		return (NULL);
	}

	if (gp_camera_init (camera, &port_info) == GP_ERROR) {
		gnome_error_dialog (_("Could not initialize camera!"));
		return (NULL);
	}
	
	/* We can return a camera. */
	return (camera);
}
