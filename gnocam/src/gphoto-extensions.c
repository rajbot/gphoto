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

/**************/
/* Prototypes */
/**************/

gboolean description_extract (gchar* description, guint* id, gchar** name, gchar** model, gchar** port, guint* speed);

/*************/
/* Functions */
/*************/

gboolean
description_extract (gchar* description, guint* id, gchar** name, gchar** model, gchar** port, guint* speed)
{
	guint 	count = 0;
	guint 	j;
	gchar* 	description_copy;

	g_return_val_if_fail (description != NULL, FALSE);

        /* Check if description is valid.                 	*/
        /* Example for description:                             */
        /* "1\nMy Camera\nHP PhotoSmart C30\nSerial Port 0\n".  */
	description_copy = g_strdup (description);
        for (j = 0; description_copy[j] != '\0'; j++) {
                if (description_copy[j] == '\n') {
                        description_copy[j] = '\0';
                        count++;
                }
        }
        g_assert (count == 4);

        *id = (guint) atoi (description_copy);
        for (j = 0; description_copy[j] != '\0'; j++);
        *name = g_strdup (&description_copy[++j]);
        for (; description_copy[j] != '\0'; j++);
        *model = g_strdup (&description_copy[++j]);
        for (; description_copy[j] != '\0'; j++);
        *port = g_strdup (&description_copy[++j]);
        for (; description_copy[j] != '\0'; j++);
        *speed = (guint) atoi (&description_copy[++j]);

	g_free (description_copy);

	return (TRUE);
}

/**
 * gp_widget_clone:
 * @widget: The widget to clone.
 * 
 * Returns a clone of the widget (including childs).
 **/
CameraWidget*
gp_widget_clone (CameraWidget* widget)
{
        CameraWidget*   widget_new;
        CameraWidget*   widget_child;
        CameraWidget*   widget_child_new = NULL;
        gfloat          low, high, increment;
        gint            i, j;
        gchar*          value;

        g_assert (widget != NULL);
        g_assert ((widget_new = gp_widget_new (gp_widget_type (widget), g_strdup (gp_widget_label (widget)))) != NULL);

        for (i = 0; i < gp_widget_child_count (widget); i++) {
                widget_child = gp_widget_child (widget, i);
                switch (gp_widget_type (widget_child)) {
                case GP_WIDGET_SECTION:
                        widget_child_new = gp_widget_clone (widget_child);
                        gp_widget_append (widget_new, widget_child_new);
                        break;
                case GP_WIDGET_TEXT:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
                        value = gp_widget_value_get (widget_child);
                        gp_widget_value_set (widget_child_new, value);
                        break;
                case GP_WIDGET_RANGE:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
                        gp_widget_range_get (widget_child, &low, &high, &increment);
                        gp_widget_range_set (widget_child_new, low, high, increment);
			value = gp_widget_value_get (widget_child);
                        gp_widget_value_set (widget_child_new, value);
                        break;
                case GP_WIDGET_TOGGLE:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
			value = gp_widget_value_get (widget_child);
                        gp_widget_value_set (widget_child_new, value);
                        break;
                case GP_WIDGET_RADIO:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
			for (j = 0; j < gp_widget_choice_count (widget_child); j++)
			{
				gp_widget_choice_add (widget_child_new, g_strdup (gp_widget_choice (widget_child, j)));
			}
			value = gp_widget_value_get (widget_child);
                        gp_widget_value_set (widget_child_new, value);
                        break;
                case GP_WIDGET_MENU:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
			for (j = 0; j < gp_widget_choice_count (widget_child); j++)
                        {
                                gp_widget_choice_add (widget_child_new, g_strdup (gp_widget_choice (widget_child, j)));
                        }
			value = gp_widget_value_get (widget_child);
                        gp_widget_value_set (widget_child_new, value);
                        break;
                case GP_WIDGET_BUTTON:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
			//FIXME
			g_warning ("Not yet implemented!");
			gp_widget_callback_set (widget_child_new, gp_widget_callback (widget_child));
                        break;
                case GP_WIDGET_DATE:
			widget_child_new = gp_widget_new (gp_widget_type (widget_child), g_strdup (gp_widget_label (widget_child)));
			gp_widget_append (widget_new, widget_child_new);
                        value = gp_widget_value_get (widget_child);
                        gp_widget_value_set (widget_child_new, value);
                        break;
                default:
                        break;
                }
        }
        return (widget_new);
}

/**
 * gp_camera_update_by_description:
 * @xml: xml of main window.
 * @@camera: The camera to update.
 * @description: The description of the camera (like "id\nname\nmodel\nport\nspeed").
 *
 * Returns TRUE if update has been successful, FALSE otherwise.
 **/
