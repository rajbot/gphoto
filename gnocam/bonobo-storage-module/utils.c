
#include <gnome.h>
#include <bonobo.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <parser.h>
#include "utils.h"

Camera*
util_camera_new (GnomeVFSURI* uri, CORBA_Environment* ev) 
{
        GConfValue*	value;
        gchar*		xml;
        gchar*		host;
        gchar*		id;
        gchar*		name;
        gchar*		model = NULL;
        gchar*		port = NULL;
        gchar*		speed = NULL;
        guint i;
        xmlDocPtr 	doc;
        xmlNodePtr 	node;
        Camera*		camera;
        GSList* 	list;

        /* Does GConf know about the camera (host)? */
        value = gconf_client_get (gconf_client_get_default (), "/apps/GnoCam/cameras", NULL);
        if (!value) {
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
                return NULL;
        }
        g_assert (value->type == GCONF_VALUE_LIST);
        g_assert (gconf_value_get_list_type (value) == GCONF_VALUE_STRING);
        g_assert (list = gconf_value_get_list (value));
        g_assert (host = gnome_vfs_unescape_string (gnome_vfs_uri_get_host_name (uri), NULL));
        for (i = 0; i < g_slist_length (list); i++) {
                value = g_slist_nth_data (list, i);
                g_assert (value->type == GCONF_VALUE_STRING);
                g_assert ((xml = g_strdup (gconf_value_get_string (value))));
                if (!(doc = xmlParseMemory (xml, strlen (xml)))) continue;
                g_assert (node = xmlDocGetRootElement (doc));
                g_assert (id = xmlGetProp (node, "ID"));
                g_assert (name = xmlGetProp (node, "Name"));
                g_assert (model = xmlGetProp (node, "Model"));
                g_assert (port = xmlGetProp (node, "Port"));
                g_assert (speed = xmlGetProp (node, "Speed"));
                if (!strcmp (name, host)) break;
        }
        g_free (host);
        if (i == g_slist_length (list)) {
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
                return NULL;
        }

	/* Create the camera. */
	if (gp_camera_new_by_name (&camera, model) != GP_OK) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
		return NULL;
	}

        /* Make ready for connection. Beware of 'Directory Browse'.*/
        if (!strcmp ("Directory Browse", model)) strcpy (camera->port->path, "");
        else {
                for (i = 0; i < gp_port_count (); i++) {
                        if (gp_port_info (i, camera->port) != GP_OK) continue;
                        if (!strcmp (camera->port->name, port)) break;
                }
                if ((i == gp_port_count ()) || (i < 0)) {
                        CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
                        return NULL;
                }
                camera->port->speed = atoi (speed);
        }

        /* Connect to the camera. */
        if (gp_camera_init (camera) != GP_OK) {
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
                return NULL;
        }

	return camera;
}

