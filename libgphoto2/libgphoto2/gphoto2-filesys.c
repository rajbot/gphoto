/* gphoto2-filesys.c
 *
 * Copyright (C) 2000 Scott Fritzinger
 *
 * Contributions:
 * 	Lutz M�ller <urc8@rz.uni-karlsruhe.de> (2001)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include <config.h>
#include "gphoto2-filesys.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gphoto2-result.h"
#include "gphoto2-file.h"
#include "gphoto2-port-log.h"

#include <libexif/exif-data.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define GP_MODULE "libgphoto2"

typedef struct {
	char name [128];
	int info_dirty;
	CameraFileInfo info;
	CameraFile *preview;
	CameraFile *normal;
	CameraFile *raw;
	CameraFile *audio;
	CameraFile *exif;
} CameraFilesystemFile;

typedef struct {
	int count;
	char name [128];
	int files_dirty;
	int folders_dirty;
	CameraFilesystemFile *file;
} CameraFilesystemFolder;

static time_t gp_filesystem_get_exif_mtime   (CameraFilesystem *, const char *,
					      const char *);
static int    gp_filesystem_get_exif_preview (CameraFilesystem *, const char *,
					      const char *, GPContext *);


/**
 * CameraFilesystem:
 *
 * The internals of the #CameraFilesystem are only visible to gphoto2. You
 * can only access them using the functions provided by gphoto2.
 **/
struct _CameraFilesystem {
	int count;
	CameraFilesystemFolder *folder;

	CameraFilesystemGetInfoFunc get_info_func;
	CameraFilesystemSetInfoFunc set_info_func;
	void *info_data;

	CameraFilesystemListFunc file_list_func;
	CameraFilesystemListFunc folder_list_func;
	void *list_data;

	CameraFilesystemGetFileFunc get_file_func;
	CameraFilesystemDeleteFileFunc delete_file_func;
	void *file_data;

	CameraFilesystemPutFileFunc put_file_func;
	CameraFilesystemDeleteAllFunc delete_all_func;
	CameraFilesystemDirFunc make_dir_func;
	CameraFilesystemDirFunc remove_dir_func;
	void *folder_data;
};

#define CHECK_NULL(r)        {if (!(r)) return (GP_ERROR_BAD_PARAMETERS);}
#define CR(result)           {int r = (result); if (r < 0) return (r);}
#define CHECK_MEM(m)         {if (!(m)) return (GP_ERROR_NO_MEMORY);}

#define CC(context)							\
{									\
	if (gp_context_cancel (context) == GP_CONTEXT_FEEDBACK_CANCEL)	\
		return GP_ERROR_CANCEL;					\
}

#define CA(f,c)							\
{									\
	if ((f)[0] != '/') {						\
		gp_context_error ((c),					\
			_("The path '%s' is not absolute."), (f));	\
		return (GP_ERROR_PATH_NOT_ABSOLUTE);			\
	}								\
}

static int
delete_all_files (CameraFilesystem *fs, int x)
{
        int y;

        CHECK_NULL (fs);

        if (fs->folder[x].count) {

		/* Get rid of cached files */
                for (y = 0; y < fs->folder[x].count; y++) {
			if (fs->folder[x].file[y].preview) {
	                        gp_file_unref (fs->folder[x].file[y].preview);
				fs->folder[x].file[y].preview = NULL;
			}
			if (fs->folder[x].file[y].normal) {
				gp_file_unref (fs->folder[x].file[y].normal);
				fs->folder[x].file[y].normal = NULL;
			}
			if (fs->folder[x].file[y].raw) {
				gp_file_unref (fs->folder[x].file[y].raw);
				fs->folder[x].file[y].raw = NULL;
			}
			if (fs->folder[x].file[y].audio) {
				gp_file_unref (fs->folder[x].file[y].audio);
				fs->folder[x].file[y].audio = NULL;
			}
			if (fs->folder[x].file[y].exif) {
				gp_file_unref (fs->folder[x].file[y].exif);
				fs->folder[x].file[y].exif = NULL;
			}
		}

                free (fs->folder[x].file);
                fs->folder[x].file = NULL;
                fs->folder[x].count = 0;
        }

        return (GP_OK);
}

static int
delete_folder (CameraFilesystem *fs, int x)
{
        CameraFilesystemFolder *new_fop;

        CHECK_NULL (fs);

        delete_all_files (fs, x);

        /* Move all folders behind one position up */
        if (x < fs->count - 1)
                memmove (&fs->folder[x], &fs->folder[x + 1],
                         sizeof (CameraFilesystemFolder) * (fs->count - x - 1));
        fs->count--;

        /* Get rid of the last one */
        new_fop = realloc (fs->folder,
                           sizeof (CameraFilesystemFolder) * (fs->count));
        if (!fs->count || (fs->count && new_fop))
                fs->folder = new_fop;

        return (GP_OK);
}

static int
delete_all_folders (CameraFilesystem *fs, const char *folder,
		    GPContext *context)
{
        int x;

	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem", "Internally deleting "
		"all folders from '%s'...", folder);

        CHECK_NULL (fs && folder);
	CC (context);
        CA (folder, context);

        for (x = 0; x < fs->count; x++)
                if (!strncmp (fs->folder[x].name, folder, strlen (folder))) {

                        /*
                         * Is this really a subfolder (and not the folder
                         * itself)?
                         */
                        if (strlen (fs->folder[x].name) <= strlen (folder))
                                continue;

                        CR (delete_all_files (fs, x));
                        CR (delete_folder (fs, x));
                        x--;
                }

        return (GP_OK);
}

