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
#include <bonobo-extensions.h>
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

#define PARENT_TYPE GTK_TYPE_VBOX
static GtkVBoxClass* parent_class = NULL;

struct _GnoCamCameraPrivate
{
	BonoboUIContainer*		container;

	BonoboUIComponent*		component;
	
	BonoboStorage*			storage;
	Bonobo_Storage_OpenMode		mode;
	
        Camera*                 	camera;
	CameraWidget*                   configuration;

	GConfClient*			client;

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

	CameraOperation			type;
	gint				duration;
};

enum {
        FOLDER_UPDATED,
        LAST_SIGNAL
};

static unsigned int signals [LAST_SIGNAL] = { 0 };

/****************/
/* UI-Variables */
/****************/

#define GNOCAM_CAMERA_UI 											\
"<placeholder name=\"Camera\">"											\
"  <submenu name=\"Camera\" _label=\"Camera\">"									\
"    <menuitem name=\"Manual\" _label=\"Manual\" verb=\"\" pixtype=\"stock\" pixname=\"Book Open\"/>"		\
"    <menuitem name=\"About\" _label=\"About\" verb=\"\" pixtype=\"stock\" pixname=\"Book Open\"/>"		\
"    <menuitem name=\"Summary\" _label=\"Summary\" verb=\"\" pixtype=\"stock\" pixname=\"Book Open\"/>"		\
"    <placeholder name=\"CaptureOperations\" delimit=\"top\"/>"							\
"    <placeholder name=\"Configuration\" delimit=\"top\"/>"							\
"  </submenu>"													\
"</placeholder>"

#define GNOCAM_CAMERA_UI_CONFIGURATION											\
"<placeholder name=\"Configuration\">"											\
"  <menuitem name=\"Configuration\" verb=\"\" _label=\"Configuration\" pixtype=\"stock\" pixname=\"Properties\"/>"	\
"</placeholder>"

#define GNOCAM_CAMERA_UI_CAPTURE_IMAGE						\
"<placeholder name=\"CaptureOperations\">"					\
"  <menuitem name=\"CaptureImage\" _label=\"Capture Image\" verb=\"\"/>"	\
"</placeholder>"

#define GNOCAM_CAMERA_UI_CAPTURE_VIDEO						\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CaptureVideo\" _label=\"Capture Video\" verb=\"\"/>"	\
"</placeholder>"

#define GNOCAM_CAMERA_UI_CAPTURE_PREVIEW					\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CapturePreview\" _label=\"Capture Preview\" verb=\"\"/>"	\
"</placeholder>"

#define GNOCAM_CAMERA_UI_CAPTURE_AUDIO						\
"<placeholder name=\"CaptureOperations\">"                                      \
"  <menuitem name=\"CaptureAudio\" _label=\"Capture Audio\" verb=\"\"/>"	\
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

static void	on_about_clicked		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void 	on_manual_clicked 		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_summary_clicked		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void 	on_capture_clicked 		(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_capture_preview_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);
static void	on_configuration_clicked	(BonoboUIComponent* component, gpointer user_data, const gchar* cname);

static void 	on_preview_clicked 	(BonoboUIComponent* component, const gchar* path, Bonobo_UIComponent_EventType type, const gchar* state, gpointer user_data);

static void 	on_title_bar_toggled 				(EShellFolderTitleBar* title_bar, gboolean state, void* data);

static void 	on_storage_view_vbox_map 			(GtkWidget* widget, void* data);
static int 	on_storage_view_vbox_button_release_event 	(GtkWidget* widget, GdkEventButton* button_event, void* data);

static void 	on_popup_storage_view_title_bar_button_clicked 	(ETitleBar* title_bar, void* data);

/**********************/
/* Internal functions */
/**********************/

