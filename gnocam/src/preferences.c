#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "preferences.h"

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_button_camera_add_clicked (GtkButton *button, gpointer user_data);
void on_button_camera_update_clicked (GtkButton *button, gpointer user_data);
void on_button_camera_delete_clicked (GtkButton *button, gpointer user_data);
void on_propertybox_preferences_apply (GnomePropertyBox *propertybox, gint arg, gpointer user_data);
gint on_propertybox_preferences_close (GnomePropertyBox *propertybox, gpointer user_data);
void on_entry_prefix_changed (GtkEditable *editable, gpointer user_data);
void on_clist_cameras_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data);
gboolean on_combo_entry_model_focus_out_event (GtkWidget *widget, GdkEventFocus *event, gpointer user_data);
void on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);

Camera *camera_check (GladeXML *xml, gboolean main_window, gchar *name, gchar *model, gchar *port, gchar *speed);
void camera_add_to_tree (GladeXML *xml, Camera *camera, gchar *name, gint position);
void folder_build (GladeXML *xml, GtkWidget *item, Camera *camera, gchar *path);

/******************************************************************************/
/* Callbacks                                                                  */
/******************************************************************************/

void
on_button_camera_add_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML *xml, *xml_preferences;
	GtkWindow *window;
	GtkCList *clist;
	gchar *model, *port, *speed, *name;
	gchar *text[4];
	GList *port_list;
	Camera *camera;

	xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences");
	g_assert (xml_preferences != NULL);
	clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"));
	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "propertybox_preferences"));
	g_assert (window != NULL);

	/* Get lists. */
	port_list = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "combo_port")), "list");

	/* Get the user's entries. */
	name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")));
	model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")));
	speed = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")));
	port = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")));

	camera = camera_check (xml_preferences, FALSE, name, model, port, speed);
	if (camera != NULL) {

		/* Add entry to list. */
		text[0] = name;
		text[1] = model;
		text[2] = port;
		text[3] = speed;
		gtk_clist_append (clist, text);
		
		/* Add the camera to the tree. */
		camera_add_to_tree (xml, camera, name, -1);
	}
}

void
on_button_camera_update_clicked (GtkButton *button, gpointer user_data)
{
        GList *selection, *cameras, *cameras_to_remove;
        GladeXML *xml_preferences, *xml;
        gchar *name, *model, *speed, *port;
	GtkWindow *window;
	gint row;
	Camera *camera;
	gchar *text[4];
	GtkCList *clist;

        xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences");
        g_assert (xml_preferences != NULL);
	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);
	window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "propertybox_preferences"));
	clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"));

        /* Check how many cameras have been selected. */
        selection = GTK_CLIST (clist)->selection;
	selection = g_list_first (selection);
        if (selection != NULL) {
		if (g_list_length (selection) == 1) {

	                /* Exactly one camera selected. */
			row = GPOINTER_TO_INT (selection->data);
	
			/* Get the user's entries. */
		        name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")));
		        model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")));
		        speed = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")));
	        	port = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")));
	
			if ((camera = camera_check (xml_preferences, FALSE, name, model, port, speed)) != NULL) {
				
				/* Add entry at previous position to list. */
				text[0] = name;
				text[1] = model;
				text[2] = port;
				text[3] = speed;
				gtk_clist_insert (clist, row, text);
	
				/* Add the camera to tree (main window). */
				camera_add_to_tree (xml, camera, name, row);
	
				/* Remove the old entry in the list. */
				gtk_clist_remove (clist, row + 1);
				
				/* Remove the old entry in the tree (main window). */
				cameras = gtk_container_children (GTK_CONTAINER (glade_xml_get_widget (xml, "tree_cameras")));
				cameras_to_remove = g_list_append (NULL, g_list_nth_data (cameras, row + 1));
				gtk_tree_remove_items (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), cameras_to_remove);
				g_list_free (cameras_to_remove);
			}
		} else {
			gnome_error_dialog_parented (_("Please select only one camera."), window);
		}
	} else {
		gnome_error_dialog_parented (_("Please select one camera first."), window);
	}
}