static int
append_folder (CameraFilesystem *fs, const char *folder, GPContext *context)
{
        CameraFilesystemFolder *new;
        int x;
        char buf[128];

	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem",
		"Internally appending folder %s...", folder);

        CHECK_NULL (fs && folder);
	CC (context);
        CA (folder, context);

        /* Make sure the directory doesn't exist */
	for (x = 0; x < fs->count; x++)
		if (!strncmp (fs->folder[x].name, folder, strlen (folder)) &&
		    (strlen (fs->folder[x].name) == strlen (folder)))
			break;
	if (x < fs->count) {
		gp_context_error (context, _("Could not append folder '%s' "
			"as this folder already exists."), folder);
                return (GP_ERROR_DIRECTORY_EXISTS);
        }

        /* Make sure the parent exist. If not, create it. */
	strncpy (buf, folder, sizeof (buf));
        for (x = strlen (buf) - 1; x >= 0; x--)
                if (buf[x] == '/')
                        break;
        if (x > 0) {
                buf[x] = '\0';
		for (x = 0; x < fs->count; x++)
			if (!strncmp (fs->folder[x].name, buf, strlen (buf)))
				break;
		if (x == fs->count)
			CR (append_folder (fs, buf, context))
        }

        /* Allocate the folder pointer and the actual folder */
        if (fs->count)
                CHECK_MEM (new = realloc (fs->folder,
                        sizeof (CameraFilesystemFolder) * (fs->count + 1)))
        else
                CHECK_MEM (new = malloc (sizeof (CameraFilesystemFolder)));
        fs->folder = new;
        fs->count++;

	/* Initialize the folder (and remove trailing slashes if necessary). */
        strcpy (fs->folder[fs->count - 1].name, folder);
        if ((strlen (folder) > 1) &&
            (fs->folder[fs->count - 1].name[strlen (folder) - 1] == '/'))
                fs->folder[fs->count - 1].name[strlen (folder) - 1] = '\0';
        fs->folder[fs->count - 1].count = 0;
        fs->folder[fs->count - 1].files_dirty = 1;
        fs->folder[fs->count - 1].folders_dirty = 1;

        return (GP_OK);
}

static int
append_file (CameraFilesystem *fs, int x, CameraFile *file)
{
	CameraFilesystemFile *new;
	const char *name;

	CHECK_NULL (fs && file);

	CR (gp_file_get_name (file, &name));

	if (!fs->folder[x].count)
		CHECK_MEM (new = malloc (sizeof (CameraFilesystemFile)))
	else
		CHECK_MEM (new = realloc (fs->folder[x].file,
					sizeof (CameraFilesystemFile) *
						(fs->folder[x].count + 1)));
	fs->folder[x].file = new;
	fs->folder[x].count++;
	memset (&(fs->folder[x].file[fs->folder[x].count - 1]), 0,
		sizeof (CameraFilesystemFile));
	strcpy (fs->folder[x].file[fs->folder[x].count - 1].name, name);
	fs->folder[x].file[fs->folder[x].count - 1].info_dirty = 1;
	fs->folder[x].file[fs->folder[x].count - 1].normal = file;
	gp_file_ref (file);

	return (GP_OK);
}

/**
 * gp_filesystem_reset:
 * @fs: a #CameraFilesystem
 *
 * Resets the filesystem. All cached information including the folder tree
 * will get lost and will be queried again on demand. 
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_reset (CameraFilesystem *fs)
{
	CR (delete_all_folders (fs, "/", NULL));
	CR (delete_all_files (fs, 0));

	fs->folder[0].folders_dirty = 1;
	fs->folder[0].files_dirty = 1;

	return (GP_OK);
}

/**
 * gp_filesystem_new:
 * @fs: a pointer to a #CameraFilesystem
 *
 * Creates a new empty #CameraFilesystem
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_new (CameraFilesystem **fs)
{
	int result;

	CHECK_NULL (fs);

	CHECK_MEM (*fs = malloc (sizeof (CameraFilesystem)));

        (*fs)->folder = NULL;
        (*fs)->count = 0;

	(*fs)->set_info_func = NULL;
	(*fs)->get_info_func = NULL;
	(*fs)->info_data = NULL;

	(*fs)->file_list_func = NULL;
	(*fs)->folder_list_func = NULL;
	(*fs)->list_data = NULL;

	(*fs)->get_file_func = NULL;
	(*fs)->file_data = NULL;

	result = append_folder (*fs, "/", NULL);
	if (result != GP_OK) {
		free (*fs);
		return (result);
	}

        return (GP_OK);
}

/**
 * gp_filesystem_free:
 * @fs: a #CameraFilesystem
 *
 * Frees the #CameraFilesystem
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_free (CameraFilesystem *fs)
{
	/* We don't care for success or failure */
	gp_filesystem_reset (fs);

	/* Now, we've only got left over the root folder. Free that and
	 * the filesystem. */
	free (fs->folder);
	free (fs);

        return (GP_OK);
}

static int
gp_filesystem_folder_number (CameraFilesystem *fs, const char *folder,
			     GPContext *context)
{
	int x, y, len;
	char buf[128];
	CameraList list;

	CHECK_NULL (fs && folder);
	CC (context);
	CA (folder, context);

	/*
	 * We are nice to front-end/camera-driver writers - we'll ignore
	 * trailing slashes (if any).
	 */
	len = strlen (folder);
	if ((len > 1) && (folder[len - 1] == '/'))
		len--;

	for (x = 0; x < fs->count; x++)
		if (!strncmp (fs->folder[x].name, folder, len) &&
		    (len == strlen (fs->folder[x].name)))
			return (x);

	/* Ok, we didn't find the folder. Do we have a parent? */
	if (!strcmp (folder, "/")) {
		gp_context_error (context, 
			_("Could not find folder '%s'."), folder);
		return (GP_ERROR_DIRECTORY_NOT_FOUND);
	}

	/* If the parent folder is not dirty, return. */
	strncpy (buf, folder, len);
	buf[len] = '\0';
	for (y = strlen (buf) - 1; y >= 0; y--)
		if (buf[y] == '/')
			break;
	if (y)
		buf[y] = '\0';
	else
		buf[y + 1] = '\0'; /* Parent is root */
	CR (x = gp_filesystem_folder_number (fs, buf, context));
	if (!fs->folder[x].folders_dirty) {
		gp_context_error (context, 
			_("Folder '%s' does not contain a folder '%s'."),
			buf, folder);
		return (GP_ERROR_DIRECTORY_NOT_FOUND);
	}

	/*
	 * The parent folder is dirty. List the folders in the parent 
	 * folder to make it clean.
	 */
	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem", "Folder %s is dirty. "
		"Listing file in there to make folder clean...", buf);
	CR (gp_filesystem_list_folders (fs, buf, &list, context));

	return (gp_filesystem_folder_number (fs, folder, context));
}

