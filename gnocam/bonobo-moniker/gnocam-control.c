/* gnocam-control.c
 * 
 * Adapted from:
 * e-shell-view.c
 *
 * Copyright (C) 2000, 2001 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Ettore Perazzoli <ettore@helixcode.com>
 *   Miguel de Icaza <miguel@helixcode.com>
 *   Matt Loper <matt@helixcode.com>
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnocam-control.h"

#include <gal/util/e-util.h>
#include <gal/widgets/e-scroll-frame.h>
#include <gal/e-paned/e-hpaned.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <gphoto-extensions.h>

#include "e-clipped-label.h"
#include "e-title-bar.h"
#include "e-shell-folder-title-bar.h"
#include "utils.h"
#include "gnocam-capture.h"
#include "gnocam-storage-view.h"
#include "gnocam-folder.h"
#include "gnocam-file.h"

#define PARENT_TYPE bonobo_control_get_type ()
static BonoboControlClass* gnocam_control_parent_class = NULL;

struct _GnoCamControlPrivate
{
	Bonobo_Storage			storage;
        Camera*                 	camera;

        CameraWidget*           	configuration;
	
	guint				hpaned_position;
	GtkWidget*			title_bar;
	GtkWidget*			hpaned;
	GtkWidget*			notebook;
	GtkWidget*			storage_view_vbox;
	GtkWidget*			storage_view_title_bar;
	GtkWidget*			storage_view;
	GnoCamControlStorageViewMode	storage_view_mode;

	GHashTable*			hash_table;

	BonoboUIComponent*		component;
};

/****************/
/* UI-Variables */
/****************/

#define GNOCAM_CONTROL_UI 						\
"<submenu name=\"Camera\" _label=\"Camera\">"				\
"  <menuitem name=\"Manual\" _label=\"Manual\" verb=\"\"/>"		\
"  <placeholder name=\"separator\"/>"					\
"  <placeholder name=\"CapturePreview\"/>" 				\
"  <placeholder name=\"CaptureImage\"/>"				\
"  <placeholder name=\"CaptureVideo\"/>"				\
"</submenu>"

#define CAPTURE_IMAGE	"<menuitem name=\"CaptureImage\" _label=\"Capture Image\" verb=\"\"/>"
#define CAPTURE_PREVIEW	"<menuitem name=\"CapturePreview\" _label=\"Capture Preview\" verb=\"\"/>"
#define CAPTURE_VIDEO	"<menuitem name=\"CaptureVideo\" _label=\"Capture Video\" verb=\"\"/>"

/**************/
/* Prototypes */
/**************/

static void 	on_title_bar_toggled 				(EShellFolderTitleBar* title_bar, gboolean state, void* data);

static void 	on_storage_view_vbox_map 			(GtkWidget* widget, void* data);
static int 	on_storage_view_vbox_button_release_event 	(GtkWidget* widget, GdkEventButton* button_event, void* data);

static void 	disconnect_popup_signals 	(GnoCamControl* control);
static void 	popdown_transient_folder_bar 	(GnoCamControl* control);

/*************/
/* Callbacks */
/*************/

static void
on_manual_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamControl* 	control;
	gint		result;
	CameraText	manual;

	control = GNOCAM_CONTROL (user_data);

        result = gp_camera_manual (control->priv->camera, &manual);
	if (result != GP_OK) {
		g_warning ("Could not get camera manual! (%s)", gp_camera_result_as_string (control->priv->camera, result));
		return;
	}
	
	gnome_ok_dialog (manual.text);
}

static void
on_capture_preview_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamControl*	control;

	control = GNOCAM_CONTROL (user_data);

	gtk_widget_show (GTK_WIDGET (gnocam_capture_new (control->priv->camera, GP_CAPTURE_PREVIEW)));
}

static void 
on_capture_image_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamControl*	control;

	control = GNOCAM_CONTROL (user_data);

	gtk_widget_show (GTK_WIDGET (gnocam_capture_new (control->priv->camera, GP_CAPTURE_IMAGE)));
}

static void
on_capture_video_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamControl*  control;

	control = GNOCAM_CONTROL (user_data);

	gtk_widget_show (GTK_WIDGET (gnocam_capture_new (control->priv->camera, GP_CAPTURE_VIDEO)));
}

