/* gnocam-configuration.c
 *
 * Copyright (C) 2000, 2001 Lutz Müller
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Lutz Müller <urc8@rz.uni-karlsruhe.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>

#include "gnocam-configuration.h"

#include <gal/util/e-util.h>

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass* parent_class = NULL;

struct _GnoCamConfigurationPrivate {

	Camera*		camera;
	CameraWidget*	widget;

	gchar*		dirname;
	gchar*		filename;

	GHashTable*	hash_table;

	GtkWidget*	notebook;
};

/**************/
/* Prototypes */
/**************/

static void 	on_button_clicked	 	(GtkButton* button, gpointer user_data);
static void 	on_entry_changed 		(GtkEntry* entry, gpointer user_data);
static void 	on_adjustment_value_changed 	(GtkAdjustment* adjustment, gpointer user_data);
static void	on_date_edit_changed		(GnomeDateEdit* date_edit, gpointer user_data);
static void	on_toggle_button_toggled	(GtkToggleButton* toggle_button, gpointer user_data);
static void	on_radio_button_toggled		(GtkToggleButton* toggle_button, gpointer user_data);

/********************/
/* Helper functions */
/********************/

static void
set_config (GnoCamConfiguration* configuration)
{
        gint    result;

        if (configuration->priv->filename) 
		result = gp_camera_file_set_config (configuration->priv->camera,
			configuration->priv->dirname, configuration->priv->filename, configuration->priv->widget);
        else if (configuration->priv->dirname) 
		result = gp_camera_folder_set_config (configuration->priv->camera, configuration->priv->dirname, configuration->priv->widget);
        else result = gp_camera_set_config (configuration->priv->camera, configuration->priv->widget);

        if (result != GP_OK)
                g_warning (_("Could not set configuration of '%s'!\n(%s)"), 
			gp_widget_label (configuration->priv->widget), gp_camera_get_result_as_string (configuration->priv->camera, result));
}