/**
 * gp_filesystem_append:
 * @fs: a #CameraFilesystem
 * @folder: the folder where to put the file in
 * @filename: filename of the file
 *
 * Tells the @fs that there is a file called @filename in folder 
 * called @folder. Usually, camera drivers will call this function after
 * capturing an image in order to tell the @fs about the new file.
 * A front-end should not use this function.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_append (CameraFilesystem *fs, const char *folder, 
		      const char *filename, GPContext *context) 
{
	CameraFilesystemFile *new;
        int x, y;

	CHECK_NULL (fs && folder);
	CC (context);
	CA (folder, context);

	/* Check for existence */
	x = gp_filesystem_folder_number (fs, folder, context);
	switch (x) {
	case GP_ERROR_DIRECTORY_NOT_FOUND:
		CR (append_folder (fs, folder, context));
	default:
		CR (x);
	}
	CR (x = gp_filesystem_folder_number (fs, folder, context));

	if (!filename)
		return (GP_OK);

	/* If file exists, return error */
	for (y = 0; y < fs->folder[x].count; y++)
		if (!strncmp (fs->folder[x].file[y].name, filename,
			     strlen (filename)) && (
		    (strlen (filename) == strlen (fs->folder[x].file[y].name))))
			break;
	if (y < fs->folder[x].count) {
		gp_context_error (context, 
			_("Could not append '%s' to folder '%s' because "
			  "this file already exists."), filename, folder);
		return (GP_ERROR_FILE_EXISTS);
	}

	/* Allocate a new file in that folder and append the file */
	if (!fs->folder[x].count)
		CHECK_MEM (new = malloc (sizeof (CameraFilesystemFile)))
	else
		CHECK_MEM (new = realloc (fs->folder[x].file,
					sizeof (CameraFilesystemFile) *
					(fs->folder[x].count + 1)));
	fs->folder[x].file = new;
	fs->folder[x].count++;
	memset (&(fs->folder[x].file[fs->folder[x].count - 1]), 0,
		sizeof (CameraFilesystemFile));
	strcpy (fs->folder[x].file[fs->folder[x].count - 1].name, filename);
	fs->folder[x].file[fs->folder[x].count - 1].info_dirty = 1;

	/*
	 * If people manually add files, they probably know the contents of
	 * this folder.
	 */
	fs->folder[x].files_dirty = 0;

        return (GP_OK);
}

int
gp_filesystem_dump (CameraFilesystem *fs)
{
	int i, j;

	GP_DEBUG("Dumping Filesystem:");
	for (i = 0; i < fs->count; i++) {
		GP_DEBUG("  Folder: %s", fs->folder[i].name);
		for (j = 0; j < fs->folder[i].count; j++) {
			GP_DEBUG("    %2i: %s", j, fs->folder[i].file[j].name);
		}
	}

	return (GP_OK);
}

static int
delete_file (CameraFilesystem *fs, int x, int y)
{
	CameraFilesystemFile *new_fip;

	/* Get rid of cached files */
	if (fs->folder[x].file[y].preview) {
		gp_file_unref (fs->folder[x].file[y].preview);
		fs->folder[x].file[y].preview = NULL;
	}
	if (fs->folder[x].file[y].normal) {
		gp_file_unref (fs->folder[x].file[y].normal);
		fs->folder[x].file[y].normal = NULL;
	}
	if (fs->folder[x].file[y].raw) {
		gp_file_unref (fs->folder[x].file[y].raw);
		fs->folder[x].file[y].raw = NULL;
	}
	if (fs->folder[x].file[y].audio) {
		gp_file_unref (fs->folder[x].file[y].audio);
		fs->folder[x].file[y].audio = NULL;
	}
	if (fs->folder[x].file[y].exif) {
		gp_file_unref (fs->folder[x].file[y].exif);
		fs->folder[x].file[y].exif = NULL;
	}

	/* Move all files behind one position up */
	if (y < fs->folder[x].count - 1)
		memmove (&fs->folder[x].file[y], &fs->folder[x].file[y + 1],
			 sizeof (CameraFilesystemFile) *
			 	(fs->folder[x].count - y - 1));
	fs->folder[x].count--;

	/* Get rid of the last one */
	new_fip = realloc (fs->folder[x].file,
		sizeof (CameraFilesystemFile) * (fs->folder[x].count));
	if (!fs->folder[x].count || (fs->folder[x].count && new_fip))
		fs->folder[x].file = new_fip;

	return (GP_OK);
}

static int
gp_filesystem_delete_all_one_by_one (CameraFilesystem *fs, const char *folder,
				     GPContext *context)
{
	CameraList list;
	int count, x;
	const char *name;

	CR (gp_filesystem_list_files (fs, folder, &list, context));
	CR (count = gp_list_count (&list));
	for (x = count - 1; x >= 0; x--) {
		CR (gp_list_get_name (&list, x, &name));
		CR (gp_filesystem_delete_file (fs, folder, name, context));
	}

	return (GP_OK);
}

/**
 * gp_filesystem_delete_all
 * @fs: a #CameraFilesystem
 * @folder: the folder in which to delete all files
 *
 * Deletes all files in the given @folder from the @fs. If the @fs has not
 * been supplied with a delete_all_func, it tries to delete the files
 * one by one using the delete_file_func. If that function has not been
 * supplied neither, an error is returned.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_delete_all (CameraFilesystem *fs, const char *folder,
			  GPContext *context)
{
	int x, r;

	CHECK_NULL (fs && folder);
	CC (context);
	CA (folder, context);

	/* Make sure this folder exists */
	CR (x = gp_filesystem_folder_number (fs, folder, context));

	if (!fs->delete_all_func)
		CR (gp_filesystem_delete_all_one_by_one (fs, folder, context))
	else {

		/*
		 * Mark the folder dirty - it could be that an error
		 * happens, and then we don't know which files have been
		 * deleted and which not.
		 */
		fs->folder[x].files_dirty = 1;

		/*
		 * First try to use the delete_all function. If that fails,
		 * fall back to deletion one-by-one.
		 */
		r = fs->delete_all_func (fs, folder, fs->folder_data, context);
		if (r < 0) {
			gp_log (GP_LOG_DEBUG, "gphoto2-filesystem",
				"delete_all failed (%s). Falling back to "
				"deletion one-by-one.",
				gp_result_as_string (r));
			CR (gp_filesystem_delete_all_one_by_one (fs, folder,
								 context));
		} else
			CR (delete_all_files (fs, x));

		/*
		 * No error happened. We can be sure that all files have been
		 * deleted.
		 */
		fs->folder[x].files_dirty = 0;
	}

	return (GP_OK);
}

