#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <libxml/parser.h>
#include "properties.h"
#include "gnocam.h"
#include "gphoto-extensions.h"

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_entry_changed (GtkEditable *editable, gpointer user_data);
void on_adjustment_value_changed (GtkAdjustment *adjustment, gpointer user_data);
void on_radiobutton_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_properties_apply (GnomePropertyBox *propertybox, gint arg, gpointer user_data);
gint on_properties_close (GnomeDialog *dialog, gpointer user_data);

void on_date_changed (GnomeDateEdit* gnomedateedit, gpointer user_data);

gboolean setting_get (GnomePropertyBox *propertybox, CameraSetting *setting, CameraWidget *camera_widget);
void page_entry_new (GtkWidget *vbox, CameraWidget *camera_widget);
GtkWidget *page_new (GnomePropertyBox *propertybox, CameraWidget *camera_widget);

/******************************************************************************/
/* Callbacks                                                                  */
/******************************************************************************/

void 
on_date_changed (GnomeDateEdit* gnomedateedit, gpointer user_data)
{
	gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (gnomedateedit), "propertybox")));
}

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

void on_properties_apply (GnomePropertyBox *propertybox, gint arg, gpointer user_data)
{
	Camera*		camera;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (propertybox), "camera")) != NULL);

	if (gp_camera_config (camera) != GP_OK) {
                gnome_error_dialog_parented (_("Could not set camera properties!"), GTK_WINDOW (propertybox));
		gnome_property_box_changed (propertybox);
	}
}

gint on_properties_close (GnomeDialog *dialog, gpointer user_data)
{
	frontend_data_t*	frontend_data;
	Camera*			camera;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (dialog), "camera")) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	/* Clean up. */
	gp_widget_unref (gtk_object_get_data (GTK_OBJECT (dialog), "window"));
	gp_camera_unref (camera);
	frontend_data->xml_properties = NULL;

        return (FALSE);
}

/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/

void
values_set (GladeXML* xml_properties, CameraWidget *camera_widget)
{
	GtkObject*	object;
	GtkWidget*	preference_widget;
	GtkWidget*	choice_widget;
	guint 		k;
	gint 		i, i_new;
	gfloat		f, f_new;
	gchar*		c;
	gchar*		c_new;

	g_assert (xml_properties != NULL);
	g_assert (camera_widget != NULL);
	g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_properties, "properties"))) != NULL);

	c = g_new (gchar, 256);
        switch (gp_widget_type (camera_widget)) {
        case GP_WIDGET_TEXT:
		gp_widget_value_get (camera_widget, c);
                if ((preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget))) == NULL) break;
		c_new = gtk_entry_get_text (GTK_ENTRY (preference_widget));
		if (strcmp (c, c_new) != 0) 
	                gp_widget_value_set (camera_widget, c_new);
                break;
        case GP_WIDGET_RANGE:
		gp_widget_value_get (camera_widget, &f);
                if ((preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget))) == NULL) break;
		f_new = gtk_range_get_adjustment (GTK_RANGE (preference_widget))->value;
		if (f != f_new) gp_widget_value_set (camera_widget, &f_new);
		break;
        case GP_WIDGET_TOGGLE:
		gp_widget_value_get (camera_widget, &i);
                if ((preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget))) == NULL) break;
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (preference_widget))) i_new = 1;
		else i_new = 0;
		if (i != i_new) gp_widget_value_set (camera_widget, &i_new);
                break;
        case GP_WIDGET_RADIO:
		gp_widget_value_get (camera_widget, c);
		if ((preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget))) == NULL) break;
                for (k = 0; k < gp_widget_choice_count (camera_widget); k++) {
                        if ((choice_widget = gtk_object_get_data (GTK_OBJECT (preference_widget), gp_widget_choice (camera_widget, k))) == NULL) break;
                        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (choice_widget))) {
				c_new = gp_widget_choice (camera_widget, k);
				if (strcmp (c_new, c) != 0) {
	                                gp_widget_value_set (camera_widget, c_new);
				}
			}
                }
                break;
        case GP_WIDGET_MENU:
		gp_widget_value_get (camera_widget, c);
                if ((preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget))) == NULL) break;
		c_new = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (preference_widget)->entry));
		if (strcmp (c, c_new) != 0) gp_widget_value_set (camera_widget, c_new);
                break;
        case GP_WIDGET_BUTTON:
		break;
	case GP_WIDGET_DATE:
		gp_widget_value_get (camera_widget, &i);
		if ((preference_widget = gtk_object_get_data (object, gp_widget_label (camera_widget))) == NULL) break;
		i_new = (int) gnome_date_edit_get_date (GNOME_DATE_EDIT (preference_widget));
		if (i_new != i) gp_widget_value_set (camera_widget, &i_new);
		break;
	case GP_WIDGET_WINDOW:
	        for (i = 0; i < gp_widget_child_count (camera_widget); i++) values_set (xml_properties, gp_widget_child (camera_widget, i));
		break;
	case GP_WIDGET_SECTION:
		for (i = 0; i < gp_widget_child_count (camera_widget); i++) values_set (xml_properties, gp_widget_child (camera_widget, i));
                break;
        default:
                g_assert_not_reached ();
        }
	g_free (c);
}

