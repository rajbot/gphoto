/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-storage-camera.c: Camera storage implementation
 *
 * Authors:
 *   Michael Meeks <michael@helixcode.com>
 *   Lutz M�ller <urc8@rz.uni-karlsruhe.de>
 */

#include <config.h>

#include "bonobo-storage-camera.h"

#include <bonobo/bonobo-exception.h>
#include <libgnome/gnome-util.h>

#include "bonobo-stream-camera.h"
#include "gnocam-util.h"

#define PARENT_TYPE BONOBO_STORAGE_TYPE
static BonoboStorageClass* parent_class = NULL;

struct _BonoboStorageCameraPrivate {
	Bonobo_Storage_OpenMode		mode;
	Camera*				camera;
	gchar*				path;
};

static Bonobo_StorageInfo *
create_info_for_folder (const CORBA_char *name, 
			const Bonobo_StorageInfoFields mask)
{
	Bonobo_StorageInfo *info;

	info = Bonobo_StorageInfo__alloc (); 
	info->name = CORBA_string_dup (name); 
	if (mask & Bonobo_FIELD_SIZE) 
		info->size = 0; 
	if (mask & Bonobo_FIELD_TYPE) 
		info->type = Bonobo_STORAGE_TYPE_DIRECTORY; 
	if (mask & Bonobo_FIELD_CONTENT_TYPE) 
		info->content_type = CORBA_string_dup ("x-directory/normal");

	return (info);
}

static Bonobo_StorageInfo *
camera_get_info (BonoboStorage *s, const CORBA_char *name, 
		 const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	Bonobo_StorageInfo *info;
	CameraList list;
	gint i;

	storage = BONOBO_STORAGE_CAMERA (s);

	g_message ("Getting info for '%s' (we are in %s)...", name, storage->priv->path);

	/* We only support CONTENT_TYPE, SIZE, and TYPE. Reject all others. */
        if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | 
		     Bonobo_FIELD_SIZE | 
		     Bonobo_FIELD_TYPE)) {
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
                return (NULL);
        }

	/* Check if this is ourselves (".") */
	if (!strcmp (name, "."))
		return (create_info_for_folder (name, mask));

	/* Check if this is a file */
	CHECK_RESULT (gp_camera_folder_list_files (storage->priv->camera, 
					storage->priv->path, &list), ev);
	if (BONOBO_EX (ev)) 
		return (NULL);
	for (i = 0; i < gp_list_count (&list); i++) 
		if (!strcmp (gp_list_entry (&list, i)->name, name)) 
			break;
	if (i != gp_list_count (&list)) {
		info = Bonobo_StorageInfo__alloc ();
		info->name = CORBA_string_dup (name);
		if (mask & Bonobo_FIELD_TYPE) 
			info->type = Bonobo_STORAGE_TYPE_REGULAR;

		if (mask & (Bonobo_FIELD_SIZE | Bonobo_FIELD_CONTENT_TYPE)) {
			CameraFileInfoStruct	fileinfostruct;
                        CameraFileInfo  	fileinfo;
	
			CHECK_RESULT (gp_camera_file_get_info (
					storage->priv->camera, 
					storage->priv->path, (gchar*) name, 
					&fileinfo), ev);
			if (BONOBO_EX (ev)) {
				CORBA_free (info);
				return (NULL);
			}

                        if (storage->priv->mode & Bonobo_Storage_COMPRESSED)
				fileinfostruct = fileinfo.preview;
                        else 
				fileinfostruct = fileinfo.file;

			/* Content type */
			if (mask & Bonobo_FIELD_CONTENT_TYPE) {
				if (fileinfostruct.fields & GP_FILE_INFO_TYPE)
					info->content_type = CORBA_string_dup (
							fileinfostruct.type);
				else
					info->content_type = CORBA_string_dup (
						"application/octet-stream");
			}

			/* Size */
			if (mask & Bonobo_FIELD_SIZE) {
				if (fileinfostruct.fields & GP_FILE_INFO_SIZE)
					info->size = fileinfostruct.size;
				else
					info->size = 0;
			}
                }

		return (info);
	}

        /* Check if this is a directory */
        CHECK_RESULT (gp_camera_folder_list_folders (storage->priv->camera, 
					storage->priv->path, &list), ev);
        if (BONOBO_EX (ev)) {
                return (NULL);
        }
        for (i = 0; i < gp_list_count (&list); i++) 
		if (!strcmp (gp_list_entry (&list, i)->name, name)) 
			break;
        if (i != gp_list_count (&list))
		return (create_info_for_folder (name, mask));

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					ex_Bonobo_Storage_NotFound, NULL);
	return (NULL);
}

