#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include "preferences.h"
#include "callbacks.h"

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_button_camera_add_clicked 			(GtkButton *button, gpointer user_data);
void on_button_camera_update_clicked 			(GtkButton *button, gpointer user_data);
void on_button_camera_delete_clicked 			(GtkButton *button, gpointer user_data);
void on_button_camera_properties_clicked 		(GtkButton *button, gpointer user_data);

void on_dialog_preferences_button_revert_clicked 	(GtkButton *button, gpointer user_data);
void on_dialog_preferences_button_ok_clicked 		(GtkButton *button, gpointer user_data);
void on_dialog_preferences_button_apply_clicked 	(GtkButton *button, gpointer user_data);
void on_dialog_preferences_button_cancel_clicked 	(GtkButton *button, gpointer user_data);

void on_clist_cameras_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data);

void on_entry_prefix_changed (GtkEditable* editable, gpointer user_data);

gboolean on_combo_entry_model_focus_out_event 	(GtkWidget *widget, GdkEventFocus *event, gpointer user_data);

void on_radiobutton_debug_level_toggled (GtkToggleButton* toggle_button);

void dialog_preferences_debug_level_update 	(GladeXML* xml_preferences, gint debug_level);
void dialog_preferences_prefix_update 		(GladeXML* xml_preferences, gchar *prefix);
void dialog_preferences_cameras_update 		(GladeXML* xml_preferences, GSList* camera_list);
void dialog_preferences_populate 		(GladeXML* xml_preferences);
void dialog_preferences_cameras_changed 	(GladeXML* xml_preferences);
void dialog_preferences_revert 			(GladeXML* xml_preferences);
void dialog_preferences_apply 			(GladeXML* xml_preferences);
void dialog_preferences_update_sensitivity 	(GladeXML* xml_preferences);

/******************************************************************************/
/* Callbacks                                                                  */
/******************************************************************************/

void
on_button_camera_add_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML*	xml_preferences;
	GtkWindow*	window;
	GtkCList*	clist;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gchar*		name;
	gchar*		text[4];
	GConfChangeSet*	change_set;
	gchar*		camera_id;
	gint		i;
	guint		j;

	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (window), "change_set")) != NULL);

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
		for (i = 0; ; i++) {
			for (j = 0; j < clist->rows; j++) {
				if (atoi (gtk_clist_get_row_data (clist, j)) == i) break;
			}
			if (j == clist->rows) break;
		}
		camera_id = g_strdup_printf ("%i", i);

		/* Add entry to clist. */
		text[0] = name;
		text[1] = model;
		text[2] = port;
		text[3] = speed;
		gtk_clist_append (clist, text);
		gtk_clist_set_row_data (clist, clist->rows - 1, camera_id);

		dialog_preferences_cameras_changed (xml_preferences);
		dialog_preferences_update_sensitivity (xml_preferences);
	}
}

void
on_button_camera_update_clicked (GtkButton *button, gpointer user_data)
{
        GladeXML*	xml_preferences;
        gchar*		name;
	gchar*		model;
	gchar*		speed;
	gchar*		port;
	GtkWindow*	window;
	gint 		row;
	gchar*		text[4];
	GtkCList*	clist;
	GConfChangeSet*	change_set;
	GList*		selection;

	g_assert (button != NULL);
        g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (window), "change_set")) != NULL);

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

			dialog_preferences_cameras_changed (xml_preferences);
			dialog_preferences_update_sensitivity (xml_preferences);
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
	GladeXML*	xml_preferences;
	GtkWindow*	window;
	GtkCList*	clist;
	gint		row;
	GConfChangeSet*	change_set;

	g_assert (button != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (window), "change_set")) != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);

	if ((selection = g_list_first (clist->selection)) != NULL) {
		
		/* Remove the rows in the camera list. */
		while (selection != NULL) {
			row = GPOINTER_TO_INT (selection->data);
			g_free (gtk_clist_get_row_data (clist, row));
			gtk_clist_remove (clist, row);
			selection = g_list_first (clist->selection);
		}

		dialog_preferences_cameras_changed (xml_preferences);
		dialog_preferences_update_sensitivity (xml_preferences);
	} else {
		gnome_error_dialog_parented (_("Please select a camera first!"), window);
	}
}

