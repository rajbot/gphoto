#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-control.h"

#include <gal/util/e-util.h>
#include <gal/widgets/e-scroll-frame.h>
#include <gal/e-paned/e-hpaned.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <gphoto-extensions.h>

#include "e-title-bar.h"
#include "e-shell-folder-title-bar.h"
#include "utils.h"
#include "gnocam-storage-view.h"
#include "gnocam-control-folder.h"
#include "gnocam-control-file.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_parent_class = NULL;

struct _GnoCamControlPrivate
{
	Bonobo_Storage		storage;
        Camera*                 camera;

        CameraWidget*           configuration;

	guint			hpaned_position;

	GtkWidget*		title_bar;
	GtkWidget*		hpaned;
	GtkWidget*		storage_view_vbox;
	GtkWidget*		storage_view_title_bar;
};

/*************/
/* Callbacks */
/*************/

static void
on_title_bar_toggled (GnoCamControl* control, gboolean state, void* data)
{
	if (!state) {
		return;
	}

	gtk_widget_show (control->priv->storage_view_vbox);
	e_paned_set_position (E_PANED (control->priv->hpaned), control->priv->hpaned_position);
}

static void
on_storage_view_button_clicked (ETitleBar* title_bar, void* data)
{
	GnoCamControl*	control;

	control = GNOCAM_CONTROL (data);
	
	gnocam_control_set_storage_view_mode (control, GNOCAM_CONTROL_STORAGE_VIEW_MODE_HIDDEN);
}

/*****************/
/* Our functions */
/*****************/

Camera*
gnocam_control_get_camera (GnoCamControl* control)
{
	return (control->priv->camera);
}

void
gnocam_control_set_storage_view_mode (GnoCamControl* control, GnoCamControlStorageViewMode mode)
{
	if (mode == GNOCAM_CONTROL_STORAGE_VIEW_MODE_STICKY) {
		if (!GTK_WIDGET_VISIBLE (control->priv->storage_view_vbox)) {
			gtk_widget_show (control->priv->storage_view_vbox);
			e_paned_set_position (E_PANED (control->priv->hpaned), control->priv->hpaned_position);
		}
		e_title_bar_set_button_mode (E_TITLE_BAR (control->priv->storage_view_title_bar), E_TITLE_BAR_BUTTON_MODE_CLOSE);
		e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (control->priv->title_bar), FALSE);
	} else {
		if (GTK_WIDGET_VISIBLE (control->priv->storage_view_vbox)) {
	                gtk_widget_hide (control->priv->storage_view_vbox);
        	        control->priv->hpaned_position =  E_PANED (control->priv->hpaned)->child1_size;
                	e_paned_set_position (E_PANED (control->priv->hpaned), 0);
		}
		
                e_title_bar_set_button_mode (E_TITLE_BAR (control->priv->storage_view_title_bar), E_TITLE_BAR_BUTTON_MODE_PIN);
                e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (control->priv->title_bar), TRUE);
	}
}

/************************/
/* Bonobo-Control stuff */
/************************/

static void
activate (BonoboControl* object, gboolean state)
{
	BonoboUIComponent*	component;
	Bonobo_UIContainer	container;
	GnoCamControl*		control;
	
	control = GNOCAM_CONTROL (object);
	container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
	component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));
	bonobo_ui_component_set_container (component, container);
	
        if (state) {
		if (control->priv->camera->abilities->config) {
			CameraWidget*	widget = NULL;
                        gint 		result;

			result = gp_camera_config_get (control->priv->camera, &widget);
			if (result == GP_OK) menu_setup (object, control->priv->camera, widget, "Camera Configuration", NULL, NULL);
		}
	} else {
		bonobo_ui_component_unset_container (component);
	}

	bonobo_object_release_unref (container, NULL);

	if (gnocam_control_parent_class->activate) gnocam_control_parent_class->activate (object, state);
}

