#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "properties.h"

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_entry_changed (GtkEditable *editable, gpointer user_data);
void on_adjustment_value_changed (GtkAdjustment *adjustment, gpointer user_data);
void on_radiobutton_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_propertybox_apply (GnomePropertyBox *propertybox, gint arg, gpointer user_data);
gint on_propertybox_close (GnomeDialog *dialog, gpointer user_data);

gboolean setting_get (GnomePropertyBox *propertybox, CameraSetting *setting, CameraWidget *camera_widget);
gboolean camera_properties_set (GnomePropertyBox *propertybox);
void page_entry_new (GtkWidget *vbox, CameraWidget *camera_widget);
GtkWidget *page_new (GnomePropertyBox *propertybox, CameraWidget *camera_widget);

/******************************************************************************/
/* Callbacks                                                                  */
/******************************************************************************/

void
on_entry_changed (GtkEditable *editable, gpointer user_data)
{
        gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (editable), "propertybox")));
}

void
on_adjustment_value_changed (GtkAdjustment *adjustment, gpointer user_data)
{
	gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (adjustment), "propertybox")));
}
void
on_radiobutton_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	GnomePropertyBox *propertybox;
	gchar *done;

	propertybox = GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (togglebutton), "propertybox"));
	done = gtk_object_get_data (GTK_OBJECT (propertybox), "done");

	/* Make sure the user toggled the button. */
	if (strcmp (done, "yes") == 0) 
		gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (togglebutton), "propertybox")));
}

void on_propertybox_apply (GnomePropertyBox *propertybox, gint arg, gpointer user_data)
{
        if (!camera_properties_set (propertybox)) {
                gnome_error_dialog_parented (_("Could not set camera properties!"), GTK_WINDOW (propertybox));
		gnome_property_box_changed (propertybox);
	}
}

gint on_propertybox_close (GnomeDialog *dialog, gpointer user_data)
{
        return (FALSE);
}

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/

gboolean
setting_get (GnomePropertyBox *propertybox, CameraSetting *setting, CameraWidget *camera_widget)
{
	GtkObject *object;
	GtkWidget *preference_widget;
	guint k;

	g_assert (setting != NULL);
	g_assert (camera_widget != NULL);
	g_assert (gp_widget_type (camera_widget) != GP_WIDGET_WINDOW);
	g_assert (gp_widget_type (camera_widget) != GP_WIDGET_SECTION);
	object = GTK_OBJECT (propertybox);
	g_assert (object != NULL);
	
        switch (gp_widget_type (camera_widget)) {
        case GP_WIDGET_TEXT:
                strcpy (setting->name, gp_widget_label (camera_widget));
                preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget));
                strcpy (setting->value, gtk_entry_get_text (GTK_ENTRY (preference_widget)));
                break;
        case GP_WIDGET_RANGE:
                strcpy (setting->name, gp_widget_label (camera_widget));
                preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget));
                strcpy (setting->value, g_strdup_printf ("%f", gtk_range_get_adjustment (GTK_RANGE (preference_widget))->value));
                break;
        case GP_WIDGET_TOGGLE:
                strcpy (setting->name, gp_widget_label (camera_widget));
                preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget));
                strcpy (setting->value, g_strdup_printf ("%i", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (preference_widget))));
                break;
        case GP_WIDGET_RADIO:
                strcpy (setting->name, gp_widget_label (camera_widget));
                for (k = 0; k < gp_widget_choice_count (camera_widget); k++) {
                        preference_widget = gtk_object_get_data (object, gp_widget_choice (camera_widget, k));
                        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (preference_widget)))
                                strcpy (setting->value, gp_widget_choice (camera_widget, k));
                }
                break;
        case GP_WIDGET_MENU:
                strcpy (setting->name, gp_widget_label (camera_widget));
                preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget));
                strcpy (setting->value, gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (preference_widget)->entry)));
                break;
        case GP_WIDGET_BUTTON:
                g_warning ("GP_WIDGET_BUTTON not implemented!");
		return (FALSE);
        default:
                g_assert_not_reached ();
        }
	return (TRUE);
}

