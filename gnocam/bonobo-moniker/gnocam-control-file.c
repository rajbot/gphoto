#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-control-file.h"

#include <bonobo/bonobo-moniker-extender.h>
#include <gal/util/e-util.h>

#include "utils.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_file_parent_class = NULL;

struct _GnoCamControlFilePrivate
{
	Camera*	camera;
	gchar*	dirname;
	gchar*	filename;
};

/***************************/
/* Bonobo-Control specific */
/***************************/

static void
activate (BonoboControl* object, gboolean state)
{
        BonoboUIComponent*      component;
        Bonobo_UIContainer      container;
        GnoCamControlFile*	control;

        control = GNOCAM_CONTROL_FILE (object);
        container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
        component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));
        bonobo_ui_component_set_container (component, container);

        if (state) {
                gint            result;
                CameraWidget*   widget = NULL;

                /* Create the menu */
                result = gp_camera_file_config_get (control->priv->camera, &widget, control->priv->dirname, control->priv->filename);
                if (result == GP_OK) menu_setup (object, control->priv->camera, widget, _("File Configuration"), control->priv->dirname, control->priv->filename);

	} else {
                bonobo_ui_component_unset_container (component);
        }
	bonobo_object_release_unref (container, NULL);

        if (BONOBO_CONTROL_CLASS (gnocam_control_file_parent_class)->activate)
                BONOBO_CONTROL_CLASS (gnocam_control_file_parent_class)->activate (object, state);
}

static void
destroy (GtkObject* object)
{
	GnoCamControlFile* control;

	control = GNOCAM_CONTROL_FILE (object);

	if (control->priv->camera) gp_camera_unref (control->priv->camera);
	if (control->priv->dirname) g_free (control->priv->dirname);
	if (control->priv->filename) g_free (control->priv->filename);
	
	g_free (control->priv);
}

static void
class_init (GnoCamControlFileClass* klass)
{
	GtkObjectClass* 	object_class;
	BonoboControlClass* 	control_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = destroy;

	control_class = BONOBO_CONTROL_CLASS (klass);
	control_class->activate = activate;

	gnocam_control_file_parent_class = gtk_type_class (bonobo_control_get_type ());
}

static void
init (GnoCamControlFile* file)
{
	GnoCamControlFilePrivate*	priv;

	priv = g_new (GnoCamControlFilePrivate, 1);
	file->priv = priv;
}

GnoCamControlFile*
gnocam_control_file_new (Camera* camera, BonoboMoniker* moniker, const Bonobo_ResolveOptions* options, Bonobo_Stream stream, CORBA_Environment* ev)
{
	GtkWidget*		widget;
	GnoCamControlFile*	new;
	Bonobo_Unknown		subcontrol;
	gchar*			name;

        /* Create the viewer */
        subcontrol = bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, "IDL:Bonobo/Control:1.0", ev);
        if (BONOBO_EX (ev)) return (NULL);
	
	/* Extract the dirname */
	for (name = (gchar*) bonobo_moniker_get_name (moniker) + 2; *name != 0; name++) if (*name == '/') break;
	
	new = gtk_type_new (gnocam_control_file_get_type ());
	new->priv->filename = g_strdup (g_basename (name));
	new->priv->dirname = g_dirname (name);
	new->priv->camera = camera;
	gp_camera_ref (camera);
	
	widget = bonobo_widget_new_control_from_objref (subcontrol, CORBA_OBJECT_NIL);
	bonobo_control_construct (BONOBO_CONTROL (new), widget);

	return (new);
}

E_MAKE_TYPE (gnocam_control_file, "GnoCamControlFile", GnoCamControlFile, class_init, init, PARENT_TYPE)


