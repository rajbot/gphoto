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

/**********************/
/* External variables */
/**********************/

extern GtkWindow*	main_window;

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
on_toggle_activated (BonoboUIComponent* component, const gchar* label, Bonobo_UIComponent_EventType type, const gchar* state, gpointer window)
{
	Camera*		camera;
	CameraWidget*	widget;
	gint		value;
	
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (component), "camera"));
	g_return_if_fail (window);
	
	widget = gp_widget_child_by_label (window, (gchar*) label);
	value = atoi (state);
	gp_widget_value_set (widget, &value);
	gp_camera_config_set (camera, window);
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
	gint 			i;
	xmlNodePtr		node;
	gchar*			label;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			node = xmlNewNode (ns, "submenu");
			xmlAddChild (popup, node);
			gp_widget_to_xml (component, window, child, node, command, ns);
			break;
		case GP_WIDGET_TOGGLE:
			label = gp_widget_label (child);
			node = xmlNewNode (ns, "menuitem");
			xmlAddChild (popup, node);
			xmlSetProp (node, "name", label);
			xmlSetProp (node, "verb", label);
			xmlSetProp (node, "accel", "");
			bonobo_ui_component_add_listener (component, label, on_toggle_activated, window);
			xmlAddChild (command, node = xmlNewNode (ns, "cmd"));
			xmlSetProp (node, "name", label);
			xmlSetProp (node, "type", "toggle");
			xmlSetProp (node, "_label", label);
			break;
		case GP_WIDGET_TEXT:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_RANGE:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_RADIO:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_MENU:
			g_warning ("Not implemented!");
			break;
		case GP_WIDGET_BUTTON:
			label = gp_widget_label (child);
			node = xmlNewNode (ns, "menuitem");
			xmlAddChild (popup, node);
			xmlSetProp (node, "name", label);
			xmlSetProp (node, "verb", label);
			break;
		case GP_WIDGET_DATE:
			g_warning ("Not implemented!");
			break;
		default:
			g_warning ("Not yet implemented!");
			break;
		}
	}
}

void ui_set_values_from_widget (BonoboUIComponent* component, CameraWidget* widget)
{
	CameraWidget*	child;
	gint		i, j;
	gchar*		tmp;
	gchar*		value;

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			ui_set_values_from_widget (component, child);
			break;
		case GP_WIDGET_TOGGLE:
			gp_widget_value_get (child, &j);
			tmp = g_strdup_printf ("/commands/%s", gp_widget_label (child));
			value = g_strdup_printf ("%i", j);
			bonobo_ui_component_set_prop (component, tmp, "state", value, NULL);
			g_free (value);
			g_free (tmp);
			break;
                case GP_WIDGET_TEXT:
			g_warning ("Not implemented!");
                        break;
                case GP_WIDGET_RANGE:
			g_warning ("Not implemented!");
                        break;
                case GP_WIDGET_RADIO:
			g_warning ("Not implemented!");
                        break;
                case GP_WIDGET_MENU:
			g_warning ("Not implemented!");
                        break;
                case GP_WIDGET_BUTTON:
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