static void
camera_set_info (BonoboStorage *s, const CORBA_char *name,
		 const Bonobo_StorageInfo *info,
		 const Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	CameraFileInfo fileinfo;

	storage = BONOBO_STORAGE_CAMERA (s);

	fileinfo.preview.fields = GP_FILE_INFO_NONE;
	fileinfo.file.fields = GP_FILE_INFO_NONE;
	
	CHECK_RESULT (gp_camera_file_set_info (storage->priv->camera,
			storage->priv->path, (gchar*) name, &fileinfo), ev);
}

static void
camera_rename (BonoboStorage *s, const CORBA_char *name_old,
	       const CORBA_char *name_new, CORBA_Environment *ev)
{
	BonoboStorageCamera*    storage;
	CameraFileInfo		info;

	storage = BONOBO_STORAGE_CAMERA (s);

	if (!strcmp (name_old, name_new)) return;

	info.preview.fields = GP_FILE_INFO_NONE;
	info.file.fields = GP_FILE_INFO_NAME;
	strcpy (info.file.name, name_new);

	CHECK_RESULT (gp_camera_file_set_info (storage->priv->camera,
			storage->priv->path, (gchar*) name_old, &info), ev);
}

static BonoboStream *
camera_open_stream (BonoboStorage *s, const CORBA_char *filename,
		    Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	BonoboStream        *new;

	storage = BONOBO_STORAGE_CAMERA (s);

	/* Absolute path? */
	if (g_path_is_absolute (filename)) {
		gchar *dirname = g_dirname (filename);

		new = bonobo_stream_camera_new (storage->priv->camera,
						dirname, g_basename (filename),
						mode, ev);
		g_free (dirname);
	} else {
		new = bonobo_stream_camera_new (storage->priv->camera,
						storage->priv->path, filename,
						mode, ev);
	}
	if (BONOBO_EX (ev))
		return (NULL);
	
	return (new);
}

static Bonobo_Storage_DirectoryList *
camera_list_contents (BonoboStorage *s, const CORBA_char *name, 
		      Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	Bonobo_Storage_DirectoryList *list = NULL;
	CameraList folder_list, file_list;
	gint i;
	gchar *full_path;

	storage = BONOBO_STORAGE_CAMERA (s);

	if (!name)
		full_path = g_strdup (storage->priv->path);
	else
		full_path = g_concat_dir_and_file (storage->priv->path, name);

	g_message ("Trying to assemble info for '%s'...", full_path);

	/* Get folder list. */
	CHECK_RESULT (gp_camera_folder_list_folders (storage->priv->camera, 
						full_path, &folder_list), ev);
	if (BONOBO_EX (ev)) {
		g_free (full_path);
		return (NULL);
	}

	/* Get file list. */
	CHECK_RESULT (gp_camera_folder_list_files (storage->priv->camera, 
						full_path, &file_list), ev);
	g_free (full_path);
	if (BONOBO_EX (ev)) 
		return (NULL);

	/* Set up the list. */
	list = Bonobo_Storage_DirectoryList__alloc ();
	list->_length = gp_list_count (&folder_list) + 
						gp_list_count (&file_list);
	list->_buffer = CORBA_sequence_Bonobo_StorageInfo_allocbuf (
								list->_length);
	CORBA_sequence_set_release (list, TRUE);

	/* Directories. */
	for (i = 0; i < gp_list_count (&folder_list); i++) {
		list->_buffer [i].name = CORBA_string_dup (
					gp_list_entry (&folder_list, i)->name);
		if (mask & Bonobo_FIELD_TYPE)	
			list->_buffer [i].type = Bonobo_STORAGE_TYPE_DIRECTORY;
		if (mask & Bonobo_FIELD_SIZE)	
			list->_buffer [i].size = 0;
		if (mask & Bonobo_FIELD_CONTENT_TYPE) 
			list->_buffer [i].content_type = 
					CORBA_string_dup ("x-directory/normal");
	}

	/* Files. */
	for (i = 0; i < gp_list_count (&file_list); i++) {
		Bonobo_StorageInfo*	info;

		list->_buffer [i + gp_list_count (&folder_list)].name = 
			CORBA_string_dup (gp_list_entry (&file_list, i)->name);
		list->_buffer [i + gp_list_count (&folder_list)].type = 
						Bonobo_STORAGE_TYPE_REGULAR;
		info = camera_get_info (s, 
				gp_list_entry (&file_list, i)->name, mask, ev);
		if (BONOBO_EX (ev)) 
			return (NULL);

		if (mask & Bonobo_FIELD_TYPE)
			list->_buffer [i + gp_list_count (&folder_list)].type = 
								info->type;
		if (mask & Bonobo_FIELD_SIZE)
			list->_buffer [i + gp_list_count (&folder_list)].size =
								info->size;
		if (mask & Bonobo_FIELD_CONTENT_TYPE)
			list->_buffer [i + gp_list_count (&folder_list)].content_type = CORBA_string_dup (info->content_type);

		CORBA_free (info);
	}

	return (list);
}