GnoCamControl*
gnocam_control_new (BonoboMoniker* moniker, CORBA_Environment* ev)
{
	GnoCamControl*	new;
	gchar*		tmp;
	gchar*		name;
	gint		i;
	Camera*		camera;
	Bonobo_Storage	storage;
	GtkWidget*	vbox;
	GtkWidget*	scroll_frame;
	GtkWidget*	storage_view;

	name = (gchar*) bonobo_moniker_get_name (moniker);

	/* Try to get a camera */
	CHECK_RESULT (gp_camera_new_from_gconf (&camera, name), ev);
	if (BONOBO_EX (ev)) return (NULL);
	
	/* Try to get a storage for the root folder */
	name += 2;
	for (i = 0; name [i] != 0; i++) if (name [i] == '/') break;
	name = g_strndup (name, i);
	tmp = g_strconcat (bonobo_moniker_get_prefix (moniker), "//", name, "/", NULL);
	storage = bonobo_get_object (tmp, "IDL:Bonobo/Storage:1.0", ev);
	g_free (tmp);
	g_free (name);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (NULL);
	}
	g_return_val_if_fail (storage != CORBA_OBJECT_NIL, NULL);

	/* Create the control */
	new = gtk_type_new (gnocam_control_get_type ());
	new->priv->storage = storage;
	new->priv->camera = camera;
	new->priv->configuration = NULL;
	gp_camera_ref (camera);

	/* Create the basic layout */
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 0));
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	bonobo_control_construct (BONOBO_CONTROL (new), vbox);

	/* Create the title bar */
	gtk_widget_show (new->priv->title_bar = e_shell_folder_title_bar_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->title_bar), "title_toggled", GTK_SIGNAL_FUNC (on_title_bar_toggled), new);
	gtk_box_pack_start (GTK_BOX (vbox), new->priv->title_bar, FALSE, FALSE, 0);

	/* Create the paned */
	gtk_widget_show (new->priv->hpaned = e_hpaned_new ());
	gtk_box_pack_start (GTK_BOX (vbox), new->priv->hpaned, TRUE, TRUE, 2); 
	new->priv->hpaned_position = E_PANED (new->priv->hpaned)->child1_size;

	/* Create another vbox for the storage view */
	gtk_widget_show (new->priv->storage_view_vbox = gtk_vbox_new (FALSE, 0));
	e_paned_pack1 (E_PANED (new->priv->hpaned), new->priv->storage_view_vbox, FALSE, FALSE);
	
	/* Create the title for the storage view */
	gtk_widget_show (new->priv->storage_view_title_bar = e_title_bar_new (_("Folders")));
	gtk_box_pack_start (GTK_BOX (vbox), new->priv->storage_view_title_bar, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view_title_bar), "button_clicked", GTK_SIGNAL_FUNC (on_storage_view_button_clicked), new);

	/* Create the scroll frame */
	gtk_widget_show (scroll_frame = e_scroll_frame_new (NULL, NULL));
	e_scroll_frame_set_policy (E_SCROLL_FRAME (scroll_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scroll_frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX (vbox), scroll_frame, TRUE, TRUE, 0);

	/* Create the folder tree */
	gtk_widget_show (storage_view = gnocam_storage_view_new ());
	gtk_container_add (GTK_CONTAINER (scroll_frame), storage_view);

	/* Select the selected file/folder */
	name = (gchar*) bonobo_moniker_get_name (moniker);
	name += 2;
	for (; *name != 0; name++) if (*name == '/') break;
	gnocam_storage_view_set (name);

	/* Set default settings */
	gnocam_control_set_storage_view_mode (new, GNOCAM_CONTROL_STORAGE_VIEW_MODE_STICKY);

	return (new);
}

static void
destroy (GtkObject* object)
{
	GnoCamControl*  control;
	
	control = GNOCAM_CONTROL (object);

	if (control->priv->configuration) gp_widget_unref (control->priv->configuration);
	if (control->priv->camera) gp_camera_unref (control->priv->camera);

	g_free (control->priv);
}

static void
init (GnoCamControl* control)
{
	GnoCamControlPrivate*	priv;

	priv = g_new (GnoCamControlPrivate, 1);
	control->priv = priv;
}

static void
class_init (GnoCamControlClass* klass)
{
        GtkObjectClass* object_class;
        BonoboControlClass* control_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = destroy;

	control_class = BONOBO_CONTROL_CLASS (klass);
	control_class->activate = activate;
	
        gnocam_control_parent_class = gtk_type_class (bonobo_control_get_type ());
}

E_MAKE_TYPE (gnocam_control, "GnoCamControl", GnoCamControl, class_init, init, PARENT_TYPE)


