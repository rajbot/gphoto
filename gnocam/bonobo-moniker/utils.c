#include <config.h>
#include <gnome.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include <parser.h>
#include <tree.h>
#include <bonobo.h>
#include "gnocam-control.h"
#include "utils.h"

/**************/
/* Prototypes */
/**************/

void on_button_clicked			(GtkObject* object, gpointer user_data);
void on_entry_changed			(GtkObject* object, gpointer user_data);
void on_radio_button_activate 		(GtkObject* object, gpointer user_data);
void on_adjustment_value_changed 	(GtkObject* object, gpointer user_data);
void on_togglebutton_toggled 		(GtkObject* object, gpointer user_data);
void on_date_changed 			(GtkObject* object, gpointer user_data);

void menu_setup 	(GnoCamControl* control, CameraWidget* widget, gchar* name, gboolean for_camera);
void menu_prepare       (CameraWidget* widget, xmlNodePtr popup, xmlNodePtr command, xmlNsPtr ns);
void menu_fill          (GnoCamControl* control, gchar* path, CameraWidget* window, CameraWidget* widget, gboolean camera);

/*************/
/* Callbacks */
/*************/

void
on_button_clicked (GtkObject* object, gpointer user_data)
{
	Camera*			camera;
	CameraWidget*		widget;
	CameraWidgetCallback	callback;
	gint			result;
	
	g_return_if_fail (object);
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	callback = gp_widget_callback (widget);
	if ((result = callback (camera, widget)) != GP_OK) g_warning (_("Could not set configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
}

void
on_entry_changed (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gchar*		value_string;
	gchar*		value_string_new;
	gint		result;

	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &value_string);
	value_string_new = gtk_entry_get_text (GTK_ENTRY (object));
	if (!value_string || (value_string && (strcmp (value_string, value_string_new) != 0))) {
		g_return_if_fail (gp_widget_value_set (widget, value_string_new) == GP_OK);
		if ((result = gp_camera_config_set (camera, window)) != GP_OK)
			g_warning (_("Could not set configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
	}
}

void
on_radio_button_activate (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gchar*		value_string;
	gchar*		value_string_new;
	gint		result;

	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &value_string);
	value_string_new = gp_widget_choice (widget, GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (object), "choice")));
	if (!value_string || (value_string && (strcmp (value_string_new, value_string) != 0))) {
		g_return_if_fail (gp_widget_value_set (widget, value_string_new) == GP_OK);
		if ((result = gp_camera_config_set (camera, window)) != GP_OK) 
			g_warning (_("Could not set configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
	}
}

void
on_adjustment_value_changed (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gfloat		f, f_new;
	gint		result;

	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	g_warning (_("on_adjustment_value_changed"));

	gp_widget_value_get (widget, &f);
	f_new = GTK_ADJUSTMENT (object)->value;
	if (f != f_new) {
		g_return_if_fail (gp_widget_value_set (widget, &f_new) == GP_OK);
		if ((result = gp_camera_config_set (camera, window)) != GP_OK)
			g_warning (_("Could not set camera configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
	}
}

void
on_togglebutton_toggled (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	window;
	CameraWidget*	widget;
	gint		i, i_new = 0;
	gint		result;
	
	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &i);
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (object))) i_new = 1;
	if (i != i_new) {
		g_return_if_fail (gp_widget_value_set (widget, &i_new) == GP_OK);
		if ((result = gp_camera_config_set (camera, window)) != GP_OK)
			g_warning (_("Could not set configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
	}
}

void
on_date_changed (GtkObject* object, gpointer user_data)
{
	Camera*		camera;
	CameraWidget*	widget;
	CameraWidget*	window;
	gint		i, i_new;
	gint		result;
	
	g_return_if_fail (object);
	g_return_if_fail (window = gtk_object_get_data (object, "window"));
	g_return_if_fail (widget = gtk_object_get_data (object, "widget"));
	g_return_if_fail (camera = gtk_object_get_data (object, "camera"));

	gp_widget_value_get (widget, &i);
	i_new = (int) gnome_date_edit_get_date (GNOME_DATE_EDIT (object));
	if (i != i_new) {
		g_return_if_fail (gp_widget_value_set (widget, &i_new) == GP_OK);
		if ((result = gp_camera_config_set (camera, window)) != GP_OK) 
			g_warning (_("Could not set configuration!\n(%s)"), gp_camera_result_as_string (camera, result));
	}
}

/*************/
/* Functions */
/*************/

Camera*
util_camera_new (gchar* name) 
{
        GConfClient*    client;
        guint           i, result;

	g_return_val_if_fail (name, NULL);

	/* Make sure GConf is initialized. */
        if (!gconf_is_initialized ()) {
		GError* gerror = NULL;
		gchar* 	argv[1] = {"Whatever"};
		g_return_val_if_fail (gconf_init (1, argv, &gerror), NULL);
	}
	g_return_val_if_fail (client = gconf_client_get_default (), NULL);

        /* Does GConf know about the camera? */
        for (i = 0; ; i++) {
                gchar* tmp;
                gchar* path = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);
                
                if (!gconf_client_dir_exists (client, path, NULL)) {
                        g_warning (_("Directory '%s' does not exist. Camera '%s' unknown!"), path, name);
                        g_free (path);
                        break;
                }

                tmp = g_strconcat (path, "/name", NULL);
                name = gconf_client_get_string (client, tmp, NULL);
                g_free (tmp);

                if (!strcmp (name, name)) {
                        Camera* camera;
                        gchar*  model;
                        gchar*  port;
                        
                        tmp = g_strconcat (path, "/model", NULL);
                        model = g_strdup (gconf_client_get_string (client, tmp, NULL));
                        g_free (tmp);

                        tmp = g_strconcat (path, "/port", NULL);
                        port = g_strdup (gconf_client_get_string (client, tmp, NULL));
                        g_free (tmp);
                        
                        g_free (path);

                        /* Create the camera */
                        if ((result = gp_camera_new (&camera)) != GP_OK) {
                                g_warning (_("Could not create camera! (%s)"), gp_result_as_string (result));
                        } else {
                                strcpy (camera->model, model);

                                /* Search for port */
                                for (i = 0; i < gp_port_count_get (); i++) {
                                        if ((result = gp_port_info_get (i, camera->port)) != GP_OK) {
                                                g_warning (_("Could not get port info for port %i! (%s)\n"), i, gp_result_as_string (result));
                                                continue;
                                        }
                                        if (!strcmp (camera->port->name, port)) break;
                                }
                                if (i == gp_port_count_get ()) g_warning (_("Port '%s' not found!"), port);
				else if (i < 0) g_warning (_("Could not get number of ports! (%s)"), gp_result_as_string (i));
                                else {
                                        camera->port->speed = 0;

                                        /* Connect to the camera */
                                        if ((result = gp_camera_init (camera)) != GP_OK) 
                                                g_warning (_("Could not initialize camera! (%s)"), gp_result_as_string (result));
                                }
                        }

                        g_free (port);
                        g_free (model);
                        
                        return (camera);
                }
                g_free (path);
        }

        return (NULL);
}

void 
menu_create (GnoCamControl* control)
{
	Bonobo_UIContainer container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));

	if (container != CORBA_OBJECT_NIL) {
		BonoboUIComponent* component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));
		
		/* Set the component's container. */
		bonobo_ui_component_set_container (component, container);

		/* Camera Configuration? */
		if (control->camera->abilities->config && !control->config_camera) {
			gint result = gp_camera_config_get (control->camera, &(control->config_camera));
			if (result != GP_OK) 
				g_warning (_("Could not get widget for camera configuration! (%s)"), 
					gp_camera_result_as_string (control->camera, result));
		}
		if (control->config_camera) menu_setup (control, control->config_camera, "CameraConfiguration", TRUE);

		/* File Configuration? */
		if (!control->config_file) {
			gchar* 	file = g_basename (control->path);
			gchar*	folder = g_dirname (control->path);
			gint result = gp_camera_file_config_get (control->camera, &(control->config_file), folder, file);
			if (result != GP_OK) 
				g_warning (_("Could not get widget for configuration of file '%s' in folder '%s'! (%s)"), 
					file, folder, gp_camera_result_as_string (control->camera, result));
			g_free (folder);
		}
		if (control->config_file) menu_setup (control, control->config_file, "FileConfiguration", FALSE);

		/* Folder Configuration? */
		if (!control->config_folder) {
			gint result = gp_camera_folder_config_get (control->camera, &(control->config_folder), control->path);
			if (result != GP_OK) 
				g_warning (_("Could not get widget for configuration of '%s'! (%s)"), 
					control->path, gp_camera_result_as_string (control->camera, result));
		}
		if (control->config_folder) menu_setup (control, control->config_folder, "FolderConfiguration", FALSE);
		
		/* Release the container. */
		bonobo_object_release_unref (container, NULL);
	}
}