gboolean
camera_properties_set (GnomePropertyBox *propertybox)
{
	CameraSetting setting[128];
	gint setting_count;
	Camera *camera;
	CameraWidget *camera_widget;
	CameraWidget *camera_widget_child;
	CameraWidget *widget;
	guint i, j;

	camera = gtk_object_get_data (GTK_OBJECT (propertybox), "camera");
	g_assert (camera != NULL);
	camera_widget = gtk_object_get_data (GTK_OBJECT (propertybox), "camera_widget");
	g_assert (camera_widget != NULL);
	g_assert (gp_widget_type (camera_widget) == GP_WIDGET_WINDOW);

	setting_count = 0;
        for (i = 0; i < gp_widget_child_count (camera_widget); i++) {
                camera_widget_child = gp_widget_child (camera_widget, i);
		if (gp_widget_type (camera_widget_child) == GP_WIDGET_SECTION) {
			for (j = 0; j < gp_widget_child_count (camera_widget_child); j++) {
				if (setting_count == 128) g_error ("setting_count too high!");
				widget = gp_widget_child (camera_widget_child, j);
				if (setting_get (propertybox, &setting[setting_count], widget)) setting_count++;
			}
		} else {
			
			/* This is an orphan. */
			if (setting_get (propertybox, &setting[setting_count], camera_widget_child)) setting_count++;
		}
	}
	return (gp_camera_config_set (camera, setting, setting_count) == GP_OK);
}

void 
page_entry_new (GtkWidget *vbox, CameraWidget *camera_widget)
{
	GtkWidget *frame;
	GtkWidget *entry;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *combo;
	GtkWidget *hscale;
	GtkAdjustment *adjustment;
	GSList *list;
	GList *combo_items;
	guint j, k;
	gfloat min, max, increment;
	GnomePropertyBox *propertybox;

	g_assert (vbox != NULL);
	g_assert (camera_widget != NULL);
	propertybox = GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (vbox), "propertybox"));

	/* Create the frame. */
	frame = gtk_frame_new (gp_widget_label (camera_widget));
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	/* Create the widget. */
	switch (gp_widget_type (camera_widget)) {
	case GP_WIDGET_TEXT:
		entry = gtk_entry_new ();
		gtk_entry_set_text (GTK_ENTRY (entry), gp_widget_value_get (camera_widget));
		gtk_container_add (GTK_CONTAINER (frame), entry);

		/* Connect the signals. */
		gtk_signal_connect_object (GTK_OBJECT (entry), "changed", GTK_SIGNAL_FUNC (on_entry_changed), (gpointer) entry);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (entry), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), entry);

		break;
	case GP_WIDGET_RANGE:
		gp_widget_range_get (camera_widget, &min, &max, &increment);
		adjustment = GTK_ADJUSTMENT (gtk_adjustment_new (atof (gp_widget_value_get (camera_widget)), min, max, increment, 0, 0));
		hscale = gtk_hscale_new (adjustment);
		gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DISCONTINUOUS);
		gtk_container_add (GTK_CONTAINER (frame), hscale);

		/* Connect the signals. */
		gtk_signal_connect_object (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_adjustment_value_changed), (gpointer) adjustment);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (adjustment), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), hscale);

		break;
	case GP_WIDGET_TOGGLE:
		button = gtk_check_button_new_with_label (gp_widget_label (camera_widget));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), atoi (gp_widget_value_get (camera_widget)) != 0);
		gtk_container_add (GTK_CONTAINER (frame), button);

		/* Connect the signals. */
		gtk_signal_connect_object (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (on_radiobutton_toggled), (gpointer) button);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (button), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), button);
	
		break;
	case GP_WIDGET_RADIO:
                hbox = gtk_hbox_new (FALSE, 10);
       	        gtk_container_add (GTK_CONTAINER (frame), hbox);
		list = NULL;
		for (k = 0; k < gp_widget_choice_count (camera_widget); k++) {
			button = gtk_radio_button_new_with_label (list, gp_widget_choice (camera_widget, k));
			list = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
			if (strcmp (gp_widget_value_get (camera_widget), gp_widget_choice (camera_widget, k)) == 0)
       				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                        gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

			/* Connect the signals. */
			gtk_signal_connect_object (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (on_radiobutton_toggled), (gpointer) button);
	
			/* Store some data. */
			gtk_object_set_data (GTK_OBJECT (button), "propertybox", propertybox);
			gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_choice (camera_widget, k), button);
			gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), button);
		}
		break;
	case GP_WIDGET_MENU:
		combo = gtk_combo_new ();
		gtk_container_add (GTK_CONTAINER (frame), combo);
		combo_items = NULL;
		for (j = 0; j < gp_widget_choice_count (camera_widget); j++) {
			combo_items = g_list_append (combo_items, g_strdup (gp_widget_choice (camera_widget, j)));
		}
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), combo_items);
		g_list_free (combo_items);
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), gp_widget_value_get (camera_widget));
		gtk_container_add (GTK_CONTAINER (frame), combo);

		/* Connect the signals. */
		gtk_signal_connect_object (
			GTK_OBJECT (GTK_COMBO (combo)->entry), 
			"changed", 
			GTK_SIGNAL_FUNC (on_entry_changed), 
			(gpointer) (GTK_COMBO (combo)->entry));

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (GTK_COMBO (combo)->entry), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), combo);
		
		break;
	case GP_WIDGET_BUTTON:
		g_warning ("GP_WIDGET_BUTTON not implemented!");
		break;
	default:
		g_assert_not_reached ();
	}
}

