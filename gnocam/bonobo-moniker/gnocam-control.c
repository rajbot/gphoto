#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <gphoto2.h>
#include <tree.h>

#include "gnocam-control.h"
#include "utils.h"

struct _GnoCamControlPrivate {
        GtkWidget*	vbox;
};

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
gnocam_control_construct (GnoCamControl* control, BonoboMoniker *moniker, const Bonobo_ResolveOptions *options)
{
	Bonobo_Unknown 		subcontrol;
	CORBA_Environment	ev;
	gint			i;
	gchar*			name;
	const gchar*		name_full;
	Camera*			camera = NULL;

printf ("BEGIN: gnocam_control_construct\n");
	
	/* Disassemble the moniker's name. */
	name_full = bonobo_moniker_get_name (moniker);
	name = (gchar*) name_full + 8;
	if (name[0] == '/') {
		
		/* Create the camera. */
		for (i = 0; name[i] != 0; i++) if (name[i] == '/') break;
		name = g_strndup (name, i);
		camera = util_camera_new (name, &ev);
	}
	
	g_return_val_if_fail (camera, NULL);
	
	gtk_widget_show (control->priv->vbox = gtk_vbox_new (TRUE, 0));

	/* Create the control. */
	if (!bonobo_control_construct (BONOBO_CONTROL (control), control->priv->vbox)) return (NULL);
	control->path = g_strdup (name + i);
	control->config_camera = NULL;
	control->config_object = NULL;

	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Create viewer. */
	subcontrol = bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, "IDL:Bonobo/Control:1.0", &ev);
	if (BONOBO_EX (&ev)) g_warning (_("Could not create viewer! (%s)"), bonobo_exception_get_text (&ev));
	else {
		Bonobo_UIContainer	container = bonobo_ui_component_get_container (bonobo_control_get_ui_component (BONOBO_CONTROL (control)));
		GtkWidget*		widget = bonobo_widget_new_control_from_objref (subcontrol, container);

		gtk_widget_show (widget);
		gtk_container_add (GTK_CONTAINER (control->priv->vbox), widget);
	}

	/* Keep the camera. */
	control->camera = camera;

	/* Free exception. */
	CORBA_exception_free (&ev);

printf ("END: gnocam_control_construct\n");
	return (control);
}

static void
gnocam_control_finalize (GtkObject *object)
{
        GnoCamControl*	control = GNOCAM_CONTROL (object);

printf ("BEGIN: gnocam_control_finalize\n");

        if (control->priv) {
		if (control->priv->vbox) gtk_widget_destroy (control->priv->vbox);
		control->priv->vbox = NULL;
		g_free (control->priv);
		control->priv = NULL;
	}

        GTK_OBJECT_CLASS (gnocam_control_parent_class)->finalize (object);

printf ("END: gnocam_control_finalize\n");

}

static void
gnocam_control_destroy (GtkObject *object)
{
        GTK_OBJECT_CLASS (gnocam_control_parent_class)->destroy (object);
}

static void
gnocam_control_init (GnoCamControl *control)
{

printf ("BEGIN: gnocam_control_init\n");

        control->priv = g_new0 (GnoCamControlPrivate, 1);

printf ("END: gnocam_control_init\n");

}


static void
gnocam_control_class_init (GnoCamControl *klass)
{
        GtkObjectClass *object_class = (GtkObjectClass *)klass;
        BonoboControlClass *control_class = (BonoboControlClass *)klass;

printf ("BEGIN: gnocam_control_class_init\n");

        gnocam_control_parent_class = gtk_type_class (bonobo_control_get_type ());

        object_class->destroy = gnocam_control_destroy;
        object_class->finalize = gnocam_control_finalize;

        control_class->activate = gnocam_control_activate;

printf ("END: gnocam_control_class_init\n");

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

GnoCamControl*
gnocam_control_new (BonoboMoniker *moniker, const Bonobo_ResolveOptions *options)
{
        GnoCamControl *control;
        
        control = gtk_type_new (gnocam_control_get_type ());

        return (gnocam_control_construct (control, moniker, options));
}


