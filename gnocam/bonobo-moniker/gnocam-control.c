#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-control.h"

#include <gal/util/e-util.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <gphoto-extensions.h>

#include "utils.h"
#include "gnocam-control-folder.h"
#include "gnocam-control-file.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_parent_class = NULL;

struct _GnoCamControlPrivate
{
        Camera*                 camera;

        CameraWidget*           config_camera;
        CameraWidget*           config_folder;
        CameraWidget*           config_file;

        gchar*                  path;
};

static void
activate (BonoboControl* object, gboolean state)
{
	BonoboUIComponent*	component;
	Bonobo_UIContainer	container;
	GnoCamControl*		control;
	
	control = GNOCAM_CONTROL (object);
	container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
	component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));
	bonobo_ui_component_set_container (component, container);
	
        if (state) {
		if (control->priv->camera->abilities->config) {
			CameraWidget*	widget;
                        gint 		result;

			result = gp_camera_config_get (control->priv->camera, &widget);
			if (result == GP_OK) menu_setup (control, widget, "Camera Configuration", NULL, NULL);
		}
	} else {
		bonobo_ui_component_unset_container (component);
	}

	bonobo_object_release_unref (container, NULL);

	if (gnocam_control_parent_class->activate) gnocam_control_parent_class->activate (object, state);
}

Camera*
gnocam_control_get_camera (GnoCamControl* control)
{
	return (control->priv->camera);
}

void
gnocam_control_complete (GnoCamControl* control, BonoboMoniker* moniker)
{
	gint			i, result;
	const gchar*		name;

	/* Create the camera. */
	name = bonobo_moniker_get_name (moniker);
	if ((result = gp_camera_new_from_gconf (&(control->priv->camera), name)) != GP_OK) {
		g_warning (_("Could not create camera '%s' (%s)!"), name, gp_result_as_string (result));
	} else {

		/* Initialize our variables. */
		for (i = 2; name[i] != 0; i++) if (name[i] == '/') break;
		control->priv->path = g_strdup (name + i);
		control->priv->config_camera = NULL;
		control->priv->config_folder = NULL;
		control->priv->config_file = NULL;
	}
}

static void
destroy (GtkObject* object)
{
	GnoCamControl*  control;
	
	control = GNOCAM_CONTROL (object);

	if (control->priv->config_camera) gp_widget_unref (control->priv->config_camera); 
	if (control->priv->config_folder) gp_widget_unref (control->priv->config_folder); 
	if (control->priv->config_file) gp_widget_unref (control->priv->config_file); 
	if (control->priv->camera) gp_camera_unref (control->priv->camera);
	if (control->priv->path) g_free (control->priv->path);

	g_free (control->priv);
}

static void
init (GnoCamControl* control)
{
	GnoCamControlPrivate*	priv;

	priv = g_new (GnoCamControlPrivate, 1);
	control->priv = priv;
}

static void
class_init (GnoCamControlClass* klass)
{
        GtkObjectClass* object_class;
        BonoboControlClass* control_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = destroy;

	control_class = BONOBO_CONTROL_CLASS (klass);
	control_class->activate = activate;
	
        gnocam_control_parent_class = gtk_type_class (bonobo_control_get_type ());
}

E_MAKE_TYPE (gnocam_control, "GnoCamControl", GnoCamControl, class_init, init, PARENT_TYPE)


