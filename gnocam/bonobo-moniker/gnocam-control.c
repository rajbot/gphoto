#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-control.h"

#include <gal/util/e-util.h>
//#include <stdio.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <gphoto-extensions.h>

#include "utils.h"
#include "gnocam-control-folder.h"
#include "gnocam-control-file.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_parent_class = NULL;

struct _GnoCamControlPrivate
{
	/* Nothing in here yet. */
};

static void
on_selection_changed (GtkCList* folder, gint row, gint column, GdkEvent* event)
{
	g_warning ("Selection changed!");
}

static void
activate (BonoboControl* object, gboolean state)
{
        if (state) menu_create (GNOCAM_CONTROL (object));
	else bonobo_ui_component_unset_container (bonobo_control_get_ui_component (object));

        if (BONOBO_CONTROL_CLASS (gnocam_control_parent_class)->activate) BONOBO_CONTROL_CLASS (gnocam_control_parent_class)->activate (object, state);
}

GnoCamControl*
gnocam_control_new (BonoboMoniker *moniker, const Bonobo_ResolveOptions *options, CORBA_Environment* ev)
{
        GnoCamControl*		new;
	Bonobo_Unknown 		subcontrol;
	gint			i, result;
	const gchar*		name;
	GtkWidget*		vbox;
	Camera*			camera = NULL;
	CORBA_Environment	ev_internal;

	/* Create the camera. */
	name = bonobo_moniker_get_name (moniker);
	if ((result = gp_camera_new_from_gconf (&camera, name)) != GP_OK) {
		g_warning (_("Could not create camera '%s' (%s)!"), name, gp_result_as_string (result));
		return (NULL);
	}

	/* Create the control. */
	new = gtk_type_new (gnocam_control_get_type ());
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
	g_return_val_if_fail (bonobo_control_construct (BONOBO_CONTROL (new), vbox), NULL);

	/* Initialize our variables. */
	for (i = 2; name[i] != 0; i++) if (name[i] == '/') break;
	new->path = g_strdup (name + i);
	new->config_camera = NULL;
	new->config_folder = NULL;
	new->config_file = NULL;
	new->camera = camera;

	/* Display something... */
	if (new->path [strlen (new->path) - 1] == '/') {
		Bonobo_Storage	storage;
		gchar*		tmp;

		/* Display the directory's contents. */
		CORBA_exception_init (&ev_internal);
		tmp = g_strconcat (bonobo_moniker_get_prefix (moniker), name, NULL);
		storage = bonobo_get_object (tmp, "IDL:Bonobo/Storage:1.0", &ev_internal);
		g_free (tmp);
		if (BONOBO_EX (&ev_internal)) {
			g_warning ("Could not create storage for '%s': %s!", name, bonobo_exception_get_text (&ev_internal));
		} else {
			GnoCamControlFolder*	folder;
			
			folder = gnocam_control_folder_new (storage, &ev_internal);
			if (BONOBO_EX (&ev_internal)) {
				g_warning ("Could not create folder control: %s!", bonobo_exception_get_text (&ev_internal));
			} else {
				GtkWidget*	widget;

				widget = bonobo_control_get_widget (BONOBO_CONTROL (folder));
				gtk_container_add (GTK_CONTAINER (vbox), widget);

				/* Connect the signals */
				gtk_signal_connect (GTK_OBJECT (widget), "select_row", (GtkSignalFunc) on_selection_changed, NULL);
				gtk_signal_connect (GTK_OBJECT (widget), "unselect_row", (GtkSignalFunc) on_selection_changed, NULL);
			}
		}
	} else {

		/* Create the viewer */
		CORBA_exception_init (&ev_internal);
		subcontrol = bonobo_moniker_use_extender ("OAFIID:Bonobo_MonikerExtender_stream", moniker, options, "IDL:Bonobo/Control:1.0", &ev_internal);
		if (BONOBO_EX (&ev_internal)) {
			g_warning ("Could not create viewer for '%s': %s!", name, bonobo_exception_get_text (&ev_internal));
		} else {
			GtkWidget*		widget;
			Bonobo_UIContainer	container;
			
			container = bonobo_ui_component_get_container (bonobo_control_get_ui_component (BONOBO_CONTROL (new)));
			gtk_widget_show (widget =  bonobo_widget_new_control_from_objref (subcontrol, container));
			gtk_container_add (GTK_CONTAINER (vbox), widget);
		}
		CORBA_exception_free (&ev_internal);
	}

	return (new);
}

static void
destroy (GtkObject* object)
{
	GnoCamControl*  control;
	
	control = GNOCAM_CONTROL (object);

	if (control->config_camera) gp_widget_unref (control->config_camera); 
	if (control->config_folder) gp_widget_unref (control->config_folder); 
	if (control->config_file) gp_widget_unref (control->config_file); 
	if (control->camera) gp_camera_unref (control->camera);
	if (control->path) g_free (control->path);

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


