#include <config.h>
#include <gnome.h>
#include <bonobo/bonobo.h>
#include <bonobo/bonobo-storage-plugin.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <parser.h>
#include "utils.h"

Camera*
util_camera_new (GnomeVFSURI* uri, CORBA_Environment* ev) 
{
	GConfClient*	client = gconf_client_get_default ();
        guint 		i, result;

	g_return_val_if_fail (gconf_is_initialized (), NULL);

        /* Does GConf know about the camera (host)? */
	for (i = 0; ; i++) {
		gchar* name;
		gchar* tmp;
		gchar* path = g_strdup_printf ("/apps/" PACKAGE "/camera/%i", i);
		gchar* host = gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (uri), NULL);
		
		if (!gconf_client_dir_exists (client, path, NULL)) {
			g_warning (_("Directory '%s' does not exist. Camera '%s' unknown!"), path, host);
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
			g_free (host);
			g_free (path);
			break;
		}

		tmp = g_strconcat (path, "/name", NULL);
		name = gconf_client_get_string (client, tmp, NULL);
		g_free (tmp);

		if (!strcmp (name, host)) {
			Camera*	camera;
			gchar* 	model;
			gchar* 	port;
			
			g_free (host);

			tmp = g_strconcat (path, "/model", NULL);
			model = g_strdup (gconf_client_get_string (client, tmp, NULL));
			g_free (tmp);

			tmp = g_strconcat (path, "/port", NULL);
			port = g_strdup (gconf_client_get_string (client, tmp, NULL));
			g_free (tmp);
			
			g_free (path);

			/* Create the camera */
			if ((result = gp_camera_new (&camera)) != GP_OK) {
				g_warning (_("Could not create camera! (%s)"), gp_result_as_string (result));
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
			} else {
				strcpy (camera->model, model);

				/* Search for port */
				for (i = 0; i < gp_port_count_get (); i++) {
					if ((result = gp_port_info_get (i, camera->port)) != GP_OK) {
						g_warning (_("Could not get port info for port %i! (%s)\n"), i, gp_result_as_string (result));
						continue;
					}
					if (!strcmp (camera->port->name, port)) break;
				}
				if ((i == gp_port_count_get ()) || (i < 0)) {
					g_warning (_("Port '%s' not found!"), port);
					CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
				} else {
					camera->port->speed = 0;

					/* Connect to the camera */
					if ((result = gp_camera_init (camera)) != GP_OK) {
						g_warning (_("Could not initialize camera! (%s)"), gp_result_as_string (result));
						CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
					}
				}
			}

			g_free (port);
			g_free (model);
			
			return (camera);
		}
		g_free (host);
		g_free (path);
	}

	return (NULL);
}

