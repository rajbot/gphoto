/* gnocam-camera.c
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

#include <gphoto-extensions.h>
#include "gnocam-camera.h"

#include <gal/util/e-util.h>
#include <gal/widgets/e-scroll-frame.h>
#include <gal/e-paned/e-hpaned.h>

#include "e-clipped-label.h"
#include "e-title-bar.h"
#include "e-shell-folder-title-bar.h"
#include "utils.h"
#include "gnocam-configuration.h"
#include "gnocam-capture.h"
#include "gnocam-storage-view.h"
#include "gnocam-folder.h"
#include "gnocam-file.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* gnocam_camera_parent_class = NULL;

struct _GnoCamCameraPrivate
{
	GtkWidget*			window;
	
	Bonobo_UIContainer		container;
	BonoboUIComponent*		component;
	
	Bonobo_Storage			storage;
	
        Camera*                 	camera;
	CameraWidget*                   configuration;

	GConfClient*			client;

	GtkWidget*			widget;

	guint				hpaned_position;
	GtkWidget*			title_bar;
	GtkWidget*			hpaned;
	GtkWidget*			notebook;
	GtkWidget*			storage_view_vbox;
	GtkWidget*			storage_view_title_bar;
	GtkWidget*			storage_view;
	GnoCamCameraStorageViewMode	storage_view_mode;

	GHashTable*			hash_table;

	gboolean			current_is_folder;
	GnoCamFolder*			folder;
	GnoCamFile*			file;
};

/****************/
/* UI-Variables */
/****************/

#define GNOCAM_CAMERA_UI 											\
"<placeholder name=\"Camera\">"											\
"  <submenu name=\"Camera\" _label=\"Camera\">"									\
"    <menuitem name=\"Manual\" _label=\"Manual\" verb=\"\" pixtype=\"stock\" pixname=\"Book Open\"/>"		\
"    <placeholder name=\"CaptureOperations\" delimit=\"top\"/>"							\
"    <placeholder name=\"Configuration\" delimit=\"top\"/>"							\
"  </submenu>"													\
"</placeholder>"

#define GNOCAM_CAMERA_UI_CONFIGURATION											\
"<placeholder name=\"Configuration\">"											\
"  <menuitem name=\"Configuration\" verb=\"\" _label=\"Configuration\" pixtype=\"stock\" pixname=\"Properties\"/>"	\
"</placeholder>"

#define CAPTURE_IMAGE								\
"<placeholder name=\"CaptureOperations\">"					\
"  <menuitem name=\"CaptureImage\" _label=\"Capture Image\" verb=\"\"/>"	\
"</placeholder>"

#define CAPTURE_PREVIEW								\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CapturePreview\" _label=\"Capture Preview\" verb=\"\"/>"	\
"</placeholder>"

#define CAPTURE_VIDEO								\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CaptureVideo\" _label=\"Capture Video\" verb=\"\"/>"	\
"</placeholder>"

#define GNOCAM_CAMERA_UI_PREVIEW		\
"<placeholder name=\"Preview\">"		\
"  <menuitem name=\"Preview\" verb=\"\"/>"	\
"</placeholder>"

#define GNOCAM_CAMERA_UI_PREVIEW_COMMAND							\
"<cmd name=\"Preview\" _label=\"Preview\" _tip=\"View previews only\" type=\"toggle\"/>"

/**************/
/* Prototypes */
/**************/

static void 	on_manual_clicked 		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void 	on_capture_preview_clicked 	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_image_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_video_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_configuration_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

static void 	on_preview_clicked 	(BonoboUIComponent* component, const gchar* path, Bonobo_UIComponent_EventType type, const gchar* state, gpointer user_data);

static void 	on_title_bar_toggled 				(EShellFolderTitleBar* title_bar, gboolean state, void* data);

static void 	on_storage_view_vbox_map 			(GtkWidget* widget, void* data);
static int 	on_storage_view_vbox_button_release_event 	(GtkWidget* widget, GdkEventButton* button_event, void* data);