void
on_button_camera_delete_clicked (GtkButton *button, gpointer user_data)
{
	GList *selection;
	GladeXML *xml_preferences, *xml;
	GtkWindow *window;
	GtkCList *clist;
	GList *camera_list, *cameras_to_remove;

	xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences");
	g_assert (xml_preferences != NULL);
	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);
	window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "propertybox_preferences"));
	g_assert (window != NULL);

	clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"));
	selection = g_list_first (clist->selection);
	if (selection != NULL) {
	
		/* Remove cameras in camera list and tree (main window). */
		while (selection != NULL) {
			camera_list = gtk_container_children (GTK_CONTAINER (glade_xml_get_widget (xml, "tree_cameras")));
			cameras_to_remove = g_list_append (NULL, g_list_nth_data (camera_list, GPOINTER_TO_INT (selection->data)));

			/* Free camera. */
			g_free (gtk_object_get_data (GTK_OBJECT (cameras_to_remove->data), "camera"));

			gtk_tree_remove_items (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), cameras_to_remove);
			g_list_free (cameras_to_remove);
			gtk_clist_remove (clist, GPOINTER_TO_INT (selection->data));
			selection = g_list_first (clist->selection);
		}
	} else {
		gnome_error_dialog_parented (_("Please select a camera first!"), window);
	}
}

void
on_propertybox_preferences_apply (GnomePropertyBox *propertybox, gint arg, gpointer user_data)
{
	GladeXML *xml_preferences, *xml;
	GnomeApp *app;
	gchar *prefix, *prefix_old;
	guint i;

	xml_preferences = gtk_object_get_data (GTK_OBJECT (propertybox), "xml_preferences");
	g_assert (xml_preferences != NULL);
	xml = gtk_object_get_data (GTK_OBJECT (propertybox), "xml");
	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));

	prefix = g_strdup (gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix"))));

	/* Save to home directory by default. */
	if (strcmp (prefix, "") == 0) {
		g_free (prefix);
		prefix = getenv ("HOME");
	}

	/* Make sure the prefix begins with '/'. */
	if (prefix[0] != '/') {
		prefix_old = prefix;
		prefix = g_strdup_printf ("/%s", prefix_old);
		g_free (prefix_old);
	}
	
	/* Make sure the prefix ends with '/'. */
	for (i = 0; ; i++) if (prefix[i] == 0) break;
	if (prefix[i - 1] != '/') {
                prefix_old = prefix;
                prefix = g_strdup_printf ("%s/", prefix_old);
                g_free (prefix_old);
	}
	prefix_old = gtk_object_get_data (GTK_OBJECT (app), "prefix");
	gtk_object_set_data (GTK_OBJECT (app), "prefix", g_strdup (prefix));
	g_free (prefix_old);
	gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix")), prefix);

	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_high"))->active) {
		gtk_object_set_data (GTK_OBJECT (app), "debug_level", GINT_TO_POINTER (GP_DEBUG_HIGH));
	} else if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_medium"))->active) {
		gtk_object_set_data (GTK_OBJECT (app), "debug_level", GINT_TO_POINTER (GP_DEBUG_MEDIUM));
	} else if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_low"))->active) {
		gtk_object_set_data (GTK_OBJECT (app), "debug_level", GINT_TO_POINTER (GP_DEBUG_LOW));
	} else if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_none"))->active) {
		gtk_object_set_data (GTK_OBJECT (app), "debug_level", GINT_TO_POINTER (GP_DEBUG_NONE));
	} else {
		g_assert_not_reached ();
	}
}

gint
on_propertybox_preferences_close (GnomePropertyBox *propertybox, gpointer user_data)
{
	gtk_widget_destroy (GTK_WIDGET (propertybox));
	return (TRUE);
}

void
on_entry_prefix_changed (GtkEditable *editable, gpointer user_data)
{
	GladeXML *xml_preferences;

	xml_preferences = gtk_object_get_data (GTK_OBJECT (editable), "xml_preferences");
	g_assert (xml_preferences != NULL);

	gnome_property_box_changed (GNOME_PROPERTY_BOX (glade_xml_get_widget (xml_preferences, "propertybox_preferences")));
}