static GtkWidget*
create_page (GnoCamConfiguration* configuration, CameraWidget* widget)
{
	GtkWidget*	label;
	GtkWidget*	vbox;
	gint*		id;

	id = g_new (gint, 1);
	if (widget) *id = gp_widget_id (widget);
	else *id = -1;

	/* Label */
	if (widget) label = gtk_label_new (gp_widget_label (widget));
	else label = gtk_label_new (_("Others"));
	gtk_widget_show (label);
	
	/* VBox */
	vbox = gtk_vbox_new (FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
	gtk_widget_show (vbox);

	g_hash_table_insert (configuration->priv->hash_table, id, vbox);
	gtk_notebook_append_page (GTK_NOTEBOOK (configuration->priv->notebook), vbox, label);

	return (vbox);
}

static void
create_widgets (GnoCamConfiguration* configuration, CameraWidget* widget)
{
	CameraWidgetType	type;
	gchar*			value_char = NULL;
	gint			value_int = 0;
	gfloat			value_float = 0.0;
	gfloat			min = 0.0;
	gfloat			max = 0.0;
	gfloat			increment = 0.0;
	gint			i;
	gint			result;
	GtkWidget*		vbox;
	GtkWidget*		button;
	GtkWidget*		gtk_widget;
	GtkWidget*		frame;
	GtkObject*		adjustment;
	GSList*			group = NULL;

	type = gp_widget_type (widget);

	switch (type) {
	case GP_WIDGET_WINDOW:
	case GP_WIDGET_SECTION:
	
		/* If section, create page */
		if (type == GP_WIDGET_SECTION) create_page (configuration, widget);

		/* Create sub-widgets */
		for (i = 0; i < gp_widget_child_count (widget); i++) create_widgets (configuration, gp_widget_child (widget, i));

		return;
		
	case GP_WIDGET_BUTTON:
	
		gtk_widget = gtk_vbox_new (FALSE, 10);
		gtk_container_set_border_width (GTK_CONTAINER (gtk_widget), 10);
		button = gtk_button_new_with_label (gp_widget_label (widget));
		gtk_widget_show (button);
		gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (on_button_clicked), widget);
		gtk_container_add (GTK_CONTAINER (gtk_widget), button);
		break;
		
	case GP_WIDGET_DATE:
	
		if ((result = gp_widget_value_get (widget, &value_int)) != GP_OK)
			g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
		gtk_widget = gnome_date_edit_new ((time_t) value_int, TRUE, TRUE);
		gtk_signal_connect (GTK_OBJECT (gtk_widget), "date_changed", GTK_SIGNAL_FUNC (on_date_edit_changed), widget);
		gtk_signal_connect (GTK_OBJECT (gtk_widget), "time_changed", GTK_SIGNAL_FUNC (on_date_edit_changed), widget);
		break;
		
	case GP_WIDGET_TEXT:

		if ((result = gp_widget_value_get (widget, &value_char)) != GP_OK)
			g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
		gtk_widget = gtk_entry_new ();
		if (value_char) gtk_entry_set_text (GTK_ENTRY (gtk_widget), value_char);
		gtk_signal_connect (GTK_OBJECT (gtk_widget), "changed", GTK_SIGNAL_FUNC (on_entry_changed), widget);
		break;
	
	case GP_WIDGET_RANGE:

		if ((result = gp_widget_value_get (widget, &value_float)) != GP_OK)
			g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
		if ((result = gp_widget_range_get (widget, &min, &max, &increment)) != GP_OK)
			g_warning (_("Could not get values of range widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
		adjustment = gtk_adjustment_new (value_float, min, max, increment, 0, 0);
		gtk_signal_connect (adjustment, "value_changed", GTK_SIGNAL_FUNC (on_adjustment_value_changed), widget);
		gtk_widget = gtk_hscale_new (GTK_ADJUSTMENT (adjustment));
		gtk_scale_set_digits (GTK_SCALE (gtk_widget), 0);
		gtk_range_set_update_policy (GTK_RANGE (gtk_widget), GTK_UPDATE_DISCONTINUOUS);
		break;
	
	case GP_WIDGET_MENU:
	case GP_WIDGET_RADIO:

		if ((result = gp_widget_value_get (widget, &value_char)) != GP_OK)
			g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));

		if (gp_widget_choice_count (widget) < 6) gtk_widget = gtk_hbox_new (FALSE, 5);
		else gtk_widget = gtk_vbox_new (FALSE, 5);
		for (i = 0; i < gp_widget_choice_count (widget); i++) {
			button = gtk_radio_button_new_with_label (group, gp_widget_choice (widget, i));
			gtk_widget_show (button);
			group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
			gtk_object_set_data (GTK_OBJECT (button), "value", gp_widget_choice (widget, i));
			gtk_box_pack_start (GTK_BOX (gtk_widget), button, FALSE, FALSE, 0);
			if (value_char && !strcmp (value_char, gp_widget_choice (widget, i))) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
			gtk_signal_connect (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (on_radio_button_toggled), widget);
		}
		break;
	
	case GP_WIDGET_TOGGLE:

		if ((result = gp_widget_value_get (widget, &value_int)) != GP_OK)
			g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));

		gtk_widget = gtk_check_button_new_with_label (gp_widget_label (widget));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtk_widget), (value_int != 0));
		gtk_signal_connect (GTK_OBJECT (gtk_widget), "toggled", GTK_SIGNAL_FUNC (on_toggle_button_toggled), widget);
		break;
	
	default:
		g_warning (_("Widget '%s' is of unknown type!"), gp_widget_label (widget));
		return;
	}
	
	gtk_widget_show (gtk_widget);
	frame = gtk_frame_new (gp_widget_label (widget));
	gtk_widget_show (frame);
	gtk_container_add (GTK_CONTAINER (frame), gtk_widget);

	if (gp_widget_type (widget->parent) == GP_WIDGET_SECTION) {
		gint 	id;
		
		id = gp_widget_id (widget->parent);
		vbox = g_hash_table_lookup (configuration->priv->hash_table, &id);
	} else {
		gint 	id;
		
		id = -1;
		vbox = g_hash_table_lookup (configuration->priv->hash_table, &id);
		if (!vbox) vbox = create_page (configuration, NULL);
	}
	g_return_if_fail (vbox);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
}