static void 	on_popup_storage_view_title_bar_button_clicked 	(ETitleBar* title_bar, void* data);

static void 	show_current_menu 	(GnoCamCamera* camera);

/**********************/
/* Internal functions */
/**********************/

static gint
create_menu (gpointer user_data)
{
	GnoCamCamera*	camera;

	g_return_val_if_fail (user_data, FALSE);
	camera = GNOCAM_CAMERA (user_data);

	camera->priv->component = bonobo_ui_component_new (PACKAGE "Camera");
	bonobo_ui_component_set_container (camera->priv->component, camera->priv->container);
	
	bonobo_ui_component_freeze (camera->priv->component, NULL);

	/* Create the main menu */
        bonobo_ui_component_set_translate (camera->priv->component, "/menu", GNOCAM_CAMERA_UI, NULL);
        bonobo_ui_component_add_verb (camera->priv->component, "Manual", on_manual_clicked, camera);

	/* Preview? */
	if (camera->priv->camera->abilities->file_preview) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/View", GNOCAM_CAMERA_UI_PREVIEW, NULL);
		bonobo_ui_component_set_translate (camera->priv->component, "/commands", GNOCAM_CAMERA_UI_PREVIEW_COMMAND, NULL);
		if (gconf_client_get_bool (camera->priv->client, "/apps/" PACKAGE "/preview", NULL))
			bonobo_ui_component_set_prop (camera->priv->component, "/commands/Preview", "state", "1", NULL);
		else 
			bonobo_ui_component_set_prop (camera->priv->component, "/commands/Preview", "state", "0", NULL);
                bonobo_ui_component_add_listener (camera->priv->component, "Preview", on_preview_clicked, camera);
	}

        /* Capture? */
        if (camera->priv->camera->abilities->capture & GP_CAPTURE_IMAGE) {
        	bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", CAPTURE_IMAGE, NULL);
                bonobo_ui_component_add_verb (camera->priv->component, "CaptureImage", on_capture_image_clicked, camera);
        }
        if (camera->priv->camera->abilities->capture & GP_CAPTURE_VIDEO) {
                bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", CAPTURE_VIDEO, NULL);
                bonobo_ui_component_add_verb (camera->priv->component, "CaptureVideo", on_capture_video_clicked, camera);
        }
	if (camera->priv->camera->abilities->capture & GP_CAPTURE_PREVIEW) {
                bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", CAPTURE_PREVIEW, NULL);
                bonobo_ui_component_add_verb (camera->priv->component, "CapturePreview", on_capture_preview_clicked, camera);
	}

	/* Configuration? */
	if (camera->priv->camera->abilities->config & GP_CONFIG_CAMERA) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", GNOCAM_CAMERA_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (camera->priv->component, "Configuration", on_configuration_clicked, camera);
	}

        bonobo_ui_component_thaw (camera->priv->component, NULL);

	return (FALSE);
}

static int
gp_frontend_status (Camera* camera, char* status)
{
        GnoCamCamera*   c;

        c = GNOCAM_CAMERA (camera->frontend_data);

        bonobo_ui_component_set_status (c->priv->component, status, NULL);

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
disconnect_popup_signals (GnoCamCamera* camera)
{
        gtk_signal_disconnect_by_func (GTK_OBJECT (camera->priv->storage_view_vbox), GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), camera);
        gtk_signal_disconnect_by_func (GTK_OBJECT (camera->priv->storage_view_vbox), GTK_SIGNAL_FUNC (on_storage_view_vbox_map), camera);
        gtk_signal_disconnect_by_func (GTK_OBJECT (camera->priv->storage_view), GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), camera);
        gtk_signal_disconnect_by_func (GTK_OBJECT (camera->priv->storage_view_title_bar), GTK_SIGNAL_FUNC (on_popup_storage_view_title_bar_button_clicked), camera);
}