void
menu_setup (GnoCamControl* control, CameraWidget* widget, gchar* name, gboolean for_camera)
{
        gchar*          tmp = NULL;
        xmlDocPtr       doc = xmlNewDoc ("1.0");
        xmlNsPtr        ns = xmlNewGlobalNs (doc, "xxx", "xxx"); 
        xmlNodePtr      node, child;
        xmlNodePtr      commands = xmlNewNode (ns, "commands");
        gint            i;

        /* Set up the basic structure. */
        xmlDocSetRootElement (doc, node = xmlNewNode (ns, "Root"));
        xmlAddChild (node, commands);
        xmlAddChild (node, child = xmlNewNode (ns, "menu"));
        xmlAddChild (child, node = xmlNewNode (ns, "submenu"));
        xmlSetProp (node, "name", "Edit");
	xmlSetProp (node, "_label", "_Edit");
        xmlAddChild (node, child = xmlNewNode (ns, "submenu"));
        xmlSetProp (child, "name", name);
        xmlSetProp (child, "_label", name);
        menu_prepare (widget, child, commands, ns);

        /* Send it to the component. */
        xmlDocDumpMemory (doc, (xmlChar**) &tmp, &i);
        xmlFreeDoc (doc);
        bonobo_ui_component_set_translate (bonobo_control_get_ui_component (BONOBO_CONTROL (control)), "/", tmp, NULL);
        g_free (tmp);

	/* Finish. */
	tmp = g_strconcat ("/menu/Edit/", name, NULL);
	menu_fill (control, tmp, widget, widget, for_camera);
	g_free (tmp);
}

