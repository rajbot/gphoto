#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <parser.h>
#include <bonobo.h>
#include "preferences.h"
#include "gnocam.h"

/********************/
/* Static Variables */
/********************/

static GladeXML*	xml_preferences = NULL;
static GConfChangeSet*	change_set = NULL;
static GConfChangeSet*	revert_change_set = NULL;

/**********************/
/* External Variables */
/**********************/

extern GConfClient*     gconf_client;

/**************/
/* Prototypes */
/**************/

void on_button_camera_add_clicked 			(GtkButton* button, gpointer user_data);
void on_button_camera_update_clicked 			(GtkButton* button, gpointer user_data);
void on_button_camera_delete_clicked 			(GtkButton* button, gpointer user_data);

void on_dialog_preferences_button_revert_clicked 	(GtkButton* button, gpointer user_data);
void on_dialog_preferences_button_ok_clicked 		(GtkButton* button, gpointer user_data);
void on_dialog_preferences_button_apply_clicked 	(GtkButton* button, gpointer user_data);
void on_dialog_preferences_button_cancel_clicked 	(GtkButton* button, gpointer user_data);

void on_clist_cameras_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data);

void on_entry_prefix_changed 		(GtkEditable* editable, gpointer user_data);

gboolean on_combo_entry_model_focus_out_event 	(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);

void on_radiobutton_debug_level_toggled (GtkToggleButton* toggle_button);

void update_speed_and_port_list			(gchar* model);

void dialog_preferences_debug_level_update 	(gint debug_level);
void dialog_preferences_prefix_update 		(gchar *prefix);
void dialog_preferences_cameras_update 		(GSList* camera_list);
void dialog_preferences_populate 		(void);
void dialog_preferences_cameras_changed 	(void);
void dialog_preferences_revert 			(void);
void dialog_preferences_apply 			(void);
void dialog_preferences_update_sensitivity 	(void);

/*************/
/* Callbacks */
/*************/

void
on_button_camera_add_clicked (GtkButton* button, gpointer user_data)
{
	GtkWindow*	window;
	GtkCList*	clist;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gchar*		name;
	gchar*		text[4];
	gint		unused_id;
	guint		j;

	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert (change_set != NULL);

	/* Get the user's entries. */
	name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")));
	model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")));
	speed = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")));
	port = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")));

	/* Tell the user to properly fill out the entries if he/she did not do so. */
	if (*name == '\0') {
		gnome_error_dialog_parented (_("Please indicate a name!"), window);
	} else if (*model == '\0') {
		gnome_error_dialog_parented (_("Please indicate a model!"), window);
	} else {
	
		/* Search for unused id. */
		for (unused_id = 0; ; unused_id++) {
			for (j = 0; j < clist->rows; j++) {
				if (GPOINTER_TO_INT (gtk_clist_get_row_data (clist, j)) == unused_id) break;
			}
			if (j == clist->rows) break;
		}

		/* Add entry to clist. */
		text[0] = name;
		text[1] = model;
		text[2] = port;
		text[3] = speed;
		gtk_clist_append (clist, text);
		gtk_clist_set_row_data (clist, clist->rows - 1, GINT_TO_POINTER (unused_id));

		dialog_preferences_cameras_changed ();
		dialog_preferences_update_sensitivity ();
	}
}