void
on_clist_cameras_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data)
{
	GList *selection;
	GladeXML *xml_preferences;
	gchar *name, *model, *speed, *port;

	xml_preferences = gtk_object_get_data (GTK_OBJECT (clist), "xml_preferences"); 
	g_assert (xml_preferences != NULL);

	/* Check how many cameras have been selected. */
	selection = g_list_first (GTK_CLIST (clist)->selection);
	if ((selection != NULL) && (g_list_length (selection) == 1)) {

		/* Exactly one camera selected. */
		gtk_clist_get_text (GTK_CLIST (clist), row, 0, &name);
		gtk_clist_get_text (GTK_CLIST (clist), row, 1, &model);
		gtk_clist_get_text (GTK_CLIST (clist), row, 2, &port);
		gtk_clist_get_text (GTK_CLIST (clist), row, 3, &speed);
                gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")), name);
                gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")), model);
                gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")), speed);
                gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")), port);

	} else {
		
		/* Clean up the settings. */
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")), "");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")), "");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")), "");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")), "");
	}
}

gboolean
on_combo_entry_model_focus_out_event (GtkWidget *widget, GdkEventFocus  *event, gpointer user_data)
{
	GladeXML *xml, *xml_preferences;
	GtkWindow *window;
	GList *list;
	GtkCombo *combo;
	gint i;
	gchar *model;
	CameraAbilities abilities;

	xml_preferences = gtk_object_get_data (GTK_OBJECT (widget), "xml_preferences");
	g_assert (xml_preferences != NULL);
	xml = gtk_object_get_data (GTK_OBJECT (widget), "xml");
	g_assert (xml != NULL);
	window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "propertybox_preferences"));
	g_assert (window != NULL);

	/* Clean up the speed list. */
	combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_speed"));
	if ((list = gtk_object_get_data (GTK_OBJECT (combo), "list")) != NULL)
		g_list_free (list);
	
	/* Build speed list. */
	model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")));
	if (strcmp ("", model) != 0) {
		
		/* User has selected a model. */
		if (gp_camera_abilities_by_name (model, &abilities) != GP_ERROR) {
			i = 0;
			list = NULL;
			while (abilities.speed[i] != 0) {
				list = g_list_append (list, g_strdup_printf ("%i", abilities.speed[i]));
				i++;
			}
			if (list != NULL) {
				gtk_combo_set_popdown_strings (combo, list);
				gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")), "");
			}
		} else {
			gnome_error_dialog_parented (_("Could not get camera abilities!"), window);
		}
	}
	return (FALSE);
}

void
on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time)
{
	GList *filenames;
	guint i;
	gchar *message;

	filenames = gnome_uri_list_extract_filenames (selection_data->data);
	for (i = 0; i < g_list_length (filenames); i++) {
		message = g_strdup_printf ("Upload not implemented (filename: %s)", (gchar *) g_list_nth_data (filenames, i));
		gnome_dialog_run_and_close (GNOME_DIALOG (gnome_error_dialog (message)));
		g_free (message);
	}
	gnome_uri_list_free_strings (filenames);
}

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/

Camera *
camera_check (GladeXML *xml, gboolean main_window, gchar *name, gchar *model, gchar *port, gchar *speed)
{
	CameraPortInfo port_info;
	gint number_of_ports;
	guint i;
	gboolean found;
	GtkWindow *window;
	Camera *camera = NULL;

	/* Main window or preference window? */
	if (main_window) window = GTK_WINDOW (glade_xml_get_widget (xml, "app"));
	else window = GTK_WINDOW (glade_xml_get_widget (xml, "propertybox_preferences"));

	/* Check name. */
	if (name == NULL) {
		gnome_error_dialog_parented (_("You have to indicate a name!"), window);
		return (NULL);
	}
	if (strcmp (name, "") == 0) {
		gnome_error_dialog_parented (_("Name of camera cannot be empty string!"), window);
		return (NULL);
	}

	/* Check port. */
	if (port == NULL) {
		gnome_error_dialog_parented (_("You have to indicate a port!"), window);
		return (NULL);
	}
	found = FALSE;
        if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
		gnome_error_dialog_parented (_("Could not get number of ports!"), window);
		return (NULL);
	}
        for (i = 0; i < number_of_ports; i++) {
                if (gp_port_info (i, &port_info) == GP_ERROR) {
			gnome_error_dialog_parented (_("Could not get port information!"), window);
			return (NULL);
		}
		if (strcmp (port_info.name, port) == 0) {
			found = TRUE;
			break;
		}
	}
	if (found == FALSE) {
		gnome_error_dialog_parented (_("Could not find port!"), window);
		return (NULL);
	}

	/* Check speed. Ok, we don't actually check anything...*/
	if (speed == NULL) port_info.speed = 0;
	else port_info.speed = atoi (speed);

	/* Check model. */
	if (model == NULL) {
		gnome_error_dialog_parented (_("You have to indicate a model!"), window);
		return (NULL);
	}
	if (gp_camera_new_by_name (&camera, model, &port_info) == GP_ERROR) {
		gnome_error_dialog_parented (_("Could not set camera model!"), window);
		return (NULL);
	} else {
		return (camera);
	}
}

