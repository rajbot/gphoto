/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-storage-camera.c: Camera storage implementation
 *
 * Author:
 *   Michael Meeks <michael@helixcode.com>
 */
#include <config.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-storage-plugin.h>
#include <gconf/gconf-client.h>
#include "bonobo-storage-camera.h"
#include "bonobo-stream-camera.h"

static BonoboStorageClass *bonobo_storage_camera_parent_class;
static GConfClient *client = NULL;
static GMutex *client_mutex = NULL;

static Bonobo_StorageInfo*
camera_get_info (BonoboStorage *storage, const CORBA_char *path, const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
	return CORBA_OBJECT_NIL;
}

static void
camera_set_info (BonoboStorage *storage, const CORBA_char *path, const Bonobo_StorageInfo *info, const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
}

static BonoboStream *
camera_open_stream (BonoboStorage *storage, const CORBA_char *path, Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	return bonobo_stream_camera_open (path, 0664, mode, ev);
}

/*
 * Creates the Gtk object and the corba server bound to it
 */
static BonoboStorage *
do_bonobo_storage_camera_create (const char *path)
{
	BonoboStorageCamera *storage_camera;
	Bonobo_Storage corba_storage;

	storage_camera = gtk_type_new (bonobo_storage_camera_get_type ());
	
	corba_storage = bonobo_storage_corba_object_create (BONOBO_OBJECT (storage_camera));
	if (corba_storage == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (storage_camera));
		return NULL;
	}

	return bonobo_storage_construct (BONOBO_STORAGE (storage_camera), corba_storage);
}

static void
camera_rename (BonoboStorage *storage, const CORBA_char *path, const CORBA_char *new_path, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
}

static void
camera_commit (BonoboStorage *storage, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static void
camera_revert (BonoboStorage *storage, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static Bonobo_Storage_DirectoryList *
camera_list_contents (BonoboStorage *storage, const CORBA_char *path, Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
	return CORBA_OBJECT_NIL;
}

/** 
 * bonobo_storage_camera_open:
 * @path: path to existing directory that represents the storage
 * @flags: open flags.
 * @mode: mode used if @flags containst BONOBO_SS_CREATE for the storage.
 *
 * Returns a BonoboStorage object that represents the storage at @path
 */
static BonoboStorage *
bonobo_storage_camera_open (const char *path, gint flags, gint mode, CORBA_Environment *ev)
{
	return do_bonobo_storage_camera_create (path);
}

static BonoboStorage *
camera_open_storage (BonoboStorage *storage, const CORBA_char *path, Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
	return NULL;
}

static void
camera_erase (BonoboStorage *storage, const CORBA_char *path, CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
}

static void
bonobo_storage_camera_destroy (GtkObject *object)
{
}

static void
bonobo_storage_camera_class_init (BonoboStorageCameraClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	BonoboStorageClass *sclass = BONOBO_STORAGE_CLASS (class);
	
	bonobo_storage_camera_parent_class = gtk_type_class (bonobo_storage_get_type ());

	sclass->get_info       = camera_get_info;
	sclass->set_info       = camera_set_info;
	sclass->open_stream    = camera_open_stream;
	sclass->open_storage   = camera_open_storage;
	sclass->copy_to        = NULL; /* use the generic method */
	sclass->rename         = camera_rename;
	sclass->commit         = camera_commit;
	sclass->revert         = camera_revert;
	sclass->list_contents  = camera_list_contents;
	sclass->erase          = camera_erase;

	object_class->destroy = bonobo_storage_camera_destroy;
}

GtkType
bonobo_storage_camera_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/StorageCamera:1.0",
			sizeof (BonoboStorageCamera),
			sizeof (BonoboStorageCameraClass),
			(GtkClassInitFunc) bonobo_storage_camera_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_storage_get_type (), &info);
	}

	return type;
}

gint 
init_storage_plugin (StoragePlugin *plugin)
{
	gchar *argv[] = {"dummy"};
	
	g_return_val_if_fail (plugin != NULL, -1);

	plugin->name = "camera";
	plugin->description = "Gnome Digital Camera Driver";
	plugin->version = BONOBO_STORAGE_VERSION;
	
	plugin->storage_open = bonobo_storage_camera_open; 
	plugin->stream_open  = bonobo_stream_camera_open; 

	/* Init GConf. */
	if (!gconf_is_initialized ()) gconf_init (1, argv, NULL);
	client = gconf_client_get_default ();
	gtk_object_ref (GTK_OBJECT (client));
	gtk_object_sink (GTK_OBJECT (client));
#ifdef G_THREADS_ENABLED
	if (g_thread_supported ()) client_mutex = g_mutex_new ();
	else client_mutex = NULL;
#endif

	/* Init GPhoto */
	gp_init (GP_DEBUG_HIGH);
	gp_frontend_register (NULL, NULL, NULL, NULL, NULL);

	return 0;
}