static void
create_menu (GnoCamCamera* camera)
{
	/* Create the main menu */
        bonobo_ui_component_set_translate (camera->priv->component, "/menu", GNOCAM_CAMERA_UI, NULL);
        bonobo_ui_component_add_verb (camera->priv->component, "Manual", on_manual_clicked, camera);
	bonobo_ui_component_add_verb (camera->priv->component, "About", on_about_clicked, camera);
	bonobo_ui_component_add_verb (camera->priv->component, "Summary", on_summary_clicked, camera);

	/* Preview? */
	if (camera->priv->camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW) {
		bonobo_ui_component_set_translate (camera->priv->component, "/commands", GNOCAM_CAMERA_UI_PREVIEW_COMMAND, NULL);
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/View", GNOCAM_CAMERA_UI_PREVIEW, NULL);
		if (gconf_client_get_bool (camera->priv->client, "/apps/" PACKAGE "/preview", NULL))
			bonobo_ui_component_set_prop (camera->priv->component, "/commands/Preview", "state", "1", NULL);
		else 
			bonobo_ui_component_set_prop (camera->priv->component, "/commands/Preview", "state", "0", NULL);
                bonobo_ui_component_add_listener (camera->priv->component, "Preview", on_preview_clicked, camera);
	}

	/* Capture Preview? */
	if (camera->priv->camera->abilities->operations & GP_OPERATION_CAPTURE_PREVIEW) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", GNOCAM_CAMERA_UI_CAPTURE_PREVIEW, NULL);
		bonobo_ui_component_add_verb (camera->priv->component, "CapturePreview", on_capture_preview_clicked, camera);
	}

	/* Capture Image? */
	if (camera->priv->camera->abilities->operations & GP_OPERATION_CAPTURE_IMAGE) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", GNOCAM_CAMERA_UI_CAPTURE_IMAGE, NULL);
		bonobo_ui_component_add_verb (camera->priv->component, "CaptureImage", on_capture_clicked, camera);
	}

	/* Capture Video? */
	if (camera->priv->camera->abilities->operations & GP_OPERATION_CAPTURE_VIDEO) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", GNOCAM_CAMERA_UI_CAPTURE_VIDEO, NULL);
		bonobo_ui_component_add_verb (camera->priv->component, "CaptureVideo", on_capture_clicked, camera);
	}

	/* Capture Audio? */
	if (camera->priv->camera->abilities->operations & GP_OPERATION_CAPTURE_AUDIO) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", GNOCAM_CAMERA_UI_CAPTURE_AUDIO, NULL);
		bonobo_ui_component_add_verb (camera->priv->component, "CaptureAudio", on_capture_clicked, camera);
	}

	/* Configuration? */
	if (camera->priv->camera->abilities->operations & GP_OPERATION_CONFIG) {
		bonobo_ui_component_set_translate (camera->priv->component, "/menu/Camera/Camera", GNOCAM_CAMERA_UI_CONFIGURATION, NULL);
		bonobo_ui_component_add_verb (camera->priv->component, "Configuration", on_configuration_clicked, camera);
	}
}

int
gp_frontend_status (Camera* camera, char* status)
{
        GnoCamCamera*   c;

        c = GNOCAM_CAMERA (camera->frontend_data);

        bonobo_ui_component_set_status (c->priv->component, status, NULL);

        return (GP_OK);
}

int
gp_frontend_message (Camera* camera, char* message)
{
        gnome_ok_dialog (message);
        return (GP_OK);
}

int
gp_frontend_confirm (Camera* camera, char* message)
{
        GtkWidget*      widget;
        gint            result;

        widget = gnome_dialog_new (message, GNOME_STOCK_BUTTON_YES, 
				   GNOME_STOCK_BUTTON_NO, NULL);
        result = gnome_dialog_run_and_close (GNOME_DIALOG (widget));
        gtk_widget_unref (widget);

        if (result == 1) return (GP_PROMPT_CANCEL);
        return (GP_PROMPT_OK);
}

static void
disconnect_popup_signals (GnoCamCamera* camera)
{
        gtk_signal_disconnect_by_func (
		GTK_OBJECT (camera->priv->storage_view_vbox), 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), 
		camera);
        gtk_signal_disconnect_by_func (
		GTK_OBJECT (camera->priv->storage_view_vbox), 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_map), 
		camera);
        gtk_signal_disconnect_by_func (
		GTK_OBJECT (camera->priv->storage_view), 
		GTK_SIGNAL_FUNC (on_storage_view_vbox_button_release_event), 
		camera);
        gtk_signal_disconnect_by_func (
		GTK_OBJECT (camera->priv->storage_view_title_bar), 
		GTK_SIGNAL_FUNC(on_popup_storage_view_title_bar_button_clicked),
		camera);
}

static void
popdown_transient_folder_bar (GnoCamCamera* camera)
{
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (camera->priv->storage_view_vbox);
        gnocam_camera_set_storage_view_mode (
					camera, 
					GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN);
        disconnect_popup_signals (camera);

	e_shell_folder_title_bar_set_toggle_state (
			E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), 
			FALSE);
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
on_configuration_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	GtkWidget*	widget;
	GtkWindow*	window;

	camera = GNOCAM_CAMERA (user_data);
	window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (camera), 
		    	     GTK_TYPE_WINDOW));
	
	widget = gnocam_configuration_new (camera->priv->camera, 
					   NULL, NULL, window);
	if (!widget) return;
	gtk_widget_show (widget);
}

