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
extern gint		counter;

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

void on_radio_button_activate (GtkWidget* widget, gpointer user_data);

void on_adjustment_value_changed (GtkAdjustment* adjustment, gpointer user_data);

void on_togglebutton_toggled (GtkToggleButton* button, gpointer user_data);

void on_date_changed (GnomeDateEdit* gnomedateedit, gpointer user_data);

void on_duration_button_ok_clicked      (GtkButton* button, gpointer user_data);
void on_duration_button_cancel_clicked  (GtkButton* button, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void
on_radio_button_activate (GtkWidget* widget, gpointer user_data)
{
	g_warning ("Not yet implemented!\n");
}

void
on_adjustment_value_changed (GtkAdjustment* adjustment, gpointer user_data)
{
	GtkTreeItem*	item;
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gfloat		f, f_new;
	gchar*		folder;
	gchar*		file;
	GnomeVFSURI* 	uri;
	
	g_return_if_fail (adjustment);
	g_return_if_fail (item = gtk_object_get_data (GTK_OBJECT (adjustment), "item"));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
	g_return_if_fail (window = gtk_object_get_data (GTK_OBJECT (adjustment), "window"));
	g_return_if_fail (widget = gtk_object_get_data (GTK_OBJECT (adjustment), "widget"));
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));

	file = (gchar*) gnome_vfs_uri_get_basename (uri);
	folder = gnome_vfs_uri_extract_dirname (uri);
	
	gp_widget_value_get (widget, &f);
	f_new = adjustment->value;
	if (f != f_new) {
		gp_widget_value_set (widget, &f_new);
		if (gtk_object_get_data (GTK_OBJECT (adjustment), "camera")) gp_camera_config_set (camera, window);
		if (file) gp_camera_file_config_set (camera, window, folder, file);
		else if (folder) gp_camera_folder_config_set (camera, window, folder);
		else g_assert_not_reached ();
	}
}

void
on_togglebutton_toggled (GtkToggleButton* button, gpointer user_data)
{
	GtkTreeItem*	item;
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gint		i, i_new = 0;
	gchar*		folder;
	gchar*		file;
	GnomeVFSURI*	uri;
	
	g_return_if_fail (button);
	g_return_if_fail (item = gtk_object_get_data (GTK_OBJECT (button), "item"));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
	g_return_if_fail (window = gtk_object_get_data (GTK_OBJECT (button), "window"));
	g_return_if_fail (widget = gtk_object_get_data (GTK_OBJECT (button), "widget"));
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));

	file = (gchar*) gnome_vfs_uri_get_basename (uri);
	folder = gnome_vfs_uri_extract_dirname (uri);
	
	gp_widget_value_get (widget, &i);
	if (gtk_toggle_button_get_active (button)) i_new = 1;
	if (i != i_new) {
		gp_widget_value_set (widget, &i);
		if (gtk_object_get_data (GTK_OBJECT (button), "camera")) gp_camera_config_set (camera, window);
		else if (file) gp_camera_file_config_set (camera, window, folder, file);
		else if (folder) gp_camera_folder_config_set (camera, window, folder);
		else g_assert_not_reached ();
	}
	g_free (folder);
}

void
on_date_changed (GnomeDateEdit* gnomedateedit, gpointer user_data)
{
	GtkTreeItem*	item;
	Camera*		camera;
	CameraWidget*	widget;
	CameraWidget*	window;
	gint		i, i_new;
	gchar*		folder;
	gchar* 		file;
	GnomeVFSURI*	uri;
	
	g_return_if_fail (gnomedateedit);
	g_return_if_fail (item = gtk_object_get_data (GTK_OBJECT (gnomedateedit), "item"));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
	g_return_if_fail (window = gtk_object_get_data (GTK_OBJECT (gnomedateedit), "window"));
	g_return_if_fail (widget = gtk_object_get_data (GTK_OBJECT (gnomedateedit), "widget"));
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));

	file = (gchar*) gnome_vfs_uri_get_basename (uri);
	folder = gnome_vfs_uri_extract_dirname (uri);

	gp_widget_value_get (widget, &i);
	i_new = (int) gnome_date_edit_get_date (gnomedateedit);
	if (i != i_new) {
		gp_widget_value_set (widget, &i_new);
		if (gtk_object_get_data (GTK_OBJECT (gnomedateedit), "camera")) gp_camera_config_set (camera, window);
		else if (file) gp_camera_file_config_set (camera, window, folder, file);
		else if (folder) gp_camera_folder_config_set (camera, window, folder);
		else g_assert_not_reached ();
	}
	g_free (folder);
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
			xmlSetProp (node, "verb", id);
			break;
		case GP_WIDGET_TEXT:
		case GP_WIDGET_MENU:
		default:
			g_warning ("Not yet implemented!");
			break;
		}
		g_free (id);
	}
}

void 
popup_fill (GtkTreeItem* item, gchar* path, CameraWidget* window, CameraWidget* widget, gboolean camera)
{
	GtkWidget*		hbox;
	GtkWidget*		gtkwidget;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	GtkObject*		adjustment;
	CameraWidget*		child;
	gint			i, j;
	gchar*			tmp;
	gchar*			value_string;
	gfloat			max, min, increment, value_float;
	gint			value_int;
	BonoboUIComponent*	component;

	g_return_if_fail (component = gtk_object_get_data (GTK_OBJECT (item), "component"));

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			popup_fill (item, tmp, window, child, camera);
			g_free (tmp);
			break;
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
				gtk_object_set_data (GTK_OBJECT (menu_item), "item", item);
				gtk_object_set_data (GTK_OBJECT (menu_item), "window", window);
				gtk_object_set_data (GTK_OBJECT (menu_item), "widget", child);
				if (camera) gtk_object_set_data (GTK_OBJECT (menu_item), "camera", GINT_TO_POINTER (1));
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
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtkwidget), (value_int == 1));
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "item", item);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "window", window);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "widget", child);
			if (camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", GINT_TO_POINTER (1));
			gtk_signal_connect_object (GTK_OBJECT (gtkwidget), "toggled", GTK_SIGNAL_FUNC (on_togglebutton_toggled), (gpointer) gtkwidget);
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
			gtk_object_set_data (adjustment, "item", item);
			gtk_object_set_data (adjustment, "window", window);
			gtk_object_set_data (adjustment, "widget", child);
			if (camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", GINT_TO_POINTER (1));
			gtk_signal_connect_object (adjustment, "value_changed", GTK_SIGNAL_FUNC (on_adjustment_value_changed), adjustment);
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
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "item", item);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "window", window);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "widget", child);
			if (camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", GINT_TO_POINTER (1));
			gtk_signal_connect_object (GTK_OBJECT (gtkwidget), "date_changed", GTK_SIGNAL_FUNC (on_date_changed), (gpointer) gtkwidget);
			gtk_signal_connect_object (GTK_OBJECT (gtkwidget), "time_changed", GTK_SIGNAL_FUNC (on_date_changed), (gpointer) gtkwidget);
			gtk_box_pack_end (GTK_BOX (hbox), gtkwidget, TRUE, TRUE, 0);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (hbox))), NULL);
			g_free (tmp);
			break;
		default:
			break;
		}
	}
}