static void
on_file_selected (GnoCamStorageView* storage_view, const gchar* path, void* data)
{
	GnoCamControl*		control;
	GtkWidget*		widget;
	GnoCamFile*		file;
	Bonobo_UIContainer      container;
	gint 			notebook_page;

	control = GNOCAM_CONTROL (data);

	if (control->priv->storage_view_mode == GNOCAM_CONTROL_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (control);

	file = g_hash_table_lookup (control->priv->hash_table, path);
	if (!file) {
		CORBA_Environment	ev;

		CORBA_exception_init (&ev);
		file = gnocam_file_new (control->priv->camera, control->priv->storage, g_basename (path), BONOBO_CONTROL (control), &ev);
		if (BONOBO_EX (&ev)) {
			g_warning ("Could not get control for '%s': %s", path, bonobo_exception_get_text (&ev));
			return;
		}
		CORBA_exception_free (&ev);
		g_return_if_fail (file);

		gtk_widget_show (widget = gnocam_file_get_widget (file));
		gtk_notebook_append_page (GTK_NOTEBOOK (control->priv->notebook), widget, NULL);
		g_hash_table_insert (control->priv->hash_table, g_strdup (path), file);
	} else {
		widget = gnocam_file_get_widget (file);
	}

	e_shell_folder_title_bar_set_title (E_SHELL_FOLDER_TITLE_BAR (control->priv->title_bar), path);

	/* Hide the old menu */
	if (control->priv->component) bonobo_ui_component_unset_container (control->priv->component);
	control->priv->component = gnocam_file_get_ui_component (file);

	/* Display the menu */
	container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
	gnocam_file_set_ui_container (file, container);
	bonobo_object_release_unref (container, NULL);

	notebook_page = gtk_notebook_page_num (GTK_NOTEBOOK (control->priv->notebook), widget);
	gtk_notebook_set_page (GTK_NOTEBOOK (control->priv->notebook), notebook_page);
}

static void
on_directory_selected (GnoCamStorageView* storage_view, const gchar* path, void* data)
{
	GnoCamControl*		control;
	GtkWidget*		widget;
	GnoCamFolder*		folder;
	Bonobo_UIContainer      container;
	gint			notebook_page;

	g_warning ("BEGIN: on_directory_selected");

	control = GNOCAM_CONTROL (data);

	if (control->priv->storage_view_mode == GNOCAM_CONTROL_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (control);

	folder = g_hash_table_lookup (control->priv->hash_table, path);
	if (!folder) {
		folder = gnocam_folder_new (control->priv->camera, control->priv->storage, path, BONOBO_CONTROL (control));
		g_return_if_fail (folder);

		gtk_widget_show (widget = gnocam_folder_get_widget (folder));
		gtk_notebook_append_page (GTK_NOTEBOOK (control->priv->notebook), widget, NULL);
		g_hash_table_insert (control->priv->hash_table, g_strdup (path), folder);
	} else {
		widget = gnocam_folder_get_widget (folder);
	}
	
	e_shell_folder_title_bar_set_title (E_SHELL_FOLDER_TITLE_BAR (control->priv->title_bar), path);

	/* Hide the old menu */
	if (control->priv->component) bonobo_ui_component_unset_container (control->priv->component);
	control->priv->component = gnocam_folder_get_ui_component (folder);

	/* Display the menu */
	container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
	gnocam_folder_set_ui_container (folder, container);
	bonobo_object_release_unref (container, NULL);

	notebook_page = gtk_notebook_page_num (GTK_NOTEBOOK (control->priv->notebook), widget);
	gtk_notebook_set_page (GTK_NOTEBOOK (control->priv->notebook), notebook_page);
}

static void
on_title_bar_toggled (EShellFolderTitleBar* title_bar, gboolean state, void* data)
{
	GnoCamControl*	control;

	control = GNOCAM_CONTROL (data);

	if (!state) return;

	if (control->priv->storage_view_mode != GNOCAM_CONTROL_STORAGE_VIEW_MODE_TRANSIENT) {

		control->priv->storage_view_mode = GNOCAM_CONTROL_STORAGE_VIEW_MODE_TRANSIENT;

		/* We need to show the storage view box and do a pointer grab to catch the
	           mouse clicks.  But until the box is shown, we cannot grab.  So we connect to
	           the "map" signal; `on_storage_view_vbox_map()' will do the grab.  */
	
		gtk_signal_connect (GTK_OBJECT (control->priv->storage_view_vbox), "map", GTK_SIGNAL_FUNC (on_storage_view_vbox_map), control);
		gtk_widget_show (control->priv->storage_view_vbox);
		e_paned_set_position (E_PANED (control->priv->hpaned), control->priv->hpaned_position);
	}
}

static void
on_popup_storage_view_title_bar_button_clicked (ETitleBar* title_bar, void* data)
{
	GnoCamControl*	control;

	control = GNOCAM_CONTROL (data);

        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (control->priv->storage_view_vbox);

        disconnect_popup_signals (control);

        gnocam_control_set_storage_view_mode (control, GNOCAM_CONTROL_STORAGE_VIEW_MODE_STICKY);
        e_shell_folder_title_bar_set_toggle_state (E_SHELL_FOLDER_TITLE_BAR (control->priv->title_bar), FALSE);
}

static void
on_storage_view_title_bar_button_clicked (ETitleBar* title_bar, void* data)
{
	GnoCamControl*	control;

	control = GNOCAM_CONTROL (data);

	gnocam_control_set_storage_view_mode (control, GNOCAM_CONTROL_STORAGE_VIEW_MODE_HIDDEN);
}

static void
on_storage_view_vbox_map (GtkWidget* widget, void* data)
{
	GnoCamControl* control;

	control = GNOCAM_CONTROL (data);

        if (gdk_pointer_grab (widget->window, TRUE,
                              (GDK_BUTTON_PRESS_MASK
                               | GDK_BUTTON_RELEASE_MASK
                               | GDK_ENTER_NOTIFY_MASK
                               | GDK_LEAVE_NOTIFY_MASK
                               | GDK_POINTER_MOTION_MASK),
                              NULL, NULL, GDK_CURRENT_TIME) != 0) {
                g_warning ("gnocam-control.c:on_storage_view_vbox_map() -- pointer grab failed.");
                gnocam_control_set_storage_view_mode (control, GNOCAM_CONTROL_STORAGE_VIEW_MODE_STICKY);
                return;
        }

        gtk_grab_add (widget);
        gtk_signal_connect (GTK_OBJECT (widget), "button_release_event", 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), control);
        gtk_signal_connect (GTK_OBJECT (control->priv->storage_view), "button_release_event", 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), control);
        gtk_signal_connect (GTK_OBJECT (control->priv->storage_view_title_bar), "button_clicked", 
		GTK_SIGNAL_FUNC (on_popup_storage_view_title_bar_button_clicked), control);
}

