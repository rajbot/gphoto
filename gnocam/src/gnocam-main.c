
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-main.h"

#include <gal/util/e-util.h>
#include <gal/e-paned/e-hpaned.h>

#include "gnocam-preferences.h"
#include "gnocam-shortcut-bar.h"
#include "gnocam-camera.h"

#define PARENT_TYPE BONOBO_TYPE_WINDOW
static BonoboWindowClass* parent_class = NULL;

struct _GnoCamMainPrivate {
	Bonobo_UIContainer	container;

	GtkWidget*		hpaned;
};

#define GNOCAM_MAIN_UI														\
"<Root>"															\
"  <menu>"															\
"    <submenu name=\"File\" _label=\"_File\">"											\
"      <placeholder name=\"FileOperations\"/>"											\
"      <menuitem name=\"Exit\" verb=\"\" _label=\"E_xit\" pixtype=\"stock\" pixname=\"Quit\"/>"					\
"    </submenu>"														\
"    <placeholder name=\"Folder\"/>"												\
"    <placeholder name=\"Camera\"/>"												\
"    <placeholder name=\"Edit\"/>"												\
"    <placeholder name=\"View\"/>"												\
"    <submenu name=\"Settings\" _label=\"_Settings\">"										\
"      <menuitem name=\"Preferences\" verb=\"\" _label=\"_Preferences\" pixtype=\"stock\" pixname=\"Preferences\"/>"		\
"    </submenu>"														\
"    <submenu name=\"Help\" _label=\"_Help\">"											\
"      <menuitem name=\"About\" verb=\"\" _label=\"_About\" pixtype=\"stock\" pixname=\"About\"/>"				\
"    </submenu>"														\
"  </menu>"															\
"  <status>"															\
"    <item name=\"main\"/>"													\
"  </status>"															\
"</Root>"

/*************/
/* Callbacks */
/*************/

static void
on_shortcut_bar_item_selected (EShortcutBar* shortcut_bar, GdkEvent* event, gint group_num, gint item_num, gpointer user_data)
{
	GnoCamMain*	m;
	GnoCamCamera*	camera;
        gchar*          url;
        gchar*          name;
        GtkWidget*      widget;

	g_return_if_fail (m = GNOCAM_MAIN (user_data));

        /* If there is an old viewer, destroy it. */
        if (E_PANED (m->priv->hpaned)->child2) gtk_container_remove (GTK_CONTAINER (m->priv->hpaned), E_PANED (m->priv->hpaned)->child2);

        /* Get information about the item. */
        e_shortcut_model_get_item_info (shortcut_bar->model, group_num, item_num, &url, &name);

	camera = gnocam_camera_new (url, m->priv->container);
	g_return_if_fail (camera);

	gtk_widget_show (widget = gnocam_camera_get_widget (camera));
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
gnocam_main_new (void)
{
	GtkWidget*		widget;
	GnoCamMain*		new;
	BonoboUIContainer*	container;
	BonoboUIComponent*	component;
	BonoboUIVerb            verb [] = {
		BONOBO_UI_UNSAFE_VERB ("Exit", gtk_main_quit),
		BONOBO_UI_VERB ("Preferences", on_preferences_activate),
		BONOBO_UI_VERB ("About", on_about_activate),
		BONOBO_UI_VERB_END};
										

	new = gtk_type_new (gnocam_main_get_type ());
	new = GNOCAM_MAIN (bonobo_window_construct (BONOBO_WINDOW (new), "GnoCamMain", "GnoCam"));

	/* Create the hpaned */
        gtk_widget_show (new->priv->hpaned = e_hpaned_new ());
        bonobo_window_set_contents (BONOBO_WINDOW (new), new->priv->hpaned);

        /* Create the shortcut bar. */
	gtk_widget_show (widget = gnocam_shortcut_bar_new ());
        e_paned_add1 (E_PANED (new->priv->hpaned), widget);
        gtk_signal_connect (GTK_OBJECT (widget), "item_selected", (GtkSignalFunc) on_shortcut_bar_item_selected, new);
	
	/* Create the container */
	container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (new));
	new->priv->container = bonobo_object_corba_objref (BONOBO_OBJECT (container));

	/* Create the menu */
	component = bonobo_ui_component_new ("main");
	bonobo_ui_component_set_container (component, new->priv->container);
	bonobo_ui_component_set_translate (component, "/", GNOCAM_MAIN_UI, NULL);
	bonobo_ui_component_add_verb_list_with_data (component, verb, new);

	gtk_window_set_default_size (GTK_WINDOW (new), 550, 550);

	return (new);
}

E_MAKE_TYPE (gnocam_main, "GnoCamMain", GnoCamMain, gnocam_main_class_init, gnocam_main_init, PARENT_TYPE)

