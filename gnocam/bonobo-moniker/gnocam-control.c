#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <gphoto2.h>
#include <tree.h>

#include "gnocam-control.h"
#include "utils.h"

static BonoboControlClass *gnocam_control_parent_class;

/*************/
/* Functions */
/*************/

static void
gnocam_control_activate (BonoboControl *object, gboolean state)
{
        if (state) menu_create (GNOCAM_CONTROL (object));
	else bonobo_ui_component_unset_container (bonobo_control_get_ui_component (object));

        if (BONOBO_CONTROL_CLASS (gnocam_control_parent_class)->activate) BONOBO_CONTROL_CLASS (gnocam_control_parent_class)->activate (object, state);
}

GnoCamControl*
gnocam_control_new (BonoboMoniker *moniker, const Bonobo_ResolveOptions *options)
{
        GnoCamControl*		control;
	Bonobo_Unknown 		subcontrol;
	gint			i;
	gchar*			name;
	const gchar*		name_full;
	GtkWidget*		vbox;
	Camera*			camera;
	CORBA_Environment	ev;

	/* Make sure we are given a camera name. */
	name_full = bonobo_moniker_get_name (moniker);
	if ((strlen(name_full) < 2) || (name_full [0] != '/') || (name_full [1] != '/')) {
		g_warning (_("Could not find camera name in '%s'!"), name_full);
		return (NULL);
	}

	/* Create the camera. */
	name = (gchar*) name_full + 2;
	for (i = 0; name[i] != 0; i++) if (name[i] == '/') break;
	name = g_strndup (name, i);
	if (!(camera = util_camera_new (name))) {
		g_warning (_("Could not create camera '%s'!"), name);
		return (NULL);
	}

	/* Create control. */
	control = gtk_type_new (gnocam_control_get_type ());
	gtk_widget_show (vbox = gtk_vbox_new (TRUE, 0));

	/* Create the control. */
	if (!bonobo_control_construct (BONOBO_CONTROL (control), vbox)) return (NULL);
	control->path = g_strdup (name + i);
	control->config_camera = NULL;
	control->config_folder = NULL;
	control->config_file = NULL;

	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Create viewer. */
	subcontrol = bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, "IDL:Bonobo/Control:1.0", &ev);
	if (BONOBO_EX (&ev)) g_warning (_("Could not create viewer! (%s)"), bonobo_exception_get_text (&ev));
	else {
		Bonobo_UIContainer	container = bonobo_ui_component_get_container (bonobo_control_get_ui_component (BONOBO_CONTROL (control)));
		GtkWidget*		widget = bonobo_widget_new_control_from_objref (subcontrol, container);

		gtk_widget_show (widget);
		gtk_container_add (GTK_CONTAINER (vbox), widget);
	}

	/* Keep the camera. */
	control->camera = camera;

	/* Free exception. */
	CORBA_exception_free (&ev);

	return (control);
}

static void
gnocam_control_finalize (GtkObject *object)
{
        GnoCamControl*	control = GNOCAM_CONTROL (object);

	if (control->config_camera) {gp_widget_unref (control->config_camera); control->config_camera = NULL;}
	if (control->config_folder) {gp_widget_unref (control->config_folder); control->config_folder = NULL;}
	if (control->config_file) {gp_widget_unref (control->config_file); control->config_file = NULL;}
	if (control->camera) {gp_camera_unref (control->camera); control->camera = NULL;};
	if (control->path) {g_free (control->path); control->path = NULL;};

        GTK_OBJECT_CLASS (gnocam_control_parent_class)->finalize (object);
}

static void
gnocam_control_destroy (GtkObject *object)
{
        GTK_OBJECT_CLASS (gnocam_control_parent_class)->destroy (object);
}

static void
gnocam_control_init (GnoCamControl *control)
{
	/* Don't do anything here... */
}


static void
gnocam_control_class_init (GnoCamControl *klass)
{
        GtkObjectClass *object_class = (GtkObjectClass *)klass;
        BonoboControlClass *control_class = (BonoboControlClass *)klass;

        gnocam_control_parent_class = gtk_type_class (bonobo_control_get_type ());

        object_class->destroy = gnocam_control_destroy;
        object_class->finalize = gnocam_control_finalize;

        control_class->activate = gnocam_control_activate;
}

GtkType
gnocam_control_get_type (void)
{
        static GtkType type = 0;

        if (!type) {
                GtkTypeInfo info = {
                        "GnoCamControl",
                        sizeof (GnoCamControl),
                        sizeof (GnoCamControlClass),
                        (GtkClassInitFunc)  gnocam_control_class_init,
                        (GtkObjectInitFunc) gnocam_control_init,
                        NULL, /* reserved 1 */
                        NULL, /* reserved 2 */
                        (GtkClassInitFunc) NULL
                };

                type = gtk_type_unique (bonobo_control_get_type (), &info);
        }

        return type;
}



