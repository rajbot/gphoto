/* bonobo-storage-camera.c
 *
 * Copyright © 2002 Lutz Müller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "bonobo-storage-camera.h"

#include <string.h>

#include <bonobo/bonobo-exception.h>

#include "gnocam-util.h"
#include "bonobo-stream-camera.h"

#define PARENT_TYPE BONOBO_TYPE_OBJECT
static BonoboObjectClass *parent_class = NULL;

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
create_info_for_file (BonoboStorageCamera *storage, const CORBA_char *name,
		      const Bonobo_StorageInfoFields mask,
		      CORBA_Environment *ev)
{
	Bonobo_StorageInfo *info;

	info = Bonobo_StorageInfo__alloc ();
	info->name = CORBA_string_dup (name);

	/* Type */
	if (mask & Bonobo_FIELD_TYPE)
		info->type = Bonobo_STORAGE_TYPE_REGULAR;

	if (mask & (Bonobo_FIELD_SIZE | Bonobo_FIELD_CONTENT_TYPE)) {
		CameraFileInfo          fi;

		CR (gp_camera_file_get_info (
					storage->priv->camera,
					storage->priv->path, name,
					&fi, NULL), ev);
		if (BONOBO_EX (ev)) {
			CORBA_free (info);
			return (NULL);
		}

		/* Content type */
		if (mask & Bonobo_FIELD_CONTENT_TYPE) {
			if (storage->priv->mode & Bonobo_Storage_COMPRESSED) {
				if (fi.preview.fields & GP_FILE_INFO_TYPE)
					info->content_type = CORBA_string_dup (
							fi.preview.type);
				else
					info->content_type = CORBA_string_dup (
						"application/octet-stream");
			} else {
				if (fi.file.fields & GP_FILE_INFO_TYPE)
					info->content_type = CORBA_string_dup (
							fi.file.type);
				else
					info->content_type = CORBA_string_dup (
						"application/octet-stream");
			}
		}

		/* Size */
		if (mask & Bonobo_FIELD_SIZE) {
			if (storage->priv->mode & Bonobo_Storage_COMPRESSED) {
				if (fi.preview.fields & GP_FILE_INFO_SIZE)
					info->size = fi.preview.size;
				else
					info->size = 0;
			} else {
				if (fi.file.fields & GP_FILE_INFO_SIZE)
					info->size = fi.file.size;
				else
					info->size = 0;
			}
		}
	}

	return (info);
}

static Bonobo_StorageInfo*
camera_get_info_impl (PortableServer_Servant servant,
		      const CORBA_char *path,
		      const Bonobo_StorageInfoFields mask,
		      CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	CameraList *list = NULL;
	gint i;
	int count = 0;
	const char *lname;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	/* We only support CONTENT_TYPE, SIZE, and TYPE. Reject all others. */
        if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | 
		     Bonobo_FIELD_SIZE | 
		     Bonobo_FIELD_TYPE)) {
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
                return (NULL);
	}

	/* Check if this is ourselves (".") */
	if (!strcmp (path, "."))
		return (create_info_for_folder (path, mask));

	/* Get the list of files in order to check if this is a file */
	CR (gp_list_new (&list), ev);
	CR (gp_camera_folder_list_files (storage->priv->camera, 
					storage->priv->path, list, NULL), ev);
	CR (count = gp_list_count (list), ev);

	/* Search our file */
	for (i = 0; i < count; i++) {
		CR (gp_list_get_name (list, i, &lname), ev);
		if (!BONOBO_EX (ev) && !strcmp (lname, path)) {

			/* We found the file */
			gp_list_free (list);
			return (create_info_for_file (storage, path, mask, ev));
		}
	}

        /* Check if this is a directory */
	count = 0;
        CR (gp_camera_folder_list_folders (storage->priv->camera, 
					storage->priv->path, list, NULL), ev);
	CR (count = gp_list_count (list), ev);
        for (i = 0; i < count; i++) {
		CR (gp_list_get_name (list, i, &lname), ev);
		if (!BONOBO_EX (ev) && !strcmp (lname, path)) {

			/* We found the directory */
			gp_list_free (list);
			return (create_info_for_folder (path, mask));
		}
	}
	gp_list_free (list);

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					ex_Bonobo_Storage_NotFound, NULL);
	return (NULL);
}

static void
camera_set_info_impl (PortableServer_Servant servant,
		      const CORBA_char *path, const Bonobo_StorageInfo *info,
		      Bonobo_StorageInfoFields mask,
		      CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	CameraFileInfo fi;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	memset (&fi, 0, sizeof (CameraFileInfo));

	CR (gp_camera_file_set_info (storage->priv->camera,
			storage->priv->path, path, fi, NULL), ev);
}

