#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-control-folder.h"

#include <gal/util/e-util.h>

#include "utils.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_folder_parent_class = NULL;

struct _GnoCamControlFolderPrivate
{
	Camera*	camera;
	
	gchar*	path;
};

/************************/
/* Bonobo-Control stuff */
/************************/

static void
activate (BonoboControl* object, gboolean state)
{
        BonoboUIComponent*      component;
        Bonobo_UIContainer      container;
        GnoCamControlFolder*	control;

        control = GNOCAM_CONTROL_FOLDER (object);
        container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
        component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));
        bonobo_ui_component_set_container (component, container);

	if (state) {
		gint		result;
		CameraWidget*	widget = NULL;

		/* Create the menu */
		result = gp_camera_folder_config_get (control->priv->camera, &widget, control->priv->path);
		if (result == GP_OK) menu_setup (object, control->priv->camera, widget, "Folder Configuration", control->priv->path, NULL);
	} else {
		bonobo_ui_component_unset_container (component);
	}
	bonobo_object_release_unref (container, NULL);

	if (BONOBO_CONTROL_CLASS (gnocam_control_folder_parent_class)->activate) 
		BONOBO_CONTROL_CLASS (gnocam_control_folder_parent_class)->activate (object, state);
}

static void
destroy (GtkObject* object)
{
	GnoCamControlFolder* folder;

	folder = GNOCAM_CONTROL_FOLDER (object);

	if (folder->priv->camera) gp_camera_unref (folder->priv->camera);
	if (folder->priv->path) g_free (folder->priv->path);
	
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
gnocam_control_folder_new (Camera* camera, Bonobo_Storage storage, const gchar* path)
{
	GtkWidget*			widget;
	GnoCamControlFolder*		new;
	Bonobo_Storage_DirectoryList*   list;
	gchar*				row [] = {NULL, NULL, NULL};
	gint				i;
	gchar*				name;
	CORBA_Environment		ev;

	CORBA_exception_init (&ev);
        list = Bonobo_Storage_listContents (storage, "", Bonobo_FIELD_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		return (NULL);
	}
	CORBA_exception_free (&ev);

	new = gtk_type_new (gnocam_control_folder_get_type ());
	new->priv->camera = camera;
	new->priv->path = g_strdup (path);
	gp_camera_ref (camera);

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

