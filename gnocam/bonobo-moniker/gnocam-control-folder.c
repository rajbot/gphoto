#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <gal/util/e-util.h>

#include "gnocam-control-folder.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_folder_parent_class = NULL;

struct _GnoCamControlFolderPrivate
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

	if (BONOBO_CONTROL_CLASS (gnocam_control_folder_parent_class)->activate) 
		BONOBO_CONTROL_CLASS (gnocam_control_folder_parent_class)->activate (object, state);
}

static void
destroy (GtkObject* object)
{
	GnoCamControlFolder* folder;

	folder = GNOCAM_CONTROL_FOLDER (object);
	g_free (folder->priv);
}

static void
class_init (GnoCamControlFolderClass* klass)
{
	GtkObjectClass* 	object_class;
	BonoboControlClass* 	control_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = destroy;

	control_class = BONOBO_CONTROL_CLASS (klass);
	control_class->activate = activate;

	gnocam_control_folder_parent_class = gtk_type_class (bonobo_control_get_type ());
}

static void
init (GnoCamControlFolder* folder)
{
	GnoCamControlFolderPrivate*	priv;

	priv = g_new (GnoCamControlFolderPrivate, 1);
	folder->priv = priv;
}

GnoCamControlFolder*
gnocam_control_folder_new (Bonobo_Storage storage, CORBA_Environment* ev)
{
	GtkWidget*			widget;
	GnoCamControlFolder*		new;
	Bonobo_Storage_DirectoryList*   list;
	gchar*				row [] = {NULL, NULL, NULL};
	gint				i;

        list = Bonobo_Storage_listContents (storage, "", Bonobo_FIELD_TYPE, ev);
        if (BONOBO_EX (ev)) return (NULL);

	new = gtk_type_new (gnocam_control_folder_get_type ());

        widget = gtk_clist_new (2);
        gtk_widget_show (widget);
        for (i = 0; i < list->_length; i++) {
		switch (list->_buffer [i].type) {
                case Bonobo_STORAGE_TYPE_DIRECTORY:
                        row [0] = g_strdup ("Directory");
                        break;
                case Bonobo_STORAGE_TYPE_REGULAR:
                        row [0] = g_strdup ("File");
                        break;
		default:
			row [0] = g_strdup ("Unknown");
			break;
                }
                row [1] = g_strdup (list->_buffer [i].name);
                gtk_clist_append (GTK_CLIST (widget), row);
        }

	bonobo_control_construct (BONOBO_CONTROL (new), widget);

	return (new);
}

E_MAKE_TYPE (gnocam_control_folder, "GnoCamControlFolder", GnoCamControlFolder, class_init, init, PARENT_TYPE)

