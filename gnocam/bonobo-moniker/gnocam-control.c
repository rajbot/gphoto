#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>

#include "gnocam-control.h"

struct _GnoCamControlPrivate {
        GtkWidget          *vbox;
};

static BonoboControlClass *gnocam_control_parent_class;

/*************/
/* Callbacks */
/*************/

void
on_configure_clicked (GtkButton* button)
{
	g_warning ("Not yet implemented!");
}

/*************/
/* Functions */
/*************/

static void
gnocam_control_activate (BonoboControl *object, gboolean state)
{
        GnoCamControl *control = GNOCAM_CONTROL (object);

printf ("BEGIN: gnocam_control_activate\n");

        if (state) {
                Bonobo_UIContainer container;

                container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
                if (container != CORBA_OBJECT_NIL) {
                        bonobo_object_release_unref (container, NULL);
                } 
	} else {
		bonobo_ui_component_unset_container (bonobo_control_get_ui_component (object));
	}

        if (BONOBO_CONTROL_CLASS (gnocam_control_parent_class)->activate)
                BONOBO_CONTROL_CLASS (gnocam_control_parent_class)->activate (object, state);

	printf ("END: gnocam_control_activate\n");

}


GnoCamControl*
gnocam_control_construct (GnoCamControl* control, BonoboMoniker *moniker, const Bonobo_ResolveOptions *options)
{
	BonoboUIComponent*	component;
	Bonobo_Unknown 		subcontrol;
	CORBA_Environment	ev;
	static const gchar*	control_menu =
		"<placeholder name=\"ZoomMenu\">\n"
		"  <menuitem name=\"ZoomIn\" _label=\"Zoom _In\" verb=\"\"/>\n"
		"  <menuitem name=\"ZoomOut\" _label=\"Zoom _Out\" verb=\"\"/>\n"
		"  <menuitem name=\"ZoomToDefault\" _label=\"Zoom to _Default\" verb=\"\"/>\n"
		"  <menuitem name=\"ZoomToFit\" _label=\"Zoom to _Fit\" verb=\"\"/>\n"
		"</placeholder>";
	GtkWidget*		widget;
	
	gtk_widget_show (control->priv->vbox = gtk_vbox_new (TRUE, 0));
	gtk_widget_show (widget = gtk_button_new_with_label ("Configure"));
	gtk_signal_connect (GTK_OBJECT (widget), "clicked", GTK_SIGNAL_FUNC (on_configure_clicked), NULL);
	gtk_container_add (GTK_CONTAINER (control->priv->vbox), widget);

	if (!bonobo_control_construct (BONOBO_CONTROL (control), control->priv->vbox)) return (NULL);
	component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));

	/* Init exception. */
	CORBA_exception_init (&ev);

	subcontrol = bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, "IDL:Bonobo/Control:1.0", &ev);
	if (BONOBO_EX (&ev)) g_warning (_("Could not create viewer! (%s)"), bonobo_exception_get_text (&ev));
	else {
		Bonobo_UIContainer	container = bonobo_ui_component_get_container (component);
		GtkWidget*		widget = bonobo_widget_new_control_from_objref (subcontrol, container);

		gtk_widget_show (widget);
		gtk_container_add (GTK_CONTAINER (control->priv->vbox), widget);
	}

	/* Free exception. */
	CORBA_exception_free (&ev);

	/* Create menu. */
	//This doesn't work...
	bonobo_ui_component_set_translate (component, "/menu/Edit", control_menu, NULL);

	return (control);
}

static void
gnocam_control_finalize (GtkObject *object)
{
        GnoCamControl*	control = GNOCAM_CONTROL (object);

printf ("BEGIN: gnocam_control_finalize\n");

        if (control->priv) g_free (control->priv);

        GTK_OBJECT_CLASS (gnocam_control_parent_class)->finalize (object);

printf ("END: gnocam_control_finalize\n");

}

static void
gnocam_control_destroy (GtkObject *object)
{
        GnoCamControl *control = GNOCAM_CONTROL (object);

printf ("BEGIN: gnocam_control_destroy\n");

	if (control->priv->vbox) gtk_widget_unref (control->priv->vbox);
        control->priv->vbox = NULL;

        GTK_OBJECT_CLASS (gnocam_control_parent_class)->destroy (object);
	
printf ("END: gnocam_control_destroy\n");

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


