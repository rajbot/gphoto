#include <config.h>
#include <gphoto2.h>
#include "gnocam-main.h"

#include <gtk/gtkmain.h>
#include <gconf/gconf-client.h>
#include <bonobo/bonobo-exception.h>
#include <libgnomeui/gnome-dialog.h>

#include "gnocam-util.h"
#include "gnocam-camera.h"
#include "gnocam-camera-selector.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass *parent_class;

struct _GnoCamMainPrivate
{
	GConfClient *client;

	GHashTable *hash_table;
};

static void
initialize_camera (GSList *list, Camera *camera, const gchar *name,
		   CORBA_Environment *ev)
{
	gint i;

	g_message ("Looking for %s...", name);
	for (i = 0; i < g_slist_length (list); i += 3)
		if (!strcmp (g_slist_nth_data (list, i), name))
			break;
	if (i >= g_slist_length (list)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_GnoCam_NotFound, NULL);
		return;
	}

	g_message ("Trying to initialize %s...", name);
	strcpy (camera->model, g_slist_nth_data (list, i + 1));
	strcpy (camera->port->name, g_slist_nth_data (list, i + 2));
	CHECK_RESULT (gp_camera_init (camera), ev);
}

static GNOME_Camera
impl_GNOME_GnoCam_getCamera (PortableServer_Servant servant,
			     CORBA_Environment *ev)
{
	GnoCamMain *gnocam_main;
	GnoCamCamera *gnocam_camera;
	Camera *camera = NULL;
	GSList *list;

	gnocam_main = GNOCAM_MAIN (bonobo_object_from_servant (servant));
	CHECK_RESULT (gp_camera_new (&camera), ev);

	list = gconf_client_get_list (gnocam_main->priv->client,
			              "/apps/" PACKAGE "/cameras",
				      GCONF_VALUE_STRING, NULL);

	/* Auto-detection? */
	if (gconf_client_get_bool (gnocam_main->priv->client,
				   "/apps/" PACKAGE "/autodetect", NULL)) {
		CHECK_RESULT (gp_camera_init (camera), ev);
		if (BONOBO_EX (ev)) {
			gp_camera_unref (camera);
			return (CORBA_OBJECT_NIL);
		}
	} else {

		/* Only one camera? */
		if (g_slist_length (list) == 3) {
			strcpy (camera->model, g_slist_nth_data (list, 1));
			strcpy (camera->port->name, g_slist_nth_data (list, 2));
			CHECK_RESULT (gp_camera_init (camera), ev);
			if (BONOBO_EX (ev)) {
				gp_camera_unref (camera);
				return (CORBA_OBJECT_NIL);
			}
		} else {

			/* Ask! */
			GnomeDialog *selector;
			const gchar *name;
			gint button;
	
			selector = gnocam_camera_selector_new ();
			do {
				button = gnome_dialog_run (selector);
				if (button == 1) {
					/* Cancel */
					gnome_dialog_close (selector);
					CORBA_exception_set (ev,
						CORBA_USER_EXCEPTION,
						ex_GNOME_Cancelled, NULL);
					gp_camera_unref (camera);
					return (CORBA_OBJECT_NIL);
				} else if (button == 0) {
					break;
				} else {
					g_warning ("Unhandled button: %i",
						   button);
				}
			} while (TRUE);
	
			name = gnocam_camera_selector_get_name (
					GNOCAM_CAMERA_SELECTOR (selector));
			gnome_dialog_close (selector);
			while (gtk_events_pending ())
				gtk_main_iteration ();

			/* It can well be that the user clicked autodetect */
			if (gconf_client_get_bool (gnocam_main->priv->client,
						"/apps/" PACKAGE "/autodetect",
						NULL)) {
				CHECK_RESULT (gp_camera_init (camera), ev);
				if (BONOBO_EX (ev)) {
					gp_camera_unref (camera);
					return (CORBA_OBJECT_NIL);
				}
			} else if (!name) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
						     ex_GNOME_GnoCam_NotFound,
						     NULL);
				gp_camera_unref (camera);
				return (CORBA_OBJECT_NIL);
			} else {
				initialize_camera (list, camera, name, ev);
				if (BONOBO_EX (ev)) {
					gp_camera_unref (camera);
					return (CORBA_OBJECT_NIL);
				}
			}
		}
	}

	gnocam_camera = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);
	
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gnocam_camera), ev));
}

