#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bonobo.h>
#include <gal/util/e-util.h>

#include "gnocam-control-file.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_file_parent_class = NULL;

struct _GnoCamControlFilePrivate
{
	/* Nothing */
};

static void
activate (BonoboControl* object, gboolean state)
{
        if (state) {
                g_warning ("Menu should be created here...");
        } else {
                BonoboUIComponent*	component;

                component = bonobo_control_get_ui_component (object);
                bonobo_ui_component_unset_container (component);
        }

        if (BONOBO_CONTROL_CLASS (gnocam_control_file_parent_class)->activate)
                BONOBO_CONTROL_CLASS (gnocam_control_file_parent_class)->activate (object, state);
}

static void
destroy (GtkObject* object)
{
	GnoCamControlFile* file;

	file = GNOCAM_CONTROL_FILE (object);
	g_free (file->priv);
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
gnocam_control_file_new (void)
{
	GnoCamControlFile*	new;

	new = gtk_type_new (gnocam_control_file_get_type ());

	return (new);
}

E_MAKE_TYPE (gnocam_control_file, "GnoCamControlFile", GnoCamControlFile, class_init, init, PARENT_TYPE)