static void
popdown_transient_folder_bar (GnoCamCamera* camera)
{
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (camera->priv->storage_view_vbox);

        gnocam_camera_set_storage_view_mode (camera, GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN);

        disconnect_popup_signals (camera);

	e_shell_folder_title_bar_set_toggle_state (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), FALSE);
}

static void
hide_current_menu (GnoCamCamera* camera)
{
	if (!camera->priv->current_is_folder && camera->priv->file) {
		gnocam_file_hide_menu (camera->priv->file);
		return;
	}

	if (camera->priv->current_is_folder && camera->priv->folder) {
		gnocam_folder_hide_menu (camera->priv->folder);
		return;
	}
	
	return;
}

static void
show_current_menu (GnoCamCamera* camera)
{
	if (!camera->priv->current_is_folder && camera->priv->file) {
		gnocam_file_show_menu (camera->priv->file);
		return;
	}
	
	if (camera->priv->current_is_folder && camera->priv->folder) {
		gnocam_folder_show_menu (camera->priv->folder);
		return;
	}

	return;
}

/*************/
/* Callbacks */
/*************/

static void
on_widget_changed (GnoCamFile* file, gpointer user_data)
{
	GnoCamCamera*	camera;
	GtkWidget*	widget;
	gint		page, current_page;
	
	g_return_if_fail (user_data);
	camera = GNOCAM_CAMERA (user_data);

	g_message ("on_widget_changed");

	current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (camera->priv->notebook));

	/* Remove old page */
	page = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (file), "page"));
	widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (camera->priv->notebook), page);
	gtk_notebook_remove_page (GTK_NOTEBOOK (camera->priv->notebook), page);
	gtk_widget_unref (widget);

	/* Show the new widget */
	widget = gnocam_file_get_widget (file);
	g_return_if_fail (widget);
	gtk_widget_show (widget);
	gtk_widget_ref (widget);
	gtk_notebook_insert_page (GTK_NOTEBOOK (camera->priv->notebook), widget, NULL, page);

	gtk_notebook_set_page (GTK_NOTEBOOK (camera->priv->notebook), current_page);

	/* Could well be that the menu changed, too */
	gnocam_file_show_menu (file);
}

static void
on_configuration_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	GtkWidget*	widget;

	camera = GNOCAM_CAMERA (user_data);
	
	widget = gnocam_configuration_new (camera->priv->camera, NULL, NULL, camera->priv->window);
	gtk_widget_show (widget);
}

static void
on_preview_clicked (BonoboUIComponent* component, const gchar* path, Bonobo_UIComponent_EventType type, const gchar* state, gpointer user_data)
{
	GnoCamCamera*	camera;
	gboolean	current;

	g_return_if_fail (user_data);
	camera = GNOCAM_CAMERA (user_data);

	/* Did the state really change? */
	current = gconf_client_get_bool (camera->priv->client, "/apps/" PACKAGE "/preview", NULL);
	if (current && !strcmp ("1", state)) return;
	if (!current && !strcmp ("0", state)) return;

	/* Tell GConf about the change */
	if (!strcmp ("0", state)) gconf_client_set_bool (camera->priv->client, "/apps/" PACKAGE "/preview", FALSE, NULL);
	else gconf_client_set_bool (camera->priv->client, "/apps/" PACKAGE "/preview", TRUE, NULL);
}

static void
on_size_request (GtkWidget* widget, GtkRequisition* requisition, gpointer user_data)
{
	GnoCamCamera*	camera;

	camera = GNOCAM_CAMERA (user_data);

	gconf_client_set_int (camera->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", e_paned_get_position (E_PANED (widget)), NULL);
}

static void
on_manual_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera* 	camera;
	gint		result;
	CameraText	manual;

	camera = GNOCAM_CAMERA (user_data);

        result = gp_camera_manual (camera->priv->camera, &manual);
	
	if (result != GP_OK) g_warning (_("Could not get camera manual!\n(%s)"), gp_camera_result_as_string (camera->priv->camera, result));
        else g_message (manual.text);
}