void
camera_add_to_tree (GladeXML *xml, Camera *camera, gchar *name, gint position)
{
	gchar *path;
	GnomeApp *app;
	GtkWidget *item;
	CameraList folder_list;
	GtkTree *tree;
	GtkTargetEntry target_table[] = {
	        {"text/uri-list", 0, 0}
	};
	g_assert (xml != NULL);
	g_assert (camera != NULL);
	g_assert (name != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);
	tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"));

	path = g_strdup ("/");
	item = gtk_tree_item_new_with_label (name);
	gtk_widget_show (item);
	if (position == -1) gtk_tree_append (tree, item);
	else gtk_tree_insert (tree, item, position);
	
	/* For drag and drop. */
	//FIXME: Right now, only drops onto the camera (= root folder) work. Why ?!?
	gtk_drag_dest_set (item, GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
	
	/* Connect the signals. */
	gtk_signal_connect (GTK_OBJECT (item), "drag_data_received", GTK_SIGNAL_FUNC (on_drag_data_received), NULL);
	
	/* Store some data. */
	gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
	gtk_object_set_data (GTK_OBJECT (item), "path", path);
	
	/* Do we have folders? */
	if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
		folder_build (xml, item, camera, path);
	} else {
		gnome_app_error (app, _("Could not get folder list!"));
	}
}

/**
 * preferences:
 * @xml: A reference to xml of the main window.
 *
 * Pops up the preferences dialog.
 **/
void
preferences (GladeXML *xml)
{
	GnomeApp *app;
	GladeXML *xml_preferences;
	GList *list;
	GtkWidget *item;
	GtkCList *clist_cameras;
	gchar *text[4];
	guint i;
	Camera *camera;
	gchar buffer[1024];
	gint number_of_models, number_of_ports;
	GtkCombo *combo;
	GtkWindow *window;
	gchar *name, *prefix;
	CameraPortInfo info;

	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);

	/* Load the interface. */
	xml_preferences = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "propertybox_preferences");
	if (xml_preferences == NULL) {
		gnome_app_error (app, _("Could not find " GNOCAM_GLADEDIR "gnocam.glade. Check if " PACKAGE " was installed correctly."));
	} else {

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_preferences);

		/* Store some data we need afterwards. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "propertybox_preferences")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "propertybox_preferences")), "xml_preferences", xml_preferences);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "entry_prefix")), "xml_preferences", xml_preferences);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "clist_cameras")), "xml_preferences", xml_preferences);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "combo_entry_model")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "combo_entry_model")), "xml_preferences", xml_preferences);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_add")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_add")), "xml_preferences", xml_preferences);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_update")), "xml", xml);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_update")), "xml_preferences", xml_preferences);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_delete")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_delete")), "xml_preferences", xml_preferences);

		/* Debug and prefix stuff. */
		prefix = gtk_object_get_data (GTK_OBJECT (app), "prefix");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix")), prefix);
		switch (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (app), "debug_level"))) {
		case GP_DEBUG_NONE:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_none")), TRUE);
			break;
		case GP_DEBUG_LOW:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_low")), TRUE);
			break;
		case GP_DEBUG_MEDIUM:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_medium")), TRUE);
			break;
		case GP_DEBUG_HIGH:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_high")), TRUE);
			break;
		default: 
			g_assert_not_reached ();
		}
	
		/* Get the current cameras from the main window. */
		window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "propertybox_preferences"));
		list = GTK_TREE ((glade_xml_get_widget (xml, "tree_cameras")))->children;
		clist_cameras = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"));
		for (i = 0; i < g_list_length (list); i++) {
			item = GTK_WIDGET (g_list_nth_data (list, i));
			camera = gtk_object_get_data (GTK_OBJECT (item), "camera");
			g_assert (camera != NULL);
			gtk_label_get (GTK_LABEL (GTK_BIN (item)->child), &text[0]);
			text[1] = camera->model;
			text[2] = camera->port->name;
			text[3] = g_strdup_printf ("%i", camera->port->speed);
			gtk_clist_append (clist_cameras, text);
		}

                /* Build model list. */
                combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_model"));
                list = NULL;
		if ((number_of_models = gp_camera_count ()) != GP_ERROR) {
	                for (i = 0; i < number_of_models; i++) {
	                        if (gp_camera_name (i, buffer) == GP_ERROR) {
	                                gnome_error_dialog_parented (_("Could not get model name!"), window);
	                                strcpy (buffer, "?");
	                        }
	                        name = g_strdup_printf ("%s", buffer);
	                        list = g_list_append (list, name);
	                }
	                if (list != NULL) {
				gtk_combo_set_popdown_strings (combo, list);
				gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")), "");
			}
		} else {
			gnome_error_dialog_parented (_("Could not get number of supported models!"), window);
		}

	        /* Build port list. */
		combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_port"));
		list = NULL;
	        if ((number_of_ports = gp_port_count ()) != GP_ERROR) {
	                for (i = 0; i < number_of_ports; i++) {
	                        if (gp_port_info (i, &info) == GP_ERROR) {
	                                gnome_error_dialog_parented (_("Could not get port info!"), window);
	                                list = g_list_append (list, g_strdup ("?"));
	                        } else {
	                                list = g_list_append (list, g_strdup (info.name));
	                        }
	                }
	                if (list != NULL) {
				gtk_combo_set_popdown_strings (combo, list);
				gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")), "");
			}
	                gtk_object_set_data (GTK_OBJECT (combo), "list", list);
	        } else {
	                gnome_error_dialog_parented (_("Could not get number of ports!"), window);
	                gtk_object_set_data (GTK_OBJECT (combo), "list", NULL);
	        }
	}
}

