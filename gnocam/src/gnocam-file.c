#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-file.h"

#include <gal/util/e-util.h>

#include "utils.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* gnocam_file_parent_class;

struct _GnoCamFilePrivate
{
	GtkWidget*		widget;
	BonoboUIComponent*	component;
	
	Camera*			camera;
	CameraWidget*		configuration;
	
	gchar*			dirname;
	gchar*			filename;
};

#define GNOCAM_FILE_UI 						\
"<placeholder name=\"FileOperations\">"				\
"  <placeholder name=\"Delete\" delimit=\"top\"/>"		\
"  <placeholder name=\"Configuration\"/>"			\
"</placeholder>"

#define GNOCAM_FILE_UI_DELETE 										\
"<placeholder name=\"Delete\">"										\
"  <menuitem name=\"delete\" _label=\"Delete\" verb=\"\" pixtype=\"stock\" pixname=\"Delete\"/>"	\
"</placeholder>"

/*************/
/* Callbacks */
/*************/

static void
on_delete_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFile*	file;
	gint		result;

	file = GNOCAM_FILE (user_data);

	result = gp_camera_file_delete (file->priv->camera, file->priv->dirname, file->priv->filename);

	if (result != GP_OK) {
		g_warning ("Could not delete file '%s' in folder '%s'! (%s)", file->priv->filename, file->priv->dirname, 
			gp_camera_result_as_string (file->priv->camera, result));
		return;
	}
	
	g_warning ("Implement: Delete the file from the storage view!");
}

/*****************/
/* Our functions */
/*****************/

BonoboUIComponent*
gnocam_file_get_ui_component (GnoCamFile* file)
{
	return (file->priv->component);
}

void
gnocam_file_set_ui_container (GnoCamFile* file, Bonobo_UIContainer container)
{
	bonobo_ui_component_set_container (file->priv->component, container);
}

GtkWidget*
gnocam_file_get_widget (GnoCamFile* file)
{
	return (file->priv->widget);
}

/****************************/
/* Bonobo-X-Object specific */
/****************************/

static void
gnocam_file_destroy (GtkObject* object)
{
	GnoCamFile* file;

	file = GNOCAM_FILE (object);

	gp_camera_unref (file->priv->camera);
	if (file->priv->configuration) gp_widget_unref (file->priv->configuration);
	
	g_free (file->priv->dirname);
	g_free (file->priv->filename);
	
	g_free (file->priv);
}

static void
gnocam_file_class_init (GnoCamFileClass* klass)
{
	GtkObjectClass* 		object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_file_destroy;

	gnocam_file_parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_file_init (GnoCamFile* file)
{
	GnoCamFilePrivate*	priv;

	priv = g_new (GnoCamFilePrivate, 1);
	file->priv = priv;
	file->priv->configuration = NULL;
}

GnoCamFile*
gnocam_file_new (Camera* camera, Bonobo_Storage storage, const gchar* path, Bonobo_UIContainer container, CORBA_Environment* ev)
{
	GnoCamFile*		new;
	Bonobo_Unknown		object;
	Bonobo_Unknown		subcontrol;
	Bonobo_Persist		persist;
	Bonobo_Stream		stream;
	Bonobo_StorageInfo*	info;
	gchar*			oaf_requirements;
	gchar*			mime_type;
	gint			result;
	OAF_ActivationID 	ret_id;

	/* Open the stream */
	stream = Bonobo_Storage_openStream (storage, path, Bonobo_Storage_READ | Bonobo_Storage_COMPRESSED, ev);
	if (BONOBO_EX (ev)) return (NULL);
	g_return_val_if_fail (stream, NULL);

	/* Get information about the stream */
	info = Bonobo_Stream_getInfo (stream, Bonobo_FIELD_CONTENT_TYPE, ev);
	if (BONOBO_EX (ev)) return (NULL);
	g_return_val_if_fail (info, NULL);
	mime_type = g_strdup (info->content_type);
	CORBA_free (info);
	
	oaf_requirements = g_strdup_printf (
		"bonobo:supported_mime_types.has ('%s') AND "
		"repo_ids.has ('IDL:Bonobo/Control:1.0') AND "
		"repo_ids.has ('IDL:Bonobo/PersistStream:1.0')", mime_type);
	
	/* Activate the object */
	object = oaf_activate (oaf_requirements, NULL, 0, &ret_id, ev);
	g_free (oaf_requirements);
	if (BONOBO_EX (ev)) {
		g_free (mime_type);
		return (NULL);
	}
	g_return_val_if_fail (object, NULL);

	/* Get the persist stream interface */
	 persist = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/PersistStream:1.0", ev);
	 if (BONOBO_EX (ev)) {
		g_free (mime_type);
		return (NULL);
	}
	g_return_val_if_fail (persist, NULL);

	/* Load the persist stream */
	Bonobo_PersistStream_load (persist, stream, (const Bonobo_Persist_ContentType) mime_type, ev);
	g_free (mime_type);
	if (BONOBO_EX (ev)) return (NULL);

	/* Release */
	bonobo_object_release_unref (persist, ev);
	bonobo_object_release_unref (stream, ev);

	subcontrol = bonobo_moniker_util_qi_return (object, "IDL:Bonobo/Control:1.0", ev);
	if (BONOBO_EX (ev)) return (NULL);

	new = gtk_type_new (gnocam_file_get_type ());
	new->priv->filename = g_strdup (g_basename (path));
	new->priv->camera = camera;
	gp_camera_ref (camera);
	new->priv->component = bonobo_ui_component_new ("GnoCamFile");

	/* Dirname? */
	new->priv->dirname = g_dirname (path);
	if (!strcmp (new->priv->dirname, ".")) {
		g_free (new->priv->dirname);
		new->priv->dirname = g_strdup ("/");
	}

	new->priv->widget = bonobo_widget_new_control_from_objref (subcontrol, container);

	/* Create menu */
        bonobo_ui_component_set_container (new->priv->component, container);
        bonobo_ui_component_set_translate (new->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI, NULL);

        /* Delete? */
        if (new->priv->camera->abilities->file_delete) {
                bonobo_ui_component_set_translate (new->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI_DELETE, NULL);
                bonobo_ui_component_add_verb (new->priv->component, "delete", on_delete_clicked, new);
        }

        /* File Configuration? */
        result = gp_camera_file_config_get (new->priv->camera, &(new->priv->configuration), new->priv->dirname, new->priv->filename);
        if (result == GP_OK) menu_setup (new->priv->component, new->priv->camera, new->priv->configuration, "/menu/File/FileOperations", 
		new->priv->dirname, new->priv->filename); 

	return (new);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamFile, GNOME_GnoCam_file, PARENT_TYPE, gnocam_file);
									   