static void
on_capture_preview_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;

	camera = GNOCAM_CAMERA (user_data);

	gtk_widget_show (GTK_WIDGET (gnocam_capture_new (camera->priv->camera, GP_CAPTURE_PREVIEW, camera->priv->client)));
}

static void 
on_capture_image_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;

	camera = GNOCAM_CAMERA (user_data);

	gtk_widget_show (GTK_WIDGET (gnocam_capture_new (camera->priv->camera, GP_CAPTURE_IMAGE, camera->priv->client)));
}

static void
on_capture_video_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*  camera;

	camera = GNOCAM_CAMERA (user_data);

	gtk_widget_show (GTK_WIDGET (gnocam_capture_new (camera->priv->camera, GP_CAPTURE_VIDEO, camera->priv->client)));
}

static void
on_file_selected (GnoCamStorageView* storage_view, const gchar* path, void* data)
{
	GnoCamCamera*		camera;
	GtkWidget*		widget;
	GnoCamFile*		file;
	GdkPixbuf*		pixbuf;

	camera = GNOCAM_CAMERA (data);

	if (camera->priv->storage_view_mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (camera);

	file = g_hash_table_lookup (camera->priv->hash_table, path);
	if (!file) {

		/* Get the file */
		file = gnocam_file_new (camera->priv->camera, camera->priv->storage, g_basename (path), camera->priv->container, 
			camera->priv->client, camera->priv->window);
		g_return_if_fail (file);

		/* Get the widget */
		widget = gnocam_file_get_widget (file);
		g_return_if_fail (widget);
		gtk_widget_show (widget);
		gtk_widget_ref (widget);

		/* Append the page, store the page number */
		gtk_notebook_append_page (GTK_NOTEBOOK (camera->priv->notebook), widget, NULL);
		gtk_object_set_data (GTK_OBJECT (file), "page", GINT_TO_POINTER (gtk_notebook_page_num (GTK_NOTEBOOK (camera->priv->notebook), widget)));

		gtk_signal_connect (GTK_OBJECT (file), "widget_changed", GTK_SIGNAL_FUNC (on_widget_changed), camera);
		
		g_hash_table_insert (camera->priv->hash_table, g_strdup (path), file);
	}
	
	hide_current_menu (camera);
	camera->priv->current_is_folder = FALSE;
	camera->priv->file = file;
	gnocam_file_show_menu (file);

	e_shell_folder_title_bar_set_title (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), path);
	if ((pixbuf = util_pixbuf_file ()))
		e_shell_folder_title_bar_set_icon (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), pixbuf);

	/* Display the (new) page */
	gtk_notebook_set_page (GTK_NOTEBOOK (camera->priv->notebook), GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (file), "page")));
}

static void
on_directory_selected (GnoCamStorageView* storage_view, const gchar* path, void* data)
{
	GnoCamCamera*		camera;
	GtkWidget*		folder;
	GdkPixbuf*		pixbuf;

	camera = GNOCAM_CAMERA (data);

	if (camera->priv->storage_view_mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (camera);

	folder = g_hash_table_lookup (camera->priv->hash_table, path);
	if (!folder) {
		GtkWidget*	scroll_frame;
	
		/* Create the folder */
		folder = gnocam_folder_new (camera->priv->camera, camera->priv->storage, path, camera->priv->container, camera->priv->client, camera->priv->window);
		if (!folder) return;
		gtk_widget_show (folder);

		/* Create the scroll-frame */
		scroll_frame = e_scroll_frame_new (NULL, NULL);
		gtk_widget_show (scroll_frame);
		e_scroll_frame_set_policy (E_SCROLL_FRAME (scroll_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scroll_frame), GTK_SHADOW_IN);
		gtk_container_add (GTK_CONTAINER (scroll_frame), folder);

		/* Append the page, store the page number */
		gtk_notebook_append_page (GTK_NOTEBOOK (camera->priv->notebook), scroll_frame, NULL);
		gtk_object_set_data (GTK_OBJECT (folder), "page", GINT_TO_POINTER (gtk_notebook_page_num (GTK_NOTEBOOK (camera->priv->notebook), scroll_frame)));
		
		g_hash_table_insert (camera->priv->hash_table, g_strdup (path), folder);
	}

	hide_current_menu (camera);
	camera->priv->current_is_folder = TRUE;
	camera->priv->folder = GNOCAM_FOLDER (folder);
	gnocam_folder_show_menu (GNOCAM_FOLDER (folder));
	
	e_shell_folder_title_bar_set_title (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), path);
	if ((pixbuf = util_pixbuf_folder ()))
		e_shell_folder_title_bar_set_icon (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), pixbuf);

	/* Display the (new) page */
	gtk_notebook_set_page (GTK_NOTEBOOK (camera->priv->notebook), GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (folder), "page")));
}