void
folder_build (GladeXML *xml, GtkWidget *item, Camera *camera, gchar *path)
{
	CameraList folder_list;
	CameraListEntry *folder_list_entry;
	guint count, i;
	GnomeApp *app;
	GtkWidget *subtree;

	g_assert (item != NULL);
	g_assert (camera != NULL);
	g_assert (path != NULL);
	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);

	if (gp_camera_folder_list (camera, &folder_list, path) == GP_OK) {
		count = gp_list_count (&folder_list);

		/* Do we have subfolders? */
		if (count > 0) {

			/* We have subfolders. */
			subtree = gtk_tree_new ();
			gtk_tree_item_set_subtree (GTK_TREE_ITEM (item), subtree);

			for (i = 0; i < count; i++) {
				folder_list_entry = gp_list_entry (&folder_list, i);

				/* Add the subfolder to the tree. */
				item = gtk_tree_item_new_with_label (folder_list_entry->name);
				gtk_widget_show (item);
				gtk_tree_append (GTK_TREE (subtree), item);
				path = g_strdup_printf ("%s/%s", path, folder_list_entry->name);
				gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
				gtk_object_set_data (GTK_OBJECT (item), "path", path);
				folder_build (xml, item, camera, path);
			}
		}
	} else {
		gnome_app_error (app, _("Could not get folder list!"));
	}
}

/**
 * preferences_get:
 * @xml: A reference to the xml of the main window.
 *
 * Restores the preferences from file. Used on startup.
 **/
