#include <gnome.h>
#include <gphoto2.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include "gnocam.h"
#include "gphoto-extensions.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*	main_window;

/*************/
/* Functions */
/*************/

Camera*
gp_camera_new_by_description (gint id, gchar* name, gchar* model, gchar* port, gint speed)
{
	gint 			number_of_ports, i;
	gint			result;
	gchar*			message;
	CameraPortInfo 		port_info;
	Camera*			camera;
	frontend_data_t*	frontend_data;

	g_return_val_if_fail (name, NULL);
	g_return_val_if_fail (model, NULL);
	g_return_val_if_fail (port, NULL);

	/* "Directory Browse needs special care. */
	if (strcmp ("Directory Browse", model) != 0) {
		/* Check port. */
		if ((number_of_ports = gp_port_count ()) < 0) {
			message = g_strdup_printf (_("Could not get number of ports!\n(%s)"), gp_result_as_string (number_of_ports));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
			return (NULL);
		}
		for (i = 0; i < number_of_ports; i++) {
			if ((result = gp_port_info (i, &port_info)) != GP_OK) {
				message = g_strdup_printf (_("Could not get port information!\n(%s)"), gp_result_as_string (result));
				gnome_error_dialog_parented (message, main_window);
				g_free (message);
				return (NULL);
			}
			if (strcmp (port_info.name, port) == 0) break;
		}
		if (i == number_of_ports) {
			gnome_error_dialog_parented (_("Could not find port!"), main_window);
			return (NULL);
		}
	
		/* Nothing to check for what concerns speed. */
		port_info.speed = speed;
	} else {
		strcpy (port_info.path, "");
	}

	/* Check model. */
	if ((result = gp_camera_new_by_name (&camera, model)) != GP_OK) {
		message = g_strdup_printf (_("Could not set camera model!\n(%s)"), gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
		return (NULL);
	}
	
	/* Initialize the camera. */
	if ((result = gp_camera_init (camera, &port_info)) != GP_OK) {
		message = g_strdup_printf (_("Could not initialize camera!\n(%s)"), gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
//FIXME: Why crashes the whole thing when initialization failed and I free the camera?
//		gp_camera_free (camera);
		return (NULL);
	}

	/* Store our data. */
	frontend_data = g_new (frontend_data_t, 1);
	frontend_data->id = id;
	frontend_data->name = g_strdup (name);
	frontend_data->xml_properties = NULL;
	frontend_data->item = NULL;
	camera->frontend_data = frontend_data;

	return (camera);
}






