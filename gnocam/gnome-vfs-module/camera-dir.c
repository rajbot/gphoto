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
	GNOME_C_IDList *dl;

	/* We put all interfaces except 'default' into subdirectories. */
	GSList *ifs;
	GSList *files;

	guint n;
};

GnomeVFSResult
camera_dir_open (GnomeVFSURI *uri, CameraDir **d)
{
	GNOME_C_Camera c;
	GNOME_C_Dir dir, subdir;
	GNOME_C_File f;
	GNOME_C_IDList *dl, *fl, *il;
	gchar *manuf, *model, *port;
	CORBA_Environment ev;
	GList *l, *p;
	guint i, j, k;
	CORBA_string n, name, type;
	GNOME_C_If interface;

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
	dir = GNOME_C_Camera__get_dir (c, &ev);
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
		g_list_foreach (l, (GFunc) g_free, NULL); g_list_free (l);
		bonobo_object_release_unref (dir, NULL);
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	    }
	    for (i = 0; i < dl->_length; i++) {
		subdir = GNOME_C_Dir_get_dir (dir, dl->_buffer[i], &ev);
		if (BONOBO_EX (&ev)) {
		    CORBA_free (dl);
		    g_list_foreach (l, (GFunc) g_free, NULL); g_list_free (l);
		    bonobo_object_release_unref (dir, NULL);
		    CORBA_exception_free (&ev); 
		    return GNOME_VFS_ERROR_GENERIC;
		}
		n = GNOME_C_Dir__get_name (subdir, &ev);
		if (BONOBO_EX (&ev)) {
		    CORBA_free (dl);
		    g_list_foreach (l, (GFunc) g_free, NULL); g_list_free (l);
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
		g_list_foreach (l, (GFunc) g_free, NULL); g_list_free (l);
		bonobo_object_release_unref (dir, NULL);
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	    }
	    CORBA_free (dl);
	}
	g_list_foreach (l, (GFunc) g_free, NULL); g_list_free (l);

	*d = g_new0 (CameraDir, 1);
	(*d)->dir = dir;
	(*d)->dl = GNOME_C_Dir_get_dirs  ((*d)->dir, &ev);

	fl = GNOME_C_Dir_get_files (dir, &ev);
	if (!BONOBO_EX (&ev)) {
	    for (i = 0; i < fl->_length; i++) {

		/* Get the file and the corresponding interfaces */
		f = GNOME_C_Dir_get_file (dir, fl->_buffer[i], &ev);
		if (BONOBO_EX (&ev)) {
			continue;
		}
		il = GNOME_C_File_get_ifs (f, &ev);
		if (BONOBO_EX (&ev)) {
			bonobo_object_release_unref (f, NULL);
			continue;
		}

		for (j = 0; j < il->_length; j++){
		    interface = GNOME_C_File_get_if (f, il->_buffer[j], &ev);
		    if (BONOBO_EX (&ev))
		        continue;
		    if (!i) {
			name = GNOME_C_If__get_name (interface, &ev);
			if (!BONOBO_EX (&ev)) {
			    (*d)->files = g_slist_append ((*d)->files,
							  g_strdup (name));
			    CORBA_free (name);
			}
		    } else {
			type = GNOME_C_If__get_type (interface, &ev);
			if (!BONOBO_EX (&ev)) {
			    for (k = 0; k < g_slist_length ((*d)->ifs); k++)
				if (!strcmp (g_slist_nth_data ((*d)->ifs, k),
					     type)) break;
			    if (k == g_slist_length ((*d)->ifs))
				(*d)->ifs = g_slist_append ((*d)->ifs,
							    g_strdup (type));
			    CORBA_free (type);
			}
		    }
		    bonobo_object_release_unref (interface, NULL);
		}
		bonobo_object_release_unref (f, NULL);
	    }
	    CORBA_free (fl);
	}

	CORBA_exception_free (&ev);

	return GNOME_VFS_OK;
}

GnomeVFSResult
camera_dir_read (CameraDir *d, GnomeVFSFileInfo *i)
{
	GNOME_C_Dir dir;
	CORBA_Environment ev;
	CORBA_string name;

	g_return_val_if_fail (d, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (i, GNOME_VFS_ERROR_BAD_PARAMETERS);

	if (d->n < d->dl->_length) {
		i->type = GNOME_VFS_FILE_TYPE_DIRECTORY;

		CORBA_exception_init (&ev);
		dir = GNOME_C_Dir_get_dir (d->dir, d->dl->_buffer[d->n], &ev);
		if (!BONOBO_EX (&ev)) {
			name = GNOME_C_Dir__get_name (dir, &ev);
			if (!BONOBO_EX (&ev)) {
				i->name = g_strdup (name);
				CORBA_free (name);
			}
			bonobo_object_release_unref (dir, NULL);
		}
		CORBA_exception_free (&ev);

	} else if (d->n < g_slist_length (d->ifs) + d->dl->_length) {
		i->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		i->name = g_strdup (g_slist_nth_data (d->ifs,
						d->n - d->dl->_length));
	} else if (d->n < g_slist_length (d->files) +
			g_slist_length (d->ifs) + d->dl->_length) {
	    i->type = GNOME_VFS_FILE_TYPE_REGULAR;
	    i->name = g_strdup (g_slist_nth_data (d->files,
		d->n - g_slist_length (d->ifs) - d->dl->_length));
	} else return GNOME_VFS_ERROR_EOF;
	d->n++;

	return GNOME_VFS_OK;
}

GnomeVFSResult
camera_dir_close (CameraDir *d)
{
	g_return_val_if_fail (d, GNOME_VFS_ERROR_BAD_PARAMETERS);

	bonobo_object_release_unref (d->dir, NULL);
	CORBA_free (d->dl);
	g_slist_foreach (d->ifs, (GFunc) g_free, NULL); g_slist_free (d->ifs);
	g_slist_foreach (d->files, (GFunc) g_free, NULL);
	g_slist_free (d->files);
	g_free (d);

	return GNOME_VFS_OK;
}
