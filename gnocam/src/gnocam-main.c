
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-main.h"

#include <gal/util/e-util.h>
#include <gal/e-paned/e-hpaned.h>

#include "e-clipped-label.h"
#include "gnocam-preferences.h"
#include "gnocam-shortcut-bar.h"
#include "gnocam-camera.h"

#define PARENT_TYPE BONOBO_TYPE_WINDOW
static BonoboWindowClass* parent_class = NULL;

struct _GnoCamMainPrivate {
	BonoboUIContainer*	container;
	BonoboUIComponent*	component;

	GConfClient*		client;

	GtkWidget*		hpaned;
};

#define GNOCAM_MAIN_UI														\
"<Root>\n"															\
"  <menu>\n"															\
"    <submenu name=\"File\" _label=\"_File\">\n"										\
"      <placeholder name=\"FileOperations\"/>\n"										\
"      <menuitem name=\"Exit\" verb=\"\" _label=\"E_xit\" pixtype=\"stock\" pixname=\"Quit\"/>\n"				\
"    </submenu>\n"														\
"    <placeholder name=\"Folder\"/>\n"                                                                                          \
"    <placeholder name=\"Camera\"/>\n"                                                                                          \
"    <submenu name=\"Edit\" _label=\"_Edit\">\n"										\
"       <placeholder/>\n"													\
"       <menuitem name=\"BonoboCustomize\" verb=\"\" _label=\"Customi_ze...\" pos=\"bottom\"/>\n"				\
"    </submenu>\n"														\
"    <placeholder name=\"Edit\"/>\n"												\
"    <placeholder name=\"View\"/>\n"												\
"    <submenu name=\"Settings\" _label=\"_Settings\">\n"									\
"      <menuitem name=\"Preferences\" verb=\"\" _label=\"_Preferences\" pixtype=\"stock\" pixname=\"Preferences\"/>"		\
"    </submenu>"														\
"    <submenu name=\"Help\" _label=\"_Help\">\n"										\
"      <menuitem name=\"About\" verb=\"\" _label=\"_About\" pixtype=\"stock\" pixname=\"About\"/>\n"				\
"    </submenu>\n"														\
"  </menu>\n"															\
"  <status>\n"															\
"    <item name=\"main\"/>\n"													\
"  </status>\n"															\
"</Root>"

/*************/
/* Callbacks */
/*************/

static void
on_window_size_request (GtkWidget* widget, GtkRequisition* requisition, gpointer user_data)
{
	GnoCamMain*	m;

	m = GNOCAM_MAIN (user_data);

	gconf_client_set_int (m->priv->client, "/apps/" PACKAGE "/width_main", widget->allocation.width, NULL);
	gconf_client_set_int (m->priv->client, "/apps/" PACKAGE "/height_main", widget->allocation.height, NULL);
}

static void
on_size_request (GtkWidget* widget, GtkRequisition* requisition, gpointer user_data)
{
        GnoCamMain*	m;

        m = GNOCAM_MAIN (user_data);

        gconf_client_set_int (m->priv->client, "/apps/" PACKAGE "/hpaned_position_main", e_paned_get_position (E_PANED (widget)), NULL);
}

static void
on_shortcut_bar_item_selected (EShortcutBar* shortcut_bar, GdkEvent* event, gint group_num, gint item_num, gpointer user_data)
{
	GnoCamMain*		m;
	GnoCamCamera*		camera;
        gchar*          	url;
        gchar*          	name;
        GtkWidget*      	widget;
	CORBA_Environment	ev;

	g_return_if_fail (m = GNOCAM_MAIN (user_data));

        /* Get information about the item. */
        e_shortcut_model_get_item_info (shortcut_bar->model, group_num, item_num, &url, &name);
	g_free (name);

	CORBA_exception_init (&ev);
	camera = gnocam_camera_new (url, m->priv->container, GTK_WINDOW (m), m->priv->client, &ev);
	if (BONOBO_EX (&ev)) {
		gchar*	message;

		message = g_strdup_printf (_("Could not display camera widget for camera '%s'!\n(%s)"), url, bonobo_exception_get_text (&ev));
		gnome_error_dialog_parented (message, GTK_WINDOW (m));
		g_free (message);
		g_free (url);
		CORBA_exception_free (&ev);
		return;
	}
	g_free (url);
	CORBA_exception_free (&ev);
	g_return_if_fail (camera);

	gtk_widget_show (widget = gnocam_camera_get_widget (camera));
	gtk_container_remove (GTK_CONTAINER (m->priv->hpaned), E_PANED (m->priv->hpaned)->child2);
	e_paned_pack2 (E_PANED (m->priv->hpaned), widget, TRUE, TRUE);
}

