#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>

#include "gnocam-configuration.h"

#include <gal/util/e-util.h>

#define PARENT_TYPE GNOME_TYPE_DIALOG
static GnomeDialogClass* parent_class = NULL;

struct _GnoCamConfigurationPrivate {

	GtkWidget*	parent;
	
	Camera*		camera;
	CameraWidget*	widget;

	gchar*		dirname;
	gchar*		filename;
};

/********************/
/* Helper functions */
/********************/

static void
set_config (GnoCamConfiguration* configuration)
{
        gint    result;

        if (configuration->priv->filename) 
		result = gp_camera_file_config_set (configuration->priv->camera, configuration->priv->widget, 
			configuration->priv->dirname, configuration->priv->filename);
        else if (configuration->priv->dirname) 
		result = gp_camera_folder_config_set (configuration->priv->camera, configuration->priv->widget, configuration->priv->dirname);
        else result = gp_camera_config_set (configuration->priv->camera, configuration->priv->widget);

        if (result != GP_OK)
                g_warning (_("Could not set configuration of '%s'!\n(%s)"), 
			gp_widget_label (configuration->priv->widget), gp_camera_result_as_string (configuration->priv->camera, result));
}

/*************/
/* Callbacks */
/*************/

static void
on_button_clicked (GtkButton* button, gint button_number, gpointer user_data)
{
	GnoCamConfiguration*	configuration;

	configuration = GNOCAM_CONFIGURATION (user_data);

	if (button_number == 0) {
		set_config (configuration);
		gtk_widget_destroy (GTK_WIDGET (configuration));
		return;
	}

	if (button_number == 1) {
		set_config (configuration);
		return;
	}

	if (button_number == 2) {
		gtk_widget_destroy (GTK_WIDGET (configuration));
		return;
	}
}

/*************************/
/* Gnome-Dialog specific */
/*************************/

static void
gnocam_configuration_destroy (GtkObject* object)
{
	GnoCamConfiguration*	configuration;

	configuration = GNOCAM_CONFIGURATION (object);

	gtk_widget_unref (configuration->priv->parent);

	if (configuration->priv->dirname) g_free (configuration->priv->dirname);
	if (configuration->priv->filename) g_free (configuration->priv->filename);
	
	gp_widget_unref (configuration->priv->widget);
	gp_camera_unref (configuration->priv->camera);

	g_free (configuration->priv);

	(*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
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
gnocam_configuration_new (Camera* camera, const gchar* dirname, const gchar* filename, GtkWidget* parent)
{
	GnoCamConfiguration*	new;
	gint			result;
	CameraWidget*		widget = NULL;
	const gchar*            buttons [] = {GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_APPLY, GNOME_STOCK_BUTTON_CANCEL, NULL};
	GtkWidget*		notebook;

	g_return_val_if_fail (camera, NULL);

	if (filename) result = gp_camera_file_config_get (camera, &widget, (gchar*) dirname, (gchar*) filename);
	else if (dirname) result = gp_camera_folder_config_get (camera, &widget, (gchar*) dirname);
	else result = gp_camera_config_get (camera, &widget);

	if (result != GP_OK) {
		g_warning (_("Could not get configuration: %s"), gp_camera_result_as_string (camera, result));
		return (NULL);
	}

	if (gp_widget_type (widget) != GP_WIDGET_WINDOW) {
		g_warning (_("Got configuration widget which is not of type GP_WIDGET_WINDOW!"));
		gp_widget_unref (widget);
		return (NULL);
	}

	new = gtk_type_new (GNOCAM_TYPE_CONFIGURATION);
	gp_camera_ref (new->priv->camera = camera);
	new->priv->widget = widget;
	gtk_widget_ref (new->priv->parent = parent);
	gnome_dialog_constructv (GNOME_DIALOG (new), gp_widget_label (new->priv->widget), buttons);
	gnome_dialog_set_close (GNOME_DIALOG (new), FALSE);

	/* Connect signals */
	gtk_signal_connect (GTK_OBJECT (new), "clicked", GTK_SIGNAL_FUNC (on_button_clicked), new);

	/* Create the notebook */
	gtk_widget_show (notebook = gtk_notebook_new ());
	gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (new)->vbox), notebook);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_configuration, "GnoCamConfiguration", GnoCamConfiguration, gnocam_configuration_class_init, gnocam_configuration_init, PARENT_TYPE)