static int
on_storage_view_vbox_button_release_event (GtkWidget* widget, GdkEventButton* button_event, void* data)
{
	GnoCamControl* control;

	control = GNOCAM_CONTROL (data);

        if (button_event->window == E_PANED (control->priv->hpaned)->handle)
                return (FALSE);

        popdown_transient_folder_bar (control);

        return (TRUE);
}

/**********************/
/* Internal functions */
/**********************/

static int 
gp_frontend_status (Camera* camera, char* status)
{
	BonoboUIComponent*	component;

	component = BONOBO_UI_COMPONENT (camera->frontend_data);
        bonobo_ui_component_set_status (component, status, NULL);

        return (GP_OK);
}

static int 
gp_frontend_message (Camera* camera, char* message)
{
        gnome_ok_dialog (message);
        return (GP_OK);
}

static int 
gp_frontend_confirm (Camera* camera, char* message)
{
        GtkWidget*      widget;
        gint            result;

        widget = gnome_dialog_new (message, GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO, NULL);
        result = gnome_dialog_run_and_close (GNOME_DIALOG (widget));
        gtk_widget_destroy (widget);

        if (result == 1) return (GP_PROMPT_CANCEL);
        return (GP_PROMPT_OK);
}

static void
hash_table_forall_destroy_control (void* name, void* value, void* data)
{
        BonoboWidget* bonobo_widget;

        bonobo_widget = BONOBO_WIDGET (value);
        gtk_widget_destroy (GTK_WIDGET (bonobo_widget));

        g_free (name);
}