static void
on_about_activate (BonoboUIComponent* component, gpointer user_data, const gchar* path)
{
	const gchar*	authors[] = {"Lutz Müller <urc8@rz.uni-karlsruhe.de>", NULL};
	const gchar*	comments = PACKAGE ", a small but powerfull front-end to GPhoto2 for the GNOME desktop.";
	const gchar*	copyright = NULL;
	
	gtk_widget_show (gnome_about_new (PACKAGE, VERSION, copyright, authors, comments, NULL));
}

static void
on_preferences_activate (BonoboUIComponent* component, gpointer user_data, const gchar* path)
{
	GnoCamMain*	m;

	m = GNOCAM_MAIN (user_data);
	
	gtk_widget_show (gnocam_preferences_new (GTK_WINDOW (m)));
}

/***********************/
/* Bonobo-Window stuff */
/***********************/

static void
gnocam_main_destroy (GtkObject* object)
{
	GnoCamMain*	m;

	m = GNOCAM_MAIN (object);

	gtk_object_unref (GTK_OBJECT (m->priv->client));
	bonobo_object_unref (BONOBO_OBJECT (m->priv->component));
	g_free (m->priv);

	(*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnocam_main_class_init (GnoCamMainClass* klass)
{
	GtkObjectClass* object_class;
	
	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_main_destroy;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gnocam_main_init (GnoCamMain* m)
{
	m->priv = g_new (GnoCamMainPrivate, 1);
}

GnoCamMain*
gnocam_main_new (GConfClient* client)
{
	GtkWidget*		widget;
	GnoCamMain*		new;
	BonoboUIVerb            verb [] = {
		BONOBO_UI_UNSAFE_VERB ("Exit", gtk_main_quit),
		BONOBO_UI_VERB ("Preferences", on_preferences_activate),
		BONOBO_UI_VERB ("About", on_about_activate),
		BONOBO_UI_VERB_END};
	gint			position, w, h;

	new = gtk_type_new (GNOCAM_TYPE_MAIN);
	new = GNOCAM_MAIN (bonobo_window_construct (BONOBO_WINDOW (new), "GnoCamMain", "GnoCam"));
	gtk_signal_connect (GTK_OBJECT (new), "size_request", GTK_SIGNAL_FUNC (on_window_size_request), new);
	gtk_signal_connect (GTK_OBJECT (new), "destroy", GTK_SIGNAL_FUNC (gtk_main_quit), new);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));

	/* Create the hpaned */
        gtk_widget_show (new->priv->hpaned = e_hpaned_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->hpaned), "size_request", GTK_SIGNAL_FUNC (on_size_request), new);
        bonobo_window_set_contents (BONOBO_WINDOW (new), new->priv->hpaned);

	/* Create label */
	gtk_widget_show (widget = e_clipped_label_new (_("(No camera selected)")));
	e_paned_add2 (E_PANED (new->priv->hpaned), widget);

        /* Create the shortcut bar. */
	gtk_widget_show (widget = gnocam_shortcut_bar_new ());
        e_paned_add1 (E_PANED (new->priv->hpaned), widget);
        gtk_signal_connect (GTK_OBJECT (widget), "item_selected", (GtkSignalFunc) on_shortcut_bar_item_selected, new);
	
	/* Create the container */
	new->priv->container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (new->priv->container, BONOBO_WINDOW (new));
//	bonobo_object_unref (BONOBO_OBJECT (new->priv->container));

	/* Create the menu */
	new->priv->component = bonobo_ui_component_new (PACKAGE "main");
	bonobo_ui_component_set_container (new->priv->component, BONOBO_OBJREF (new->priv->container));
	bonobo_ui_component_freeze (new->priv->component, NULL);
	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (BONOBO_WINDOW (new)), "/" PACKAGE "/UIConf/main");
	bonobo_ui_component_set_translate (new->priv->component, "/", GNOCAM_MAIN_UI, NULL);
	bonobo_ui_component_add_verb_list_with_data (new->priv->component, verb, new);
	bonobo_ui_component_thaw (new->priv->component, NULL);

	/* Set the default settings */
	w = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/width_main", NULL);
	h = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/height_main", NULL);
	if (w + h > 0) gtk_window_set_default_size (GTK_WINDOW (new), w, h);
	position = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/hpaned_position_main", NULL);
	if (position) e_paned_set_position (E_PANED (new->priv->hpaned), position);

	return (new);
}

E_MAKE_TYPE (gnocam_main, "GnoCamMain", GnoCamMain, gnocam_main_class_init, gnocam_main_init, PARENT_TYPE)

