#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-file.h"

#include <gal/util/e-util.h>

#include "e-clipped-label.h"
#include "utils.h"
#include "gnocam-configuration.h"

#define PARENT_TYPE GTK_TYPE_VBOX
static GtkVBoxClass* parent_class;

struct _GnoCamFilePrivate
{
	Bonobo_UIContainer	container;

	BonoboUIComponent*      component;

	Bonobo_Storage		storage;

	Bonobo_Control		control;
	GtkWidget*		widget;

	GConfClient*		client;
	guint			cnxn;
	
	Camera*			camera;
	CameraWidget*		configuration;
	
	gchar*			dirname;
	gchar*			filename;
	gchar*			path;
};

#define GNOCAM_FILE_UI 						\
"<placeholder name=\"FileOperations\">"				\
"  <placeholder name=\"Delete\" delimit=\"top\"/>"		\
"  <placeholder name=\"Configuration\"/>"			\
"</placeholder>"

#define GNOCAM_FILE_UI_DELETE 												\
"<placeholder name=\"Delete\">"												\
"  <menuitem name=\"Delete\" _label=\"Delete\" verb=\"\" pixtype=\"stock\" pixname=\"Delete\"/>"			\
"</placeholder>"

#define GNOCAM_FILE_UI_CONFIGURATION											\
"<placeholder name=\"Configuration\">"											\
"  <menuitem name=\"Configuration\" _label=\"Configuration\" verb=\"\" pixtype=\"stock\" pixname=\"Properties\"/>"	\
"</placeholder>"													\

/**************/
/* Prototypes */
/**************/

static void 	on_delete_clicked 	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_config_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

/********************/
/* Helper functions */
/********************/

