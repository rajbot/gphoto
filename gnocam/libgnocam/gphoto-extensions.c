#include <config.h>
#include <gphoto2.h>
#include <gnome.h>
#include <bonobo/bonobo.h>
#include <bonobo/bonobo-storage-plugin.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>

#include "gphoto-extensions.h"

gint
gp_camera_new_from_gconf (Camera** camera, const gchar* name_or_url)
{
	GConfClient*	client;
        guint 		i, result;
	gchar*		name;

	g_return_val_if_fail (camera, 		GP_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (!((name_or_url [0] == '/') && (name_or_url [1] != '/')), GP_ERROR_BAD_PARAMETERS);

	/* Make sure GConf is initialized. */
	if (!gconf_is_initialized ()) {
		GError* gerror = NULL;
		gchar*  argv[1] = {"Whatever"};
		g_return_val_if_fail (gconf_init (1, argv, &gerror), GP_ERROR);
	}
	g_return_val_if_fail (client = gconf_client_get_default (), GP_ERROR);

	/* Make sure we are given a camera name. */
	if (!strncmp (name_or_url, "camera:", 7)) name_or_url += 7;
	if (name_or_url [0] == '/') {
		name_or_url += 2;
		for (i = 0; name_or_url [i] != 0; i++) if (name_or_url [i] == '/') break;
		name = g_strndup (name_or_url, i);
	} else {
		name = g_strdup (name_or_url);
	}
	
        /* Does GConf know about the camera? */
	for (i = 0; ; i++) {
		gchar* tmp;
		gchar* path = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);

		/* Does such an entry exist? */
		if (!gconf_client_dir_exists (client, path, NULL)) {
			g_warning (_("Directory '%s' does not exist. Camera '%s' unknown!"), path, name);
			g_free (name);
			g_free (path);
			return (GP_ERROR);
		}

		/* Is this our camera? */
		{
			gchar* name_gconf;
			
			tmp = g_strconcat (path, "/name", NULL);
			name_gconf = gconf_client_get_string (client, tmp, NULL);
			g_free (tmp);
			if (strcmp (name, name_gconf)) {
				g_free (path);
				g_free (name_gconf);
				continue;
			}

			g_free (name_gconf);
		}

		/* This is our camera. Free name - we don't need it no more. */
		g_free (name);

		/* Create a camera */
		if ((result = gp_camera_new (camera)) != GP_OK) {
			g_free (path);
			return (result);
		}

		/* Get model. */
		{
			gchar* model;
			
			tmp = g_strconcat (path, "/model", NULL);
			model = gconf_client_get_string (client, tmp, NULL);
			g_free (tmp);
			
			strcpy ((*camera)->model, model);
			g_free (model);
		}

		/* Get port. */
		{
			gchar* port;
			
			tmp = g_strconcat (path, "/port", NULL);
			port = gconf_client_get_string (client, tmp, NULL);
			g_free (tmp);

			strcpy ((*camera)->port->name, port);
			g_free (port);
		}
		
		/* Free path - we've got from gconf everything we need. */
		g_free (path);

		/* Prepare initialization. */
		(*camera)->port->speed = 0;

		/* Connect to the camera */
		if ((result = gp_camera_init (*camera)) != GP_OK) {
			g_warning (_("Could not initialize camera! (%s)"), gp_camera_result_as_string (*camera, result));
			gp_camera_unref (*camera);
			*camera = NULL;
		}
		return (result);
	}
}