static void
camera_rename_impl (PortableServer_Servant servant,
		    const CORBA_char *path, const CORBA_char *new_path,
		    CORBA_Environment *ev)
{
	BonoboStorageCamera*    storage;
	CameraFileInfo		info;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	/* Is the renaming really necessary? */
	if (!strcmp (path, new_path))
		return;

	memset (&info, 0, sizeof (CameraFileInfo));
	info.file.fields = GP_FILE_INFO_NAME;
	strcpy (info.file.name, new_path);

	CR (gp_camera_file_set_info (storage->priv->camera,
		storage->priv->path, path, info, NULL), ev);
}

static Bonobo_Stream
camera_open_stream_impl (PortableServer_Servant servant,
			 const CORBA_char *path, Bonobo_Storage_OpenMode mode,
			 CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	BonoboStreamCamera *stream;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	/* Absolute path? */
	if (g_path_is_absolute (path)) {
		gchar *dirname = g_dirname (path);

		stream = bonobo_stream_camera_new (storage->priv->camera,
				dirname, g_basename (path), mode, ev);
		g_free (dirname);
	} else {
		stream = bonobo_stream_camera_new (storage->priv->camera,
					storage->priv->path, path, mode, ev);
	}
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	return (bonobo_object_dup_ref (BONOBO_OBJREF (stream), ev));
}

static Bonobo_Storage_DirectoryList *
camera_list_contents_impl (PortableServer_Servant servant,
			   const CORBA_char *path,
			   const Bonobo_StorageInfoFields mask,
			   CORBA_Environment *ev)
{
	BonoboStorageCamera *storage;
	Bonobo_Storage_DirectoryList *list = NULL;
	CameraList *folder_list = NULL, *file_list = NULL;
	gint i;
	gchar *full_path;
	const char *lname;
	int file_count = 0, folder_count = 0;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	full_path = gnocam_build_path (storage->priv->path, path);

	g_message ("Trying to assemble info for '%s'...", full_path);

	/* Get folder list. */
	CR (gp_list_new (&folder_list), ev);
	CR (gp_camera_folder_list_folders (storage->priv->camera, 
					full_path, folder_list, NULL), ev);
	CR (folder_count = gp_list_count (folder_list), ev);

	/* Get file list. */
	CR (gp_list_new (&file_list), ev);
	CR (gp_camera_folder_list_files (storage->priv->camera, 
					full_path, file_list, NULL), ev);
	g_free (full_path);
	CR (file_count = gp_list_count (file_list), ev);
	if (BONOBO_EX (ev)) {
		gp_list_free (file_list);
		gp_list_free (folder_list);
		return (NULL);
	}

	/* Set up the list. */
	list = Bonobo_Storage_DirectoryList__alloc ();
	list->_length = folder_count + file_count;
	list->_buffer = CORBA_sequence_Bonobo_StorageInfo_allocbuf (
								list->_length);
	CORBA_sequence_set_release (list, TRUE);

	/* Directories. */
	for (i = 0; i < folder_count; i++) {
		CR (gp_list_get_name (folder_list, i, &lname), ev);
		if (BONOBO_EX (ev)) {
			gp_list_free (file_list);
			gp_list_free (folder_list);
			CORBA_free (list);
			return (NULL);
		}
		list->_buffer [i].name = CORBA_string_dup (lname);
		if (mask & Bonobo_FIELD_TYPE)	
			list->_buffer [i].type = Bonobo_STORAGE_TYPE_DIRECTORY;
		if (mask & Bonobo_FIELD_SIZE)	
			list->_buffer [i].size = 0;
		if (mask & Bonobo_FIELD_CONTENT_TYPE) 
			list->_buffer [i].content_type = 
					CORBA_string_dup ("x-directory/normal");
	}
	gp_list_free (folder_list);

	/* Files. */
	for (i = 0; i < file_count; i++) {
		Bonobo_StorageInfo*	info;

		CR (gp_list_get_name (file_list, i, &lname), ev);
		if (BONOBO_EX (ev)) {
			gp_list_free (file_list);
			CORBA_free (list);
			return (NULL);
		}

		list->_buffer [folder_count + i].name = CORBA_string_dup(lname);
		list->_buffer [folder_count + i].type = 
						Bonobo_STORAGE_TYPE_REGULAR;
		info = create_info_for_file (storage, lname, mask, ev);
		if (BONOBO_EX (ev)) {
			gp_list_free (file_list);
			CORBA_free (list);
			return (NULL);
		}

		if (mask & Bonobo_FIELD_TYPE)
			list->_buffer [folder_count + i].type = info->type;
		if (mask & Bonobo_FIELD_SIZE)
			list->_buffer [folder_count + i].size = info->size;
		if (mask & Bonobo_FIELD_CONTENT_TYPE)
			list->_buffer [folder_count + i].content_type =
					CORBA_string_dup (info->content_type);

		CORBA_free (info);
	}
	gp_list_free (file_list);

	return (list);
}