static void
disconnect_popup_signals (GnoCamControl* control)
{
        gtk_signal_disconnect_by_func (GTK_OBJECT (control->priv->storage_view_vbox), GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), control);
	gtk_signal_disconnect_by_func (GTK_OBJECT (control->priv->storage_view_vbox), GTK_SIGNAL_FUNC (on_storage_view_vbox_map), control);
        gtk_signal_disconnect_by_func (GTK_OBJECT (control->priv->storage_view), GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), control);
        gtk_signal_disconnect_by_func (GTK_OBJECT (control->priv->storage_view_title_bar), GTK_SIGNAL_FUNC (on_popup_storage_view_title_bar_button_clicked), control);
}

static void
popdown_transient_folder_bar (GnoCamControl* control)
{
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (control->priv->storage_view_vbox);

        gnocam_control_set_storage_view_mode (control, GNOCAM_CONTROL_STORAGE_VIEW_MODE_HIDDEN);

        disconnect_popup_signals (control);

        e_shell_folder_title_bar_set_toggle_state (E_SHELL_FOLDER_TITLE_BAR (control->priv->title_bar), FALSE);
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
	if (control->priv->storage_view_mode == mode) return;

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
	control->priv->storage_view_mode = mode;
}

/************************/
/* Bonobo-Control stuff */
/************************/

static void
activate (BonoboControl* object, gboolean state)
{
	BonoboUIComponent*	component;
	GnoCamControl*		control;

	control = GNOCAM_CONTROL (object);
	component = bonobo_control_get_ui_component (BONOBO_CONTROL (control));

        if (state) {
		Bonobo_UIContainer	container;

		container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control));
		bonobo_ui_component_set_container (component, container);
		bonobo_object_release_unref (container, NULL);

		/* Manual */
		bonobo_ui_component_set_translate (component, "/menu/Camera", GNOCAM_CONTROL_UI, NULL);
		bonobo_ui_component_add_verb (component, "Manual", on_manual_clicked, control);

                /* Camera Configuration? */
                if (control->priv->camera->abilities->config) {
                        CameraWidget*   widget = NULL;
                        gint            result;

                        result = gp_camera_config_get (control->priv->camera, &widget);
                        if (result == GP_OK) menu_setup (component, control->priv->camera, widget, "Camera Configuration", NULL, NULL);
       	        }

               	/* Capture? */
		if (control->priv->camera->abilities->capture != GP_CAPTURE_NONE) {
			bonobo_ui_component_set_translate (component, "/menu/Camera/Camera/separator", "<separator name=\"separator\"/>", NULL);
		}
                if (control->priv->camera->abilities->capture && GP_CAPTURE_IMAGE) {
                        bonobo_ui_component_set_translate (component, "/menu/Camera/Camera/CaptureImage", CAPTURE_IMAGE, NULL);
                        bonobo_ui_component_add_verb (component, "CaptureImage", on_capture_image_clicked, control);
                }
                if (control->priv->camera->abilities->capture && GP_CAPTURE_VIDEO) {
                        bonobo_ui_component_set_translate (component, "/menu/Camera/Camera/CaptureVideo", CAPTURE_VIDEO, NULL);
                        bonobo_ui_component_add_verb (component, "CaptureVideo", on_capture_video_clicked, control);
                }
                if (control->priv->camera->abilities->capture && GP_CAPTURE_PREVIEW) {
                        bonobo_ui_component_set_translate (component, "/menu/Camera/Camera/CapturePreview", CAPTURE_PREVIEW, NULL);
                        bonobo_ui_component_add_verb (component, "CapturePreview", on_capture_preview_clicked, control);
                }

	} else bonobo_ui_component_unset_container (component);

	if (gnocam_control_parent_class->activate) gnocam_control_parent_class->activate (object, state);
}