void 
page_entry_new (GtkWidget *vbox, CameraWidget *camera_widget)
{
	GtkWidget*	frame;
	GtkWidget*	widget;
	GtkWidget*	button;
	GtkWidget*	hbox;
	GtkWidget*	combo;
	GtkWidget*	hscale;
	GtkAdjustment*	adjustment;
	GSList*		list;
	GList*		combo_items;
	guint 		j, k;
	gfloat 			min, max, increment;
	GnomePropertyBox*	propertybox;
	void*			value = NULL;

	g_assert (vbox != NULL);
	g_assert (camera_widget != NULL);
	propertybox = GNOME_PROPERTY_BOX (gtk_object_get_data (GTK_OBJECT (vbox), "propertybox"));

	/* Create the frame. */
	frame = gtk_frame_new (gp_widget_label (camera_widget));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	/* Create the widget. */
	gp_widget_value_get (camera_widget, value);
	switch (gp_widget_type (camera_widget)) {
	case GP_WIDGET_TEXT:
		widget = gtk_entry_new ();
		gtk_widget_show (widget);
		gtk_entry_set_text (GTK_ENTRY (widget), (gchar*) value);
		gtk_container_add (GTK_CONTAINER (frame), widget);

		/* Connect the signals. */
		gtk_signal_connect_object (GTK_OBJECT (widget), "changed", GTK_SIGNAL_FUNC (on_entry_changed), (gpointer) widget);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (widget), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), widget);

		break;
	case GP_WIDGET_RANGE:
		gp_widget_range_get (camera_widget, &min, &max, &increment);
		adjustment = GTK_ADJUSTMENT (gtk_adjustment_new (*((gfloat*) value), min, max, increment, 0, 0));
		hscale = gtk_hscale_new (adjustment);
		gtk_widget_show (hscale);
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
		gtk_widget_show (button);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ((*((gint*) value)) == 1));
		gtk_container_add (GTK_CONTAINER (frame), button);

		/* Connect the signals. */
		gtk_signal_connect_object (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (on_radiobutton_toggled), (gpointer) button);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (button), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), button);
	
		break;
	case GP_WIDGET_RADIO:
                hbox = gtk_hbox_new (FALSE, 10);
		gtk_widget_show (hbox);
       	        gtk_container_add (GTK_CONTAINER (frame), hbox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), hbox);
		list = NULL;
		for (k = 0; k < gp_widget_choice_count (camera_widget); k++) {
			button = gtk_radio_button_new_with_label (list, gp_widget_choice (camera_widget, k));
			gtk_widget_show (button);
			list = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
			if (strcmp ((gchar*) value, gp_widget_choice (camera_widget, k)) == 0)
       				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
                        gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

			/* Connect the signals. */
			gtk_signal_connect_object (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (on_radiobutton_toggled), (gpointer) button);
	
			/* Store some data. */
			gtk_object_set_data (GTK_OBJECT (button), "propertybox", propertybox);
			gtk_object_set_data (GTK_OBJECT (hbox), gp_widget_choice (camera_widget, k), button);
		}
		break;
	case GP_WIDGET_MENU:
		combo = gtk_combo_new ();
		gtk_widget_show (combo);
		gtk_container_add (GTK_CONTAINER (frame), combo);
		combo_items = NULL;
		for (j = 0; j < gp_widget_choice_count (camera_widget); j++) {
			combo_items = g_list_append (combo_items, g_strdup (gp_widget_choice (camera_widget, j)));
		}
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), combo_items);
		g_list_free (combo_items);
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), (gchar*) value);
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
	case GP_WIDGET_DATE:
		hbox = gtk_hbox_new (FALSE, 10);
		gtk_widget_show (hbox);
		gtk_container_add (GTK_CONTAINER (frame), hbox);

		/* Create the clock. */