void 
on_button_camera_properties_clicked (GtkButton *button, gpointer user_data)
{
	gnome_error_dialog ("Not yet implemented!");
}

void
on_dialog_preferences_button_ok_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML*	xml_preferences;
	GtkWidget*	dialog;
	GConfChangeSet*	change_set;
	GConfChangeSet*	revert_change_set;

	g_assert (button != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);
        g_assert ((dialog = glade_xml_get_widget (xml_preferences, "dialog_preferences")) != NULL);
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (dialog), "change_set")) != NULL);

	dialog_preferences_apply (xml_preferences);

	/* Clean up and exit. */
	gconf_change_set_unref (change_set);
	if ((revert_change_set = gtk_object_get_data (GTK_OBJECT (dialog), "revert_change_set"))) gconf_change_set_unref (revert_change_set);
	gtk_widget_destroy (dialog);
}

void
on_dialog_preferences_button_revert_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML*	xml_preferences;

	g_assert (button != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);

	dialog_preferences_revert (xml_preferences);
}

void
on_dialog_preferences_button_apply_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML*	xml_preferences;

	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);

	dialog_preferences_apply (xml_preferences);
}

void
on_dialog_preferences_button_cancel_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML*	xml_preferences;
	GtkWidget*	dialog;
	GConfChangeSet* change_set;
	GConfChangeSet* revert_change_set;

	g_assert (button != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (button), "xml_preferences")) != NULL);
	g_assert ((dialog = glade_xml_get_widget (xml_preferences, "dialog_preferences")) != NULL);
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (dialog), "change_set")) != NULL);

	dialog_preferences_revert (xml_preferences);

	/* Clean up and exit. */
	gconf_change_set_unref (change_set);
	if ((revert_change_set = gtk_object_get_data (GTK_OBJECT (dialog), "revert_change_set"))) gconf_change_set_unref (revert_change_set);
	gtk_widget_destroy (dialog);
}

void
on_entry_prefix_changed (GtkEditable* editable, gpointer user_data)
{
	GladeXML*	xml_preferences;
	gchar*		prefix;
	GConfChangeSet*	change_set;
	GtkEntry*	entry;

	g_assert (editable != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (editable), "xml_preferences")) != NULL);
	g_assert ((entry = GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix"))) != NULL);
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")), "change_set")) != NULL);

	/* Did the user really change something? */
	if (gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")), "done")) {

		/* Tell gconf about the change. */
		prefix = gtk_entry_get_text (entry);
		if (*prefix != '0') gconf_change_set_set_string (change_set, "/apps/" PACKAGE "/prefix", prefix);
		else gconf_change_set_unset (change_set, "/apps/" PACKAGE "/prefix");
                        
		dialog_preferences_update_sensitivity (xml_preferences);
	}
}

void
on_clist_cameras_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data)
{
	GList *selection;
	GladeXML *xml_preferences;
	gchar *name, *model, *speed, *port;

	g_assert (clist != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (clist), "xml_preferences")) != NULL); 

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

void
on_radiobutton_debug_level_toggled (GtkToggleButton* toggle_button)
{
	GladeXML*	xml_preferences;
	GConfChangeSet*	change_set;
	gint		debug_level = GP_DEBUG_NONE;

	g_assert (toggle_button != NULL);
	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (toggle_button), "xml_preferences")) != NULL);
        g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")), "change_set")) != NULL);
	
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
			dialog_preferences_update_sensitivity (xml_preferences);
		}
	}
}

