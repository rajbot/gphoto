#include <bonobo.h>
#include <gnome.h>
#include <gphoto2.h>
#include <glade/glade.h>
#include "gnocam.h"
#include "gphoto-extensions.h"

/*************/
/* Functions */
/*************/

gint
gp_camera_new_by_description (gint id, gchar* name, gchar* model, gchar* port, gint speed, Camera** camera)
{
	gint 			number_of_ports, i;
	gint			result = GP_OK;
	frontend_data_t*	frontend_data;

	g_return_val_if_fail (name, 	GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (model, 	GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (port, 	GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (camera, 	GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (!*camera, GP_ERROR_BAD_PARAMETERS);

	/* Check the model. */
	if ((result = gp_camera_new_by_name (camera, model)) != GP_OK) return (result);

	/* "Directory Browse needs special care. */
	if (strcmp ("Directory Browse", model) != 0) {
	
		/* Check port. */
		if ((number_of_ports = gp_port_count ()) < 0) return (result);
		for (i = 0; i < number_of_ports; i++) {
			if ((result = gp_port_info (i, (*camera)->port)) != GP_OK) return (result);
			if (strcmp ((*camera)->port->name, port) == 0) break;
		}
		if (i == number_of_ports) return (GP_ERROR);
	
		/* Nothing to check for what concerns speed. */
		(*camera)->port->speed = speed;
		
	} else strcpy ((*camera)->port->path, "");

	/* Initialize the camera. */
	if ((result = gp_camera_init (*camera)) != GP_OK) return (result);

	/* Store our data. */
	frontend_data = g_new (frontend_data_t, 1);
	frontend_data->id = id;
	frontend_data->name = g_strdup (name);
	(*camera)->frontend_data = frontend_data;

	return (GP_OK);
}