static void
on_title_bar_toggled (EShellFolderTitleBar* title_bar, gboolean state, void* data)
{
	GnoCamCamera*	camera;

	camera = GNOCAM_CAMERA (data);

	if (!state) return;

	if (camera->priv->storage_view_mode != GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT) {

		camera->priv->storage_view_mode = GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT;

		/* We need to show the storage view box and do a pointer grab to catch the
	           mouse clicks.  But until the box is shown, we cannot grab.  So we connect to
	           the "map" signal; `on_storage_view_vbox_map()' will do the grab.  */
	
		gtk_signal_connect (GTK_OBJECT (camera->priv->storage_view_vbox), "map", GTK_SIGNAL_FUNC (on_storage_view_vbox_map), camera);
		gtk_widget_show (camera->priv->storage_view_vbox);
		e_paned_set_position (E_PANED (camera->priv->hpaned), camera->priv->hpaned_position);
	}
}

static void
on_popup_storage_view_title_bar_button_clicked (ETitleBar* title_bar, void* data)
{
	GnoCamCamera*	camera;

	camera = GNOCAM_CAMERA (data);

        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (camera->priv->storage_view_vbox);

        disconnect_popup_signals (camera);

        gnocam_camera_set_storage_view_mode (camera, GNOCAM_CAMERA_STORAGE_VIEW_MODE_STICKY);
        e_shell_folder_title_bar_set_toggle_state (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), FALSE);
}

static void
on_storage_view_title_bar_button_clicked (ETitleBar* title_bar, void* data)
{
	GnoCamCamera*	camera;

	camera = GNOCAM_CAMERA (data);

	gnocam_camera_set_storage_view_mode (camera, GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN);
}

static void
on_storage_view_vbox_map (GtkWidget* widget, void* data)
{
	GnoCamCamera* camera;

	camera = GNOCAM_CAMERA (data);

        if (gdk_pointer_grab (widget->window, TRUE,
                              (GDK_BUTTON_PRESS_MASK
                               | GDK_BUTTON_RELEASE_MASK
                               | GDK_ENTER_NOTIFY_MASK
                               | GDK_LEAVE_NOTIFY_MASK
                               | GDK_POINTER_MOTION_MASK),
                              NULL, NULL, GDK_CURRENT_TIME) != 0) {
                g_warning ("gnocam-camera.c:on_storage_view_vbox_map() -- pointer grab failed.");
                gnocam_camera_set_storage_view_mode (camera, GNOCAM_CAMERA_STORAGE_VIEW_MODE_STICKY);
                return;
        }

        gtk_grab_add (widget);
        gtk_signal_connect (GTK_OBJECT (widget), "button_release_event", 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), camera);
        gtk_signal_connect (GTK_OBJECT (camera->priv->storage_view), "button_release_event", 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), camera);
        gtk_signal_connect (GTK_OBJECT (camera->priv->storage_view_title_bar), "button_clicked", 
		GTK_SIGNAL_FUNC (on_popup_storage_view_title_bar_button_clicked), camera);
}