static GNOME_Camera
impl_GNOME_GnoCam_getCameraByName (PortableServer_Servant servant, 
			           const CORBA_char *name,
			           CORBA_Environment *ev)
{
	GnoCamMain *gnocam_main;
	GnoCamCamera *gnocam_camera;
	Camera *camera = NULL;
	GSList *list;

	g_message ("impl_GNOME_GnoCam_getCamera: %s", name);

	gnocam_main = GNOCAM_MAIN (bonobo_object_from_servant (servant));
	list = gconf_client_get_list (gnocam_main->priv->client,
				      "/apps/" PACKAGE "/cameras",
				      GCONF_VALUE_STRING, NULL);

	if (!*name)
		return GNOME_GnoCam_getCamera (BONOBO_OBJREF (gnocam_main), ev);

	camera = g_hash_table_lookup (gnocam_main->priv->hash_table, name);
	if (camera)
		gp_camera_ref (camera);
	else {
		CHECK_RESULT (gp_camera_new (&camera), ev);
		if (BONOBO_EX (ev))
			return (CORBA_OBJECT_NIL);
		initialize_camera (list, camera, name, ev);
		if (BONOBO_EX (ev)) {
			gp_camera_unref (camera);
			return (CORBA_OBJECT_NIL);
		}
	}

	gnocam_camera = gnocam_camera_new (camera, ev);
	gp_camera_unref (camera);
	if (BONOBO_EX (ev))
		return (CORBA_OBJECT_NIL);

	return (CORBA_Object_duplicate (BONOBO_OBJREF (gnocam_camera), ev));
}

static gboolean
foreach_func (gpointer key, gpointer value, gpointer user_data)
{
	Camera *camera;

	camera = value;
	gp_camera_unref (camera);

	return (TRUE);
}

static void
gnocam_main_destroy (GtkObject *object)
{
	GnoCamMain *gnocam_main;

	gnocam_main = GNOCAM_MAIN (object);

	g_message ("Destroying GnoCamMain...");

	if (gnocam_main->priv->hash_table) {
		g_hash_table_foreach_remove (gnocam_main->priv->hash_table,
					     foreach_func, NULL);
		g_hash_table_destroy (gnocam_main->priv->hash_table);
		gnocam_main->priv->hash_table = NULL;
	}

	if (gnocam_main->priv->client) {
		gtk_object_unref (GTK_OBJECT (gnocam_main->priv->client));
		gnocam_main->priv->client = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_main_finalize (GtkObject *object)
{
	GnoCamMain *gnocam_main;

	gnocam_main = GNOCAM_MAIN (object);

	g_free (gnocam_main->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_main_class_init (GnoCamMainClass *klass)
{
	GtkObjectClass *object_class;
	POA_GNOME_GnoCam__epv *epv; 
	
	/* Make sure GConf is initialized. */
	if (!gconf_is_initialized ()) {
		GError* gerror = NULL;
		gchar*  argv[1] = {"Whatever"};

		g_message ("Initializing GConf...");
		g_assert (gconf_init (1, argv, &gerror));
	}

	/* Make sure GPhoto is initialized. */
	if (!gp_is_initialized ()) {
		g_message ("Initializing GPhoto...");
		g_assert (gp_init (GP_DEBUG_NONE) == GP_OK);
	}

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = gnocam_main_destroy;
	object_class->finalize = gnocam_main_finalize;

	epv = &klass->epv;
	epv->getCamera = impl_GNOME_GnoCam_getCamera;
	epv->getCameraByName = impl_GNOME_GnoCam_getCameraByName;
}

static void
gnocam_main_init (GnoCamMain *gnocam_main)
{
	gnocam_main->priv = g_new0 (GnoCamMainPrivate, 1);
	gnocam_main->priv->hash_table = g_hash_table_new (g_str_hash,
							  g_str_equal);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamMain, GNOME_GnoCam, PARENT_TYPE, gnocam_main);

GnoCamMain *
gnocam_main_new (void)
{
	GnoCamMain *gnocam_main;

	g_message ("Creating a GnoCamMain...");

	gnocam_main = gtk_type_new (GNOCAM_TYPE_MAIN);

	gnocam_main->priv->client = gconf_client_get_default ();

	return (gnocam_main);
}

