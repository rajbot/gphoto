/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-storage-camera.c: Camera storage implementation
 *
 * Authors:
 *   Michael Meeks <michael@helixcode.com>
 *   Lutz Müller <urc8@rz.uni-karlsruhe.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gphoto2.h>

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

#define PARENT_TYPE BONOBO_STORAGE_TYPE
static BonoboStorageClass* bonobo_storage_camera_parent_class = NULL;

struct _BonoboStorageCameraPrivate {
	Camera*	camera;
	gchar*	path;
};

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
camera_open_stream (BonoboStorage* s, const CORBA_char* filename, Bonobo_Storage_OpenMode mode, CORBA_Environment* ev)
{
	BonoboStorageCamera* 	storage;
	BonoboStreamCamera*	new;

	storage = BONOBO_STORAGE_CAMERA (s);

	/* Create a new stream. */
	new = bonobo_stream_camera_new (storage->priv->camera, storage->priv->path, filename, mode, ev);
	if (BONOBO_EX (ev)) return (NULL);
	
	return (BONOBO_STREAM (new));
}

static Bonobo_Storage_DirectoryList*
camera_list_contents (BonoboStorage* s, const CORBA_char* path, Bonobo_StorageInfoFields mask, CORBA_Environment* ev)
{
	BonoboStorageCamera*		storage;
	Bonobo_Storage_DirectoryList*	list = NULL;
	CameraList 			folder_list, file_list;
	gint 				i;
	gchar*				combined_path;

	storage = BONOBO_STORAGE_CAMERA (s);

	/* Reject unsupported masks. */
	if (mask & ( Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
	}

	if (!strcmp ("/", storage->priv->path)) combined_path = g_strconcat ("/", path, NULL);
	else combined_path = g_strconcat (storage->priv->path, G_DIR_SEPARATOR, path, NULL);

	/* Get folder list. */
	CHECK_RESULT (gp_camera_folder_list (storage->priv->camera, &folder_list, combined_path), ev);
	if (BONOBO_EX (ev)) {
		g_free (combined_path);
		return (NULL);
	}

	/* Get file list. */
	CHECK_RESULT (gp_camera_file_list (storage->priv->camera, &file_list, combined_path), ev);
	g_free (combined_path);
	if (BONOBO_EX (ev)) return (NULL);

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

	return (list);
}

static BonoboStorage*
bonobo_storage_camera_open (const gchar* path, gint flags, gint mode, CORBA_Environment* ev)
{
        BonoboStorageCamera*	new;
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

	/* Does the folder exist? */
	if (!strncmp (path, "camera:", 7)) path += 7;
	for (path += 2; *path != 0; path++) if (*path == '/') break;
	CHECK_RESULT (gp_camera_folder_list (camera, &list, (gchar*) path), ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (NULL);
	}

	/* Create the storage. */
        new = gtk_type_new (bonobo_storage_camera_get_type ());
	new->priv->path = g_strdup (path);
	new->priv->camera = camera;

        return (BONOBO_STORAGE (new));
}

static BonoboStorage *
camera_open_storage (BonoboStorage* s, const CORBA_char* path, Bonobo_Storage_OpenMode mode, CORBA_Environment* ev)
{
	CameraList 		list;
	BonoboStorageCamera*	storage;
	BonoboStorageCamera*	new;
	gint 			i;

	storage = BONOBO_STORAGE_CAMERA (s);

	/* Reject some unsupported open modes. */
	if (mode & (Bonobo_Storage_COMPRESSED | Bonobo_Storage_TRANSACTED)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NotSupported, NULL);
		return NULL;
	}
							
	/* Does the folder exist? */
	if (mode & Bonobo_Storage_FAILIFEXIST) {
		CHECK_RESULT (gp_camera_file_list (storage->priv->camera, &list, storage->priv->path), ev);
		if (BONOBO_EX (ev)) return (NULL);
		for (i = 0; i < gp_list_count (&list); i++) {
			if (strcmp ((gp_list_entry (&list, i))->name, path) == 0) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);
				return (NULL);
			}
		}
		CHECK_RESULT (gp_camera_folder_list (storage->priv->camera, &list, storage->priv->path), ev);
		if (BONOBO_EX (ev)) return (NULL);
		for (i = 0; i < gp_list_count (&list); i++) {
			if (strcmp ((gp_list_entry (&list, i))->name, path) == 0) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Storage_NameExists, NULL);
				return (NULL);
			}
		}
	}

	/* Create the storage. */
	new = gtk_type_new (BONOBO_STORAGE_CAMERA_TYPE);
	new->priv->camera = storage->priv->camera;
	gp_camera_ref (storage->priv->camera);
	
	if (!strcmp (storage->priv->path, "/")) new->priv->path = g_strconcat (storage->priv->path, path, NULL);
	else new->priv->path = g_strconcat (storage->priv->path, G_DIR_SEPARATOR, path, NULL);

	return (BONOBO_STORAGE (new));
}

static void
camera_erase (BonoboStorage* s, const CORBA_char* path, CORBA_Environment* ev)
{
	BonoboStorageCamera* storage;
	
	storage = BONOBO_STORAGE_CAMERA (s);

	CHECK_RESULT (gp_camera_file_delete (storage->priv->camera, storage->priv->path, (gchar*) path), ev);
}

static void
bonobo_storage_camera_destroy (GtkObject *object)
{
	BonoboStorageCamera *storage = BONOBO_STORAGE_CAMERA (object);

	g_free (storage->priv->path);
	gp_camera_unref (storage->priv->camera);
	g_free (storage->priv);
}

static void
bonobo_storage_camera_class_init (BonoboStorageCameraClass *klass)
{
	GtkObjectClass* 	object_class;
	BonoboStorageClass*	sclass;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = bonobo_storage_camera_destroy;
	
	sclass = BONOBO_STORAGE_CLASS (klass);
	sclass->get_info       = camera_get_info;
	sclass->set_info       = camera_set_info;
	sclass->open_stream    = camera_open_stream;
	sclass->open_storage   = camera_open_storage;
	sclass->copy_to        = NULL; /* use the generic method */
	sclass->rename         = NULL;
	sclass->commit         = NULL;
	sclass->revert         = NULL;
	sclass->list_contents  = camera_list_contents;
	sclass->erase          = camera_erase;

	bonobo_storage_camera_parent_class = gtk_type_class (PARENT_TYPE);
}

static void 
bonobo_storage_camera_init (BonoboStorageCamera* storage)
{
	storage->priv = g_new (BonoboStorageCameraPrivate, 1);
}

BONOBO_X_TYPE_FUNC (BonoboStorageCamera, PARENT_TYPE, bonobo_storage_camera); 

gint 
init_storage_plugin (StoragePlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, -1);

	plugin->name = "camera";
	plugin->description = "Gnome Digital Camera Driver";
	plugin->version = BONOBO_STORAGE_VERSION;
	
	plugin->storage_open = bonobo_storage_camera_open; 
	plugin->stream_open  = bonobo_stream_camera_open; 

	/* Init GPhoto */
	gp_init (GP_DEBUG_NONE);
	gp_frontend_register (NULL, NULL, NULL, NULL, NULL);

	return 0;
}