static int
on_storage_view_vbox_button_release_event (GtkWidget* widget, GdkEventButton* button_event, void* data)
{
	GnoCamCamera* camera;

	camera = GNOCAM_CAMERA (data);

        if (button_event->window == E_PANED (camera->priv->hpaned)->handle)
                return (FALSE);

        popdown_transient_folder_bar (camera);

        return (TRUE);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_camera_set_storage_view_mode (GnoCamCamera* camera, GnoCamCameraStorageViewMode mode)
{
	if (mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_STICKY) {
		if (!GTK_WIDGET_VISIBLE (camera->priv->storage_view_vbox)) {
			gtk_widget_show (camera->priv->storage_view_vbox);
			e_paned_set_position (E_PANED (camera->priv->hpaned), camera->priv->hpaned_position);
		}
		e_title_bar_set_button_mode (E_TITLE_BAR (camera->priv->storage_view_title_bar), E_TITLE_BAR_BUTTON_MODE_CLOSE);
		e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), FALSE);
	} else {
		if (GTK_WIDGET_VISIBLE (camera->priv->storage_view_vbox)) {
	                gtk_widget_hide (camera->priv->storage_view_vbox);
        	        camera->priv->hpaned_position =  e_paned_get_position (E_PANED (camera->priv->hpaned));
                	e_paned_set_position (E_PANED (camera->priv->hpaned), 0);
		}
                e_title_bar_set_button_mode (E_TITLE_BAR (camera->priv->storage_view_title_bar), E_TITLE_BAR_BUTTON_MODE_PIN);
                e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), TRUE);
	}
	camera->priv->storage_view_mode = mode;
	gconf_client_set_int (camera->priv->client, "/apps/" PACKAGE "/storage_view_mode", mode, NULL);
}

GtkWidget*
gnocam_camera_get_widget (GnoCamCamera* camera)
{
	return (camera->priv->widget);
}

void
gnocam_camera_show_menu (GnoCamCamera* camera)
{
	Bonobo_UIContainer	container;
	
	g_return_if_fail (camera);

	container = bonobo_ui_component_get_container (camera->priv->component);
	bonobo_object_release_unref (container, NULL);
	if (container == camera->priv->container) return;

	bonobo_ui_component_set_container (camera->priv->component, camera->priv->container);
	show_current_menu (camera);
}

void
gnocam_camera_hide_menu (GnoCamCamera* camera)
{
	g_return_if_fail (camera);

	bonobo_ui_component_unset_container (camera->priv->component);
	hide_current_menu (camera);
}

/*************************/
/* Bonobo-X-Object stuff */
/*************************/

