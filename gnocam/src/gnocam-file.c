#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-file.h"

#include <gal/util/e-util.h>

#include "e-clipped-label.h"
#include "utils.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* gnocam_file_parent_class;

struct _GnoCamFilePrivate
{
	BonoboUIContainer*	container;
	BonoboUIComponent*      component;

	Bonobo_Storage		storage;

	Bonobo_Control		control;
	Bonobo_Unknown		object;
	GtkWidget*		widget;
	gboolean		error;

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

#define GNOCAM_FILE_UI_DELETE 										\
"<placeholder name=\"Delete\">"										\
"  <menuitem name=\"delete\" _label=\"Delete\" verb=\"\" pixtype=\"stock\" pixname=\"Delete\"/>"	\
"</placeholder>"

/**************/
/* Prototypes */
/**************/

static void 	on_delete_clicked 	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

/********************/
/* Helper functions */
/********************/

static gint
create_menu (gpointer user_data)
{
	GnoCamFile*	file;
	gint		result;

	g_return_val_if_fail (user_data, FALSE);
	file = GNOCAM_FILE (user_data);

        file->priv->component = bonobo_ui_component_new ("GnoCamFile");
        bonobo_ui_component_set_container (file->priv->component, BONOBO_OBJREF (file->priv->container));
        bonobo_ui_component_set_translate (file->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI, NULL);

        /* Delete? */
        if (file->priv->camera->abilities->file_delete) {
                bonobo_ui_component_set_translate (file->priv->component, "/menu/File/FileOperations", GNOCAM_FILE_UI_DELETE, NULL);
                bonobo_ui_component_add_verb (file->priv->component, "delete", on_delete_clicked, file);
        }

        /* File Configuration? */
        result = gp_camera_file_config_get (file->priv->camera, &(file->priv->configuration), file->priv->dirname, file->priv->filename);
        if (result == GP_OK) menu_setup (file->priv->component, file->priv->camera, file->priv->configuration, "/menu/File/FileOperations",
                file->priv->dirname, file->priv->filename);

	return (FALSE);
}

static gint
set_container (gpointer user_data)
{
	GnoCamFile*	file;

	file = GNOCAM_FILE (user_data);

	bonobo_ui_component_set_container (file->priv->component, BONOBO_OBJREF (file->priv->container));
	if (!file->priv->error && file->priv->control) Bonobo_Control_activate (file->priv->control, TRUE, NULL);

	return (FALSE);
}

static void
create_widget_from_control (GnoCamFile* file)
{
	/* Unref old widget */
	if (file->priv->widget) gtk_widget_unref (file->priv->widget);

	/* Get new widget */
	file->priv->widget = bonobo_widget_new_control_from_objref (file->priv->control, BONOBO_OBJREF (file->priv->container));
	gtk_widget_ref (file->priv->widget);

	file->priv->error = FALSE;
}

static void
create_error_widget (GnoCamFile* file, CORBA_Environment* ev)
{
	gchar*	tmp;

	g_return_if_fail (file);

	if (file->priv->widget) gtk_widget_unref (file->priv->widget);

        /* Create label with error message */
        tmp = g_strdup_printf (_("Could not display '%s': %s!"), file->priv->path, bonobo_exception_get_text (ev));
        file->priv->widget = e_clipped_label_new (tmp);
        gtk_widget_ref (file->priv->widget);
        g_free (tmp);

	file->priv->error = TRUE;
}

static void
create_widget (GnoCamFile* file)
{
	Bonobo_Stream		stream;
	Bonobo_Persist		persist;
	Bonobo_StorageInfo*	info;
	gchar*			oaf_requirements;
	gchar*			mime_type;
	OAF_ActivationID        ret_id;
	CORBA_Environment	ev;

	CORBA_exception_init (&ev);

        /* Open the stream */
	if (gconf_client_get_bool (file->priv->client, "/apps/" PACKAGE "/preview", NULL)) {
	        stream = Bonobo_Storage_openStream (file->priv->storage, file->priv->path, Bonobo_Storage_READ | Bonobo_Storage_COMPRESSED, &ev);
	} else {
		stream = Bonobo_Storage_openStream (file->priv->storage, file->priv->path, Bonobo_Storage_READ, &ev);
	}
        if (BONOBO_EX (&ev)) goto exit_clean;
        g_return_if_fail (stream);

        /* Get information about the stream */
        info = Bonobo_Stream_getInfo (stream, Bonobo_FIELD_CONTENT_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
                Bonobo_Stream_unref (stream, &ev);
		CORBA_Object_release (stream, &ev);
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
	file->priv->object = oaf_activate (oaf_requirements, NULL, 0, &ret_id, &ev);
        g_free (oaf_requirements);
        if (BONOBO_EX (&ev)) {
		Bonobo_Stream_unref (stream, &ev);
		CORBA_Object_release (stream, &ev);
                g_free (mime_type);
                goto exit_clean;
        }
        g_return_if_fail (file->priv->object != CORBA_OBJECT_NIL);

        /* Get the persist stream interface */
         persist = Bonobo_Unknown_queryInterface (file->priv->object, "IDL:Bonobo/PersistStream:1.0", &ev);
         if (BONOBO_EX (&ev)) {
		Bonobo_Unknown_unref (file->priv->object, &ev);
		CORBA_Object_release (file->priv->object, &ev);
		file->priv->object = CORBA_OBJECT_NIL;
		Bonobo_Stream_unref (stream, &ev);
		CORBA_Object_release (stream, &ev);
                g_free (mime_type);
                goto exit_clean;
        }
        g_return_if_fail (persist != CORBA_OBJECT_NIL);

        /* Load the persist stream */
        Bonobo_PersistStream_load (persist, stream, (const Bonobo_Persist_ContentType) mime_type, &ev);
        Bonobo_PersistStream_unref (persist, &ev);
	CORBA_Object_release (persist, &ev);
        Bonobo_Stream_unref (stream, &ev);
	CORBA_Object_release (stream, &ev);
        g_free (mime_type);
        if (BONOBO_EX (&ev)) {
		Bonobo_Unknown_unref (file->priv->object, &ev);
		CORBA_Object_release (file->priv->object, &ev);
		file->priv->object = CORBA_OBJECT_NIL;
		goto exit_clean;
	}

	file->priv->control = Bonobo_Unknown_queryInterface (file->priv->object, "IDL:Bonobo/Control:1.0", &ev);
        if (BONOBO_EX (&ev)) {
		Bonobo_Unknown_unref (file->priv->object, &ev);
		CORBA_Object_release (file->priv->object, &ev);
		file->priv->object = CORBA_OBJECT_NIL;
		goto exit_clean;
	}
	g_return_if_fail (file->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_free (&ev);

	create_widget_from_control (file);

	return;

  exit_clean:

	create_error_widget (file, &ev);
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
        Bonobo_Stream           stream;
        Bonobo_StorageInfo*     info;
        Bonobo_Persist          persist;
        gchar*                  mime_type;
        CORBA_Environment       ev;

	file = GNOCAM_FILE (user_data);

	if (file->priv->object == CORBA_OBJECT_NIL) {
		create_widget (file);
		gtk_signal_emit (GTK_OBJECT (file), signals [WIDGET_CHANGED]);
		return;
	}

        CORBA_exception_init (&ev);

        /* Open the stream */
        if (gconf_client_get_bool (file->priv->client, "/apps/" PACKAGE "/preview", NULL)) {
                stream = Bonobo_Storage_openStream (file->priv->storage, file->priv->path, Bonobo_Storage_READ | Bonobo_Storage_COMPRESSED, &ev);
        } else {
                stream = Bonobo_Storage_openStream (file->priv->storage, file->priv->path, Bonobo_Storage_READ, &ev);
        }
        if (BONOBO_EX (&ev)) goto exit_clean;
        g_return_if_fail (stream);

        /* Get information about the stream */
        info = Bonobo_Stream_getInfo (stream, Bonobo_FIELD_CONTENT_TYPE, &ev);
        if (BONOBO_EX (&ev)) {
                Bonobo_Stream_unref (stream, &ev);
                CORBA_Object_release (stream, &ev);
                goto exit_clean;
        }
        g_return_if_fail (info);
        mime_type = g_strdup (info->content_type);
        CORBA_free (info);

        /* Get the persist stream interface */
	persist = Bonobo_Unknown_queryInterface (file->priv->object, "IDL:Bonobo/PersistStream:1.0", &ev);
         if (BONOBO_EX (&ev)) {
                Bonobo_Stream_unref (stream, &ev);
                CORBA_Object_release (stream, &ev);
                g_free (mime_type);
                goto exit_clean;
        }
	g_return_if_fail (persist != CORBA_OBJECT_NIL);

        /* Load the persist stream */
        Bonobo_PersistStream_load (persist, stream, (const Bonobo_Persist_ContentType) mime_type, &ev);
        Bonobo_PersistStream_unref (persist, &ev);
        CORBA_Object_release (persist, &ev);
        Bonobo_Stream_unref (stream, &ev);
        CORBA_Object_release (stream, &ev);
        g_free (mime_type);
	if (BONOBO_EX (&ev)) goto exit_clean;

        CORBA_exception_free (&ev);

	if (file->priv->error) {
		create_widget_from_control (file);
		gtk_signal_emit (GTK_OBJECT (file), signals [WIDGET_CHANGED]);
	}

        return;

  exit_clean:

	if (file->priv->control) Bonobo_Control_activate (file->priv->control, FALSE, NULL);
	create_error_widget (file, &ev);
        CORBA_exception_free (&ev);

	/* Tell the world */
	gtk_signal_emit (GTK_OBJECT (file), signals [WIDGET_CHANGED]);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_file_hide_menu (GnoCamFile* file)
{
	bonobo_ui_component_unset_container (file->priv->component);
	if (!file->priv->error && file->priv->control) Bonobo_Control_activate (file->priv->control, FALSE, NULL);
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

	if (file->priv->object != CORBA_OBJECT_NIL) {
		Bonobo_Control_unref (file->priv->object, NULL);
		CORBA_Object_release (file->priv->object, NULL);
	}

	gtk_widget_unref (file->priv->widget);

	gp_camera_unref (file->priv->camera);
	if (file->priv->configuration) gp_widget_unref (file->priv->configuration);

	gtk_object_unref (GTK_OBJECT (file->priv->client));
	gconf_client_notify_remove (file->priv->client, file->priv->cnxn);
	
	g_free (file->priv->dirname);
	g_free (file->priv->filename);
	g_free (file->priv->path);
	
	g_free (file->priv);
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

	gnocam_file_parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_file_init (GnoCamFile* file)
{
	file->priv = g_new0 (GnoCamFilePrivate, 1);
	file->priv->widget = NULL;
}

GnoCamFile*
gnocam_file_new (Camera* camera, Bonobo_Storage storage, const gchar* path, BonoboUIContainer* container, GConfClient* client)
{
	GnoCamFile*		new;

	g_return_val_if_fail (camera, NULL);

	new = gtk_type_new (GNOCAM_TYPE_FILE);
	new->priv->filename = g_strdup (g_basename (path));
	new->priv->path = g_strdup (path);
	new->priv->container = container;
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
									   

