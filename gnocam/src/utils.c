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
extern gint		counter;

/**************/
/* Prototypes */
/**************/

void on_entry_changed			(GtkObject* object, gpointer user_data);
void on_radio_button_activate 		(GtkObject* object, gpointer user_data);
void on_adjustment_value_changed 	(GtkObject* object, gpointer user_data);
void on_togglebutton_toggled 		(GtkObject* object, gpointer user_data);
void on_date_changed 			(GtkObject* object, gpointer user_data);

void on_duration_button_ok_clicked      (GtkButton* button, gpointer user_data);
void on_duration_button_cancel_clicked  (GtkButton* button, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void
on_entry_changed (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	GnomeVFSURI*	uri;
	gchar*		folder;
	gchar*		file;
	gchar*		value_string;
	gchar*		value_string_new;
	gchar*		tmp;
	gint		result = GP_OK;

	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &value_string);
	value_string_new = gtk_entry_get_text (GTK_ENTRY (object));
	if (!value_string || (value_string && (strcmp (value_string, value_string_new) != 0))) {
		g_return_if_fail (gp_widget_value_set (widget, value_string_new) == GP_OK);
		if (gtk_object_get_data (object, "for_camera")) result = gp_camera_config_set (camera, window);
		else {
			g_return_if_fail (uri = gtk_object_get_data (object, "uri"));
			file = (gchar*) gnome_vfs_uri_get_basename (uri);
			folder = gnome_vfs_uri_extract_dirname (uri);
			if (file) result = gp_camera_file_config_set (camera, window, folder, file);
			else if (folder) result = gp_camera_folder_config_set (camera, window, folder);
			else g_assert_not_reached ();
			g_free (folder);
		}
		if (result != GP_OK) {
			tmp = g_strdup_printf ("Could not set configuration!\n(%s)", gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}
	}
}

void
on_radio_button_activate (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	GnomeVFSURI*	uri;
	gchar*		folder;
	gchar*		file;
	gchar*		value_string;
	gchar*		value_string_new;
	gchar*		tmp;
	gint		result = GP_OK;

	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &value_string);
	value_string_new = gp_widget_choice (widget, GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (object), "choice")));
	if (!value_string || (value_string && (strcmp (value_string_new, value_string) != 0))) {
		g_return_if_fail (gp_widget_value_set (widget, value_string_new) == GP_OK);
		if (gtk_object_get_data (object, "for_camera")) result = gp_camera_config_set (camera, window);
		else {
			g_return_if_fail (uri = gtk_object_get_data (object, "uri"));
			file = (gchar*) gnome_vfs_uri_get_basename (uri);
			folder = gnome_vfs_uri_extract_dirname (uri);
			if (file) result = gp_camera_file_config_set (camera, window, folder, file);
			else if (folder) result = gp_camera_folder_config_set (camera, window, folder);
			else g_assert_not_reached ();
			g_free (folder);
		}
		if (result != GP_OK) {
			tmp = g_strdup_printf ("Could not set configuration!\n(%s)", gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}
	}
}

void
on_adjustment_value_changed (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gfloat		f, f_new;
	gchar*		folder;
	gchar*		file;
	gchar*		tmp;
	GnomeVFSURI* 	uri;
	gint		result = GP_OK;

	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &f);
	f_new = GTK_ADJUSTMENT (object)->value;
	if (f != f_new) {
		g_return_if_fail (gp_widget_value_set (widget, &f_new) == GP_OK);
		if (gtk_object_get_data (object, "for_camera")) result = gp_camera_config_set (camera, window);
		else {
			g_return_if_fail (uri = gtk_object_get_data (object, "uri"));
			file = (gchar*) gnome_vfs_uri_get_basename (uri);
			folder = gnome_vfs_uri_extract_dirname (uri);
			if (file) result = gp_camera_file_config_set (camera, window, folder, file);
			else if (folder) result = gp_camera_folder_config_set (camera, window, folder);
			else g_assert_not_reached ();
			g_free (folder);
		}
		if (result != GP_OK) {
			tmp = g_strdup_printf ("Could not set camera configuration!\n(%s)", gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}
	}
}

