#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-file.h"

#include <gal/util/e-util.h>

#include "e-clipped-label.h"
#include "utils.h"
#include "gnocam-configuration.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* parent_class;

struct _GnoCamFilePrivate
{
	GtkWindow*		window;
	BonoboUIContainer*	container;

	BonoboUIComponent*      component;

	BonoboStorage*		storage;

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

enum {
	WIDGET_CHANGED,
        LAST_SIGNAL
};

static unsigned int signals[LAST_SIGNAL] = { 0 };

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
set_container (gpointer user_data)
{
	GnoCamFile*     	file;

	g_return_val_if_fail (GNOCAM_IS_FILE (user_data), FALSE);
	file = GNOCAM_FILE (user_data);

	if (!file->priv->component) return (TRUE);

	if (bonobo_ui_component_get_container (file->priv->component) == BONOBO_OBJREF (file->priv->container)) return (FALSE);

	bonobo_ui_component_set_container (file->priv->component, BONOBO_OBJREF (file->priv->container));
        if (file->priv->control) Bonobo_Control_activate (file->priv->control, TRUE, NULL);

	return (FALSE);
}
	
static gint
create_menu (gpointer user_data)
{
	GnoCamFile*	file;

	g_return_val_if_fail (user_data, FALSE);
	file = GNOCAM_FILE (user_data);

	file->priv->component = bonobo_ui_component_new (PACKAGE "File");
	bonobo_ui_component_set_container (file->priv->component, BONOBO_OBJREF (file->priv->container));

        bonobo_ui_component_set_translate (file->priv->component, "/menu/File", GNOCAM_FILE_UI, NULL);

        /* Delete? */
        if (file->priv->camera->abilities->file_delete) {
                bonobo_ui_component_set_translate (file->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI_DELETE, NULL);
                bonobo_ui_component_add_verb (file->priv->component, "Delete", on_delete_clicked, file);
        }

        /* File Configuration? */
	if (file->priv->camera->abilities->config & GP_CONFIG_FILE) {
		bonobo_ui_component_set_translate (file->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (file->priv->component, "Configuration", on_config_clicked, file);
	}

	return (FALSE);
}

static void
create_error_widget (GnoCamFile* file, CORBA_Environment* ev)
{
	gchar*	tmp;

	g_return_if_fail (file);

        /* Create label with error message */
        tmp = g_strdup_printf (_("Could not display '%s': %s!"), file->priv->path, bonobo_exception_get_text (ev));
        file->priv->widget = e_clipped_label_new (tmp);
        gtk_widget_ref (file->priv->widget);
        g_free (tmp);
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

	/* Clean up */
	if (file->priv->control != CORBA_OBJECT_NIL) {
		CORBA_exception_init (&ev);
		Bonobo_Control_activate (file->priv->control, FALSE, &ev);
		file->priv->control = CORBA_OBJECT_NIL;
		CORBA_exception_free (&ev);
	}
	if (file->priv->widget != NULL) {
		gtk_widget_unref (file->priv->widget);
		file->priv->widget = NULL;
	}

	CORBA_exception_init (&ev);

        /* Open the stream */
	mode = Bonobo_Storage_READ;
	if (file->priv->camera->abilities->file_preview && gconf_client_get_bool (file->priv->client, "/apps/" PACKAGE "/preview", NULL))
		mode |= Bonobo_Storage_COMPRESSED;
	stream = Bonobo_Storage_openStream (BONOBO_OBJREF (file->priv->storage), file->priv->path, mode, &ev);
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not get stream");
		goto exit_clean;
	}
        g_return_if_fail (stream);

        /* Get information about the stream */
        info = Bonobo_Stream_getInfo (stream, Bonobo_FIELD_CONTENT_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not get info");
		goto exit_clean;
	}
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
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not get object");
		goto exit_clean;
	}
        g_return_if_fail (object != CORBA_OBJECT_NIL);

        /* Get the persist stream interface */
         persist = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/PersistStream:1.0", &ev);
         if (BONOBO_EX (&ev)) {
	 	g_warning ("Could not get persist");
		goto exit_clean;
	}
        g_return_if_fail (persist != CORBA_OBJECT_NIL);

        /* Load the persist stream */
        Bonobo_PersistStream_load (persist, stream, (const Bonobo_Persist_ContentType) mime_type, &ev);
        g_free (mime_type);
	mime_type = NULL;
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not load");
		goto exit_clean;
	}

	/* Get the control */
	file->priv->control = Bonobo_Unknown_queryInterface (object, "IDL:Bonobo/Control:1.0", &ev);
        if (BONOBO_EX (&ev)) {
		g_warning ("Could not get control");
		goto exit_clean;
	}
	g_return_if_fail (file->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_free (&ev);

	file->priv->widget = bonobo_widget_new_control_from_objref (file->priv->control, BONOBO_OBJREF (file->priv->container));
	gtk_widget_ref (file->priv->widget);

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

static void
on_preview_changed (GConfClient *client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GnoCamFile*	file;

	file = GNOCAM_FILE (user_data);

	create_widget (file);
	gtk_signal_emit (GTK_OBJECT (file), signals [WIDGET_CHANGED]);
}

static void
on_config_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamFile*	file;
	GtkWidget*	widget;

	file = GNOCAM_FILE (user_data);

	widget = gnocam_configuration_new (file->priv->camera, file->priv->dirname, file->priv->filename, file->priv->window);
	if (!widget) return;
	gtk_widget_show (widget);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_file_hide_menu (GnoCamFile* file)
{
	bonobo_ui_component_unset_container (file->priv->component);
	if (file->priv->control) Bonobo_Control_activate (file->priv->control, FALSE, NULL);
}

void
gnocam_file_show_menu (GnoCamFile* file)
{
	gtk_idle_add (set_container, file);
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

        signals[WIDGET_CHANGED] = gtk_signal_new ("widget_changed",
                                        GTK_RUN_LAST,
                                        object_class->type,
                                        GTK_SIGNAL_OFFSET (GnoCamFileClass, widget_changed),
					gtk_marshal_NONE__NONE, 
                                        GTK_TYPE_NONE, 0);

        gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_file_init (GnoCamFile* file)
{
	file->priv = g_new0 (GnoCamFilePrivate, 1);
	file->priv->widget = NULL;
	file->priv->control = CORBA_OBJECT_NIL;
}

GnoCamFile*
gnocam_file_new (Camera* camera, BonoboStorage* storage, const gchar* path, BonoboUIContainer* container, GConfClient* client, GtkWindow* window)
{
	GnoCamFile*		new;

	g_return_val_if_fail (camera, NULL);

	new = gtk_type_new (GNOCAM_TYPE_FILE);
	new->priv->filename = g_strdup (g_basename (path));
	new->priv->path = g_strdup (path);
	new->priv->container = container;
	new->priv->window = window;
	gp_camera_ref (new->priv->camera = camera);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));
	new->priv->cnxn = gconf_client_notify_add (client, "/apps/" PACKAGE "/preview", on_preview_changed, new, NULL, NULL);
	new->priv->storage = storage;
	
        /* Dirname? */  
        new->priv->dirname = g_dirname (path);  
        if (!strcmp (new->priv->dirname, ".")) {
                g_free (new->priv->dirname);    
                new->priv->dirname = g_strdup ("/");    
        }       

	/* Create the menu */
	gtk_idle_add (create_menu, new);

	create_widget (new);

	return (new);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamFile, GNOME_GnoCam_file, PARENT_TYPE, gnocam_file);
									   

