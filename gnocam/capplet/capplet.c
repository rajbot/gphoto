#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <capplet-widget.h>
#include <gphoto2/gphoto2.h>
#include "capplet-callbacks.h"
#include "capplet.h"


static GtkWidget *capplet;


int gp_interface_status (Camera *camera, char *status) 
{
	//FIXME: Yet to come...
	return (0);
}


int gp_interface_progress (Camera *camera, CameraFile *file, float percentage)
{
	//FIXME: Yet to come...
	return (0);
}

int gp_interface_message (Camera *camera, char *message)
{
	//FIXME: Do that better...
	gnome_error_dialog (message);
	return (0);
}

int gp_interface_confirm (Camera *camera, char *message)
{
	//FIXME: Yet to come...
        return (0);
}



GtkWidget *
camera_capplet_current (void)
{
	return (capplet);
}

GtkWidget *
camera_capplet_new (void)
{
        GtkWidget *table, *scrolledwindow, *clist, *label, *frame;
	GtkWidget *entry_name, *entry_model, *entry_speed, *entry_port;
        GtkWidget *combo_model, *combo_port, *combo_speed;
        GtkWidget *vbuttonbox, *button_camera_add, *button_camera_update, *button_camera_delete, *button_camera_properties;
	gint number_of_models, number_of_ports, i;
	CameraPortInfo port_info;
	gchar buffer[1024];
	GList *list;

        /* Set up the capplet. */
        capplet = capplet_widget_new ();

        /* Add the table. */
        table = gtk_table_new (2, 2, FALSE);
        gtk_table_set_row_spacings (GTK_TABLE (table), 5);
        gtk_table_set_col_spacings (GTK_TABLE (table), 5);
        gtk_container_add (GTK_CONTAINER (capplet), table);

        /* Add the scrolled window for the clist. */
        scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_table_attach (GTK_TABLE (table), scrolledwindow, 0, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        /* Add the clist. */
        clist = gtk_clist_new (4);
        gtk_container_add (GTK_CONTAINER (scrolledwindow), clist);
        gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_MULTIPLE);
        gtk_clist_column_titles_show (GTK_CLIST (clist));

        /* Add the headings. */
        label = gtk_label_new (_("Name"));
        gtk_clist_set_column_widget (GTK_CLIST (clist), 0, label);
        label = gtk_label_new (_("Model"));
        gtk_clist_set_column_widget (GTK_CLIST (clist), 1, label);
        label = gtk_label_new (_("Port"));
        gtk_clist_set_column_widget (GTK_CLIST (clist), 2, label);
        label = gtk_label_new (_("Speed"));
        gtk_clist_set_column_widget (GTK_CLIST (clist), 3, label);

        /* Add the buttons. */
        vbuttonbox = gtk_vbutton_box_new ();
        gtk_table_attach (GTK_TABLE (table), vbuttonbox, 1, 2, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
        button_camera_add = gtk_button_new_with_label (_("Add"));
        gtk_container_add (GTK_CONTAINER (vbuttonbox), button_camera_add);
        button_camera_update = gtk_button_new_with_label (_("Update"));
        gtk_container_add (GTK_CONTAINER (vbuttonbox), button_camera_update);
        button_camera_delete = gtk_button_new_with_label (_("Delete"));
        gtk_container_add (GTK_CONTAINER (vbuttonbox), button_camera_delete);
        button_camera_properties = gtk_button_new_with_label (_("Properties"));
        gtk_container_add (GTK_CONTAINER (vbuttonbox), button_camera_properties);

        /* Add the frame for the setting. */
        frame = gtk_frame_new (_("Setting"));
        gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
        /* Add the table to the frame. */
        table = gtk_table_new (4, 2, FALSE);
        gtk_container_add (GTK_CONTAINER (frame), table);
        gtk_table_set_row_spacings (GTK_TABLE (table), 5);
        gtk_table_set_col_spacings (GTK_TABLE (table), 5);

        /* Add the labels. */
        label = gtk_label_new (_("Name"));
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
        label = gtk_label_new (_("Model"));
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
        label = gtk_label_new (_("Port"));
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
        label = gtk_label_new (_("Speed"));
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);

        /* Add entry for name. */
        entry_name = gtk_entry_new ();
        gtk_table_attach (GTK_TABLE (table), entry_name, 1, 2, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

        /* Add combo boxes. */
        combo_model = gtk_combo_new ();
        gtk_table_attach (GTK_TABLE (table), combo_model, 1, 2, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry_model = GTK_COMBO (combo_model)->entry;
        combo_port = gtk_combo_new ();
        gtk_table_attach (GTK_TABLE (table), combo_port, 1, 2, 2, 3, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry_port = GTK_COMBO (combo_port)->entry;
        combo_speed = gtk_combo_new ();
        gtk_table_attach (GTK_TABLE (table), combo_speed, 1, 2, 3, 4, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	entry_speed = GTK_COMBO (combo_speed)->entry;

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (button_camera_add), "capplet", capplet);
        gtk_object_set_data (GTK_OBJECT (button_camera_update), "capplet", capplet);
        gtk_object_set_data (GTK_OBJECT (button_camera_delete), "capplet", capplet);
        gtk_object_set_data (GTK_OBJECT (button_camera_properties), "capplet", capplet);
	gtk_object_set_data (GTK_OBJECT (clist), "capplet", capplet);

        gtk_object_set_data (GTK_OBJECT (capplet), "clist", clist);

	gtk_object_set_data (GTK_OBJECT (capplet), "entry_name", entry_name);
	gtk_object_set_data (GTK_OBJECT (capplet), "entry_model", entry_model);
        gtk_object_set_data (GTK_OBJECT (capplet), "entry_port", entry_port);
        gtk_object_set_data (GTK_OBJECT (capplet), "entry_speed", entry_speed);

        /* Connect the signals. */
        gtk_signal_connect (GTK_OBJECT (clist), "select_row", GTK_SIGNAL_FUNC (on_clist_row_selection_changed), NULL);
        gtk_signal_connect (GTK_OBJECT (clist), "unselect_row", GTK_SIGNAL_FUNC (on_clist_row_selection_changed), NULL);
        gtk_signal_connect (GTK_OBJECT (button_camera_add), "clicked", GTK_SIGNAL_FUNC (on_button_camera_add_clicked), NULL);
        gtk_signal_connect (GTK_OBJECT (button_camera_update), "clicked", GTK_SIGNAL_FUNC (on_button_camera_update_clicked), NULL);
        gtk_signal_connect (GTK_OBJECT (button_camera_delete), "clicked", GTK_SIGNAL_FUNC (on_button_camera_delete_clicked), NULL);
        gtk_signal_connect (GTK_OBJECT (button_camera_properties), "clicked", GTK_SIGNAL_FUNC (on_button_camera_properties_clicked), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "help", GTK_SIGNAL_FUNC (camera_help), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "try", GTK_SIGNAL_FUNC (camera_try), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "revert", GTK_SIGNAL_FUNC (camera_revert), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "ok", GTK_SIGNAL_FUNC (camera_ok), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "cancel", GTK_SIGNAL_FUNC (camera_cancel), NULL);

	/* Build model list. */
	list = NULL;
	if ((number_of_models = gp_camera_count ()) == GP_ERROR) {
		gnome_error_dialog (_("Could not get number of models!"));
	} else {
		for (i = 0; i < number_of_models; i++) {
			if (gp_camera_name (i, buffer) == GP_ERROR) {
				gnome_error_dialog (_("Could not get model name!"));
				list = g_list_append (list, g_strdup ("?"));
			} else {
				list = g_list_append (list, g_strdup (buffer));
			}
		}
		if (list) {
			gtk_combo_set_popdown_strings (GTK_COMBO (combo_model), list);
			gtk_entry_set_text (GTK_ENTRY (entry_model), "");
		}
	}

	/* Build port list. */
	list = NULL;
	if ((number_of_ports = gp_port_count ()) == GP_ERROR) {
		gnome_error_dialog (_("Could not get number of ports!"));
	} else {
		for (i = 0; i < number_of_ports; i++) {
			if (gp_port_info (i, &port_info) == GP_ERROR) {
				gnome_error_dialog (_("Could not get port info!"));
			list = g_list_append (list, g_strdup ("?"));
		} else
			list = g_list_append (list, g_strdup (port_info.name));
		}
		if (list) {
			gtk_combo_set_popdown_strings (GTK_COMBO (combo_port), list);
			gtk_entry_set_text (GTK_ENTRY (entry_port), "");
		}
	}

        /* Show the capplet. */
        gtk_widget_show_all (capplet);

	/* Return the capplet. */
	return (capplet);
}