void
on_togglebutton_toggled (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gint		i, i_new = 0;
	gchar*		folder;
	gchar*		file;
	gchar*		tmp;
	GnomeVFSURI*	uri;
	gint		result = GP_OK;
	
	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &i);
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (object))) i_new = 1;
	if (i != i_new) {
		g_return_if_fail (gp_widget_value_set (widget, &i_new) == GP_OK);
		if (gtk_object_get_data (object, "for_camera")) result = gp_camera_config_set (camera, window);
		else {
			g_return_if_fail (uri = gtk_object_get_data (object, "uri"));
			file = (gchar*) gnome_vfs_uri_get_basename (uri);
			folder = gnome_vfs_uri_extract_dirname (uri);
			if (file) result = gp_camera_file_config_set (camera, window, folder, file);
			else if (folder) result = gp_camera_folder_config_set (camera, window, folder);
			else g_assert_not_reached ();
			g_free (folder);
		}
		if (result != GP_OK) {
			tmp = g_strdup_printf ("Could not set configuration!\n(%s)", gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}
	}
}

void
on_date_changed (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	widget;
	CameraWidget*	window;
	gint		i, i_new;
	gchar*		folder;
	gchar* 		file;
	gchar*		tmp;
	GnomeVFSURI*	uri;
	gint		result = GP_OK;
	
	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &i);
	i_new = (int) gnome_date_edit_get_date (GNOME_DATE_EDIT (object));
	if (i != i_new) {
		g_return_if_fail (gp_widget_value_set (widget, &i_new) == GP_OK);
		if (gtk_object_get_data (object, "for_camera")) result = gp_camera_config_set (camera, window);
		else {
			g_return_if_fail (uri = gtk_object_get_data (object, "uri"));
			file = (gchar*) gnome_vfs_uri_get_basename (uri);
			folder = gnome_vfs_uri_extract_dirname (uri);
			if (file) result = gp_camera_file_config_set (camera, window, folder, file);
			else if (folder) result = gp_camera_folder_config_set (camera, window, folder);
			else g_assert_not_reached ();
			g_free (folder);
		}
		if (result != GP_OK) {
			tmp = g_strdup_printf ("Could not set configuration!\n(%s)", gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}
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
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_ok")), "hscale",  glade_xml_get_widget (xml_hscale, "duration_hscale"));
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_cancel")), "camera", camera);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_duration, "duration_button_cancel")), "messagebox", messagebox);

	/* Ref the camera. */
	gp_camera_ref (camera);
}

void 
popup_prepare (BonoboUIComponent* component, CameraWidget* widget, xmlNodePtr popup, xmlNodePtr command, xmlNsPtr ns)
{
	CameraWidget*		child;
	gint 			i;
	xmlNodePtr		node;
	gchar*			id;

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
			xmlSetProp (node, "_tip", gp_widget_label (child));
			popup_prepare (component, child, node, command, ns);
			break;
		case GP_WIDGET_TEXT:
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO:
		case GP_WIDGET_DATE:
		case GP_WIDGET_TOGGLE:
		case GP_WIDGET_RANGE:
			xmlAddChild (popup, node = xmlNewNode (ns, "control"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_tip", gp_widget_label (child));
			break;
		case GP_WIDGET_BUTTON:
			xmlAddChild (popup, node = xmlNewNode (ns, "menuitem"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "_tip", gp_widget_label (child));
			g_warning ("GP_WIDGET_BUTTON is not yet implemented!");
			break;
		default:
			g_warning ("Encountered unsupported widget!");
			break;
		}
		g_free (id);
	}
}

void 
popup_fill (BonoboUIComponent* component, gchar* path, CameraWidget* window, CameraWidget* widget, gboolean for_camera)
{
	GtkWidget*		hbox;
	GtkWidget*		gtkwidget;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	GtkObject*		adjustment;
	Camera*			camera;
	CameraWidget*		child;
	gint			i, j;
	gchar*			tmp;
	gchar*			value_string;
	gfloat			max, min, increment, value_float;
	gint			value_int;
	GnomeVFSURI*		uri;

	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (component), "camera"));
	
	uri = gtk_object_get_data (GTK_OBJECT (component), "uri");
	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			popup_fill (component, tmp, window, child, for_camera);
			g_free (tmp);
			break;
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO:
			gp_widget_value_get (child, &value_string);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			gtk_widget_show (gtkwidget = gtk_option_menu_new ());
			gtk_widget_show (menu = gtk_menu_new ());
			gtk_option_menu_set_menu (GTK_OPTION_MENU (gtkwidget), menu);
			for (j = 0; j < gp_widget_choice_count (child); j++) {
				gtk_widget_show (menu_item = gtk_menu_item_new_with_label (gp_widget_choice (child, j)));
				gtk_menu_append (GTK_MENU (menu), menu_item);
				gtk_object_set_data (GTK_OBJECT (menu_item), "uri", uri);
				gtk_object_set_data (GTK_OBJECT (menu_item), "camera", camera);
				gtk_object_set_data (GTK_OBJECT (menu_item), "window", window);
				gtk_object_set_data (GTK_OBJECT (menu_item), "widget", child);
				gtk_object_set_data (GTK_OBJECT (menu_item), "choice", GINT_TO_POINTER (j));
				if (for_camera) gtk_object_set_data (GTK_OBJECT (menu_item), "for_camera", GINT_TO_POINTER (1));
				if (value_string && (strcmp (value_string, gp_widget_choice (child, j)) == 0)) 
					gtk_option_menu_set_history (GTK_OPTION_MENU (gtkwidget), j);
				gtk_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (on_radio_button_activate), NULL);
			}
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_TOGGLE:
			gp_widget_value_get (child, &value_int);
			gtk_widget_show (gtkwidget = gtk_check_button_new_with_label (gp_widget_label (child)));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtkwidget), (value_int != 0));
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "uri", uri);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", camera);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "window", window);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "widget", child);
			if (for_camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "for_camera", GINT_TO_POINTER (1));
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "toggled", GTK_SIGNAL_FUNC (on_togglebutton_toggled), NULL);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (gtkwidget))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_RANGE:
			gp_widget_value_get (child, &value_float);
			gp_widget_range_get (child, &min, &max, &increment);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			adjustment = gtk_adjustment_new (value_float, min, max, increment, 0, 0);
			gtk_object_set_data (adjustment, "uri", uri);
			gtk_object_set_data (adjustment, "camera", camera);
			gtk_object_set_data (adjustment, "window", window);
			gtk_object_set_data (adjustment, "widget", child);
			if (for_camera) gtk_object_set_data (GTK_OBJECT (adjustment), "for_camera", GINT_TO_POINTER (1));
			gtk_signal_connect (adjustment, "value_changed", GTK_SIGNAL_FUNC (on_adjustment_value_changed), NULL);
			gtk_widget_show (gtkwidget = gtk_hscale_new (GTK_ADJUSTMENT (adjustment)));
			gtk_range_set_update_policy (GTK_RANGE (gtkwidget), GTK_UPDATE_DISCONTINUOUS);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_DATE:
			gp_widget_value_get (child, &value_int);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			gtk_widget_show (gtkwidget = gnome_date_edit_new ((time_t) value_int, TRUE, TRUE));
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "uri", uri);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", camera);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "window", window);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "widget", child);
			if (for_camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "for_camera", GINT_TO_POINTER (1));
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "date_changed", GTK_SIGNAL_FUNC (on_date_changed), NULL);
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "time_changed", GTK_SIGNAL_FUNC (on_date_changed), NULL);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_TEXT:
			gp_widget_value_get (child, &value_string);
			gtk_widget_show (hbox = gtk_hbox_new (FALSE, 5));
			gtk_widget_show (gtkwidget = gtk_label_new (gp_widget_label (child)));
			gtk_box_pack_start (GTK_BOX (hbox), gtkwidget, FALSE, FALSE, 0);
			gtk_widget_show (gtkwidget = gtk_entry_new ());
			if (value_string) gtk_entry_set_text (GTK_ENTRY (gtkwidget), value_string);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "uri", uri);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", camera);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "window", window);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "widget", child);
			if (for_camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "for_camera", GINT_TO_POINTER (1));
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "changed", GTK_SIGNAL_FUNC (on_entry_changed), NULL);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		case GP_WIDGET_BUTTON:
			g_warning ("GP_WIDGET_BUTTON not implemented!");
			break;
		default:
			g_warning ("Encountered unsupported widget!");
			break;
		}
	}
}



