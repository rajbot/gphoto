#include <gnome.h>
#include <gphoto2.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include "gnocam.h"
#include "gphoto-extensions.h"
#include "information.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;

/*************/
/* Functions */
/*************/

Camera*
gp_camera_new_by_description (gint id, gchar* name, gchar* model, gchar* port, gint speed)
{
	gint 			number_of_ports, i;
	CameraPortInfo 		port_info;
	Camera*			camera;
	frontend_data_t*	frontend_data;

	g_assert (name != NULL);
	g_assert (model != NULL);
	g_assert (port != NULL);

	/* "Directory Browse needs special care. */
	if (strcmp ("Directory Browse", model) != 0) {
		/* Check port. */
		if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
			dialog_information (_("Could not get number of ports!"));
			return (NULL);
		}
		for (i = 0; i < number_of_ports; i++) {
			if (gp_port_info (i, &port_info) == GP_ERROR) {
				dialog_information (_("Could not get port information!"));
				return (NULL);
			}
			if (strcmp (port_info.name, port) == 0) break;
		}
		if (i == number_of_ports) {
			dialog_information (_("Could not find port!"));
			return (NULL);
		}
	
		/* Nothing to check for what concerns speed. */
		port_info.speed = speed;
	} else {
		strcpy (port_info.path, "");
	}

	/* Check model. */
	if (gp_camera_new_by_name (&camera, model) == GP_ERROR) {
		dialog_information (_("Could not set camera model!"));
		return (NULL);
	}
	
	/* Initialize the camera. */
	if (gp_camera_init (camera, &port_info) == GP_ERROR) {
		dialog_information (_("Could not initialize camera!"));
//FIXME: Why crashes the whole thing when initialization failed and I free the camera?
//		gp_camera_free (camera);
		return (NULL);
	}

	/* Store our data. */
	frontend_data = g_new (frontend_data_t, 1);
	frontend_data->id = id;
	frontend_data->name = g_strdup (name);
	frontend_data->xml = xml;
	frontend_data->xml_properties = NULL;
	frontend_data->xml_preview = NULL;
	camera->frontend_data = frontend_data;

	return (camera);
}



