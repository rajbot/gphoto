#include <config.h>
#include <gphoto2.h>
#include "gnocam-main.h"

#include <gconf/gconf-client.h>

#include "gnocam-camera.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass *parent_class;

struct _GnoCamMainPrivate
{
	GConfClient *client;
	GHashTable *hash_table;
};

static GNOME_Camera
impl_GNOME_GnoCam_getCamera (PortableServer_Servant servant, 
			     const CORBA_char *name,
			     CORBA_Environment *ev)
{
	GnoCamMain *gnocam_main;
	GnoCamCamera *gnocam_camera;
	Camera *camera = NULL;
	GSList *list;
	gint i, result;
	const gchar *model = NULL, *port = NULL;

	g_message ("impl_GNOME_GnoCam_getCamera: %s", name);

	gnocam_main = GNOCAM_MAIN (bonobo_object_from_servant (servant));
	g_assert (gnocam_main);

	camera = g_hash_table_lookup (gnocam_main->priv->hash_table, name);
	if (camera) {
		gp_camera_ref (camera);
	} else {
		g_message ("Getting list of configured cameras...");
		list = gconf_client_get_list (gnocam_main->priv->client,
					      "/apps/" PACKAGE "/cameras",
					      GCONF_VALUE_STRING, NULL);
		for (i = 0; i < g_slist_length (list); i += 3) {
			if (!strcmp (g_slist_nth_data (list, i), name)) {
				model = g_slist_nth_data (list, i + 1);
				port = g_slist_nth_data (list, i + 2);
				break;
			}
		}
		g_assert (i != g_slist_length (list));

		g_message ("Creating camera...");
		result = gp_camera_new (&camera);
		g_assert (result == GP_OK);

		g_message ("Initializing camera...");
		strcpy (camera->model, model);
		strcpy (camera->port->name, port);
		camera->port->speed = 0;
		result = gp_camera_init (camera);
		g_assert (result == GP_OK);
	}
	g_assert (camera);

	g_message ("Trying to create a GnoCamCamera...");
	gnocam_camera = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gnocam_camera), ev));
}

static void
gnocam_main_destroy (GtkObject *object)
{
	GnoCamMain *gnocam_main;

	gnocam_main = GNOCAM_MAIN (object);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
foreach_func (gpointer key, gpointer value, gpointer user_data)
{
	Camera *camera;

	camera = value;
	gp_camera_unref (camera);
}

static void
gnocam_main_finalize (GtkObject *object)
{
	GnoCamMain *gnocam_main;

	gnocam_main = GNOCAM_MAIN (object);

	g_hash_table_foreach (gnocam_main->priv->hash_table,
			      foreach_func, NULL);
	g_hash_table_destroy (gnocam_main->priv->hash_table);

	g_free (gnocam_main->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_main_class_init (GnoCamMainClass *klass)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);
	POA_GNOME_GnoCam__epv *epv;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = gnocam_main_destroy;
	object_class->finalize = gnocam_main_finalize;

	epv = &klass->epv;
	epv->getCamera = impl_GNOME_GnoCam_getCamera;
}

static void
gnocam_main_init (GnoCamMain *gnocam_main)
{
	/* Make sure GConf is initialized. */
	if (!gconf_is_initialized ()) {
		GError* gerror = NULL;
		gchar*  argv[1] = {"Whatever"};

		g_assert (gconf_init (1, argv, &gerror));
	}

	/* Make sure GPhoto is initialized. */
	if (!gp_is_initialized ())
		g_assert (gp_init (GP_DEBUG_NONE) == GP_OK);
	
	gnocam_main->priv = g_new0 (GnoCamMainPrivate, 1);
	gnocam_main->priv->hash_table = g_hash_table_new (g_str_hash,
							  g_str_equal);
	gnocam_main->priv->client = gconf_client_get_default ();
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamMain, GNOME_GnoCam, PARENT_TYPE, gnocam_main);

GnoCamMain *
gnocam_main_new (void)
{
	GnoCamMain *gnocam_main;

	g_message ("Creating a GnoCamMain...");

	gnocam_main = gtk_type_new (GNOCAM_TYPE_MAIN);

	return (gnocam_main);
}

