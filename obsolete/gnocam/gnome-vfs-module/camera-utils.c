#include <config.h>
#include "camera-utils.h"

#include <string.h>
#include <glib.h>

GList *
camera_uri_get_dirs (GnomeVFSURI *uri)
{
	gchar *us, *start, *end;
	GList *l = NULL;

	us = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	if (strncmp (us, "camera:", strlen ("camera:"))) {
		g_warning ("URI '%s' does not start with 'camera:'!", us);
		g_free (us);
		return NULL;
	}
	start = strchr (us + strlen ("camera:"), ':');
	if (!start) {g_free (us); return NULL;}
	start = strchr (start + 1, ':');
	if (!start) {g_free (us); return NULL;}
	start = strchr (start + 1, ':');
	if (!start) {g_free (us); return NULL;}
	start = strchr (start + 1, ':');
	if (!start) {g_free (us); return NULL;}
	if (*start != '/') {g_free (us); return NULL;}

	while (start) {
		end = strchr (start + 1, '/');
		if (!end) l = g_list_append (l, g_strdup (start + 1));
		else l = g_list_append (l, g_strndup (start + 1, end - start));
		start = end;
	}
	g_free (us);
	return l;
}

gchar *
camera_uri_get_manuf (GnomeVFSURI *uri)
{
        gchar *us, *start, *end, *s; 

        us = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
        if (!us) {
		g_warning ("Could not convert URI to string!");
		return NULL;
	}
        if (strncmp (us, "camera:", strlen ("camera:"))) {
		g_warning ("URI '%s' does not start with 'camera:'!", us);
                g_free (us);
                return NULL;
        }
        start = us + strlen ("camera:");
        end = strchr (us + strlen ("camera:"), ':');
        if (end) *end = '\0';
        s = g_strdup (start);
        g_free (us);
        return s;
}

gchar *
camera_uri_get_model (GnomeVFSURI *uri)
{
        gchar *us, *start, *end, *s;

        us = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
        if (strncmp (us, "camera:", strlen ("camera:"))) {
		g_warning ("URI '%s' does not start with 'camera:'!", us);
                g_free (us);
                return NULL;
        }
        start = strchr (us + strlen ("camera:"), ':');
        if (!start) {g_free (us); return NULL;}
        end = strchr (++start, ':');
        if (end) *end = '\0';
        s = g_strdup (start);
        g_free (us);
        return s;
}

gchar *
camera_uri_get_port (GnomeVFSURI *uri)
{
        gchar *us, *start, *end, *s;

        us = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
        if (strncmp (us, "camera:", strlen ("camera:"))) {
		g_warning ("URI '%s' does not start with 'camera:'!", us);
                g_free (us);
                return NULL;
        }
        start = strchr (us + strlen ("camera:"), ':');
        if (!start) {g_free (us); return NULL;}
        start = strchr (start + 1, ':');
        if (!start) {g_free (us); return NULL;}
        end = strchr (++start, ':');
        if (end) *end = '\0';
        s = g_strdup (start);
        g_free (us);
        return s;
}