//FIXME: Can I set the date and time of a gtk-clock?
//		widget = gtk_clock_new (GTK_CLOCK_INCREASING);
//		gtk_widget_show (widget);
//		gtk_clock_set_format (GTK_CLOCK (widget), _("%Y/%m/%d %H:%M:%S"));
//		gtk_clock_start (GTK_CLOCK (widget));
//		gtk_container_add (GTK_CONTAINER (hbox), widget);

		/* Create the widget to edit the date. */
		widget = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
		gnome_date_edit_set_time (GNOME_DATE_EDIT (widget), *((time_t*) value));
		gtk_widget_show (widget);
		gtk_container_add (GTK_CONTAINER (hbox), widget);

		/* Connect the signals. */
		gtk_signal_connect_object (GTK_OBJECT (widget), "date_changed", GTK_SIGNAL_FUNC (on_date_changed), (gpointer) widget);
		gtk_signal_connect_object (GTK_OBJECT (widget), "time_changed", GTK_SIGNAL_FUNC (on_date_changed), (gpointer) widget);
		
		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (widget), "propertybox", propertybox);
		gtk_object_set_data (GTK_OBJECT (propertybox), gp_widget_label (camera_widget), widget);
		break;
	default:
		g_assert_not_reached ();
	}
}

GtkWidget*
page_new (GnomePropertyBox *propertybox, CameraWidget *camera_widget)
{
	GtkWidget *vbox, *label;
	guint i;

	if (camera_widget != NULL) g_assert (gp_widget_type (camera_widget) == GP_WIDGET_SECTION);

	/* Set up the page for this section. */
	if (camera_widget == NULL) label = gtk_label_new ("Other");
	else label = gtk_label_new (gp_widget_label (camera_widget));
	gtk_widget_show (label);
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
	gtk_widget_show (vbox);
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

void
camera_properties (GladeXML* xml, Camera* camera, CameraWidget* window)
{
	GladeXML*		xml_properties;
	GnomePropertyBox*	propertybox;
	GtkWidget*		vbox_for_orphans = NULL;
	CameraWidget*		camera_widget;
	gint 			i;
	gboolean 		orphans = FALSE;
	GnomeApp*		app;
	frontend_data_t*	frontend_data;

	g_assert (camera != NULL);
	g_assert (window != NULL);
	g_assert (gp_widget_type (window) == GP_WIDGET_WINDOW);
	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);
	gp_widget_ref (window);

	/* Create the propertybox */
	g_assert ((xml_properties = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "properties")) != NULL);
	frontend_data->xml_properties = xml_properties;
	propertybox = GNOME_PROPERTY_BOX (glade_xml_get_widget (xml_properties, "properties"));
	gtk_window_set_title (GTK_WINDOW (propertybox), frontend_data->name);

	/* Store some data for later use. */
	gtk_object_set_data (GTK_OBJECT (propertybox), "done", g_strdup ("no"));
	gtk_object_set_data (GTK_OBJECT (propertybox), "window", window);
	gtk_object_set_data (GTK_OBJECT (propertybox), "camera", camera);

	/* Populate the propertybox. */
	for (i = 0; i < gp_widget_child_count (window); i++) {
		camera_widget = gp_widget_child (window, i);
		if (gp_widget_type (camera_widget) == GP_WIDGET_SECTION) {

			/* Set up the page for this section. */
			page_new (propertybox, camera_widget);
		} else {
			if (!orphans) {
			
				/* Set up a section for orphan widgets. */
				vbox_for_orphans = page_new (propertybox, NULL);
			}
			page_entry_new (vbox_for_orphans, camera_widget);
		}
	}

	/* Connect the signals. */
	glade_xml_signal_autoconnect (xml_properties);

	/* Show the propertybox. Clean up. */
	g_free (gtk_object_get_data (GTK_OBJECT (propertybox), "done"));
	gtk_object_set_data (GTK_OBJECT (propertybox), "done", g_strdup ("yes"));
}

		
