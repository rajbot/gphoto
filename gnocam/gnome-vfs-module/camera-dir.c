#include <config.h>
#include "camera-dir.h"
#include "camera-utils.h"

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
	gchar *manuf, *model, *port;
	CORBA_Environment ev;

	g_return_val_if_fail (d, GNOME_VFS_ERROR_BAD_PARAMETERS);

	manuf = camera_uri_get_manuf (uri);
	model = camera_uri_get_model (uri);
	port  = camera_uri_get_port  (uri);

	CORBA_exception_init (&ev);
	c = gnocam_util_get_camera (manuf, model, port, &ev);
	g_free (manuf);
	g_free (model);
	g_free (port);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		return GNOME_VFS_ERROR_GENERIC;
	}
	CORBA_exception_free (&ev);

	*d = g_new0 (CameraDir, 1);

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