/**
 * gp_filesystem_list_files:
 * @fs: a #CameraFilesystem
 * @folder: a folder of which a file list should be generated
 * @list: a #CameraList where to put the list of files into
 *
 * Lists the files in @folder using either cached values or (if there
 * aren't any) the file_list_func which (hopefully) has been previously
 * supplied.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_list_files (CameraFilesystem *fs, const char *folder, 
		          CameraList *list, GPContext *context)
{
	int x, y, count;
	const char *name;

	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem",
		"Listing files in '%s'...", folder);

	CHECK_NULL (fs && list && folder);
	CC (context);
	CA (folder, context);

	gp_list_reset (list);

	/* Search the folder */
	CR (x = gp_filesystem_folder_number (fs, folder, context));

	/* If the folder is dirty, delete the contents and query the camera */
	if (fs->folder[x].files_dirty && fs->file_list_func) {

		gp_log (GP_LOG_DEBUG, "gphoto2-filesystem",
			"Querying folder %s...", folder);
		CR (delete_all_files (fs, x));
		CR (fs->file_list_func (fs, folder, list,
					fs->list_data, context));

		CR (count = gp_list_count (list));
		for (y = 0; y < count; y++) {
			CR (gp_list_get_name (list, y, &name));
			gp_log (GP_LOG_DEBUG, "gphoto2-filesystem",
					 "Added '%s'", name);
			CR (gp_filesystem_append (fs, folder, name, context));
		}
		gp_list_reset (list);
	}

	/* The folder is clean now */
	fs->folder[x].files_dirty = 0;

	for (y = 0; y < fs->folder[x].count; y++) {
		gp_log (GP_LOG_DEBUG, "filesys",
			"Listed '%s'", fs->folder[x].file[y].name);
		CR (gp_list_append (list, fs->folder[x].file[y].name, NULL));
	}

	return (GP_OK);
}

/**
 * gp_filesystem_list_folders:
 * @fs: a #CameraFilesystem
 * @folder: a folder
 * @list: a #CameraList where subfolders should be listed
 *
 * Generates a list of subfolders of the supplied @folder either using 
 * cached values (if there are any) or the folder_list_func if it has been 
 * supplied previously. If not, it is assumed that only a root folder 
 * exists (which is the case for many cameras).
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_list_folders (CameraFilesystem *fs, const char *folder,
			    CameraList *list, GPContext *context)
{
	int x, y, j, offset, count;
	char buf[128];
	const char *name;

	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem",
		"Listing folders in '%s'...", folder);

	CHECK_NULL (fs && folder && list);
	CC (context);
	CA (folder, context);

	gp_list_reset (list);

	/* Search the folder */
	CR (x = gp_filesystem_folder_number (fs, folder, context));

	/* If the folder is dirty, query the contents. */
	if (fs->folder[x].folders_dirty && fs->folder_list_func) {
		CR (fs->folder_list_func (fs, folder, list,
					  fs->list_data, context));
		CR (delete_all_folders (fs, folder, context));
		CR (count = gp_list_count (list));
		for (y = 0; y < count; y++) {
			CR (gp_list_get_name (list, y, &name));
			strcpy (buf, folder);
			if (strlen (folder) != 1)
				strcat (buf, "/");
			strcat (buf, name);
			CR (append_folder (fs, buf, context));
		}
		gp_list_reset (list);
	}

	for (x = 0; x < fs->count; x++)
		if (!strncmp (fs->folder[x].name, folder, strlen (folder))) {
			
			/*
			 * Is this really a subfolder (and not the folder
			 * itself)?
			 */
			if (strlen (fs->folder[x].name) <= strlen (folder))
				continue;

			/*
			 * Is this really a direct subfolder (and not a 
			 * subsubfolder)?
			 */
			for (j = strlen (folder) + 1; 
			     fs->folder[x].name[j] != '\0'; j++)
				if (fs->folder[x].name[j] == '/')
					break;
			if (j == strlen (fs->folder[x].name)) {
				if (!strcmp (folder, "/"))
					offset = 1;
				else
					offset = strlen (folder) + 1;
				CR (gp_list_append (list,
						fs->folder[x].name + offset,
						NULL));
			}
		}

	/* The folder is clean now */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	fs->folder[x].folders_dirty = 0;

	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem", "Folder %s contains %i "
		"files.", folder, fs->folder[x].count);

	return (GP_OK);
}

/**
 * gp_filesystem_count:
 * @fs: a #CameraFilesystem
 * @folder: a folder in which to count the files
 *
 * Counts the files in the @folder.
 *
 * Return value: The number of files in the @folder or a gphoto2 error code.
 **/
int
gp_filesystem_count (CameraFilesystem *fs, const char *folder,
		     GPContext *context)
{
        int x;

	CHECK_NULL (fs && folder);
	CC (context);
	CA (folder, context);

	CR (x = gp_filesystem_folder_number (fs, folder, context));

	return (fs->folder[x].count);
}

/**
 * gp_filesystem_delete_file:
 * @fs: a #CameraFilesystem
 * @folder: a folder in which to delete the file
 * @filename: the name of the file to delete
 *
 * If a delete_file_func has been supplied to the @fs, this function will
 * be called and, if this function returns without error, the file will be 
 * removed from the @fs.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_delete_file (CameraFilesystem *fs, const char *folder, 
			   const char *filename, GPContext *context)
{
        int x, y;

	CHECK_NULL (fs && folder && filename);
	CC (context);
	CA (folder, context);

	/* First of all, do we support file deletion? */
	if (!fs->delete_file_func) {
		gp_context_error (context, _("You have been trying to delete "
			"'%s' from folder '%s', but the filesystem does not "
			"support deletion of files."), filename, folder);
		return (GP_ERROR_NOT_SUPPORTED);
	}

	/* Search the folder and the file */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (y = gp_filesystem_number (fs, folder, filename, context));

	/* Delete the file */
	CR (fs->delete_file_func (fs, folder, filename,
				  fs->file_data, context));
	CR (delete_file (fs, x, y));

	return (GP_OK);
}

int
gp_filesystem_delete_file_noop (CameraFilesystem *fs, const char *folder,
				const char *filename, GPContext *context)
{
	int x, y;

	CHECK_NULL (fs && folder && filename);
	CC (context);
	CA (folder, context);

	/* Search the folder and the file */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (y = gp_filesystem_number (fs, folder, filename, context));
	CR (delete_file (fs, x, y));

	return (GP_OK);
}

/**
 * gp_filesystem_make_dir:
 * @fs: a #CameraFilesystem
 * @folder: the folder in which the directory should be created
 * @name: the name of the directory to be created
 *
 * Creates a new directory called @name in given @folder.
 *
 * Return value: a gphoto2 error code
 **/
int
gp_filesystem_make_dir (CameraFilesystem *fs, const char *folder,
			const char *name, GPContext *context)
{
	int x;
	char path[2048];

	CHECK_NULL (fs && folder && name);
	CC (context);
	CA (folder, context);

	if (!fs->make_dir_func)
		return (GP_ERROR_NOT_SUPPORTED);

	/* Search the folder */
	CR (x = gp_filesystem_folder_number (fs, folder, context));

	strncpy (path, folder, sizeof (path));
	if (strlen (folder) > 1)
		strncat (path, "/", sizeof (path));
	strncat (path, name, sizeof (path));

	/* Create the directory */
	CR (fs->make_dir_func (fs, folder, name, fs->folder_data, context));
	CR (append_folder (fs, path, context));

	return (GP_OK);
}