void 
preferences_get (GladeXML *xml)
{
	gchar *prefix, *prefix_old;
	gboolean def = TRUE;
	Camera *camera;
	gchar *name;
	GnomeApp *app;
	gint i;
	gint debug_level;

	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);

	/* Restore 'other' settings. */
	gnome_config_push_prefix ("/" PACKAGE "/Other/");
	debug_level = gnome_config_get_int ("debug level");
	gtk_object_set_data (GTK_OBJECT (app), "debug_level", GINT_TO_POINTER (debug_level));
	prefix = gnome_config_get_string ("prefix");

	/* Save to home directory by default. */
	if (prefix == NULL) prefix = getenv ("HOME");

        /* Make sure the prefix begins with '/'. */
        if (prefix[0] != '/') {
                prefix_old = prefix;
                prefix = g_strdup_printf ("/%s", prefix_old);
                g_free (prefix_old);
        }

        /* Make sure the prefix ends with '/'. */
        for (i = 0; ; i++) if (prefix[i] == 0) break;
        if (prefix[i - 1] != '/') {
                prefix_old = prefix;
                prefix = g_strdup_printf ("%s/", prefix_old);
                g_free (prefix_old);
        }
	gtk_object_set_data (GTK_OBJECT (app), "prefix", prefix);
	gnome_config_pop_prefix ();

	/* Set up cameras. */
	i = 0;
	prefix = g_strdup_printf ("/" PACKAGE "/Camera %i/", i);
	g_assert (prefix != NULL);
	gnome_config_push_prefix (prefix);

	/* Check if we've got entries. */
	name = gnome_config_get_string_with_default ("name", &def);
	if (!def) {
		/* We've got at least one entry. */
		while (def == FALSE) {

			/* Read this entry. */
			camera = camera_check (
				xml, 
				TRUE, 
				gnome_config_get_string ("name"), 
				gnome_config_get_string ("model"), 
				gnome_config_get_string ("port"), 
				gnome_config_get_string ("speed"));
			if (camera == NULL) {
				gnome_app_error (app, _("The configuration file is corrupt!"));
			} else {
	
				/* Add camera (= root folder) to tree. */
				camera_add_to_tree (xml, camera, name, -1);
			}

			gnome_config_pop_prefix ();
			g_free (prefix);
			prefix = g_strdup_printf ("/" PACKAGE "/Camera %i/", ++i);
			g_assert (prefix != NULL);
			gnome_config_push_prefix (prefix);

			/* Check if we've got another entry. */
			name = gnome_config_get_string_with_default ("name", &def);
		}
	} else {
		gnome_app_message (
			app, 
			_("It seems that you are running\n"
			PACKAGE " for the first time.\n"
			PACKAGE " is a small but powerful\n"
			"front-end to gphoto. Welcome to\n"
			"the wunderful world of gphoto!\n\n"
			"Before you do anything else, you \n"
			"should probably first open the \n"
			"preferences dialog and add a \n"
			"camera.\n\n"
			"Enjoy!"));
	}
	g_free (prefix);
	gnome_config_pop_prefix ();
}

/**
 * preferences_set:
 * @xml: A reference to the xml of the main window.
 *
 * Stores the preferences to file.
 **/
void 
preferences_set (GladeXML *xml)
{
	GList *cameras;
	GnomeApp *app;
	Camera *camera;
	gchar *prefix;
	guint i = 0;
	gchar *name;
	gboolean def;

	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);

	/* Store 'other' settings. */
	gnome_config_push_prefix ("/" PACKAGE "/Other/");
	gnome_config_set_int ("debug level", GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (app), "debug_level")));
	gnome_config_set_string ("prefix", gtk_object_get_data (GTK_OBJECT (app), "prefix"));
	gnome_config_pop_prefix ();
	
	/* Store camera settings. */
	cameras = gtk_container_children (GTK_CONTAINER (glade_xml_get_widget (xml, "tree_cameras")));
	if (cameras != NULL) {
		for (i = 0; i < g_list_length (cameras); i++) {
			prefix = g_strdup_printf ("/" PACKAGE "/Camera %i/", i);
			gnome_config_push_prefix (prefix);
			camera = gtk_object_get_data (GTK_OBJECT (g_list_nth_data (cameras, i)), "camera");
			g_assert (camera != NULL);
			gtk_label_get (GTK_LABEL (GTK_BIN (g_list_nth_data (cameras, i))->child), &name);
			gnome_config_set_string ("name", name);
			gnome_config_set_string ("model", camera->model);
			gnome_config_set_string ("port", camera->port->name);
			gnome_config_set_int ("speed", camera->port->speed);
			g_free (prefix);
			gnome_config_pop_prefix ();
		}
	}

	/* Check if we have to clean up. */
	prefix = g_strdup_printf ("/" PACKAGE "/Camera %i/", i++);
	gnome_config_push_prefix (prefix);
	gnome_config_get_string_with_default ("model", &def);
	if (def == FALSE) {

		/* We have to clean up. */
		while (def == FALSE) {
			gnome_config_clean_section (prefix);
			g_free (prefix);
			gnome_config_pop_prefix ();
			prefix = g_strdup_printf ("/" PACKAGE "/Camera %i/", i++);
			gnome_config_push_prefix (prefix);
			gnome_config_get_string_with_default ("model", &def);
		}
	}
	g_free (prefix);
	gnome_config_pop_prefix ();
	gnome_config_sync ();
}