void
on_button_camera_update_clicked (GtkButton *button, gpointer user_data)
{
        gchar*		name;
	gchar*		model;
	gchar*		speed;
	gchar*		port;
	GtkWindow*	window;
	gint 		row;
	gchar*		text[4];
	GtkCList*	clist;
	GList*		selection;

	g_assert (button != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);
	g_assert (change_set != NULL);

        /* Check how many cameras have been selected. */
        selection = GTK_CLIST (clist)->selection;
	selection = g_list_first (selection);
        if (selection != NULL) {
		if (g_list_length (selection) == 1) {

	                /* Exactly one camera selected. */
			row = GPOINTER_TO_INT (selection->data);
	
			/* Get the user's entries. */
		        g_assert ((name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")))) != NULL);
		        g_assert ((model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")))) != NULL);
		        g_assert ((speed = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")))) != NULL);;
	        	g_assert ((port = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")))) != NULL);
	
			/* Add entry at previous position to list. */
			text[0] = name;
			text[1] = model;
			text[2] = port;
			text[3] = speed;
			gtk_clist_insert (clist, row, text);
			gtk_clist_set_row_data (clist, row, gtk_clist_get_row_data (clist, row + 1));

			/* Remove the old entry in the list. */
			gtk_clist_remove (clist, row + 1);

			dialog_preferences_cameras_changed ();
			dialog_preferences_update_sensitivity ();
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
	GList*		selection;
	GtkWindow*	window;
	GtkCList*	clist;
	gint		row;

	g_assert (button != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert (change_set != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);

	if ((selection = g_list_first (clist->selection)) != NULL) {
		
		/* Remove the rows in the camera list. */
		while (selection != NULL) {
			row = GPOINTER_TO_INT (selection->data);
			gtk_clist_remove (clist, row);
			selection = g_list_first (clist->selection);
		}

		dialog_preferences_cameras_changed ();
		dialog_preferences_update_sensitivity ();
	} else {
		gnome_error_dialog_parented (_("Please select a camera first!"), window);
	}
}

void
on_dialog_preferences_button_ok_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget*		dialog;

	g_assert (button != NULL);
        g_assert ((dialog = glade_xml_get_widget (xml_preferences, "dialog_preferences")) != NULL);
	g_assert (change_set != NULL);

	dialog_preferences_apply ();

	/* Clean up and exit. */
	xml_preferences = NULL;
	gconf_change_set_unref (change_set);
	change_set = NULL;
	if (revert_change_set) gconf_change_set_unref (revert_change_set);
	revert_change_set = NULL;
	gtk_widget_destroy (dialog);
}

void
on_dialog_preferences_button_revert_clicked (GtkButton *button, gpointer user_data)
{
	dialog_preferences_revert ();
}

void
on_dialog_preferences_button_apply_clicked (GtkButton *button, gpointer user_data)
{
	dialog_preferences_apply ();
}

void
on_dialog_preferences_button_cancel_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget*	dialog;

	g_assert (button != NULL);
	g_assert ((dialog = glade_xml_get_widget (xml_preferences, "dialog_preferences")) != NULL);
	g_assert (change_set != NULL);

	if (revert_change_set) dialog_preferences_revert ();

	/* Clean up and exit. */
	xml_preferences = NULL;
	gconf_change_set_unref (change_set);
	change_set = NULL;
	if (revert_change_set) gconf_change_set_unref (revert_change_set);
	revert_change_set = NULL;
	gtk_widget_destroy (dialog);
}

void
on_entry_prefix_changed (GtkEditable* editable, gpointer user_data)
{
	gchar*		prefix;
	GtkEntry*	entry;

	g_assert (editable);
	g_assert ((entry = GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix"))) != NULL);
	g_assert (change_set);

	/* Did the user really change something? */
	if (gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")), "done")) {

		/* Tell gconf about the change. */
		prefix = gtk_entry_get_text (entry);
		if (*prefix != '0') gconf_change_set_set_string (change_set, "/apps/" PACKAGE "/prefix", prefix);
		else gconf_change_set_unset (change_set, "/apps/" PACKAGE "/prefix");
                        
		dialog_preferences_update_sensitivity ();
	}
}

void
on_clist_cameras_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data)
{
	GList *selection;
	gchar *name, *model, *speed, *port;

	g_return_if_fail (clist);

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
		update_speed_and_port_list (model);

	} else {
		
		/* Clean up the settings. */
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_name")), "");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")), "");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_speed")), "");
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_port")), "");
	}
}

void
on_radiobutton_debug_level_toggled (GtkToggleButton* toggle_button)
{
	gint		debug_level = GP_DEBUG_NONE;

	g_assert (toggle_button != NULL);
        g_assert (change_set != NULL);
	
	/* Which debug level has been selected? */
	if (toggle_button == GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_none"))) debug_level = GP_DEBUG_NONE;
	else if (toggle_button == GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_low"))) debug_level = GP_DEBUG_LOW;
	else if (toggle_button == GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_medium"))) debug_level = GP_DEBUG_MEDIUM;
	else if (toggle_button == GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_high"))) debug_level = GP_DEBUG_HIGH;
	else g_assert_not_reached ();

	/* Did the user really change something? */
        if (gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")), "done")) {

		/* Tell gconf about it. */
		if (toggle_button->active) {
			gconf_change_set_set_int (change_set, "/apps/" PACKAGE "/debug_level", debug_level);
			dialog_preferences_update_sensitivity ();
		}
	}
}

gboolean
on_combo_entry_model_focus_out_event (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	gchar*		model;

	model = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml_preferences, "combo_entry_model")));
	if (strcmp ("", model) != 0) update_speed_and_port_list (model);
	return (FALSE);
}

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/

void
update_speed_and_port_list (gchar* model)
{
	GtkWindow*	window;
	GtkCombo*	combo_speed;
	GtkCombo*	combo_port;
	CameraAbilities	abilities;
	CameraPortInfo	info;
	gint		i, result;
	GList*		list;
	gchar*		tmp;

	g_return_if_fail (model);
        g_return_if_fail (window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences")));
        g_return_if_fail (combo_speed = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_speed")));
        g_return_if_fail (combo_port = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_port")));

        /* User has selected a model. */
        if ((result = gp_camera_abilities_by_name (model, &abilities)) == GP_OK) {

                /* Construct list for speed. */
                i = 0;
		list = g_list_append (NULL, g_strdup (""));
                while (abilities.speed[i] != 0) {
                        list = g_list_append (list, g_strdup_printf ("%i", abilities.speed[i]));
                        i++;
                }
                gtk_combo_set_popdown_strings (combo_speed, list);

                /* Construct list for ports. */
                i = 0;
                list = g_list_append (NULL, g_strdup (""));
                for (i = 0; i < gp_port_count (); i++) {
                        if ((result = gp_port_info (i, &info)) != GP_OK) {
                                tmp = g_strdup_printf (_("Could not get information about port number %i!\n(%s)"), i, gp_result_as_string (result));
                                gnome_error_dialog_parented (tmp, window);
                                g_free (tmp);
                        } else {
                                if (    ((info.type == GP_PORT_SERIAL) && (SERIAL_SUPPORTED (abilities.port))) ||
                                        ((info.type == GP_PORT_PARALLEL) && (PARALLEL_SUPPORTED (abilities.port))) ||
                                        ((info.type == GP_PORT_USB) && (USB_SUPPORTED (abilities.port))) ||
                                        ((info.type == GP_PORT_IEEE1394) && (IEEE1394_SUPPORTED (abilities.port))) ||
                                        ((info.type == GP_PORT_NETWORK) && (NETWORK_SUPPORTED (abilities.port))))
                                        list = g_list_append (list, g_strdup (info.name));
			}
                }
		gtk_combo_set_popdown_strings (combo_port, list);

        } else {
                tmp = g_strdup_printf (_("Could not get abilities for model '%s'!\n(%s)"), model, gp_result_as_string (result));
                gnome_error_dialog_parented (tmp, window);
                g_free (tmp);
        }
}

void
dialog_preferences_prefix_update (gchar* prefix)
{
	GtkEntry*	entry_prefix;

	g_return_if_fail (xml_preferences);
	g_return_if_fail (entry_prefix = GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix")));

	gtk_entry_set_text (entry_prefix, prefix);
}

void
dialog_preferences_debug_level_update (gint debug_level)
{
        GtkToggleButton*        toggle_none;
        GtkToggleButton*        toggle_low;
        GtkToggleButton*        toggle_medium;
        GtkToggleButton*        toggle_high;
	
	g_assert (xml_preferences != NULL);
        g_assert ((toggle_none = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_none"))) != NULL);
        g_assert ((toggle_low = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_low"))) != NULL);
        g_assert ((toggle_medium = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_medium"))) != NULL);
        g_assert ((toggle_high = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_high"))) != NULL);

        switch (debug_level) {
        case GP_DEBUG_NONE:
                gtk_toggle_button_set_active (toggle_none, TRUE);
                break;
        case GP_DEBUG_LOW:
                gtk_toggle_button_set_active (toggle_low, TRUE);
                break;
        case GP_DEBUG_MEDIUM:
                gtk_toggle_button_set_active (toggle_medium, TRUE);
                break;
        case GP_DEBUG_HIGH:
                gtk_toggle_button_set_active (toggle_high, TRUE);
                break;
        default:
                g_assert_not_reached ();
        }
}

void
dialog_preferences_cameras_update (GSList* list_cameras)
{
	xmlDocPtr	doc;
	xmlNodePtr	node;
	GtkCList*	clist;
	GConfValue*	value;
	gchar*		text[4];
	gchar*		xml;
	gchar*		id;
	guint		i;

	g_assert (xml_preferences != NULL);
        g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);

	gtk_clist_clear (clist);
        for (i = 0; i < g_slist_length (list_cameras); i++) {
                value = g_slist_nth_data (list_cameras, i);
                g_assert (value->type = GCONF_VALUE_STRING);
                g_assert ((xml = g_strdup (gconf_value_get_string (value))) != NULL);
                if (!(doc = xmlParseMemory (g_strdup (xml), strlen (xml)))) continue;
                g_assert ((node = xmlDocGetRootElement (doc)) != NULL);

		/* This sanity check only seems to work with libxml2. */
#ifdef GNOCAM_USES_LIBXML2
                if (strcmp (node->name, "Camera") != 0) continue;
#endif
                g_assert ((id = xmlGetProp (node, "ID")) != NULL);
                g_assert ((text[0] = xmlGetProp (node, "Name")) != NULL);
                g_assert ((text[1] = xmlGetProp (node, "Model")) != NULL);
                g_assert ((text[2] = xmlGetProp (node, "Port")) != NULL);
                g_assert ((text[3] = xmlGetProp (node, "Speed")) != NULL);
                gtk_clist_append (clist, text);
		gtk_clist_set_row_data (clist, clist->rows - 1, GINT_TO_POINTER (atoi (id)));
        }
}

void
dialog_preferences_cameras_changed ()
{
	GtkCList*	clist;
	gint		i;
	GSList*		list_cameras = NULL;
	gchar*		name;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gchar*		xml;
	gint		id;

	g_assert (xml_preferences != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);
	g_assert (change_set != NULL);

        /* Build up the camera list for gconf from scratch. */
        for (i = 0; i < clist->rows; i++) {
		id = GPOINTER_TO_INT (gtk_clist_get_row_data (clist, i));
		gtk_clist_get_text (clist, i, 0, &name);
		gtk_clist_get_text (clist, i, 1, &model);
		gtk_clist_get_text (clist, i, 2, &port);
		gtk_clist_get_text (clist, i, 3, &speed);
		xml = g_strdup_printf (
			"<?xml version='1.0'?>"
			"<Camera ID='%i' Name='%s' Model='%s' Port='%s' Speed='%s'/>", id, name, model, port, speed);
		list_cameras = g_slist_append (list_cameras, xml);
        }

        /* Tell gconf about the new values. */
        gconf_change_set_set_list (change_set, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, list_cameras);
}

void
dialog_preferences_populate ()
{
	GConfValue*	value;
	GSList*		list_cameras;

        /* Get gconf's value for prefix. */
        if ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL))) {
                g_assert (value->type == GCONF_VALUE_STRING);
                dialog_preferences_prefix_update ((gchar*) gconf_value_get_string (value));
                gconf_value_free (value);
        }

        /* Get gconf's value for debug level. */
        if ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/debug_level", NULL))) {
                g_assert (value->type == GCONF_VALUE_INT);
                dialog_preferences_debug_level_update (gconf_value_get_int (value));
                gconf_value_free (value);
        }

        /* Get gconf's values for cameras. */
        if ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/cameras", NULL))) {
                g_assert (value->type == GCONF_VALUE_LIST);
                list_cameras = gconf_value_get_list (value);
		g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
		dialog_preferences_cameras_update (list_cameras);
                gconf_value_free (value);
        }
}

void
dialog_preferences_apply ()
{
        GtkObject*      object;

        g_assert (xml_preferences != NULL);
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
        g_assert (change_set != NULL);

	if (!revert_change_set) {

		/* Create the revert changeset on the first apply. */
		revert_change_set = gconf_client_change_set_from_current (
			gconf_client, 
			NULL, 
			"/apps/" PACKAGE "/cameras",
			"/apps/" PACKAGE "/prefix",
			"/apps/" PACKAGE "/debug_level",
			NULL);
	}

	gconf_client_commit_change_set (gconf_client, change_set, TRUE, NULL);

	dialog_preferences_update_sensitivity ();
}

void
dialog_preferences_revert ()
{
	GConfValue*	value = NULL;
        GtkObject*      object;
	GSList*		list_cameras = NULL;
	gchar*		prefix;

        g_return_if_fail (xml_preferences);
        g_return_if_fail (object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")));
        g_return_if_fail (change_set);
	g_return_if_fail (revert_change_set);

        /* When reverting, we want to discard any pending changes. */
        gconf_change_set_clear (change_set);

	/* Revert prefix (if necessary). */
	if (gconf_change_set_check_value (revert_change_set, "/apps/" PACKAGE "/prefix", &value)) {
		if (value) {
			g_assert (value->type == GCONF_VALUE_STRING);
			prefix = (gchar*) gconf_value_get_string (value);
			dialog_preferences_prefix_update (prefix);
		} else dialog_preferences_prefix_update ("");
	} 

	/* Revert debug level (if necessary). */
	if (gconf_change_set_check_value (revert_change_set, "/apps/" PACKAGE "/debug_level", &value)) {
		if (value) {
			g_assert (value->type == GCONF_VALUE_INT);
			dialog_preferences_debug_level_update (gconf_value_get_int (value));
		} else dialog_preferences_debug_level_update (GP_DEBUG_NONE);
	}

	/* Revert camera list (if necessary). */
	if (gconf_change_set_check_value (revert_change_set, "/apps/" PACKAGE "/cameras", &value)) {
		if (value) {
			g_assert (value->type == GCONF_VALUE_LIST);
			list_cameras = gconf_value_get_list (value);
			g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
		} else list_cameras = NULL;
		dialog_preferences_cameras_update (list_cameras);
	}

	gconf_client_commit_change_set (gconf_client, revert_change_set, FALSE, NULL);

	dialog_preferences_update_sensitivity ();
}

void 
dialog_preferences_update_sensitivity ()
{
	GtkWidget* 	apply;
	GtkWidget* 	revert;
	GtkWidget* 	ok;
	GtkWidget* 	cancel;
	GtkObject*	object;

	g_assert (xml_preferences != NULL);
	g_assert (change_set != NULL);
	g_assert ((apply = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_apply")) != NULL);
	g_assert ((cancel = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_cancel")) != NULL);
	g_assert ((ok = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_ok")) != NULL);
	g_assert ((revert = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_revert")) != NULL);
	g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);

	/* Did we change something? */
	gtk_widget_set_sensitive (apply, (gconf_change_set_size (change_set) > 0));

	/* Can we revert something? */
	gtk_widget_set_sensitive (revert, (revert_change_set != NULL));
}

/**
 * preferences:
 *
 * Pops up the preferences dialog.
 **/
void
preferences ()
{
	GList*		list;
	gchar 		buffer[1024];
	gint 		number_of_models;
	gint		i;
	GtkCombo*	combo;
	GtkWindow*	window;
	gchar*		name;

	/* Check if preferences dialog is already open. */
	if (!xml_preferences) {
		g_assert ((xml_preferences = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "dialog_preferences")) != NULL);
		g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);

		change_set = gconf_change_set_new ();

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_preferences);
	
		/* Store some data we need afterwards. */
		gtk_object_set_data (GTK_OBJECT (window), "xml_preferences", xml_preferences);
	
		/* Get the current values from gconf. */
		dialog_preferences_populate ();
		gtk_object_set_data (GTK_OBJECT (window), "done", GINT_TO_POINTER (1));
	
	        /* Build model list. */
		g_assert ((combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_model"))) != NULL);
	        list = NULL;
		if ((number_of_models = gp_camera_count ()) >= 0) {
	                for (i = 0; i < number_of_models; i++) {
	                        if (gp_camera_name (i, buffer) != GP_OK) strcpy (buffer, "?");
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
	
		dialog_preferences_update_sensitivity ();
	}
}