gboolean
gp_camera_update_by_description (Camera** camera, gchar* description)
{
	Camera*		camera_old;
	guint 		id;
	gchar*		name;
	gchar*		model;
	gchar* 		port;
	guint		speed;
	gboolean 	port_changed;
	gboolean	speed_changed;
	gint		i;
	gint		number_of_ports;
	CameraPortInfo		port_info;
	frontend_data_t*	frontend_data;

	g_assert (camera != NULL);
	g_assert (*camera != NULL);
	g_assert (description != NULL);

        /* Extract information from description. */
        description_extract (description, &id, &name, &model, &port, &speed);

	frontend_data = (frontend_data_t*) (*camera)->frontend_data;
	g_assert (id == frontend_data->id);

	/* Did the model change? */
	if (strcmp ((*camera)->model, model) != 0) {
		camera_old = *camera;
		if (gp_camera_new_by_name (&(*camera), model) == GP_ERROR) {
			dialog_information (_("Could not set camera model to '%s'!"), model);

			/* Clean up. */
			g_free (frontend_data->name);
			g_free (frontend_data);
			g_free (name);
			g_free (model);
			g_free (port);
			*camera = camera_old;

			return (FALSE);
		}
		(*camera)->frontend_data = frontend_data;
		gp_camera_free (camera_old);
	}
	g_free (model);

        /* Did name change? */
        if (strcmp (frontend_data->name, name) != 0) {
		g_free (frontend_data->name);
		frontend_data->name = name;
        } else {
		g_free (name);
	}
        
        /* Did the port change? */
	port_changed = FALSE;
        if (strcmp ((*camera)->port->name, port) != 0) {
	        if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
			dialog_information (_("Could not get number of ports!"));

			/* Clean up. */
			g_free (frontend_data->name);
			g_free (frontend_data);
	                g_free (port);

	                return (FALSE);
	        }
	        for (i = 0; i < number_of_ports; i++) {
	                if (gp_port_info (i, &port_info) == GP_ERROR) {
	                        dialog_information (_("Could not get port information for port number %i!"), i);

				/* Clean up. */
				g_free (frontend_data->name);
				g_free (frontend_data);
	                        g_free (port);

	                        return (FALSE);
	                }
	                if (strcmp (port_info.name, port) == 0) break;
	        }
	        if (i == number_of_ports) {
			dialog_information (_("Could not find port '%s'!"), port);

			/* Clean up. */
			g_free (frontend_data->name);
			g_free (frontend_data);
	                g_free (port);

	                return (FALSE);
	        }
                port_changed = TRUE;
        }
	g_free (port);

        /* Did the speed change? */
	speed_changed = FALSE;
        if ((*camera)->port->speed != (gint) speed) {
                if (port_changed) {
			port_info.speed = (gint) speed;
		} else {
			(*camera)->port->speed = (gint) speed;
			speed_changed = TRUE;
		}
        }

	/* If port or speed changed, we initialize the camera with the new settings. */	
	if (port_changed || speed_changed) {
	        if (gp_camera_init (*camera, &port_info) == GP_ERROR) {
	                dialog_information (_("Could not initialize camera on port '%s' with speed %i!"), (*camera)->port->name, (*camera)->port->speed);
			
			/* Clean up. */
			g_free (frontend_data->name);
			g_free (frontend_data);
			
			return (FALSE);
		}
	}

	return (TRUE);
}

Camera*
gp_camera_new_by_description (gchar* description)
{
	GnomeApp*	app;
	gint 		number_of_ports, i;
	CameraPortInfo 	port_info;
	guint		id;
	gchar*		name;
	gchar*		model;
	gchar*		port;
	guint		speed;
	Camera*		camera;
	frontend_data_t*	frontend_data;

	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);
	g_assert (description != NULL);

	/* Extract information from description. */
	g_return_val_if_fail (description_extract (description, &id, &name, &model, &port, &speed), NULL);
	
	/* Check port. */
	if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
		gnome_app_error (app, _("Could not get number of ports!"));
		g_free (name);
		g_free (model);
		g_free (port);
		return (NULL);
	}
	for (i = 0; i < number_of_ports; i++) {
		if (gp_port_info (i, &port_info) == GP_ERROR) {
			gnome_app_error (app, _("Could not get port information!"));
			g_free (name);
			g_free (model);
			g_free (port);
			return (NULL);
		}
		if (strcmp (port_info.name, port) == 0) break;
	}
	if (i == number_of_ports) {
		gnome_app_error (app, _("Could not find port!"));
		g_free (name);
		g_free (model);
		g_free (port);
		return (NULL);
	}

	/* Nothing to check for what concerns speed. */
	port_info.speed = (gint) speed;

	/* Check model. */
	if (gp_camera_new_by_name (&camera, model) == GP_ERROR) {
		gnome_app_error (app, _("Could not set camera model!"));
		g_free (name);
		g_free (model);
		g_free (port);
		return (NULL);
	}
	g_free (model);
	
	/* Initialize the camera. */
	if (gp_camera_init (camera, &port_info) == GP_ERROR) {
		gnome_app_error (app, _("Could not initialize camera!"));
		g_free (name);
		g_free (port);
//FIXME: Why crashes the whole thing when initialization failed and I free the camera?
//		gp_camera_free (camera);
		return (NULL);
	}
	g_free (port);

	/* Store our data. */
	frontend_data = g_new (frontend_data_t, 1);
	frontend_data->id = id;
	frontend_data->name = name;
	frontend_data->ref_count = 0;
	frontend_data->xml = xml;
	frontend_data->xml_properties = NULL;
	frontend_data->xml_preview = NULL;
	camera->frontend_data = frontend_data;

	return (camera);
}

void
gp_camera_ref (Camera* camera)
{
	frontend_data_t*	frontend_data;

	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	frontend_data->ref_count++;
}

void
gp_camera_unref (Camera* camera)
{
	frontend_data_t*        frontend_data;

        g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);
        g_assert (frontend_data->ref_count > 0);

        frontend_data->ref_count--;
	if (frontend_data->ref_count == 0) {
		g_free (frontend_data->name);
		gp_camera_free (camera);
	}
}