int
gp_filesystem_remove_dir (CameraFilesystem *fs, const char *folder,
			  const char *name, GPContext *context)
{
	int x;
	char path[2048];
	CameraList list;

	CHECK_NULL (fs && folder && name);
	CC (context);
	CA (folder, context);

	if (!fs->remove_dir_func)
		return (GP_ERROR_NOT_SUPPORTED);

	/*
	 * Make sure there are neither files nor folders in the folder
	 * that is to be removed.
	 */
	strncpy (path, folder, sizeof (path));
	if (strlen (folder) > 1)
		strncat (path, "/", sizeof (path));
	strncat (path, name, sizeof (path));
	CR (gp_filesystem_list_folders (fs, path, &list, context));
	if (gp_list_count (&list)) {
		gp_context_error (context, _("There are still files in "
			"folder '%s' that you are trying to remove."), path);
		return (GP_ERROR_DIRECTORY_EXISTS);
	}
	CR (gp_filesystem_list_files (fs, path, &list, context));
	if (gp_list_count (&list)) {
		gp_context_error (context, _("There are still subfolders in "
			"folder '%s' that you are trying to remove."), path);
		return (GP_ERROR_FILE_EXISTS);
	}

	/* Search the folder */
	CR (x = gp_filesystem_folder_number (fs, path, context));

	/* Remove the directory */
	CR (fs->remove_dir_func (fs, folder, name, fs->folder_data, context));
	CR (delete_folder (fs, x));

	return (GP_OK);
}

/**
 * gp_filesystem_put_file:
 * @fs: a #CameraFilesystem
 * @folder: the folder where to put the @file into
 * @file: the file
 *
 * Uploads a file to the camera if a put_file_func has been previously 
 * supplied to the @fs. If the upload is successful, the file will get
 * cached in the @fs.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_put_file (CameraFilesystem *fs, const char *folder,
			CameraFile *file, GPContext *context)
{
	int x;

	CHECK_NULL (fs && folder && file);
	CC (context);
	CA (folder, context);

	/* Do we support file upload? */
	if (!fs->put_file_func) {
		gp_context_error (context, _("The filesystem does not support "
			"upload of files."));
		return (GP_ERROR_NOT_SUPPORTED);
	}

	/* Search the folder */
	CR (x = gp_filesystem_folder_number (fs, folder, context));

	/* Upload the file */
	CR (fs->put_file_func (fs, folder, file, fs->folder_data, context));
	CR (append_file (fs, x, file));

	return (GP_OK);
}

/**
 * gp_filesystem_name:
 * @fs: a #CameraFilesystem
 * @folder: the folder where to look up the file with the @filenumber
 * @filenumber: the number of the file
 * @filename:
 *
 * Looks up the @filename of file with given @filenumber in given @folder.
 * See gp_filesystem_number for exactly the opposite functionality.
 * 
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_name (CameraFilesystem *fs, const char *folder, int filenumber,
		    const char **filename, GPContext *context)
{
        int x;

	CHECK_NULL (fs && folder);
	CC (context);
	CA (folder, context);

	CR (x = gp_filesystem_folder_number (fs, folder, context));
	
	if (filenumber > fs->folder[x].count) {
		gp_context_error (context, _("Folder '%s' does only contain "
			"%i files, but you requested a file with number %i."),
			folder, fs->folder[x].count, filenumber);
		return (GP_ERROR_FILE_NOT_FOUND);
	}
	
	*filename = fs->folder[x].file[filenumber].name;
	return (GP_OK);
}

/**
 * gp_filesystem_number:
 * @fs: a #CameraFilesystem
 * @folder: the folder where to look for file called @filename
 * @filename: the file to look for
 *
 * Looks for a file called @filename in the given @folder. See
 * gp_filesystem_name for exactly the opposite functionality.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_number (CameraFilesystem *fs, const char *folder, 
		      const char *filename, GPContext *context)
{
	CameraList list;
        int x, y;

	CHECK_NULL (fs && folder && filename);
	CC (context);
	CA (folder, context);

	CR (x = gp_filesystem_folder_number (fs, folder, context));

	for (y = 0; y < fs->folder[x].count; y++)
		if (!strcmp (fs->folder[x].file[y].name, filename))
			return (y);

	/* Ok, we didn't find the file. Is the folder dirty? */
	if (!fs->folder[x].files_dirty) {
		gp_context_error (context, _("File '%s' could not be found "
			"in folder '%s'."), filename, folder);
		return (GP_ERROR_FILE_NOT_FOUND);
	}

	/* The folder is dirty. List all files to make it clean */
	CR (gp_filesystem_list_files (fs, folder, &list, context));

        return (gp_filesystem_number (fs, folder, filename, context));
}

static int
gp_filesystem_scan (CameraFilesystem *fs, const char *folder,
		    const char *filename, GPContext *context)
{
	int count, x;
	CameraList list;
	const char *name;
	char path[128];

	gp_log (GP_LOG_DEBUG, "gphoto2-filesystem", "Scanning %s for %s...",
		folder, filename);

	CHECK_NULL (fs && folder && filename);
	CC (context);
	CA (folder, context);

	CR (gp_filesystem_list_files (fs, folder, &list, context));
	CR (count = gp_list_count (&list));
	for (x = 0; x < count; x++) {
		CR (gp_list_get_name (&list, x, &name));
		if (!strcmp (filename, name))
			return (GP_OK);
	}

	CR (gp_filesystem_list_folders (fs, folder, &list, context));
	CR (count = gp_list_count (&list));
	for (x = 0; x < count; x++) {
		CR (gp_list_get_name (&list, x, &name));
		strcpy (path, folder);
		if (strcmp (path, "/"))
			strcat (path, "/");
		strcat (path, name);
		CR (gp_filesystem_scan (fs, path, filename, context));
	}

	return (GP_OK);
}