static gint
create_menu (gpointer user_data)
{
	GnoCamFile*	file;

	g_return_val_if_fail (user_data, FALSE);
	file = GNOCAM_FILE (user_data);

        bonobo_ui_component_set_translate (file->priv->component, "/menu/File", GNOCAM_FILE_UI, NULL);

        /* Delete? */
        if (file->priv->camera->abilities->file_operations & GP_FILE_OPERATION_DELETE) {
                bonobo_ui_component_set_translate (file->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI_DELETE, NULL);
                bonobo_ui_component_add_verb (file->priv->component, "Delete", on_delete_clicked, file);
        }

        /* File Configuration? */
	if (file->priv->camera->abilities->file_operations & GP_FILE_OPERATION_CONFIG) {
		bonobo_ui_component_set_translate (file->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (file->priv->component, "Configuration", on_config_clicked, file);
	}

	return (FALSE);
}

static void
create_error_widget (GnoCamFile* file, CORBA_Environment* ev)
{
	gchar*		tmp;

	g_return_if_fail (file);

        /* Create label with error message */
        tmp = g_strdup_printf (_("Could not display '%s' in folder '%s': %s!"), file->priv->filename, file->priv->dirname, bonobo_exception_get_text (ev));
        file->priv->widget = e_clipped_label_new (tmp);
        g_free (tmp);
	gtk_widget_show (file->priv->widget);
	gtk_container_add (GTK_CONTAINER (file), file->priv->widget);
}

static void
create_widget (GnoCamFile* file)
{
	Bonobo_Stream		stream = CORBA_OBJECT_NIL;
	Bonobo_Unknown		object = CORBA_OBJECT_NIL;
	Bonobo_Persist		persist = CORBA_OBJECT_NIL;
	Bonobo_Storage_OpenMode	mode;
	Bonobo_StorageInfo*	info;
	gchar*			oaf_requirements;
	gchar*			mime_type = NULL;
	OAF_ActivationID        ret_id;
	CORBA_Environment	ev;

	CORBA_exception_init (&ev);

        /* Open the stream */
	mode = Bonobo_Storage_READ;
	if ((file->priv->camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW) && 
	    gconf_client_get_bool (file->priv->client, "/apps/" PACKAGE "/preview", NULL))
		mode |= Bonobo_Storage_COMPRESSED;
	stream = Bonobo_Storage_openStream (file->priv->storage, file->priv->filename, mode, &ev);
        if (BONOBO_EX (&ev)) {
		g_warning (_("Could not open stream for file '%s' in folder '%s': %s!"), file->priv->filename, file->priv->dirname, bonobo_exception_get_text (&ev));
		goto exit_clean;
	}
        g_return_if_fail (stream);

        /* Get information about the stream */
        info = Bonobo_Stream_getInfo (stream, Bonobo_FIELD_CONTENT_TYPE, &ev);
        if (BONOBO_EX (&ev)) goto exit_clean;
        g_return_if_fail (info);
        mime_type = g_strdup (info->content_type);
        CORBA_free (info);

        oaf_requirements = g_strdup_printf (
                "bonobo:supported_mime_types.has ('%s') AND "
                "repo_ids.has ('IDL:Bonobo/Control:1.0') AND "
                "repo_ids.has ('IDL:Bonobo/PersistStream:1.0')", mime_type);

        /* Activate the object */
	object = oaf_activate (oaf_requirements, NULL, 0, &ret_id, &ev);
        g_free (oaf_requirements);
        if (BONOBO_EX (&ev)) goto exit_clean;
        g_return_if_fail (object != CORBA_OBJECT_NIL);

        /* Get the persist stream interface */
         persist = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/PersistStream:1.0", &ev);
         if (BONOBO_EX (&ev)) goto exit_clean;
        g_return_if_fail (persist != CORBA_OBJECT_NIL);

        /* Load the persist stream */
        Bonobo_PersistStream_load (persist, stream, (const Bonobo_Persist_ContentType) mime_type, &ev);
        g_free (mime_type);
	mime_type = NULL;
        if (BONOBO_EX (&ev)) goto exit_clean;

	/* Get the control */
	file->priv->control = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/Control:1.0", &ev);
        if (BONOBO_EX (&ev)) goto exit_clean;
	g_return_if_fail (file->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_free (&ev);

	file->priv->widget = bonobo_widget_new_control_from_objref (file->priv->control, file->priv->container);
	gtk_widget_show (file->priv->widget);
	gtk_container_add (GTK_CONTAINER (file), file->priv->widget);

	return;

  exit_clean:

	create_error_widget (file, &ev);

	/* Clean up */
	if (mime_type) g_free (mime_type);
	if (stream != CORBA_OBJECT_NIL) bonobo_object_release_unref (stream, &ev);
	if (object != CORBA_OBJECT_NIL) bonobo_object_release_unref (object, NULL);
	if (persist != CORBA_OBJECT_NIL) bonobo_object_release_unref (persist, NULL);
	CORBA_exception_free (&ev);

	return;
}

/*************/
/* Callbacks */
/*************/

static void
on_delete_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	GnoCamFile*	file;
	gint		result;

	file = GNOCAM_FILE (user_data);

	result = gp_camera_file_delete (file->priv->camera, file->priv->dirname, file->priv->filename);

	if (result != GP_OK) {
		g_warning ("Could not delete file '%s' in folder '%s'! (%s)", file->priv->filename, file->priv->dirname, 
			gp_camera_get_result_as_string (file->priv->camera, result));
		return;
	}

	camera = GNOCAM_CAMERA (gtk_widget_get_ancestor (GTK_WIDGET (file), GNOCAM_TYPE_CAMERA));
	gtk_signal_emit_by_name (GTK_OBJECT (camera), "folder_updated", file->priv->dirname);
}

static void
on_preview_changed (GConfClient *client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GnoCamFile*		file;

	file = GNOCAM_FILE (user_data);

        /* Clean up */
        if (file->priv->widget != NULL) {
		gtk_container_remove (GTK_CONTAINER (file), file->priv->widget);
                file->priv->widget = NULL;
        }

	create_widget (file);
}

static void
on_config_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFile*	file;
	GtkWidget*	widget;
	GtkWindow*	window;

	file = GNOCAM_FILE (user_data);
	window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (file), GTK_TYPE_WINDOW));

	widget = gnocam_configuration_new (file->priv->camera, file->priv->dirname, file->priv->filename, window);
	if (!widget) return;
	gtk_widget_show (widget);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_file_hide_menu (GnoCamFile* file)
{
	if (file->priv->control) Bonobo_Control_activate (file->priv->control, FALSE, NULL);
	bonobo_ui_component_unset_container (file->priv->component);
}