void 
menu_prepare (CameraWidget* widget, xmlNodePtr menu, xmlNodePtr command, xmlNsPtr ns)
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
			xmlAddChild (menu, node = xmlNewNode (ns, "submenu"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_label", gp_widget_label (child));
			xmlSetProp (node, "_tip", gp_widget_label (child));
			menu_prepare (child, node, command, ns);
			break;
		case GP_WIDGET_TEXT:
		case GP_WIDGET_MENU:
		case GP_WIDGET_RADIO:
		case GP_WIDGET_DATE:
		case GP_WIDGET_TOGGLE:
		case GP_WIDGET_BUTTON:
		case GP_WIDGET_RANGE:
			xmlAddChild (menu, node = xmlNewNode (ns, "control"));
			xmlSetProp (node, "name", id);
			xmlSetProp (node, "_tip", gp_widget_label (child));
			break;
		default:
			g_warning ("Encountered unsupported widget!");
			break;
		}
		g_free (id);
	}
}

void 
menu_fill (GnoCamControl* control, gchar* path, CameraWidget* window, CameraWidget* widget, gboolean for_camera)
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
	BonoboUIComponent*	component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));

	for (i = 0; i < gp_widget_child_count (widget); i++) {
		child = gp_widget_child (widget, i);
		switch (gp_widget_type (child)) {
		case GP_WIDGET_WINDOW:
			break;
		case GP_WIDGET_SECTION:
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			menu_fill (control, tmp, window, child, for_camera);
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
				gtk_object_set_data (GTK_OBJECT (menu_item), "camera", control->camera);
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
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", control->camera);
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
			gtk_object_set_data (adjustment, "camera", control->camera);
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
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", control->camera);
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
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", control->camera);
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
			gtk_widget_show (gtkwidget = gtk_button_new_with_label (gp_widget_label (child)));
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "camera", control->camera);
			gtk_object_set_data (GTK_OBJECT (gtkwidget), "widget", child);
			if (for_camera) gtk_object_set_data (GTK_OBJECT (gtkwidget), "for_camera", GINT_TO_POINTER (1));
			gtk_signal_connect (GTK_OBJECT (gtkwidget), "clicked", GTK_SIGNAL_FUNC (on_button_clicked), NULL);
			tmp = g_strdup_printf ("%s/%i", path, gp_widget_id (child));
			bonobo_ui_component_object_set (component, tmp, bonobo_object_corba_objref (BONOBO_OBJECT (bonobo_control_new (gtkwidget))), NULL);
			g_free (tmp);
			break;
		default:
			g_warning ("Encountered unsupported widget!");
			break;
		}
	}
}



