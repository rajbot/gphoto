#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gnome.h>
#include <gphoto2/gphoto2.h>
#include <capplet-widget.h>
#include "capplet-callbacks.h"
#include "gphoto-extensions.h"
#include "preferences.h"
#include "properties.h"
#include "capplet.h"


void
camera_try (void)
{
	printf ("camera_try\n");
}

void
camera_help (void)
{
        printf ("camera_help\n");
}

void
camera_apply (void)
{
        printf ("camera_apply\n");
}

void
camera_ok (void)
{
	preferences_write (camera_capplet_current ());
}

void
camera_revert (void)
{
        printf ("camera_revert\n");
}

void
camera_cancel (void)
{
        printf ("camera_cancel\n");
}

void
on_button_camera_add_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget *capplet;
	GtkCList *clist;
	gchar *model, *port, *speed, *name;
	Camera *camera;
	gchar *text[4];

	capplet = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "capplet"));
	clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));

	name = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_name")));
	model = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_model")));
	port = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_port")));
	speed = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_speed")));

	camera = gp_camera_new_by_description (model, port, speed);
	if (camera) {
		
		/* Add entry to clist. */
		text[0] = name;
		text[1] = model;
		text[2] = port;
		text[3] = speed;
		gtk_clist_append (clist, text);

		/* Store some data. */
		gtk_clist_set_row_data (clist, clist->rows, camera);
		capplet_widget_state_changed (CAPPLET_WIDGET (capplet), TRUE);
	}
}

void
on_button_camera_update_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget *capplet;
	GtkCList *clist;
	gchar *model, *port, *speed, *name;
	gchar *text[4];
	GList *selection;
	gint row;

	capplet = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "capplet"));
        clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));

	/* Get the user's entries. */
        name = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_name")));
        model = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_model")));
        port = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_port")));
        speed = gtk_entry_get_text (GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_speed")));

	selection = g_list_first (clist->selection);
	if (g_list_length (selection) == 1) {
		row = GPOINTER_TO_INT (selection->data);

		/* Add entry at previous position to list. */
		text[0] = name;
                text[1] = model;
                text[2] = port;
                text[3] = speed;
                gtk_clist_insert (clist, row, text);
        
		/* Clean up the old entry. */
		g_free (gtk_clist_get_row_data (clist, row + 1));
		gtk_clist_remove (clist, row + 1);

                capplet_widget_state_changed (CAPPLET_WIDGET (capplet), TRUE);

	} else {
		gnome_error_dialog (_("Please select exactly one camera."));
	}
}

void
on_button_camera_delete_clicked (GtkButton *button, gpointer user_data)
{
	GList *selection = NULL;
	GtkWidget *capplet;
	GtkCList *clist;
	gint row;

        capplet = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "capplet"));
        clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));

	selection = clist->selection;
	while (selection != NULL) {
		row = GPOINTER_TO_INT (selection->data);

		/* Remove selected camera. */
		g_free (gtk_clist_get_row_data (clist, row));
		gtk_clist_remove (clist, row);

		selection = g_list_first (clist->selection);
                capplet_widget_state_changed (CAPPLET_WIDGET (capplet), TRUE);
	}
}

void 
on_button_camera_properties_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget *capplet;
	GtkCList *clist;
	GList *selection;
	gchar *name;
	gint i;

	capplet = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "capplet"));
        clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));

	/* Create a camera property dialog for each selected camera. */
	selection = g_list_first (clist->selection);
	for (i = 0; i < g_list_length (selection); i++) {
		gtk_clist_get_text (clist, i, 0, &name);
		camera_properties (gtk_clist_get_row_data (clist, GPOINTER_TO_INT (g_list_nth_data (selection, i))), name);
	}
}

void
on_clist_row_selection_changed (GtkWidget *widget, gint row, gint column, GdkEventButton *event, gpointer user_data)
{
	GList *selection = NULL;
	GtkWidget *capplet;
	GtkCList *clist;
	gchar *name, *model, *speed, *port;
	gint selected_row;
	GtkEntry *entry_name, *entry_model, *entry_port, *entry_speed;

        capplet = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (widget), "capplet"));
        clist = GTK_CLIST (gtk_object_get_data (GTK_OBJECT (capplet), "clist"));
	entry_name = GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_name"));
	entry_model = GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_model"));
	entry_port = GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_port"));
	entry_speed = GTK_ENTRY (gtk_object_get_data (GTK_OBJECT (capplet), "entry_speed"));

        selection = clist->selection;
        if (g_list_length (selection) == 1) {

		/* Exactly one row selected. */
                selected_row = GPOINTER_TO_INT (selection->data);

		/* Fill the settings frame with selected values. */
		gtk_clist_get_text (clist, selected_row, 0, &name);
                gtk_clist_get_text (clist, selected_row, 1, &model);
                gtk_clist_get_text (clist, selected_row, 2, &port);
                gtk_clist_get_text (clist, selected_row, 3, &speed);
                gtk_entry_set_text (entry_name, name);
		gtk_entry_set_text (entry_model, model);
		gtk_entry_set_text (entry_port, port);
		gtk_entry_set_text (entry_speed, speed);

        } else {
                
                /* Clean up the settings frame. */
                gtk_entry_set_text (entry_name, "");
                gtk_entry_set_text (entry_model, "");
                gtk_entry_set_text (entry_port, "");
                gtk_entry_set_text (entry_speed, "");
	}
}