GnoCamCamera*
gnocam_camera_new (const gchar* url, Bonobo_UIContainer container, GtkWidget* window, GConfClient* client, CORBA_Environment* ev)
{
	GnoCamCamera*		new;
	gchar*			name;
	gint			position;
	Camera*			camera;
	BonoboStorage*		storage;
	Bonobo_Storage_OpenMode	mode;
	GtkWidget*		scroll_frame;
	GtkWidget*		label;

	g_return_val_if_fail (url, NULL);
	g_return_val_if_fail (window, NULL);
	g_return_val_if_fail (ev, NULL);

	/* Try to get a camera */
	CHECK_RESULT (gp_camera_new_from_gconf (&camera, url), ev);
	if (BONOBO_EX (ev)) return (NULL);
	g_return_val_if_fail (camera, NULL);
	
	/* Try to get a storage */
	mode = Bonobo_Storage_READ;
	if (gconf_client_get_bool (client, "/apps/" PACKAGE "/preview", NULL) && camera->abilities->file_preview) mode |= Bonobo_Storage_COMPRESSED;
	if (camera->abilities->file_put) mode |= Bonobo_Storage_WRITE;
	storage = bonobo_storage_open_full ("camera", url, mode, 0664, ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (NULL);
	}
	g_return_val_if_fail (storage, NULL);

	new = gtk_type_new (GNOCAM_TYPE_CAMERA);
	gtk_widget_ref (new->priv->window = window);
	gp_camera_ref (new->priv->camera = camera);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));
	new->priv->configuration = NULL;
	new->priv->storage = BONOBO_OBJREF (storage);
	new->priv->container = bonobo_object_dup_ref (container, NULL);
	new->priv->hash_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* Callbacks for the backend */
	camera->frontend_data = (void*) new;
	gp_frontend_register (gp_frontend_status, NULL, gp_frontend_message, gp_frontend_confirm, NULL);

	/* Create the basic layout */
	gtk_widget_show (new->priv->widget = gtk_vbox_new (FALSE, 0));
	gtk_container_set_border_width (GTK_CONTAINER (new->priv->widget), 2);

	/* Create the title bar */
	gtk_widget_show (new->priv->title_bar = e_shell_folder_title_bar_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->title_bar), "title_toggled", GTK_SIGNAL_FUNC (on_title_bar_toggled), new);
	gtk_box_pack_start (GTK_BOX (new->priv->widget), new->priv->title_bar, FALSE, FALSE, 0);

	/* Create the paned */
	gtk_widget_show (new->priv->hpaned = e_hpaned_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->hpaned), "size_request", GTK_SIGNAL_FUNC (on_size_request), new);
	gtk_box_pack_start (GTK_BOX (new->priv->widget), new->priv->hpaned, TRUE, TRUE, 2); 
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
	new->priv->storage_view = gnocam_storage_view_new (new->priv->storage);
	gtk_widget_show (new->priv->storage_view);
	gtk_container_add (GTK_CONTAINER (scroll_frame), new->priv->storage_view);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "directory_selected", GTK_SIGNAL_FUNC (on_directory_selected), new);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "file_selected", GTK_SIGNAL_FUNC (on_file_selected), new);

        /* Create the menu */
	create_menu (new);

	/* Select the selected file/folder */
	name = (gchar*) url + 9;
	for (; *name != 0; name++) if (*name == '/') break;
	gtk_signal_emit_by_name (GTK_OBJECT (new->priv->storage_view), "directory_selected", name);

	/* Set default settings */
	position = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", NULL);
	if (position) e_paned_set_position (E_PANED (new->priv->hpaned), position);
	new->priv->storage_view_mode = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/storage_view_mode", NULL);
	gnocam_camera_set_storage_view_mode (new, new->priv->storage_view_mode);

	return (new);
}

static void
gnocam_camera_destroy (GtkObject* object)
{
	GnoCamCamera*  camera;
	
	camera = GNOCAM_CAMERA (object);

	gtk_widget_unref (camera->priv->window);

	bonobo_object_release_unref (camera->priv->storage, NULL);
	bonobo_object_unref (BONOBO_OBJECT (camera->priv->component));
	bonobo_object_release_unref (camera->priv->container, NULL);

	gtk_object_unref (GTK_OBJECT (camera->priv->client));

	if (camera->priv->configuration) gp_widget_unref (camera->priv->configuration);
	gp_camera_unref (camera->priv->camera);

	g_hash_table_destroy (camera->priv->hash_table);

	g_free (camera->priv);
}

static void
gnocam_camera_init (GnoCamCamera* camera)
{
	camera->priv = g_new0 (GnoCamCameraPrivate, 1);
        camera->priv->hpaned_position = 0;
        camera->priv->storage_view_mode = GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN;
}

static void
gnocam_camera_class_init (GnoCamCameraClass* klass)
{
        GtkObjectClass* object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_camera_destroy;

        gnocam_camera_parent_class = gtk_type_class (PARENT_TYPE);
}

BONOBO_X_TYPE_FUNC_FULL (GnoCamCamera, GNOME_GnoCam_camera, PARENT_TYPE, gnocam_camera);