/**
 * gp_filesystem_get_folder:
 * @fs: a #CameraFilesystem
 * @filename: the name of the file to search in the @fs
 * @folder:
 *
 * Searches a file called @filename in the @fs and returns the first 
 * occurrency. This functionality is needed for camera drivers that cannot
 * figure out where a file gets created after capturing an image although the
 * name of the image is known. Usually, those drivers will call
 * gp_filesystem_reset in order to tell the @fs that something has 
 * changed and then gp_filesystem_get_folder in order to find the file.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_get_folder (CameraFilesystem *fs, const char *filename, 
			  const char **folder, GPContext *context)
{
	int x, y;

	CHECK_NULL (fs && filename && folder);
	CC (context);

	CR (gp_filesystem_scan (fs, "/", filename, context));

	for (x = 0; x < fs->count; x++)
		for (y = 0; y < fs->folder[x].count; y++)
			if (!strcmp (fs->folder[x].file[y].name, filename)) {
				*folder = fs->folder[x].name;
				return (GP_OK);
			}

	gp_context_error (context, _("Could not find file '%s'."), filename);
	return (GP_ERROR_FILE_NOT_FOUND);
}

/**
 * gp_filesystem_set_list_funcs:
 * @fs: a #CameraFilesystem
 * @file_list_func: the function that will return listings of files
 * @folder_list_func: the function that will return listings of folders
 * @data:
 *
 * Tells the @fs which functions to use to retrieve listings of folders 
 * and/or files. Typically, a camera driver would call this function
 * on initialization. Each function can be NULL indicating that this 
 * functionality is not supported. For example, many cameras don't support
 * folders. In this case, you would supply NULL for folder_list_func. Then,
 * the @fs assumes that there is only a root folder.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_set_list_funcs (CameraFilesystem *fs,
			      CameraFilesystemListFunc file_list_func,
			      CameraFilesystemListFunc folder_list_func,
			      void *data)
{
	CHECK_NULL (fs);

	fs->file_list_func = file_list_func;
	fs->folder_list_func = folder_list_func;
	fs->list_data = data;

	return (GP_OK);
}

/**
 * gp_filesystem_set_file_funcs:
 * @fs: a #CameraFilesystem
 * @get_file_func: the function downloading files
 * @del_file_func: the function deleting files
 * @data:
 *
 * Tells the @fs which functions to use for file download or file deletion.
 * Typically, a camera driver would call this function on initialization.
 * A function can be NULL indicating that this functionality is not supported.
 * For example, if a camera does not support file deletion, you would supply 
 * NULL for del_file_func.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_set_file_funcs (CameraFilesystem *fs,
			      CameraFilesystemGetFileFunc get_file_func,
			      CameraFilesystemDeleteFileFunc del_file_func,
			      void *data)
{
	CHECK_NULL (fs);

	fs->delete_file_func = del_file_func;
	fs->get_file_func = get_file_func;
	fs->file_data = data;

	return (GP_OK);
}

/**
 * gp_filesystem_set_folder_funcs:
 * @fs: a #CameraFilesystem
 * @put_file_func: function used to upload files
 * @delete_all_func: function used to delete all files in a folder
 * @make_dir_func: function used to create a new directory
 * @remove_dir_func: function used to remove an existing directory
 * @data: a data object that will passed to all called functions
 *
 * Tells the filesystem which functions to call for file upload, deletion
 * of all files in a given folder, creation or removal of a folder. 
 * Typically, a camera driver would call this function on initialization. 
 * If one functionality is not supported, NULL can be supplied. 
 * If you don't call this function, the @fs will assume that neither
 * of these features is supported.
 *
 * The @fs will try to compensate missing @delete_all_func
 * functionality with the delete_file_func if such a function has been
 * supplied.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_set_folder_funcs (CameraFilesystem *fs,
				CameraFilesystemPutFileFunc put_file_func,
				CameraFilesystemDeleteAllFunc delete_all_func,
				CameraFilesystemDirFunc make_dir_func,
				CameraFilesystemDirFunc remove_dir_func,
				void *data)
{
	CHECK_NULL (fs);

	fs->put_file_func = put_file_func;
	fs->delete_all_func = delete_all_func;
	fs->make_dir_func = make_dir_func;
	fs->remove_dir_func = remove_dir_func;
	fs->folder_data = data;

	return (GP_OK);
}

/**
 * gp_filesystem_get_file:
 * @fs: a #CameraFilesystem
 * @folder: the folder in which the file can be found
 * @filename: the name of the file to download
 * @type: the type of the file
 * @file:
 *
 * Downloads the file called @filename from the @folder using the 
 * get_file_func if such a function has been previously supplied. If the 
 * file has been previously downloaded, the file is retrieved from cache.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_get_file (CameraFilesystem *fs, const char *folder,
			const char *filename, CameraFileType type,
			CameraFile *file, GPContext *context)
{
	int x, y, r;
	time_t t = 0;

	CHECK_NULL (fs && folder && file && filename);
	CC (context);
	CA (folder, context);

	GP_DEBUG ("Getting file '%s' from folder '%s' (type %i)...",
		  filename, folder, type);

	CR (gp_file_set_type (file, type));
	CR (gp_file_set_name (file, filename));

	if (!fs->get_file_func) {
		gp_context_error (context,
			_("The filesystem doesn't support getting files"));
		return (GP_ERROR_NOT_SUPPORTED);
	}

	/* Search folder and file */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (y = gp_filesystem_number (fs, folder, filename, context));

	switch (type) {
	case GP_FILE_TYPE_PREVIEW:
		if (fs->folder[x].file[y].preview)
			return (gp_file_copy (file,
					fs->folder[x].file[y].preview));
		break;
	case GP_FILE_TYPE_NORMAL:
		if (fs->folder[x].file[y].normal)
			return (gp_file_copy (file,
					fs->folder[x].file[y].normal));
		break;
	case GP_FILE_TYPE_RAW:
		if (fs->folder[x].file[y].raw)
			return (gp_file_copy (file,
					fs->folder[x].file[y].raw));
		break;
	case GP_FILE_TYPE_AUDIO:
		if (fs->folder[x].file[y].audio)
			return (gp_file_copy (file,
					fs->folder[x].file[y].audio));
		break;
	case GP_FILE_TYPE_EXIF:
		if (fs->folder[x].file[y].exif)
			return (gp_file_copy (file,
					fs->folder[x].file[y].exif));
		break;
	default:
		gp_context_error (context, _("Unknown file type %i."), type);
		return (GP_ERROR);
	}

	r = fs->get_file_func (fs, folder, filename, type, file,
			       fs->file_data, context);
	if ((type == GP_FILE_TYPE_PREVIEW) &&
	    (r == GP_ERROR_NOT_SUPPORTED)) {

		/* Try EXIF data */
		GP_DEBUG ("Getting previews is not supported. Trying "
			  "EXIF data...");
		CR (gp_filesystem_get_exif_preview (fs, folder, filename,
						    context));
	} else
		CR (r);

	/* We don't trust the camera drivers */
	CR (gp_file_set_type (file, type));
	CR (gp_file_set_name (file, filename));

	/* Cache this file */
	CR (gp_filesystem_set_file_noop (fs, folder, file, context));

	/* If we didn't get a mtime, try to get it from EXIF data */
	gp_file_get_mtime (file, &t);
	if (!t) {
		GP_DEBUG ("Did not get mtime. Trying EXIF information...");
		t = gp_filesystem_get_exif_mtime (fs, folder, filename);
		gp_file_set_mtime (file, t);
	}

	/*
	 * Often, thumbnails are of a different mime type than the normal
	 * picture. In this case, we should rename the file.
	 */
	if (type != GP_FILE_TYPE_NORMAL)
		CR (gp_file_adjust_name_for_mime_type (file));

	return (GP_OK);
}