void
gnocam_file_show_menu (GnoCamFile* file)
{
	/* Do we already show the menu? */
	if (bonobo_ui_component_get_container (file->priv->component) == file->priv->container) 
		return;

	bonobo_ui_component_set_container (file->priv->component, file->priv->container);
	if (file->priv->control) Bonobo_Control_activate (file->priv->control, TRUE, NULL);
	gtk_idle_add (create_menu, file);
}

/****************************/
/* Bonobo-X-Object specific */
/****************************/

static void
gnocam_file_destroy (GtkObject* object)
{
	GnoCamFile* file;

	file = GNOCAM_FILE (object);

	bonobo_object_unref (BONOBO_OBJECT (file->priv->component));

	if (file->priv->control) {
		Bonobo_Control_unref (file->priv->control, NULL);
		CORBA_Object_release (file->priv->control, NULL);
	}

	if (file->priv->widget) gtk_widget_unref (file->priv->widget);

	gp_camera_unref (file->priv->camera);
	if (file->priv->configuration) gp_widget_unref (file->priv->configuration);

	gtk_object_unref (GTK_OBJECT (file->priv->client));
	gconf_client_notify_remove (file->priv->client, file->priv->cnxn);
	
	g_free (file->priv->dirname);
	g_free (file->priv->filename);
	g_free (file->priv->path);
	
	g_free (file->priv);
	file->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_file_class_init (GnoCamFileClass* klass)
{
	GtkObjectClass* 		object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_file_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_file_init (GnoCamFile* file)
{
	file->priv = g_new0 (GnoCamFilePrivate, 1);
	file->priv->widget = NULL;
	file->priv->control = CORBA_OBJECT_NIL;
}

GtkWidget*
gnocam_file_new (GnoCamCamera* camera, const gchar* path)
{
	GnoCamFile*		new;
	Bonobo_Storage		storage;
	Bonobo_Storage		storage_new;
	Bonobo_Storage_OpenMode	mode;
	gchar*			dirname;
	CORBA_Environment	ev;
	Camera*			cam;
	GConfClient*		client;

	g_return_val_if_fail (camera, NULL);

	/* Open the storage */
	dirname = g_dirname (path);
	if (!strcmp (dirname, ".")) {
		g_free (dirname);
		dirname = g_strdup ("/");
	}
	mode = Bonobo_Storage_READ;
	cam = gnocam_camera_get_camera (camera);
	client = gnocam_camera_get_client (camera);
	if ((cam->abilities->file_operations & GP_FILE_OPERATION_PREVIEW) && gconf_client_get_bool (client, "/apps/" PACKAGE "/preview", NULL))
		mode |= Bonobo_Storage_COMPRESSED;
	storage = gnocam_camera_get_storage (camera);
	CORBA_exception_init (&ev);
	storage_new = Bonobo_Storage_openStorage (storage, dirname + 1, mode, &ev);
	bonobo_object_release_unref (storage, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning (_("Could not open storage for '%s': %s!"), dirname, bonobo_exception_get_text (&ev));
		gtk_object_unref (GTK_OBJECT (client));
		g_free (dirname);
		gp_camera_unref (cam);
		CORBA_exception_free (&ev);
		return (NULL);
	}

	/* Create the widget */
	new = gtk_type_new (GNOCAM_TYPE_FILE);
	gtk_box_set_homogeneous (GTK_BOX (new), FALSE);

	new->priv->filename = g_strdup (g_basename (path));
	new->priv->path = g_strdup (path);
	new->priv->container = gnocam_camera_get_container (camera);
	new->priv->camera = cam;
	new->priv->client = client;
	new->priv->cnxn = gconf_client_notify_add (new->priv->client, "/apps/" PACKAGE "/preview", on_preview_changed, new, NULL, NULL);
	new->priv->storage = storage_new;
	new->priv->dirname = dirname;

	/* Create the menu */
	new->priv->component = bonobo_ui_component_new (PACKAGE "File");
	bonobo_ui_component_set_container (new->priv->component, new->priv->container);
	gtk_idle_add (create_menu, new);

	create_widget (new);

	return (GTK_WIDGET (new));
}

E_MAKE_TYPE (gnocam_file, "GnoCamFile", GnoCamFile, gnocam_file_class_init, gnocam_file_init, PARENT_TYPE);