static Bonobo_Storage
camera_open_storage_impl (PortableServer_Servant servant, 
			  const CORBA_char *path, 
			  Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageCamera *storage, *new;
	gchar *path_new;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	/* Create the storage. */
	path_new = gnocam_build_path (storage->priv->path, path);
	new = bonobo_storage_camera_new (storage->priv->camera, path_new,
					 mode, ev);
	g_free (path_new);
	if (BONOBO_EX (ev))
		return (NULL);

	return (bonobo_object_dup_ref (BONOBO_OBJREF (new), ev));
}

BonoboStorageCamera *
bonobo_storage_camera_new (Camera *camera, const gchar *path, 
		           Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageCamera *new;
	gchar *dirname;
	gint i;
	CameraList *list = NULL;
	const char *name;
	int result, count = 0;
	CameraAbilities a;

	g_return_val_if_fail (camera, NULL);
	g_return_val_if_fail (path, NULL);
	g_return_val_if_fail (*path, NULL);

	/* Reject unsupported open modes. */
	if (mode & Bonobo_Storage_TRANSACTED) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
	}
	gp_camera_get_abilities (camera, &a);
	if ((mode & Bonobo_Storage_COMPRESSED) && 
	    !(a.file_operations & GP_FILE_OPERATION_PREVIEW)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
			             ex_Bonobo_Storage_NotSupported, NULL);
		return (NULL);
	}

	/* Make sure the requested storage isn't a file */
	dirname = g_dirname (path);
	CR (gp_list_new (&list), ev);
	CR (gp_camera_folder_list_files (camera, dirname, 
						list, NULL), ev);
	g_free (dirname);
	CR (count = gp_list_count (list), ev);
	for (i = 0; i < count; i++) {
		CR (gp_list_get_name (list, i, &name), ev);
		if (!BONOBO_EX (ev) && !strcmp (name, g_basename (path))) {

			/* We found a file! */
			gp_list_free (list);
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Storage_NotStorage, 
					     NULL); 
			return (NULL);
		}
	}

	/* Make sure the requested folder exists */
	CR (gp_camera_folder_list_files (camera, path, list, NULL), ev);
	if (BONOBO_EX (ev)) {
		gp_list_free (list);
		return (NULL);
	}

	/* Does a folder with this name already exist? */
	if (mode & Bonobo_Storage_FAILIFEXIST) {
		result = gp_camera_folder_list_files (camera, path, list, NULL);
		if (result == GP_OK)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					ex_Bonobo_Storage_NameExists, NULL);
		else if (result != GP_ERROR_DIRECTORY_NOT_FOUND)
			CR (result, ev);
	}
	gp_list_free (list);
	if (BONOBO_EX (ev))
		return (NULL);

        /* Create the storage. */
	new = g_object_new (BONOBO_TYPE_STORAGE_CAMERA, NULL);
	new->priv->camera = camera;
	gp_camera_ref (camera);
	new->priv->path = g_strdup (path);
	new->priv->mode = mode;

	return (new);
}

static void
camera_erase_impl (PortableServer_Servant servant, const CORBA_char *path,
		   CORBA_Environment *ev)
{
	BonoboStorageCamera* storage;

	storage = BONOBO_STORAGE_CAMERA (bonobo_object (servant));

	CR (gp_camera_file_delete (storage->priv->camera,
				storage->priv->path, path, NULL), ev);
}

static void
bonobo_storage_camera_finalize (GObject *object)
{
	BonoboStorageCamera *storage;

	storage = BONOBO_STORAGE_CAMERA (object);

	if (storage->priv) {
		if (storage->priv->path) {
			g_free (storage->priv->path);
			storage->priv->path = NULL;
		}

		if (storage->priv->camera) {
			gp_camera_unref (storage->priv->camera);
			storage->priv->camera = NULL;
		}

		g_free (storage->priv);
		storage->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bonobo_storage_camera_class_init (BonoboStorageCameraClass *klass)
{
        GObjectClass *object_class;
	POA_Bonobo_Storage__epv *epv = &klass->epv;
       
        parent_class = g_type_class_peek_parent (klass);

	object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = bonobo_storage_camera_finalize;

	epv->getInfo      = camera_get_info_impl;
        epv->setInfo      = camera_set_info_impl;
        epv->listContents = camera_list_contents_impl;
        epv->openStream   = camera_open_stream_impl;
        epv->openStorage  = camera_open_storage_impl;
        epv->erase        = camera_erase_impl;
        epv->rename       = camera_rename_impl;
}

static void 
bonobo_storage_camera_init (BonoboStorageCamera* storage)
{
	storage->priv = g_new (BonoboStorageCameraPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (BonoboStorageCamera,Bonobo_Storage,PARENT_TYPE,bonobo_storage_camera)