gboolean
on_combo_entry_model_focus_out_event (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	GladeXML*	xml_preferences;
	GtkWindow*	window;
	GList*		list;
	GtkCombo*	combo;
	gint 		i;
	gchar*		model;
	CameraAbilities abilities;

	g_assert ((xml_preferences = gtk_object_get_data (GTK_OBJECT (widget), "xml_preferences")) != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert ((combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_speed"))) != NULL);

	/* Clean up the speed list. */
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

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/

void
dialog_preferences_prefix_update (GladeXML *xml_preferences, gchar *prefix)
{
	GtkEntry*	entry_prefix;

	g_assert (xml_preferences != NULL);
	g_assert ((entry_prefix = GTK_ENTRY (glade_xml_get_widget (xml_preferences, "entry_prefix"))) != NULL);

	gtk_entry_set_text (entry_prefix, prefix);
}

void
dialog_preferences_debug_level_update (GladeXML *xml_preferences, gint debug_level)
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
dialog_preferences_cameras_update (GladeXML* xml_preferences, GSList* list_cameras)
{
	GtkCList*	clist;
	GConfValue*	value;
	gchar*		text[4];
	gchar*		camera_description;
	guint		i;
	guint		j;
	gchar*		camera_id;
	guint		count;

	g_assert (xml_preferences != NULL);
        g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);

	gtk_clist_clear (clist);
        for (i = 0; i < g_slist_length (list_cameras); i++) {
                value = g_slist_nth_data (list_cameras, i);
                g_assert (value->type = GCONF_VALUE_STRING);
                camera_description = (gchar*) gconf_value_get_string (value);
		count = 0;
                for (j = 0; camera_description[j] != '\0'; j++) {
			if (camera_description[j] == '\n') {
				camera_description[j] = '\0';
				count++;
			}
		}
		g_assert (count == 4);
		camera_id = g_strdup (camera_description);
		for (j = 0; camera_description[j] != '\0'; j++);
                text[0] = g_strdup (&camera_description[++j]);
                for (; camera_description[j] != '\0'; j++);
                text[1] = g_strdup (&camera_description[++j]);
                for (; camera_description[j] != '\0'; j++);
                text[2] = g_strdup (&camera_description[++j]);
	        for (; camera_description[j] != '\0'; j++);
	        text[3] = g_strdup (&camera_description[++j]);
                gtk_clist_append (clist, text);
		gtk_clist_set_row_data (clist, clist->rows - 1, camera_id);
		for (j = 0; j < 4; j++) g_free (text[j]);
        }
}

void
dialog_preferences_cameras_changed (GladeXML* xml_preferences)
{
	GConfChangeSet*	change_set;
	GtkObject*	object;
	GtkCList*	clist;
	gint		i;
	gchar*		camera_description;
	GSList*		list_cameras = NULL;
	gchar*		name;
	gchar*		model;
	gchar*		port;
	gchar*		speed;
	gchar*		camera_id;

	g_assert (xml_preferences != NULL);
	g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
	g_assert ((clist = GTK_CLIST (glade_xml_get_widget (xml_preferences, "clist_cameras"))) != NULL);
	g_assert ((change_set = gtk_object_get_data (object, "change_set")) != NULL);

        /* Build up the camera list for gconf from scratch. */
        for (i = 0; i < clist->rows; i++) {
		camera_id = gtk_clist_get_row_data (clist, i);
		gtk_clist_get_text (clist, i, 0, &name);
		gtk_clist_get_text (clist, i, 1, &model);
		gtk_clist_get_text (clist, i, 2, &port);
		gtk_clist_get_text (clist, i, 3, &speed);
                camera_description = g_strdup_printf ("%s\n%s\n%s\n%s\n%s", camera_id, name, model, port, speed);
		list_cameras = g_slist_append (list_cameras, camera_description);
        }

        /* Tell gconf about the new values. */
        gconf_change_set_set_list (change_set, "/apps/" PACKAGE "/cameras", GCONF_VALUE_STRING, list_cameras);
}

void
dialog_preferences_populate (GladeXML *xml_preferences)
{
	GConfValue*	value;
	GConfClient*	client;
	GSList*		list_cameras;

	g_assert (xml_preferences != NULL);
	g_assert ((client = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences")), "client")) != NULL);

        /* Get gconf's value for prefix. */
        value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL);
        if (value) {
                g_assert (value->type == GCONF_VALUE_STRING);
                dialog_preferences_prefix_update (xml_preferences, (gchar*) gconf_value_get_string (value));
                gconf_value_free (value);
        }

        /* Get gconf's value for debug level. */
        value = gconf_client_get (client, "/apps/" PACKAGE "/debug_level", NULL);
        if (value) {
                g_assert (value->type == GCONF_VALUE_INT);
                dialog_preferences_debug_level_update (xml_preferences, gconf_value_get_int (value));
                gconf_value_free (value);
        }

        /* Get gconf's values for cameras. */
        value = gconf_client_get (client, "/apps/" PACKAGE "/cameras", NULL);
        if (value) {
                g_assert (value->type == GCONF_VALUE_LIST);
                list_cameras = gconf_value_get_list (value);
		g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
		dialog_preferences_cameras_update (xml_preferences, list_cameras);
                gconf_value_free (value);
        }
}

void
dialog_preferences_apply (GladeXML *xml_preferences)
{
        GConfChangeSet* change_set;
	GConfChangeSet* revert_change_set;
        GConfClient*    client;
        GtkObject*      object;

        g_assert (xml_preferences != NULL);
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
        g_assert ((client = gtk_object_get_data (object, "client")) != NULL);
        g_assert ((change_set = gtk_object_get_data (object, "change_set")) != NULL);

	if (!(revert_change_set = gtk_object_get_data (object, "revert_change_set"))) {

		/* Create the revert changeset on the first apply. */
//FIXME: Revert does not work with revert_change_set. The camera list gets totally mixed up. I have no idea why...
//		revert_change_set = gconf_client_change_set_from_current (
//			client, 
//			NULL, 
//			"/apps/" PACKAGE "/cameras",
//			"/apps/" PACKAGE "/prefix",
//			"/apps/" PACKAGE "/debug_level",
//			NULL);
//		gtk_object_set_data (object, "revert_change_set", revert_change_set);
	}

	gconf_client_commit_change_set (client, change_set, TRUE, NULL);

	dialog_preferences_update_sensitivity (xml_preferences);
}

void
dialog_preferences_revert (GladeXML *xml_preferences)
{
        GConfChangeSet* change_set;
        GConfChangeSet* revert_change_set;
	GConfValue*	value = NULL;
        GConfClient*    client;
        GtkObject*      object;
	GSList*		list_cameras = NULL;
	gchar*		prefix;

        g_assert (xml_preferences != NULL);
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);
        g_assert ((client = gtk_object_get_data (object, "client")) != NULL);
        g_assert ((change_set = gtk_object_get_data (object, "change_set")) != NULL);

        /* Do we have something to revert? */
	if ((revert_change_set = gtk_object_get_data (object, "revert_change_set"))) {

                /* When reverting, we want to discard any pending changes. */
                gconf_change_set_clear (change_set);

		/* Revert prefix (if necessary). */
		if (gconf_change_set_check_value (revert_change_set, "/apps/" PACKAGE "/prefix", &value)) {
			if (value) {
				g_assert (value->type == GCONF_VALUE_STRING);
				prefix = (gchar*) gconf_value_get_string (value);
				dialog_preferences_prefix_update (xml_preferences, prefix);
			} else {
				dialog_preferences_prefix_update (xml_preferences, "");
			}
		} 

		/* Revert debug level (if necessary). */
		if (gconf_change_set_check_value (revert_change_set, "/apps/" PACKAGE "/debug_level", &value)) {
			if (value) {
				g_assert (value->type == GCONF_VALUE_INT);
				dialog_preferences_debug_level_update (xml_preferences, gconf_value_get_int (value));
			} else {
				dialog_preferences_debug_level_update (xml_preferences, GP_DEBUG_NONE);
			}
		}

		/* Revert camera list (if necessary). */
		if (gconf_change_set_check_value (revert_change_set, "/apps/" PACKAGE "/cameras", &value)) {
			if (value) {
				g_assert (value->type == GCONF_VALUE_LIST);
				list_cameras = gconf_value_get_list (value);
				g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
			} else {
				list_cameras = NULL;
			}
			dialog_preferences_cameras_update (xml_preferences, list_cameras);
		}

		gconf_client_commit_change_set (client, revert_change_set, FALSE, NULL);

		dialog_preferences_update_sensitivity (xml_preferences);
	}
}

void 
dialog_preferences_update_sensitivity (GladeXML *xml_preferences)
{
	GtkWidget* 	apply;
	GtkWidget* 	revert;
	GtkWidget* 	ok;
	GtkWidget* 	cancel;
	GtkObject*	object;
	GConfChangeSet*	change_set;
	GConfChangeSet*	revert_change_set;

	g_assert (xml_preferences != NULL);
	g_assert ((apply = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_apply")) != NULL);
	g_assert ((cancel = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_cancel")) != NULL);
	g_assert ((ok = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_ok")) != NULL);
	g_assert ((revert = glade_xml_get_widget (xml_preferences, "dialog_preferences_button_revert")) != NULL);
	g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);

	/* Did we change something? */
	g_assert ((change_set = gtk_object_get_data (GTK_OBJECT (object), "change_set")) != NULL);
	gtk_widget_set_sensitive (apply, (gconf_change_set_size (change_set) > 0));

	/* Can we revert something? */
	revert_change_set = gtk_object_get_data (GTK_OBJECT (object), "revert_change_set");
	gtk_widget_set_sensitive (revert, (revert_change_set != NULL));
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
	GnomeApp*	app;
	GladeXML*	xml_preferences;
	GList*		list;
	gchar 		buffer[1024];
	gint 		number_of_models;
	gint		number_of_ports;
	gint		i;
	GtkCombo*	combo;
	GtkWindow*	window;
	gchar*		name;
	CameraPortInfo 	info;
	GConfClient*	client = NULL;
	GConfChangeSet*	change_set = NULL;

	g_assert (xml != NULL);
	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);
	g_assert ((client = gtk_object_get_data (GTK_OBJECT (app), "client")) != NULL);
	g_assert ((xml_preferences = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "dialog_preferences")) != NULL);
	g_assert ((window = GTK_WINDOW (glade_xml_get_widget (xml_preferences, "dialog_preferences"))) != NULL);

	change_set = gconf_change_set_new ();

	/* Connect the signals. */
	glade_xml_signal_autoconnect (xml_preferences);

	/* Store some data we need afterwards. */
	gtk_object_set_data (GTK_OBJECT (window), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (window), "client", client);
	gtk_object_set_data (GTK_OBJECT (window), "change_set", change_set);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "entry_prefix")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "clist_cameras")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "combo_entry_model")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_add")), "xml_preferences", xml_preferences);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_update")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_delete")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "button_camera_properties")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences_button_apply")), "xml_preferences", xml_preferences);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences_button_revert")), "xml_preferences", xml_preferences);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences_button_cancel")), "xml_preferences", xml_preferences);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "dialog_preferences_button_ok")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_none")), "xml_preferences", xml_preferences);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_low")), "xml_preferences", xml_preferences);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_medium")), "xml_preferences", xml_preferences);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preferences, "radiobutton_debug_level_high")), "xml_preferences", xml_preferences);

	/* Get the current values from gconf. */
	dialog_preferences_populate (xml_preferences);
	gtk_object_set_data (GTK_OBJECT (window), "done", GINT_TO_POINTER (1));

        /* Build model list. */
	g_assert ((combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_model"))) != NULL);
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
	g_assert ((combo = GTK_COMBO (glade_xml_get_widget (xml_preferences, "combo_port"))) != NULL);
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

	dialog_preferences_update_sensitivity (xml_preferences);
}