static BonoboStorage *
camera_open_storage (BonoboStorage *s, const CORBA_char *name, 
		     Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	BonoboStorage *new;
	gchar *path_new;

	storage = BONOBO_STORAGE_CAMERA (s);

	if (g_path_is_absolute (name))
		path_new = g_strdup (name);
	else 
		path_new = g_concat_dir_and_file (storage->priv->path, name);

	/* Create the storage. */
	new = bonobo_storage_camera_new (storage->priv->camera, path_new,
					 mode, ev);
	g_free (path_new);
	if (BONOBO_EX (ev))
		return (NULL);
	
	return (new);
}

BonoboStorage *
bonobo_storage_camera_new (Camera *camera, const gchar *path, 
		           Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageCamera *new;
	gchar *dirname;
	gint i;
	CameraList list;
	
	g_return_val_if_fail (camera, NULL);
	g_return_val_if_fail (path, NULL);
	g_return_val_if_fail (*path, NULL);

	/* Reject unsupported open modes. */
	if (mode & Bonobo_Storage_TRANSACTED) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
	}
	if ((mode & Bonobo_Storage_COMPRESSED) && 
	    !(camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
			             ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
	}

	/* Make sure the requested storage isn't a file */
	dirname = g_dirname (path);
	CHECK_RESULT (gp_camera_folder_list_files (camera, dirname, &list), ev);
	g_free (dirname);
	if (BONOBO_EX (ev))
		return (NULL);
	for (i = 0; i < gp_list_count (&list); i++)
		if (strcmp ((gp_list_entry (&list, i))->name, 
			    g_basename (path)) == 0) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Storage_NotStorage, 
					     NULL); 
			return (NULL);
		}

	/* Make sure the requested folder exists */
	CHECK_RESULT (gp_camera_folder_list_files (camera, path, &list), ev);
	if (BONOBO_EX (ev))
		return (NULL);

	/* Does a folder with this name already exist? */
	if (mode & Bonobo_Storage_FAILIFEXIST) {
		dirname = g_dirname (path);
		CHECK_RESULT (gp_camera_folder_list_folders (camera, 
					            dirname, &list), ev);
		g_free (dirname);
		if (BONOBO_EX (ev)) {
			return (NULL);
		}
		for (i = 0; i < gp_list_count (&list); i++)
			if (strcmp ((gp_list_entry (&list, i))->name, 
						    g_basename (path)) == 0) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					ex_Bonobo_Storage_NameExists, NULL);
				return (NULL);
			}
	}
	
        /* Create the storage. */
	new = gtk_type_new (BONOBO_STORAGE_CAMERA_TYPE);
	new->priv->camera = camera;
	gp_camera_ref (camera);
	new->priv->path = g_strdup (path);
	new->priv->mode = mode;

	return (BONOBO_STORAGE (new));
}

static void
camera_erase (BonoboStorage* s, const CORBA_char* name, CORBA_Environment* ev)
{
	BonoboStorageCamera* storage;
	
	storage = BONOBO_STORAGE_CAMERA (s);

	CHECK_RESULT (gp_camera_file_delete (storage->priv->camera,
				storage->priv->path, (gchar*) name), ev);
}

static void
bonobo_storage_camera_destroy (GtkObject *object)
{
	BonoboStorageCamera *storage = BONOBO_STORAGE_CAMERA (object);

	g_message ("Destroying BonoboStorageCamera...");

	if (storage->priv->path) {
		g_free (storage->priv->path);
		storage->priv->path = NULL;
	}

	if (storage->priv->camera) {
		gp_camera_unref (storage->priv->camera);
		storage->priv->camera = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
bonobo_storage_camera_finalize (GtkObject *object)
{
	BonoboStorageCamera *storage;

	storage = BONOBO_STORAGE_CAMERA (object);

	g_free (storage->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bonobo_storage_camera_class_init (BonoboStorageCameraClass *klass)
{
	GtkObjectClass* 	object_class;
	BonoboStorageClass*	sclass;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = bonobo_storage_camera_destroy;
	object_class->finalize = bonobo_storage_camera_finalize;
	
	sclass = BONOBO_STORAGE_CLASS (klass);
	sclass->get_info       = camera_get_info;
	sclass->set_info       = camera_set_info;
	sclass->open_stream    = camera_open_stream;
	sclass->open_storage   = camera_open_storage;
	sclass->copy_to        = NULL; /* use the generic method */
	sclass->rename         = camera_rename;
	sclass->commit         = NULL;
	sclass->revert         = NULL;
	sclass->list_contents  = camera_list_contents;
	sclass->erase          = camera_erase;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void 
bonobo_storage_camera_init (BonoboStorageCamera* storage)
{
	storage->priv = g_new (BonoboStorageCameraPrivate, 1);
}

BONOBO_X_TYPE_FUNC (BonoboStorageCamera, PARENT_TYPE, bonobo_storage_camera); 

