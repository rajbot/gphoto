/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-storage-camera.c: Camera storage implementation
 *
 * Authors:
 *   Michael Meeks <michael@helixcode.com>
 *   Lutz Müller <urc8@rz.uni-karlsruhe.de>
 */
#include <config.h>

#include "bonobo-storage-camera.h"

#include <sys/stat.h>
#include <unistd.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-storage-plugin.h>
#include <bonobo/bonobo-exception.h>
#include <gconf/gconf-client.h>

#include <gphoto-extensions.h>

#include "bonobo-stream-camera.h"

static BonoboStorageClass *bonobo_storage_camera_parent_class;

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
camera_open_stream (BonoboStorage *storage, const CORBA_char *filename, Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageCamera *s = BONOBO_STORAGE_CAMERA (storage);
	BonoboStreamCamera *stream;
	CameraFile* file = NULL;
	CameraList list;
	gint i;

	/* Reject some unsupported open modes. */
	if (mode & Bonobo_Storage_TRANSACTED) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return NULL;
	}

	/* Does the camera support upload? */
	if ((mode & (Bonobo_Storage_WRITE | Bonobo_Storage_CREATE)) && (!s->camera->abilities->file_put)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return NULL;
	}

	/* Does the requested file exist? */
	if (mode & Bonobo_Storage_FAILIFEXIST) {
		CHECK_RESULT (gp_camera_file_list (s->camera, &list, s->path), ev);
		if (BONOBO_EX (ev)) return NULL;
		for (i = 0; i < gp_list_count (&list); i++) {
			if (strcmp ((gp_list_entry (&list, i))->name, filename) == 0) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);
				return NULL;
			}
		}
	}

	/* Get the file. */
	if (mode & Bonobo_Storage_READ) {
		file = gp_file_new ();
		if (mode & Bonobo_Storage_COMPRESSED) 
			CHECK_RESULT (gp_camera_file_get_preview (s->camera, file, s->path, (gchar*) filename), ev);
		else 
			CHECK_RESULT (gp_camera_file_get (s->camera, file, s->path, (gchar*) filename), ev);
		if (BONOBO_EX (ev)) {
			gp_file_unref (file);
			return NULL;
		}
	}

	/* Create an empty file. */
	if ((mode & Bonobo_Storage_WRITE) && !file) file = gp_file_new ();

	/* Create a new stream. */
	stream = gtk_type_new (bonobo_stream_camera_get_type ());

	/* Store some data in this stream. */
	stream->mode = mode;
	stream->dirname = g_strdup (s->path);
	stream->filename = g_strdup (filename);
	stream->camera = s->camera;
	gp_camera_ref (s->camera);
	stream->position = 0;
	stream->file = file;

	return BONOBO_STREAM (bonobo_object_construct (BONOBO_OBJECT (stream), bonobo_stream_corba_object_create (BONOBO_OBJECT (stream))));
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
	BonoboStorageCamera *s = BONOBO_STORAGE_CAMERA (storage);
	Bonobo_Storage_DirectoryList *list = NULL;
	CameraList folder_list, file_list;
	gint i;

	/* Reject unsupported masks. */
	if (mask & ( Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return NULL;
	}

	/* Get folder list. */
	CHECK_RESULT (gp_camera_folder_list (s->camera, &folder_list, "/"), ev);
	if (BONOBO_EX (ev)) return NULL;

	/* Get file list. */
	CHECK_RESULT (gp_camera_file_list (s->camera, &file_list, "/"), ev);
	if (BONOBO_EX (ev)) return NULL;

	/* Set up the list. */
	list = Bonobo_Storage_DirectoryList__alloc ();
	list->_length = gp_list_count (&folder_list) + gp_list_count (&file_list);
	list->_buffer = CORBA_sequence_Bonobo_StorageInfo_allocbuf (list->_length);
	CORBA_sequence_set_release (list, TRUE);

	/* Directories. */
	for (i = 0; i < gp_list_count (&folder_list); i++) {
		list->_buffer [i].name = CORBA_string_dup (gp_list_entry (&folder_list, i)->name);
		list->_buffer [i].type = Bonobo_STORAGE_TYPE_DIRECTORY;
		list->_buffer [i].size = 0;
		list->_buffer [i].content_type = CORBA_string_dup ("x-directory/normal");
	}

	/* Files. */
	for (i = 0; i < gp_list_count (&file_list); i++) {
		list->_buffer [i + gp_list_count (&folder_list)].name = CORBA_string_dup (gp_list_entry (&file_list, i)->name);
		list->_buffer [i + gp_list_count (&folder_list)].type = Bonobo_STORAGE_TYPE_REGULAR;
		list->_buffer [i + gp_list_count (&folder_list)].size = 0;
		list->_buffer [i + gp_list_count (&folder_list)].content_type = CORBA_string_dup ("image/jpeg");
	}

	return list;
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
        BonoboStorageCamera*	storage;
	CameraList 		list;
	Camera*			camera = NULL;

        /* Reject some unsupported open modes. */
	if (flags & (Bonobo_Storage_COMPRESSED | Bonobo_Storage_TRANSACTED)) {
        	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
        	return NULL;
        }

	/* Create the camera. */
	CHECK_RESULT (gp_camera_new_from_gconf (&camera, path), ev);
	if (BONOBO_EX (ev)) {
		return (NULL);
	}
							
	/* Create the storage. */
        storage = gtk_type_new (bonobo_storage_camera_get_type ());

	for (path += 2; *path != 0; path++) if (*path == '/') break;
	storage->path = g_strdup (path);
	storage->camera = camera;

	/* Does the folder exist? */
	CHECK_RESULT (gp_camera_folder_list (storage->camera, &list, (gchar*) path), ev);
	if (BONOBO_EX (ev)) {
		bonobo_object_unref (BONOBO_OBJECT (storage));
		return NULL;
	}

        return (BONOBO_STORAGE (storage));
}