GtkWidget *
page_new (GnomePropertyBox *propertybox, CameraWidget *camera_widget)
{
	GtkWidget *vbox, *label;
	guint i;

	if (camera_widget != NULL) g_assert (gp_widget_type (camera_widget) == GP_WIDGET_SECTION);

	/* Set up the page for this section. */
	if (camera_widget == NULL) label = gtk_label_new ("Other");
	else label = gtk_label_new (gp_widget_label (camera_widget));
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
	gnome_property_box_append_page (propertybox, vbox, label);

	/* Store some data for later use. */
	gtk_object_set_data (GTK_OBJECT (vbox), "propertybox", propertybox);
	
	if (camera_widget != NULL) {

		/* Populate the page. */
		for (i = 0; i < gp_widget_child_count (camera_widget); i++) {
			page_entry_new (vbox, gp_widget_child (camera_widget, i));
		}
	} 
	return (vbox);
}

/**
 * dialog_camera_preferences:
 * @camera: The camera of which we want to set the properties
 * @name: The name of that camera.
 *
 * This function pops up a dialog that lets the user change the properties
 * of a camera.
 */
void
camera_properties (Camera *camera, gchar *camera_name)
{
	GnomePropertyBox *propertybox;
	GtkWidget *vbox_for_orphans = NULL;
	CameraWidget *camera_widget, *camera_widget_child;
	gint i;
	gboolean orphans = FALSE;
	gchar *done;

	g_assert (camera != NULL);
	g_assert (camera_name != NULL);

	/* Get the properties from the gphoto2 backend. */
	camera_widget = gp_widget_new (GP_WIDGET_WINDOW, camera_name);
	g_assert (camera_widget != NULL);
	if (gp_camera_config_get (camera, camera_widget) == GP_OK) {
		/* Create the propertybox */
		propertybox = GNOME_PROPERTY_BOX (gnome_property_box_new ());
		gtk_window_set_title (GTK_WINDOW (propertybox), camera_name);

		/* Store some data for later use. */
		gtk_object_set_data (GTK_OBJECT (propertybox), "done", g_strdup ("no"));
		gtk_object_set_data (GTK_OBJECT (propertybox), "camera_widget", camera_widget);
		gtk_object_set_data (GTK_OBJECT (propertybox), "camera", camera);

		/* Populate the propertybox. */
		for (i = 0; i < gp_widget_child_count (camera_widget); i++) {
			camera_widget_child = gp_widget_child (camera_widget, i);
			if (gp_widget_type (camera_widget_child) == GP_WIDGET_SECTION) {

				/* Set up the page for this section. */
				page_new (propertybox, camera_widget_child);
			} else {
				if (!orphans) {
				
					/* Set up a section for orphan widgets. */
					vbox_for_orphans = page_new (propertybox, NULL);
				}
				page_entry_new (vbox_for_orphans, camera_widget_child);
			}
		}

		/* Connect the signals. */
		gtk_signal_connect (GTK_OBJECT (propertybox), "apply", GTK_SIGNAL_FUNC (on_propertybox_apply), NULL);
		gtk_signal_connect (GTK_OBJECT (propertybox), "close", GTK_SIGNAL_FUNC (on_propertybox_close), NULL);

		/* Show the propertybox. Clean up. */
		gtk_widget_show_all (GTK_WIDGET (propertybox));
		done = gtk_object_get_data (GTK_OBJECT (propertybox), "done");
		gtk_object_set_data (GTK_OBJECT (propertybox), "done", g_strdup ("yes"));
		g_free (done);
	} else {
		g_free (camera_widget);
		gnome_error_dialog (_("Could not get camera properties!"));
	}
}


		