static void
on_preview_clicked (BonoboUIComponent* component, const gchar* path, Bonobo_UIComponent_EventType type, const gchar* state, gpointer user_data)
{
	GnoCamCamera*	camera;
	gboolean	current;

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
	gint		position;

	camera = GNOCAM_CAMERA (user_data);
	position = e_paned_get_position (E_PANED (widget));
	
	if (!position) return;
	if (camera->priv->storage_view_mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT) return;

	gconf_client_set_int (camera->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", position, NULL);
}

static void
on_about_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	gint		result;
	CameraText	about;

	camera = GNOCAM_CAMERA (user_data);

	result = gp_camera_get_about (camera->priv->camera, &about);
	if (result != GP_OK) {
		g_warning (_("Could not get information about the camera driver: %s!"), gp_camera_get_result_as_string (camera->priv->camera, result));
		return;
	}
	g_message (about.text);
}

static void
on_summary_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*   camera;
	gint            result;
	CameraText	summary;

	camera = GNOCAM_CAMERA (user_data);

	result = gp_camera_get_summary (camera->priv->camera, &summary);
	if (result != GP_OK) {
		g_warning (_("Could not get camera summary: %s!"), gp_camera_get_result_as_string (camera->priv->camera, result));
		return;
	}
	g_message (summary.text);
}

static void
on_manual_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera* 	camera;
	gint		result;
	CameraText	manual;

	camera = GNOCAM_CAMERA (user_data);

        result = gp_camera_get_manual (camera->priv->camera, &manual);
	
	if (result != GP_OK) {
		g_warning (_("Could not get camera manual: %s!"), gp_camera_get_result_as_string (camera->priv->camera, result));
		return;
	}
        g_message (manual.text);
}

static void
on_capture_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	CameraFilePath	filepath;
	gint		result;

	camera = GNOCAM_CAMERA (user_data);

	if (!strcmp (cname, "CaptureImage"))	camera->priv->type = GP_OPERATION_CAPTURE_IMAGE;
	if (!strcmp (cname, "CaptureVideo")) 	camera->priv->type = GP_OPERATION_CAPTURE_VIDEO;
	if (!strcmp (cname, "CaptureAudio")) 	camera->priv->type = GP_OPERATION_CAPTURE_AUDIO;
	if (!strcmp (cname, "CapturePreview"))	camera->priv->type = GP_OPERATION_CAPTURE_PREVIEW;

	result = gp_camera_capture (camera->priv->camera, camera->priv->type, &filepath);
	if (result != GP_OK) {
		g_warning (_("Could not capture: %s!"), gp_camera_get_result_as_string (camera->priv->camera, result));
		return;
	}

	gtk_signal_emit_by_name (GTK_OBJECT (camera), "folder_updated", filepath.folder);
}

static void
on_capture_preview_clicked (BonoboUIComponent* component, gpointer user_data, const gchar* cname)
{
	GnoCamCamera*	camera;
	GtkWindow*	window;

	camera = GNOCAM_CAMERA (user_data);
	window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (camera), GTK_TYPE_WINDOW));

	gtk_widget_show (gnocam_capture_new (camera->priv->camera, camera->priv->client, window));
}

