#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-folder.h"

#include <gal/util/e-util.h>

#include "utils.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* gnocam_folder_parent_class;

struct _GnoCamFolderPrivate
{
	GtkWidget*		widget;
	BonoboUIComponent*	component;
	
	Camera*			camera;
	CameraWidget*		configuration;
	
	gchar*			path;
};

#define GNOCAM_FOLDER_UI	 				\
"<placeholder name=\"Folder\">"					\
"  <submenu name=\"Folder\" _label=\"Folder\">"			\
"    <placeholder name=\"Upload\"/>"				\
"    <placeholder name=\"Configuration\"/>"			\
"  </submenu>"							\
"</placeholder>"

#define GNOCAM_FOLDER_UI_UPLOAD 				\
"<placeholder name=\"Upload\">"					\
"  <menuitem name=\"Upload\" _label=\"Upload\" verb=\"\"/>"	\
"</placeholder/>"


/*************/
/* Callbacks */
/*************/

static void
on_upload_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFolder*	folder;

	folder = GNOCAM_FOLDER (user_data);
	
	g_warning ("Implement upload!");
}

/*****************/
/* Our functions */
/*****************/

BonoboUIComponent*
gnocam_folder_get_ui_component (GnoCamFolder* folder)
{
	return (folder->priv->component);
}

void
gnocam_folder_set_ui_container (GnoCamFolder* folder, Bonobo_UIContainer container)
{
        bonobo_ui_component_set_container (folder->priv->component, container);
}

GtkWidget*
gnocam_folder_get_widget (GnoCamFolder* folder)
{
        return (folder->priv->widget);
}

/****************************/
/* Bonobo-X-Object specific */
/****************************/

static void
gnocam_folder_destroy (GtkObject* object)
{
	GnoCamFolder* folder;

	folder = GNOCAM_FOLDER (object);

	if (folder->priv->configuration) gp_widget_unref (folder->priv->configuration);
	gp_camera_unref (folder->priv->camera);
	g_free (folder->priv->path);
	
	g_free (folder->priv);
}

static void
gnocam_folder_class_init (GnoCamFolderClass* klass)
{
	GtkObjectClass* 	object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_folder_destroy;

	gnocam_folder_parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_folder_init (GnoCamFolder* folder)
{
	GnoCamFolderPrivate*	priv;

	priv = g_new (GnoCamFolderPrivate, 1);
	folder->priv = priv;
	folder->priv->configuration = NULL;
}

GnoCamFolder*
gnocam_folder_new (Camera* camera, Bonobo_Storage storage, const gchar* path, Bonobo_UIContainer container)
{
	GnoCamFolder*			new;
	Bonobo_Storage_DirectoryList*   list;
	gchar*				row [] = {NULL, NULL, NULL};
	gint				i;
	gint				result;
	CORBA_Environment		ev;

	CORBA_exception_init (&ev);
        list = Bonobo_Storage_listContents (storage, "", Bonobo_FIELD_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		return (NULL);
	}
	CORBA_exception_free (&ev);

	new = gtk_type_new (gnocam_folder_get_type ());
	new->priv->camera = camera;
	new->priv->path = g_strdup (path);
	gp_camera_ref (camera);
	new->priv->component = bonobo_ui_component_new ("GnoCamFolder");

        gtk_widget_show (new->priv->widget = gtk_clist_new (2));
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
                gtk_clist_append (GTK_CLIST (new->priv->widget), row);
        }

	/* Create menu */
        bonobo_ui_component_set_container (new->priv->component, container);
        bonobo_ui_component_set_translate (new->priv->component, "/menu", GNOCAM_FOLDER_UI, NULL);

        /* Upload? */
        if (new->priv->camera->abilities->file_put) {
                bonobo_ui_component_set_translate (new->priv->component, "/menu/Folder/Folder", GNOCAM_FOLDER_UI_UPLOAD, NULL);
                bonobo_ui_component_add_verb (new->priv->component, "Upload", on_upload_clicked, new);
        }

        /* Folder configuration? */
        result = gp_camera_folder_config_get (new->priv->camera, &(new->priv->configuration), new->priv->path);
        if (result == GP_OK) menu_setup (new->priv->component, new->priv->camera, new->priv->configuration, "/menu/Folder/Folder", new->priv->path, NULL);

	return (new);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamFolder, GNOME_GnoCam_folder, PARENT_TYPE, gnocam_folder);

