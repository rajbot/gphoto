#include <config.h>
#include <gnome.h>
#include <gphoto2.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include <parser.h>
#include <tree.h>
#include <bonobo.h>
#include "gnocam.h"
#include "utils.h"
#include "cameras.h"

//FIXME: UserData is butt-ugly. And doesn't get freed on exit.

/**********************/
/* External variables */
/**********************/

extern GtkWindow*	main_window;

/********************/
/* Type definitions */
/********************/

typedef struct {
	CameraWidget*	window;
	CameraWidget*	widget;
	gint		i;
} UserData;

/**************/
/* Prototypes */
/**************/

void on_toggle_activated	(BonoboUIComponent* component, const gchar* path, Bonobo_UIComponent_EventType type, const gchar* state, gpointer user_data);

void on_duration_button_ok_clicked      (GtkButton* button, gpointer user_data);
void on_duration_button_cancel_clicked  (GtkButton* button, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void
on_toggle_activated (BonoboUIComponent* component, const gchar* label, Bonobo_UIComponent_EventType type, const gchar* state, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	widget;
	gint		value_int;
	gchar*		value_char;
	gchar*		value_char_current;
	
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (component), "camera"));
	g_return_if_fail (user_data);

	/* Did the _user_ toggle? */
	if (!gtk_object_get_data (GTK_OBJECT (component), "done")) return;
	
	widget = ((UserData*) user_data)->widget;

	switch (gp_widget_type (widget)) {
	case GP_WIDGET_TOGGLE:
		value_int = atoi (state);
		gp_widget_value_set (widget, &value_int);
		gp_camera_config_set (camera, ((UserData*) user_data)->window);
		break;
	case GP_WIDGET_RADIO:
		value_char = gp_widget_choice (widget, ((UserData*) user_data)->i);
		gp_widget_value_get (widget, &value_char_current);
		if (strcmp (value_char, value_char_current) != 0) {
			gp_widget_value_set (widget, value_char);
			gp_camera_config_set (camera, ((UserData*) user_data)->window);
		}
		break;
	default:
		g_assert_not_reached ();
	}
}

void
on_duration_button_ok_clicked (GtkButton* button, gpointer user_data)
{
        GtkAdjustment*          adjustment;
        CameraCaptureInfo       info;
        CameraFile*             file;
	Camera*			camera;

        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (button), "camera")));
        g_assert ((adjustment = gtk_range_get_adjustment (GTK_RANGE (gtk_object_get_data (GTK_OBJECT (button), "hscale")))));

        /* Prepare the video. */
        info.type = GP_CAPTURE_VIDEO;
        info.duration = adjustment->value;
        file = gp_file_new ();

        /* Capture. */
        gp_camera_capture (camera, file, &info);

        /* Clean up. */
        gtk_widget_destroy (gtk_object_get_data (GTK_OBJECT (button), "messagebox"));
        gp_file_free (file);
	gp_camera_unref (camera);
}

void
on_duration_button_cancel_clicked (GtkButton* button, gpointer user_data)
{
	Camera*	camera;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (button), "camera")));

	/* Clean up. */
	gtk_widget_destroy (gtk_object_get_data (GTK_OBJECT (button), "messagebox"));
	gp_camera_unref (camera);
}

/*************/
/* Functions */
/*************/

void
capture_image (Camera* camera)
{
        CameraCaptureInfo       info;
        CameraFile*             file;

        g_assert (camera);

        /* Prepare the image. */
        info.type = GP_CAPTURE_IMAGE;
        info.duration = 0;
        file = gp_file_new ();

        /* Capture. */
        gp_camera_capture (camera, file, &info);

        /* Clean up. */
        gp_file_free (file);
}

void
capture_video (Camera* camera)
{
        GladeXML*               xml_duration;
        GladeXML*               xml_hscale;
        GnomeMessageBox*        messagebox;

        /* Ask for duration. */
        g_assert ((xml_duration = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "duration_messagebox")));
        glade_xml_signal_autoconnect (xml_duration);

        //FIXME: As soon libglade is fixed, delete this code.
        /* This is annoying: The hscale doesn't get displayed... We have to create it manually. */
        g_assert ((xml_hscale = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "duration_hscale")));
        g_assert ((messagebox = GNOME_MESSAGE_BOX (glade_xml_get_widget (xml_duration, "duration_messagebox"))));
        gtk_container_add (GTK_CONTAINER ((GNOME_DIALOG (messagebox))->vbox), glade_xml_get_widget (xml_hscale, "duration_hscale"));

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "messagebox", messagebox);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "hscale",  glade_xml_get_widget (xml_hscale, "duration_hscale"));        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_cancel")), "camera", camera);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_cancel")), "messagebox", messagebox);

	/* Ref the camera. */
	gp_camera_ref (camera);
}