/**
 * gp_filesystem_set_info_funcs:
 * @fs: a #CameraFilesystem
 * @get_info_func: the function to retrieve file information
 * @set_info_func: the function to set file information
 * @data:
 *
 * Tells the filesystem which functions to call when file information 
 * about a file should be retrieved or set. Typically, this function will
 * get called by the camera driver on initialization. 
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_set_info_funcs (CameraFilesystem *fs,
			      CameraFilesystemGetInfoFunc get_info_func,
			      CameraFilesystemSetInfoFunc set_info_func,
			      void *data)
{
	CHECK_NULL (fs);

	fs->get_info_func = get_info_func;
	fs->set_info_func = set_info_func;
	fs->info_data = data;

	return (GP_OK);
}

static int
gp_filesystem_get_exif_preview (CameraFilesystem *fs, const char *folder,
				const char *filename, GPContext *context)
{
	CameraFile *file;
	ExifData *ed;
	int r;
	const char *data = NULL;
	unsigned long int size = 0;

	gp_file_new (&file);
	r = gp_filesystem_get_file (fs, folder, filename, GP_FILE_TYPE_EXIF,
				    file, context);
	if (r < 0) {
		GP_DEBUG ("Could not get EXIF data of '%s' in folder '%s'.",
			  filename, folder);
		gp_file_unref (file);
		return (r);
	}

	gp_file_get_data_and_size (file, &data, &size);
	ed = exif_data_new_from_data (data, size);
	gp_file_unref (file);
	if (!ed) {
		GP_DEBUG ("Could not parse EXIF data of '%s' in folder '%s'.",
			  filename, folder);
		return GP_ERROR_CORRUPTED_DATA;
	}

	if (ed->data) {
		file = NULL;
		gp_file_new (&file);
		gp_file_set_data_and_size (file, ed->data, ed->size);
		gp_file_set_type (file, GP_FILE_TYPE_PREVIEW);
		gp_file_set_name (file, filename);
		gp_file_detect_mime_type (file);
		gp_filesystem_set_file_noop (fs, folder, file, context);
		gp_file_unref (file);
	}
	exif_data_unref (ed);

	return (GP_OK);
}

static time_t
gp_filesystem_get_exif_mtime (CameraFilesystem *fs, const char *folder,
			      const char *filename)
{
	CameraFile *file;
	ExifData *ed;
	ExifEntry *e;
	const char *data = NULL;
	unsigned long int size = 0;
	struct tm ts;
	time_t t;

	if (!fs)
		return 0;

	gp_file_new (&file);
	if (gp_filesystem_get_file (fs, folder, filename, GP_FILE_TYPE_EXIF,
				    file, NULL) != GP_OK) {
		GP_DEBUG ("Could not get EXIF data of '%s' in folder '%s'.",
			  filename, folder);
		gp_file_unref (file);
		return 0;
	}

	gp_file_get_data_and_size (file, &data, &size);
	ed = exif_data_new_from_data (data, size);
	gp_file_unref (file);
	if (!ed) {
		GP_DEBUG ("Could not parse EXIF data of '%s' in folder '%s'.",
			  filename, folder);
		return 0;
	}

	/*
	 * HP PhotoSmart C30 has the date and time in ifd_exif.
	 */
	e = exif_content_get_entry (ed->ifd_exif, EXIF_TAG_DATE_TIME_ORIGINAL);
	if (!e) {
		GP_DEBUG ("EXIF data of '%s' in folder '%s' "
			  "has not date/time tag.", filename, folder);
		exif_data_unref (ed);
		return 0;
	}

	e->data[4] = e->data[ 7] = e->data[10] = e->data[13] = e->data[16] = 0;
	ts.tm_year = atoi (data) - 1900;
	ts.tm_mon  = atoi (data + 5);
	ts.tm_mday = atoi (data + 8);
	ts.tm_hour = atoi (data + 11);
	ts.tm_min  = atoi (data + 14);
	ts.tm_sec  = atoi (data + 17);
	exif_data_unref (ed);

	t = mktime (&ts);
	GP_DEBUG ("Found time in EXIF data: '%s'.", asctime (localtime (&t)));

	return (t);
}

/**
 * gp_filesystem_get_info:
 * @fs: a #CameraFilesystem
 * @folder:
 * @filename:
 * @info:
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_get_info (CameraFilesystem *fs, const char *folder,
			const char *filename, CameraFileInfo *info,
			GPContext *context)
{
	int x, y;
	time_t t;

	CHECK_NULL (fs && folder && filename && info);
	CC (context);
	CA (folder, context);

	GP_DEBUG ("Getting information about '%s' in '%s'...", filename,
		  folder);

	if (!fs->get_info_func) {
		gp_context_error (context,
			_("The filesystem doesn't support getting file "
			  "information"));
		return (GP_ERROR_NOT_SUPPORTED);
	}

	/* Search folder and file and get info if needed */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (y = gp_filesystem_number (fs, folder, filename, context));
	if (fs->folder[x].file[y].info_dirty) {
		CR (fs->get_info_func (fs, folder, filename, 
						&fs->folder[x].file[y].info,
						fs->info_data, context));
		fs->folder[x].file[y].info_dirty = 0;
	}

	/*
	 * If we didn't get GP_FILE_INFO_MTIME, we'll have a look if we
	 * can get it from EXIF data.
	 */
	if (!(fs->folder[x].file[y].info.file.fields & GP_FILE_INFO_MTIME)) {
		GP_DEBUG ("Did not get mtime. Trying EXIF information...");
		t = gp_filesystem_get_exif_mtime (fs, folder, filename);
		if (t) {
			fs->folder[x].file[y].info.file.mtime = t;
			fs->folder[x].file[y].info.file.fields |=
							GP_FILE_INFO_MTIME;
		}
	}

	memcpy (info, &fs->folder[x].file[y].info, sizeof (CameraFileInfo));

	return (GP_OK);
}