static void
on_file_selected (GnoCamStorageView* storage_view, const gchar* path, void* data)
{
	GnoCamCamera*		camera;
	GtkWidget*		file;
	GdkPixbuf*		pixbuf;

	camera = GNOCAM_CAMERA (data);

	if (camera->priv->storage_view_mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (camera);

	file = g_hash_table_lookup (camera->priv->hash_table, path);
	if (!file) {

		/* Get the file */
		file = gnocam_file_new (camera, path);
		g_return_if_fail (file);
		gtk_widget_show (file);

		/* Append the page, store the page number */
		gtk_notebook_append_page (GTK_NOTEBOOK (camera->priv->notebook), file, NULL);
		gtk_object_set_data (GTK_OBJECT (file), "page", GINT_TO_POINTER (gtk_notebook_page_num (GTK_NOTEBOOK (camera->priv->notebook), file)));

		g_hash_table_insert (camera->priv->hash_table, g_strdup (path), file);
	}
	
	hide_current_menu (camera);
	camera->priv->current_is_folder = FALSE;
	camera->priv->file = GNOCAM_FILE (file);
	gnocam_file_show_menu (GNOCAM_FILE (file));

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
		folder = gnocam_folder_new (camera, path);
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
		e_paned_set_position (E_PANED (camera->priv->hpaned), gconf_client_get_int (camera->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", NULL));
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
	GnoCamCamera* 	camera;
	gint		position;

	camera = GNOCAM_CAMERA (data);
	position = e_paned_get_position (E_PANED (camera->priv->hpaned));

        if (button_event->window == E_PANED (camera->priv->hpaned)->handle) {
		gconf_client_set_int (camera->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", position, NULL);
		return (FALSE);
	}

        popdown_transient_folder_bar (camera);

        return (TRUE);
}

/*****************/
/* Our functions */
/*****************/

void
gnocam_camera_set_storage_view_mode (GnoCamCamera* camera, GnoCamCameraStorageViewMode mode)
{
	gint	position;

	position = gconf_client_get_int (camera->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", NULL);
	
	if (mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_STICKY) {
		if (!GTK_WIDGET_VISIBLE (camera->priv->storage_view_vbox)) {
			gtk_widget_show (camera->priv->storage_view_vbox);
			e_paned_set_position (E_PANED (camera->priv->hpaned), position);
		}
		e_title_bar_set_button_mode (E_TITLE_BAR (camera->priv->storage_view_title_bar), E_TITLE_BAR_BUTTON_MODE_CLOSE);
		e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), FALSE);
	} else {
		if (GTK_WIDGET_VISIBLE (camera->priv->storage_view_vbox)) {
	                gtk_widget_hide (camera->priv->storage_view_vbox);
                	e_paned_set_position (E_PANED (camera->priv->hpaned), 0);
		}
                e_title_bar_set_button_mode (E_TITLE_BAR (camera->priv->storage_view_title_bar), E_TITLE_BAR_BUTTON_MODE_PIN);
                e_shell_folder_title_bar_set_clickable (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), TRUE);
	}
	camera->priv->storage_view_mode = mode;
	gconf_client_set_int (camera->priv->client, "/apps/" PACKAGE "/storage_view_mode", mode, NULL);
}

void
gnocam_camera_show_menu (GnoCamCamera* camera)
{
	if (bonobo_ui_component_get_container (camera->priv->component) == BONOBO_OBJREF (camera->priv->container)) return;

	bonobo_ui_component_set_container (camera->priv->component, BONOBO_OBJREF (camera->priv->container));
	create_menu (camera);
	show_current_menu (camera);
}

void
gnocam_camera_hide_menu (GnoCamCamera* camera)
{
	g_return_if_fail (camera);

	bonobo_ui_component_unset_container (camera->priv->component);
	hide_current_menu (camera);
}

Camera*
gnocam_camera_get_camera (GnoCamCamera* camera)
{
	gp_camera_ref (camera->priv->camera);
	return (camera->priv->camera);
}

Bonobo_Storage
gnocam_camera_get_storage (GnoCamCamera* camera)
{
	return (bonobo_object_dup_ref (BONOBO_OBJREF (camera->priv->storage), NULL));
}

Bonobo_UIContainer
gnocam_camera_get_container (GnoCamCamera* camera)
{
	return (bonobo_object_dup_ref (BONOBO_OBJREF (camera->priv->container), NULL));
}

GConfClient*
gnocam_camera_get_client (GnoCamCamera* camera)
{
	gtk_object_ref (GTK_OBJECT (camera->priv->client));
	return (camera->priv->client);
}

/********************/
/* Gtk-Object stuff */
/********************/

GtkWidget*
gnocam_camera_new (const gchar* url, BonoboUIContainer* container, GConfClient* client, CORBA_Environment* ev)
{
	GnoCamCamera*		new;
	gint			position;
	Camera*			camera;
	BonoboStorage*		storage;
	Bonobo_Storage_OpenMode	mode;
	GtkWidget*		label;

	g_return_val_if_fail (url, NULL);
	g_return_val_if_fail (ev, NULL);
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (container), NULL);
	g_return_val_if_fail (GCONF_IS_CLIENT (client), NULL);

	/* Try to get a camera */
	CHECK_RESULT (gp_camera_new_from_gconf (&camera, url), ev);
	if (BONOBO_EX (ev)) return (NULL);
	g_return_val_if_fail (camera, NULL);
	
	/* Try to get a storage */
	mode = Bonobo_Storage_READ;
	if (gconf_client_get_bool (client, "/apps/" PACKAGE "/preview", NULL) && (camera->abilities->file_operations & GP_FILE_OPERATION_PREVIEW)) 
		mode |= Bonobo_Storage_COMPRESSED;
	if (camera->abilities->file_operations & GP_FOLDER_OPERATION_PUT_FILE) mode |= Bonobo_Storage_WRITE;
	storage = bonobo_storage_open_full_with_data ("camera", url, mode, 0664, ev, camera);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (NULL);
	}
	g_return_val_if_fail (storage, NULL);

	/* Create the widget */
	new = gtk_type_new (GNOCAM_TYPE_CAMERA);
	gtk_box_set_homogeneous (GTK_BOX (new), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (new), 2);
	
	gp_camera_ref (new->priv->camera = camera);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));
	new->priv->configuration = NULL;
	new->priv->storage = storage;
	new->priv->container = container;
	new->priv->hash_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* Callbacks for the backend */
	camera->frontend_data = (void*) new;
	gp_frontend_register (gp_frontend_status, NULL, gp_frontend_message, gp_frontend_confirm, NULL);

	/* Create the title bar */
	gtk_widget_show (new->priv->title_bar = e_shell_folder_title_bar_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->title_bar), "title_toggled", GTK_SIGNAL_FUNC (on_title_bar_toggled), new);
	gtk_box_pack_start (GTK_BOX (new), new->priv->title_bar, FALSE, FALSE, 0);

	/* Create the paned */
	gtk_widget_show (new->priv->hpaned = e_hpaned_new ());
	gtk_signal_connect (GTK_OBJECT (new->priv->hpaned), "size_request", GTK_SIGNAL_FUNC (on_size_request), new);
	gtk_box_pack_start (GTK_BOX (new), new->priv->hpaned, TRUE, TRUE, 2); 

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

	/* Create the storage view */
	new->priv->storage_view = gnocam_storage_view_new (new);
	gtk_widget_show (new->priv->storage_view);
	gtk_box_pack_start (GTK_BOX (new->priv->storage_view_vbox), new->priv->storage_view, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "directory_selected", GTK_SIGNAL_FUNC (on_directory_selected), new);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "file_selected", GTK_SIGNAL_FUNC (on_file_selected), new);

        /* Create the menu */
	new->priv->component = bonobo_ui_component_new (PACKAGE "Camera");
	bonobo_ui_component_set_container (new->priv->component, BONOBO_OBJREF (container));
	create_menu (new);

	/* Set default settings */
	position = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", NULL);
	if (position) e_paned_set_position (E_PANED (new->priv->hpaned), position);
	else gconf_client_set_int (new->priv->client, "/apps/" PACKAGE "/hpaned_position_camera", 100, NULL);
	new->priv->storage_view_mode = gconf_client_get_int (new->priv->client, "/apps/" PACKAGE "/storage_view_mode", NULL);
	gnocam_camera_set_storage_view_mode (new, new->priv->storage_view_mode);

	return (GTK_WIDGET (new));
}

