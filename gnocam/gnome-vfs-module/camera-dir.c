#include <config.h>
#include "camera-dir.h"
#include "camera-utils.h"

#include <string.h>

#include <libgnocam/gnocam-util.h>
#include <libgnocam/GNOME_C.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-object.h>

struct _CameraDir
{
	GNOME_C_Dir dir;
	GNOME_C_IDList *fl;
	GNOME_C_IDList *dl;

	guint n;
};

GnomeVFSResult
camera_dir_open (GnomeVFSURI *uri, CameraDir **d)
{
	GNOME_C_Camera c;
	GNOME_C_Dir dir, subdir;
	GNOME_C_IDList *dl;
	gchar *manuf, *model, *port;
	CORBA_Environment ev;
	GList *l, *p;
	guint i;
	CORBA_string n;

	g_return_val_if_fail (d, GNOME_VFS_ERROR_BAD_PARAMETERS);

	manuf = camera_uri_get_manuf (uri);
	model = camera_uri_get_model (uri);
	port  = camera_uri_get_port  (uri);

	CORBA_exception_init (&ev);
	c = gnocam_util_get_camera (manuf, model, port, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Failed to get camera ('%s', '%s', '%s'): %s",
			manuf, model, port, bonobo_exception_get_text (&ev));
		g_free (manuf); g_free (model); g_free (port);
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	}
	g_free (manuf); g_free (model); g_free (port);
	if (c == CORBA_OBJECT_NIL) {
		g_warning ("Didn't get a camera although no error has been "
			   "reported. Blame Lutz Müller "
			   "<lutz@users.sourceforge.net>.");
		return GNOME_VFS_ERROR_GENERIC;
	}

	/* So, we've got a camera. Get the dir. */
	dir = GNOME_C_Camera_get_dir (c, &ev);
	bonobo_object_release_unref (c, NULL);
	if (BONOBO_EX (&ev)) {
		g_warning ("Failed to get root directory: %s",
			   bonobo_exception_get_text (&ev));
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	}
	p = l = camera_uri_get_dirs (uri);
	while (p) {
	    dl = GNOME_C_Dir_get_dirs (dir, &ev);
	    if (BONOBO_EX (&ev)) {
		g_warning ("Failed to list directories in '%s': %s",
			   (char *) p->data, bonobo_exception_get_text (&ev));
		for (i = 0; i < g_list_length (l); i++) 
		    g_free (g_list_nth_data (l, i));
		g_list_free (l);
		bonobo_object_release_unref (dir, NULL);
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	    }
	    for (i = 0; i < dl->_length; i++) {
		subdir = GNOME_C_Dir_get_dir (dir, dl->_buffer[i], &ev);
		if (BONOBO_EX (&ev)) {
		    CORBA_free (dl);
		    for (i = 0; i < g_list_length (l); i++) 
			g_free (g_list_nth_data (l, i));
		    g_list_free (l);
		    bonobo_object_release_unref (dir, NULL);
		    CORBA_exception_free (&ev); 
		    return GNOME_VFS_ERROR_GENERIC;
		}
		n = GNOME_C_Dir__get_name (subdir, &ev);
		if (BONOBO_EX (&ev)) {
		    CORBA_free (dl);
		    for (i = 0; i < g_list_length (l); i++)
			g_free (g_list_nth_data (l, i));
		    g_list_free (l);
		    bonobo_object_release_unref (dir, NULL);
		    bonobo_object_release_unref (subdir, NULL);
		    CORBA_exception_free (&ev);
		    return GNOME_VFS_ERROR_GENERIC;
		}
		if (!strcmp (n, p->data)) {
		    CORBA_free (dl);
		    CORBA_free (n);
		    bonobo_object_release_unref (dir, NULL);
		    dir = subdir;
		    p = p->next;
		    break;
		}
		bonobo_object_release_unref (subdir, NULL);
		CORBA_free (n);
	    }
	    if (i == dl->_length) {
		CORBA_free (dl);
		for (i = 0; i < g_list_length (l); i++)
		    g_free (g_list_nth_data (l, i));
		g_list_free (l);
		bonobo_object_release_unref (dir, NULL);
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	    }
	    CORBA_free (dl);
	}
	for (i = 0; i < g_list_length (l); i++) g_free (g_list_nth_data (l, i));
	g_list_free (l);

	*d = g_new0 (CameraDir, 1);
	(*d)->dir = dir;
	(*d)->fl = GNOME_C_Dir_get_files ((*d)->dir, &ev);
	(*d)->dl = GNOME_C_Dir_get_dirs  ((*d)->dir, &ev);

	CORBA_exception_free (&ev);

	return GNOME_VFS_OK;
}

GnomeVFSResult
camera_dir_read (CameraDir *d, GnomeVFSFileInfo *i)
{
	g_return_val_if_fail (d, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (i, GNOME_VFS_ERROR_BAD_PARAMETERS);

	return GNOME_VFS_OK;
}

GnomeVFSResult
camera_dir_close (CameraDir *d)
{
	g_return_val_if_fail (d, GNOME_VFS_ERROR_BAD_PARAMETERS);

	bonobo_object_release_unref (d->dir, NULL);
	CORBA_free (d->dl);
	CORBA_free (d->fl);
	g_free (d);

	return GNOME_VFS_OK;
}