/*************/
/* Callbacks */
/*************/

static void
on_toggle_button_toggled (GtkToggleButton* toggle_button, gpointer user_data)
{
	CameraWidget*	widget;
	gint		value = 0;
	gint		value_new = 0;
	gint		result;

	widget = (CameraWidget*) user_data;
	g_return_if_fail (gp_widget_type (widget) == GP_WIDGET_TOGGLE);

	if ((result = gp_widget_value_get (widget, &value)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
	if (toggle_button->active) value_new = 1;
	if (value != value_new)
		if ((result = gp_widget_value_set (widget, &value_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
}

static void
on_radio_button_toggled (GtkToggleButton* toggle_button, gpointer user_data)
{
	CameraWidget* 		widget;
	gchar*			value = NULL;
	gchar*			value_new = NULL;
	gint			result;

	g_return_if_fail (user_data);
	g_return_if_fail (toggle_button);

	widget = (CameraWidget*) user_data;

	if (!toggle_button->active) return;

	if ((result = gp_widget_value_get (widget, &value)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
	value_new = gtk_object_get_data (GTK_OBJECT (toggle_button), "value");
	if (!value || strcmp (value, value_new))
		if ((result = gp_widget_value_set (widget, value_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
}

static void
on_button_clicked (GtkButton* button, gpointer user_data)
{
	GnoCamConfiguration* 	configuration;
	CameraWidget*		widget;
	CameraWidgetCallback	callback;
	gint			result;

	configuration = GNOCAM_CONFIGURATION (gtk_widget_get_ancestor (GTK_WIDGET (button), GNOCAM_TYPE_CONFIGURATION));
	widget = (CameraWidget*) user_data;
	callback = gp_widget_callback (widget);

	if ((result = callback (configuration->priv->camera, widget)) != GP_OK)
		g_warning (_("Could not execute command '%s': %s!"), gp_widget_label (widget), gp_camera_get_result_as_string (configuration->priv->camera, result));
}

static void
on_date_edit_changed (GnomeDateEdit* date_edit, gpointer user_data)
{
	CameraWidget*           widget;
	gint			value = 0;
	gint			value_new = 0;
	gint			result;

	widget = (CameraWidget*) user_data;

	if ((result = gp_widget_value_get (widget, &value)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
	value_new = (gint) gnome_date_edit_get_date (date_edit);
	if (value_new != value)
		if ((result = gp_widget_value_set (widget, &value_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
}

static void
on_entry_changed (GtkEntry* entry, gpointer user_data)
{
	CameraWidget*           widget;
	gchar*			value_new = NULL;
	gchar*			value = NULL;
	gint			result;

	widget = (CameraWidget*) user_data;

	if ((result = gp_widget_value_get (widget, value)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
	value_new = gtk_entry_get_text (entry);
	if (!value || strcmp (value, value_new))
		if ((result = gp_widget_value_set (widget, value_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
}

static void
on_adjustment_value_changed (GtkAdjustment* adjustment, gpointer user_data)
{
	CameraWidget*           widget;
	gint			result;
	gfloat			value = 0;
	gfloat			value_new = 0.0;

	widget = (CameraWidget*) user_data;
	if ((result = gp_widget_value_get (widget, &value)) != GP_OK)
		g_warning (_("Could not get value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
	value_new = adjustment->value;
	if (value_new != value) 
		if ((result = gp_widget_value_set (widget, &value_new)) != GP_OK)
			g_warning (_("Could not set value of widget '%s': %s!"), gp_widget_label (widget), gp_result_as_string (result));
}

static void
on_dialog_clicked (GtkButton* button, gint button_number, gpointer user_data)
{
	GnoCamConfiguration*	configuration;

	configuration = GNOCAM_CONFIGURATION (user_data);

	if ((button_number == 0) || (button_number == 1)) set_config (configuration);

	if ((button_number == 0) || (button_number == 2)) gtk_widget_unref (GTK_WIDGET (configuration));
}

/*************************/
/* Gnome-Dialog specific */
/*************************/

static void
gnocam_configuration_destroy (GtkObject* object)
{
	GnoCamConfiguration*	configuration;

	configuration = GNOCAM_CONFIGURATION (object);

	if (configuration->priv->dirname) g_free (configuration->priv->dirname);
	if (configuration->priv->filename) g_free (configuration->priv->filename);
	
	gp_widget_unref (configuration->priv->widget);
	gp_camera_unref (configuration->priv->camera);

	g_hash_table_destroy (configuration->priv->hash_table);

	g_free (configuration->priv);
	configuration->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_configuration_class_init (GnoCamConfigurationClass* klass)
{
	GtkObjectClass*         object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_configuration_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_configuration_init (GnoCamConfiguration* configuration)
{
	configuration->priv = g_new0 (GnoCamConfigurationPrivate, 1);
}

GtkWidget*
gnocam_configuration_new (Camera* camera, const gchar* dirname, const gchar* filename, GtkWindow* window)
{
	GnoCamConfiguration*	new;
	gint			result;
	CameraWidget*		widget = NULL;
	const gchar*            buttons [] = {GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_APPLY, GNOME_STOCK_BUTTON_CANCEL, NULL};

	g_return_val_if_fail (camera, NULL);

	if (filename) result = gp_camera_file_get_config (camera, (gchar*) dirname, (gchar*) filename, &widget);
	else if (dirname) result = gp_camera_folder_get_config (camera, (gchar*) dirname, &widget);
	else result = gp_camera_get_config (camera, &widget);

	if (result != GP_OK) {
		g_warning (_("Could not get configuration: %s"), gp_camera_get_result_as_string (camera, result));
		return (NULL);
	}

	if (gp_widget_type (widget) != GP_WIDGET_WINDOW) {
		g_warning (_("Got configuration widget which is not of type GP_WIDGET_WINDOW!"));
		gp_widget_unref (widget);
		return (NULL);
	}

	/* Set up the dialog */
	new = gtk_type_new (GNOCAM_TYPE_CONFIGURATION);
	gnome_dialog_constructv (GNOME_DIALOG (new), gp_widget_label (widget), buttons);
	gnome_dialog_set_parent (GNOME_DIALOG (new), window);
	gnome_dialog_set_close (GNOME_DIALOG (new), FALSE);

	/* Keep filename and dirname */
	if (filename) new->priv->filename = g_strdup (filename);
	if (dirname) new->priv->dirname = g_strdup (dirname);

	/* Hash table. We need it for looking up the notebook page. */
	new->priv->hash_table = g_hash_table_new (g_int_hash, g_int_equal);

	/* Keep the camera and widget */
	gp_camera_ref (new->priv->camera = camera);
	new->priv->widget = widget;

	/* Connect signals */
	gtk_signal_connect (GTK_OBJECT (new), "clicked", GTK_SIGNAL_FUNC (on_dialog_clicked), new);

	/* Create the notebook */
	gtk_widget_show (new->priv->notebook = gtk_notebook_new ());
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (new)->vbox), new->priv->notebook);

	/* Fill the notebook */
	create_widgets (new, new->priv->widget);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_configuration, "GnoCamConfiguration", GnoCamConfiguration, gnocam_configuration_class_init, gnocam_configuration_init, PARENT_TYPE)