static void
gnocam_camera_destroy (GtkObject* object)
{
	GnoCamCamera*  camera;

	camera = GNOCAM_CAMERA (object);

	bonobo_object_unref (BONOBO_OBJECT (camera->priv->storage));

	bonobo_object_unref (BONOBO_OBJECT (camera->priv->component));

	gtk_object_unref (GTK_OBJECT (camera->priv->client));

	if (camera->priv->configuration) gp_widget_unref (camera->priv->configuration);
	gp_camera_unref (camera->priv->camera);

	g_hash_table_destroy (camera->priv->hash_table);

	g_free (camera->priv);
	camera->priv = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_camera_init (GnoCamCamera* camera)
{
	camera->priv = g_new0 (GnoCamCameraPrivate, 1);
        camera->priv->storage_view_mode = GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN;
}

static void
gnocam_camera_class_init (GnoCamCameraClass* klass)
{
        GtkObjectClass* object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = gnocam_camera_destroy;

        signals [FOLDER_UPDATED] = gtk_signal_new ("folder_updated",
                                        GTK_RUN_FIRST,
                                        object_class->type,
                                        GTK_SIGNAL_OFFSET (GnoCamCameraClass, folder_updated),
                                        gtk_marshal_NONE__STRING,
                                        GTK_TYPE_NONE, 1,
                                        GTK_TYPE_STRING);

        gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	parent_class = gtk_type_class (PARENT_TYPE);
}

E_MAKE_TYPE (gnocam_camera, "GnoCamCamera", GnoCamCamera, gnocam_camera_class_init, gnocam_camera_init, PARENT_TYPE)