static BonoboStorage *
camera_open_storage (BonoboStorage *storage, const CORBA_char *path, Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	CameraList list;
	BonoboStorageCamera *s = BONOBO_STORAGE_CAMERA (storage);
	BonoboStorageCamera *storage_new;
	gint i;

	/* Reject some unsupported open modes. */
	if (mode & (Bonobo_Storage_COMPRESSED | Bonobo_Storage_TRANSACTED)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return NULL;
	}
							
	/* Does the folder exist? */
	if (mode & Bonobo_Storage_FAILIFEXIST) {
		CHECK_RESULT (gp_camera_file_list (s->camera, &list, s->path), ev);
		if (BONOBO_EX (ev)) return NULL;
		for (i = 0; i < gp_list_count (&list); i++) {
			if (strcmp ((gp_list_entry (&list, i))->name, path) == 0) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);
				return NULL;
			}
		}
	}

	/* Create the storage. */
	storage_new = gtk_type_new (bonobo_storage_camera_get_type ());
	storage_new->camera = s->camera;
	gp_camera_ref (s->camera);
	if (!strcmp (s->path, "/")) storage_new->path = g_strconcat (s->path, path, NULL);
	else storage_new->path = g_strconcat (s->path, G_DIR_SEPARATOR, path, NULL);

	return (BONOBO_STORAGE (storage_new));
}

static void
camera_erase (BonoboStorage *storage, const CORBA_char *path, CORBA_Environment *ev)
{
	BonoboStorageCamera *s = BONOBO_STORAGE_CAMERA (storage);

	/* Delete the file. */
	CHECK_RESULT (gp_camera_file_delete (s->camera, s->path, (gchar*) path), ev);
}

static void
bonobo_storage_camera_destroy (GtkObject *object)
{
	BonoboStorageCamera *storage = BONOBO_STORAGE_CAMERA (object);

	g_free (storage->path);
	gp_camera_unref (storage->camera);
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

static void 
bonobo_storage_camera_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC (BonoboStorageCamera, bonobo_storage_get_type (), bonobo_storage_camera); 

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

	/* Init GPhoto */
	gp_init (GP_DEBUG_NONE);
	gp_frontend_register (NULL, NULL, NULL, NULL, NULL);

	return 0;
}
