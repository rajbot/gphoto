#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gnome.h>
#include <gphoto2.h>
#include "gphoto-extensions.h"


Camera *
gp_camera_new_by_description (gchar* description)
{
	gint 		number_of_ports, i;
	CameraPortInfo 	port_info;
	Camera*		camera;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gint		j;
	gint		count;

	g_assert (description != NULL);

	/* Check first if description is valid. 		*/
	/* Example for description: 				*/
	/* "1\nMy Camera\nHP PhotoSmart C30\nSerial Port 0\n". 	*/
	count = 0;
	for (j = 0; description[j] != '\0'; j++) {
                if (description[j] == '\n') {
                        description[j] = '\0';
                        count++;
                }
        }
        g_assert (count == 4);
        for (j = 0; description[j] != '\0'; j++);
	j++;
        for (; description[j] != '\0'; j++);
        model = g_strdup (&description[++j]);
        for (; description[j] != '\0'; j++);
        port = g_strdup (&description[++j]);
        for (; description[j] != '\0'; j++);
        speed = g_strdup (&description[++j]);

	/* Check port. */
	if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
		gnome_error_dialog (_("Could not get number of ports!"));
		g_free (model);
		g_free (port);
		g_free (speed);
		return (NULL);
	}
	for (i = 0; i < number_of_ports; i++) {
		if (gp_port_info (i, &port_info) == GP_ERROR) {
			gnome_error_dialog (_("Could not get port information!"));
			g_free (model);
			g_free (port);
			g_free (speed);
			return (NULL);
		}
		if (strcmp (port_info.name, port) == 0) break;
	}
	if (i == number_of_ports) {
		gnome_error_dialog (_("Could not find port!"));
		g_free (model);
		g_free (port);
		g_free (speed);
		return (NULL);
	}

	/* Check speed. */
	//FIXME: Check the speed...
	if (strcmp (speed, "") == 0) port_info.speed = 0;
	else port_info.speed = atoi (speed);

	/* Check model. */
	if (gp_camera_new_by_name (&camera, model) == GP_ERROR) {
		gnome_error_dialog (_("Could not set camera model!"));
		g_free (model);
		g_free (port);
		g_free (speed);
		return (NULL);
	}

	if (gp_camera_init (camera, &port_info) == GP_ERROR) {
		gnome_error_dialog (_("Could not initialize camera!"));
		g_free (model);
		g_free (port);
		g_free (speed);
		return (NULL);
	}
	
	/* Clean up. */
	g_free (model);
	g_free (port);
	g_free (speed);

	/* We can return a camera. */
	return (camera);
}