GnoCamControl*
gnocam_control_new (BonoboMoniker* moniker, CORBA_Environment* ev)
{
	GnoCamControl*		new;
	gchar*			tmp;
	gchar*			name;
	gint			i;
	Camera*			camera;
	Bonobo_Storage		storage;
	GtkWidget*		vbox;
	GtkWidget*		scroll_frame;
	GtkWidget*		label;

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
	new->priv->component = NULL;
	new->priv->hash_table = g_hash_table_new (g_str_hash, g_str_equal);
	gp_camera_ref (camera);

	/* Callbacks for the backend */
	camera->frontend_data = (void*) bonobo_control_get_ui_component (BONOBO_CONTROL (new));
	gp_frontend_register (gp_frontend_status, NULL, gp_frontend_message, gp_frontend_confirm, NULL);

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

	/* Create the notebook */
	gtk_widget_show (new->priv->notebook = gtk_notebook_new ());
	gtk_notebook_set_show_border (GTK_NOTEBOOK (new->priv->notebook), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (new->priv->notebook), FALSE);
	e_paned_pack2 (E_PANED (new->priv->hpaned), new->priv->notebook, TRUE, FALSE);

	/* Create label for empty page */
	gtk_widget_show (label = e_clipped_label_new (_("(No folder/file displayed)")));
	gtk_notebook_append_page (GTK_NOTEBOOK (new->priv->notebook), label, NULL);

	/* Create another vbox for the storage view */
	gtk_widget_show (new->priv->storage_view_vbox = gtk_vbox_new (FALSE, 0));
	e_paned_pack1 (E_PANED (new->priv->hpaned), new->priv->storage_view_vbox, FALSE, FALSE);
	
	/* Create the title for the storage view */
	gtk_widget_show (new->priv->storage_view_title_bar = e_title_bar_new (_("Contents")));
	gtk_box_pack_start (GTK_BOX (new->priv->storage_view_vbox), new->priv->storage_view_title_bar, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view_title_bar), "button_clicked", GTK_SIGNAL_FUNC (on_storage_view_title_bar_button_clicked), new);

	/* Create the scroll frame */
	gtk_widget_show (scroll_frame = e_scroll_frame_new (NULL, NULL));
	e_scroll_frame_set_policy (E_SCROLL_FRAME (scroll_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scroll_frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX (new->priv->storage_view_vbox), scroll_frame, TRUE, TRUE, 0);

	/* Create the storage view */
	gtk_widget_show (new->priv->storage_view = gnocam_storage_view_new (storage));
	gtk_container_add (GTK_CONTAINER (scroll_frame), new->priv->storage_view);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "directory_selected", GTK_SIGNAL_FUNC (on_directory_selected), new);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "file_selected", GTK_SIGNAL_FUNC (on_file_selected), new);

	/* Select the selected file/folder */
//FIXME: Problems with container == CORBA_OBJECT_NIL
//	name = (gchar*) bonobo_moniker_get_name (moniker);
//	name += 2;
//	for (; *name != 0; name++) if (*name == '/') break;
//	gtk_signal_emit_by_name (GTK_OBJECT (new->priv->storage_view), "directory_selected", name);

	/* Set default settings */
	new->priv->storage_view_mode = GNOCAM_CONTROL_STORAGE_VIEW_MODE_STICKY;
	e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (new->priv->title_bar), FALSE);

	return (new);
}

static void
destroy (GtkObject* object)
{
	GnoCamControl*  control;
	
	control = GNOCAM_CONTROL (object);

	if (control->priv->configuration) gp_widget_unref (control->priv->configuration);
	if (control->priv->camera) gp_camera_unref (control->priv->camera);

//	g_hash_table_foreach (control->priv->hash_table, hash_table_forall_destroy_control, NULL);
	g_hash_table_destroy (control->priv->hash_table);

	g_free (control->priv);
}

static void
init (GnoCamControl* control)
{
	control->priv = g_new (GnoCamControlPrivate, 1);
        control->priv->camera = NULL;
        control->priv->configuration = NULL;
        control->priv->hpaned_position = 0;
        control->priv->title_bar = NULL;
        control->priv->hpaned = NULL;
	control->priv->notebook = NULL;
        control->priv->storage_view_vbox = NULL;
        control->priv->storage_view_title_bar = NULL;
        control->priv->storage_view = NULL;
        control->priv->storage_view_mode = GNOCAM_CONTROL_STORAGE_VIEW_MODE_HIDDEN;
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