void
properties (Camera* camera)
{
        frontend_data_t*        frontend_data;
	gchar*			message;
	gint			result;

        g_return_if_fail (camera);
        g_return_if_fail (frontend_data = (frontend_data_t*) camera->frontend_data);

        if (!(frontend_data->xml_properties)) {

                /* Reference the camera. */
                gp_camera_ref (camera);

                /* Get the camera properties from the backend. */
                if ((result = gp_camera_config (camera)) != GP_OK) {
			message = g_strdup_printf (
				_("Could not get camera properties of camera '%s'!\n(%s)"), 
				frontend_data->name, 
				gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
                }
        } else {
		message = g_strdup_printf (_("The camera properties dialog for camera '%s' is already open."), frontend_data->name);
		gnome_ok_dialog_parented (message, main_window);
		g_free (message);
	}
}

void gp_widget_to_xml (BonoboUIComponent* component, CameraWidget* window, CameraWidget* widget, xmlNodePtr popup, xmlNodePtr command, xmlNsPtr ns)
{
	CameraWidget*		child;
	gint 			i, j;
	xmlNodePtr		menu, node, item;
	gchar*			id;
	gchar*			tmp;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		id = g_strdup_printf ("%i", gp_widget_id (child));
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			xmlAddChild (popup, node = xmlNewNode (ns, "submenu"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_label", gp_widget_label (child));
			gp_widget_to_xml (component, window, child, node, command, ns);
			break;
		case GP_WIDGET_RADIO:
			xmlAddChild (popup, menu = xmlNewNode (ns, "submenu"));
			xmlSetProp (menu, "name", id);
			xmlSetProp (menu, "_label", gp_widget_label (child));
			for (j = 0; j < gp_widget_choice_count (child); j++) {
				tmp = g_strdup_printf ("%s.%i", id, j);
				xmlAddChild (menu, item = xmlNewNode (ns, "menuitem"));
				xmlSetProp (item, "name", tmp);
				xmlSetProp (item, "verb", tmp);
				xmlAddChild (command, node = xmlNewNode (ns, "cmd"));
				xmlSetProp (node, "name", tmp);
				xmlSetProp (node, "_label", gp_widget_choice (child, j));
				xmlSetProp (node, "type", "radio");
				xmlSetProp (node, "group", id);
				g_free (tmp);
			}
			break;
		case GP_WIDGET_TEXT:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_RANGE:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_TOGGLE:
			xmlAddChild (popup, node = xmlNewNode (ns, "menuitem"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "verb", id);
			xmlAddChild (command, node = xmlNewNode (ns, "cmd"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "type", "toggle");
			break;
		case GP_WIDGET_MENU:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_BUTTON:
			node = xmlNewNode (ns, "menuitem");
			xmlAddChild (popup, node);
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "verb", id);
			break;
		case GP_WIDGET_DATE:
			g_warning ("Not implemented!");
			break;
		default:
			g_warning ("Not yet implemented!");
			break;
		}
		g_free (id);
	}
}

void ui_set_values_from_widget (BonoboUIComponent* component, CameraWidget* window, CameraWidget* widget)
{
	CameraWidget*	child;
	gint		i, j;
	gchar*		tmp;
	gchar*		value;
	UserData*	user_data;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_BUTTON:
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			ui_set_values_from_widget (component, window, child);
			break;
		case GP_WIDGET_TOGGLE:
			gp_widget_value_get (child, &j);
			tmp = g_strdup_printf ("/commands/%i", gp_widget_id (child));
			value = g_strdup_printf ("%i", j);
			bonobo_ui_component_set_prop (component, tmp, "state", value, NULL);
			g_free (value);
			g_free (tmp);
			user_data = g_new (UserData, 1);
                        user_data->window = window,
                        user_data->widget = child;
			tmp = g_strdup_printf ("%i", gp_widget_id (child));
                        bonobo_ui_component_add_listener (component, tmp, on_toggle_activated, user_data);
			g_free (tmp);
			break;
                case GP_WIDGET_TEXT:
			g_warning ("Not implemented!");
                        break;
                case GP_WIDGET_RANGE:
			g_warning ("Not implemented!");
                        break;
                case GP_WIDGET_RADIO:
			gp_widget_value_get (child, &value);
			for (j = 0; j < gp_widget_choice_count (child); j++) {
				if (value && (strcmp (gp_widget_choice (child, j), value) == 0)) {
					tmp = g_strdup_printf ("/commands/%i.%i", gp_widget_id (child), j);
					bonobo_ui_component_set_prop (component, tmp, "state", "1", NULL);
					g_free (tmp);
				}
				user_data = g_new (UserData, 1);
        	                user_data->window = window;
                	        user_data->widget = child;
                        	user_data->i = j;
				tmp = g_strdup_printf ("%i.%i", gp_widget_id (child), j);
	                	bonobo_ui_component_add_listener (component, tmp, on_toggle_activated, user_data);
				g_free (tmp);
			}
                        break;
                case GP_WIDGET_MENU:
			g_warning ("Not implemented!");
                        break;
		case GP_WIDGET_DATE:
			g_warning ("Not implemented!");
			break;
		default:
			g_warning ("Not implemented!");
			break;
		}
	}
}