/**
 * gp_filesystem_set_file_noop:
 * @fs: a #CameraFilesystem
 * @folder:
 * @file: a #CameraFile
 *
 * Tells the @fs about a file. Typically, camera drivers will call this
 * function in case they get information about a file (i.e. preview) "for free"
 * on #gp_camera_capture or #gp_camera_folder_list_files.
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_set_file_noop (CameraFilesystem *fs, const char *folder,
			     CameraFile *file, GPContext *context)
{
	CameraFileType type;
	const char *filename;
	int x, y;

	CHECK_NULL (fs && folder && file);
	CC (context);
	CA (folder, context);

	/* Search folder and file */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (gp_file_get_name (file, &filename));
	CR (y = gp_filesystem_number (fs, folder, filename, context));

	CR (gp_file_get_type (file, &type));
	switch (type) {
	case GP_FILE_TYPE_PREVIEW:
		if (fs->folder[x].file[y].preview)
			gp_file_unref (fs->folder[x].file[y].preview);
		fs->folder[x].file[y].preview = file;
		gp_file_ref (file);
		break;
	case GP_FILE_TYPE_NORMAL:
		if (fs->folder[x].file[y].normal)
			gp_file_unref (fs->folder[x].file[y].normal);
		fs->folder[x].file[y].normal = file;
		gp_file_ref (file);
		break;
	case GP_FILE_TYPE_RAW:
		if (fs->folder[x].file[y].raw)
			gp_file_unref (fs->folder[x].file[y].raw);
		fs->folder[x].file[y].raw = file;
		gp_file_ref (file);
		break;
	case GP_FILE_TYPE_AUDIO:
		if (fs->folder[x].file[y].audio)
			gp_file_unref (fs->folder[x].file[y].audio);
		fs->folder[x].file[y].audio = file;
		gp_file_ref (file);
		break;
	case GP_FILE_TYPE_EXIF:
		if (fs->folder[x].file[y].exif)
			gp_file_unref (fs->folder[x].file[y].exif);
		fs->folder[x].file[y].exif = file;
		gp_file_ref (file);
		break;
	default:
		gp_context_error (context, _("Unknown file type %i."), type);
		return (GP_ERROR);
	}

	return (GP_OK);
}

/**
 * gp_filesystem_set_info_noop:
 * @fs: a #CameraFilesystem
 * @folder:
 * @info:
 *
 * In contrast to #gp_filesystem_set_info, #gp_filesystem_set_info_noop
 * will only change the file information in the @fs. Typically, camera
 * drivers will use this function in case they get file information "for free"
 * on #gp_camera_capture or #gp_camera_folder_list_files.
 *
 * Return value: a gphoto2 error code
 **/
int
gp_filesystem_set_info_noop (CameraFilesystem *fs, const char *folder,
			     CameraFileInfo info, GPContext *context)
{
	int x, y;

	CHECK_NULL (fs && folder);
	CC (context);
	CA (folder, context);

	/* Search folder and file */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (y = gp_filesystem_number (fs, folder, info.file.name, context));

	memcpy (&fs->folder[x].file[y].info, &info, sizeof (CameraFileInfo));
	fs->folder[x].file[y].info_dirty = 0;

	return (GP_OK);
}

/**
 * gp_filesystem_set_info:
 * @fs: a #CameraFilesystem
 * @folder:
 * @filename:
 * @info:
 *
 * Return value: a gphoto2 error code.
 **/
int
gp_filesystem_set_info (CameraFilesystem *fs, const char *folder,
			const char *filename, CameraFileInfo info,
			GPContext *context)
{
	int x, y, result, name, e;

	CHECK_NULL (fs && folder && filename);
	CC (context);
	CA (folder, context);

	if (!fs->set_info_func) {
		gp_context_error (context, 
			_("The filesystem doesn't support setting file "
			  "information"));
		return (GP_ERROR_NOT_SUPPORTED);
	}

	/* Search folder and file */
	CR (x = gp_filesystem_folder_number (fs, folder, context));
	CR (y = gp_filesystem_number (fs, folder, filename, context));

	/* Check if people want to set read-only attributes */
	if ((info.file.fields    & GP_FILE_INFO_TYPE)   ||
	    (info.file.fields    & GP_FILE_INFO_SIZE)   ||
	    (info.file.fields    & GP_FILE_INFO_WIDTH)  ||
	    (info.file.fields    & GP_FILE_INFO_HEIGHT) ||
	    (info.file.fields    & GP_FILE_INFO_STATUS) ||
	    (info.preview.fields & GP_FILE_INFO_TYPE)   ||
	    (info.preview.fields & GP_FILE_INFO_SIZE)   ||
	    (info.preview.fields & GP_FILE_INFO_WIDTH)  ||
	    (info.preview.fields & GP_FILE_INFO_HEIGHT) ||
	    (info.preview.fields & GP_FILE_INFO_STATUS) ||
	    (info.audio.fields   & GP_FILE_INFO_TYPE)   ||
	    (info.audio.fields   & GP_FILE_INFO_SIZE)   ||
	    (info.audio.fields   & GP_FILE_INFO_STATUS)) {
		gp_context_error (context, _("Read-only file attributes "
			"like width and height can not be changed."));
		return (GP_ERROR_BAD_PARAMETERS);
	}

	/*
	 * Set the info. If anything goes wrong, mark info as dirty, 
	 * because the operation could have been partially successful.
	 *
	 * Handle name changes in a separate round.
	 */
	name = (info.file.fields & GP_FILE_INFO_NAME);
	info.file.fields &= ~GP_FILE_INFO_NAME;
	result = fs->set_info_func (fs, folder, filename, info, fs->info_data,
				    context);
	if (result < 0) {
		fs->folder[x].file[y].info_dirty = 1;
		return (result);
	}
	if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
		fs->folder[x].file[y].info.file.permissions = 
						info.file.permissions;

	/* Handle name change */
	if (name) {

		/* Make sure the file does not exist */
		e = gp_filesystem_number (fs, folder, info.file.name, context);
		if (e != GP_ERROR_FILE_NOT_FOUND)
			return (e);
		
		info.preview.fields = GP_FILE_INFO_NONE;
		info.file.fields = GP_FILE_INFO_NAME;
		info.audio.fields = GP_FILE_INFO_NONE;
		CR (fs->set_info_func (fs, folder, filename, info,
				       fs->info_data, context));
		strncpy (fs->folder[x].file[y].info.file.name, info.file.name,
			 sizeof (fs->folder[x].file[y].info.file.name));
		strncpy (fs->folder[x].file[y].name, info.file.name,
			 sizeof (fs->folder[x].file[y].name));
	}

	return (GP_OK);
}
