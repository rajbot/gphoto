#include <config.h>

#include "gnocam-control.h"

#include <stdio.h>
//#include <gtk/gtksignal.h>
//#include <gtk/gtkmarshal.h>
//#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-moniker-extender.h>

#include <gphoto-extensions.h>

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
	gint			i, result;
	const gchar*		name;
	GtkWidget*		vbox;
	GtkWidget*		widget;
	CORBA_Environment	ev;
	Bonobo_UIContainer	container;
	Camera*			camera = NULL;

	/* Create the camera. */
	name = bonobo_moniker_get_name (moniker);
	if ((result = gp_camera_new_from_gconf (&camera, name)) != GP_OK) {
		g_warning (_("Could not create camera '%s'!"), name);
		return (NULL);
	}

	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Create the viewer through the moniker-extender. */
	subcontrol = bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, "IDL:Bonobo/Control:1.0", &ev);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not create viewer for '%s'! (%s)"), name, bonobo_exception_get_text (&ev));
		return (NULL);
	}

	/* Free exception. */
	CORBA_exception_free (&ev);

	/* Create the control. */
	control = gtk_type_new (gnocam_control_get_type ());
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
	g_return_val_if_fail (bonobo_control_construct (BONOBO_CONTROL (control), vbox), NULL);

	/* Insert the viewer */
	container = bonobo_ui_component_get_container (bonobo_control_get_ui_component (BONOBO_CONTROL (control)));
	gtk_widget_show (widget =  bonobo_widget_new_control_from_objref (subcontrol, container));
	gtk_container_add (GTK_CONTAINER (vbox), widget);

	/* Initialize our variables. */
	for (i = 2; name[i] != 0; i++) if (name[i] == '/') break;
	control->path = g_strdup (name + i);
	control->config_camera = NULL;
	control->config_folder = NULL;
	control->config_file = NULL;
	control->camera = camera;

	return (control);
}

static void
gnocam_control_finalize (GtkObject *object)
{
	g_warning ("BEGIN: gnocam_control_finalize");

        GTK_OBJECT_CLASS (gnocam_control_parent_class)->finalize (object);

	g_warning ("END: gnocam_control_finalize");
}

static void
gnocam_control_destroy (GtkObject *object)
{
	GnoCamControl*  control = GNOCAM_CONTROL (object);

	g_warning ("BEGIN: gnocam_control_destroy");
	if (control->config_camera) {gp_widget_unref (control->config_camera); control->config_camera = NULL;}
	if (control->config_folder) {gp_widget_unref (control->config_folder); control->config_folder = NULL;}
	if (control->config_file) {gp_widget_unref (control->config_file); control->config_file = NULL;}
	if (control->camera) {gp_camera_unref (control->camera); control->camera = NULL;};
	if (control->path) {g_free (control->path); control->path = NULL;};
        GTK_OBJECT_CLASS (gnocam_control_parent_class)->destroy (object);
	g_warning ("END: gnocam_control_destroy");
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
                        (GtkObjectInitFunc) NULL,
                        NULL, /* reserved 1 */
                        NULL, /* reserved 2 */
                        (GtkClassInitFunc) NULL
                };

                type = gtk_type_unique (bonobo_control_get_type (), &info);
        }

        return type;
}



