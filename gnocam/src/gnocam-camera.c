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
#include "gnocam-capture.h"
#include "gnocam-storage-view.h"
#include "gnocam-folder.h"
#include "gnocam-file.h"

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE
static BonoboObjectClass* gnocam_camera_parent_class = NULL;

struct _GnoCamCameraPrivate
{
	BonoboUIContainer*		container;
	BonoboUIComponent*		component;
	
	GtkWindow*			window;
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

	BonoboUIComponent*		subcomponent;
};

/****************/
/* UI-Variables */
/****************/

#define GNOCAM_CAMERA_UI 						\
"<placeholder name=\"Camera\">"						\
"  <submenu name=\"Camera\" _label=\"Camera\">"				\
"    <menuitem name=\"Manual\" _label=\"Manual\" verb=\"\"/>"		\
"    <placeholder name=\"separator\"/>"					\
"    <placeholder name=\"CapturePreview\"/>" 				\
"    <placeholder name=\"CaptureImage\"/>"				\
"    <placeholder name=\"CaptureVideo\"/>"				\
"    <placeholder name=\"Configuration\"/>"				\
"  </submenu>"								\
"</placeholder>"

#define CAPTURE_IMAGE	"<menuitem name=\"CaptureImage\" _label=\"Capture Image\" verb=\"\"/>"
#define CAPTURE_PREVIEW	"<menuitem name=\"CapturePreview\" _label=\"Capture Preview\" verb=\"\"/>"
#define CAPTURE_VIDEO	"<menuitem name=\"CaptureVideo\" _label=\"Capture Video\" verb=\"\"/>"

/**************/
/* Prototypes */
/**************/

static void 	on_title_bar_toggled 				(EShellFolderTitleBar* title_bar, gboolean state, void* data);

static void 	on_storage_view_vbox_map 			(GtkWidget* widget, void* data);
static int 	on_storage_view_vbox_button_release_event 	(GtkWidget* widget, GdkEventButton* button_event, void* data);

static void 	disconnect_popup_signals 	(GnoCamCamera* camera);
static void 	popdown_transient_folder_bar 	(GnoCamCamera* camera);

/*************/
/* Callbacks */
/*************/

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
	if (result != GP_OK) {
                gchar*  message;

                message = g_strdup_printf (_("Could not get camera manual!\n(%s)"), gp_camera_result_as_string (camera->priv->camera, result));
                gnome_error_dialog_parented (message, camera->priv->window);
                g_free (message);
                return;
        }

        gnome_ok_dialog_parented (manual.text, camera->priv->window);
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
	gint 			notebook_page;
	GdkPixbuf*		pixbuf;

	camera = GNOCAM_CAMERA (data);

	if (camera->priv->storage_view_mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (camera);

	file = g_hash_table_lookup (camera->priv->hash_table, path);
	if (!file) {
		CORBA_Environment	ev;

		CORBA_exception_init (&ev);
		file = gnocam_file_new (camera->priv->camera, camera->priv->storage, g_basename (path), camera->priv->container, &ev);
		if (BONOBO_EX (&ev)) {
			g_warning ("Could not get widget for '%s': %s", path, bonobo_exception_get_text (&ev));
			return;
		}
		CORBA_exception_free (&ev);
		g_return_if_fail (file);

		gtk_widget_show (widget = gnocam_file_get_widget (file));
		gtk_notebook_append_page (GTK_NOTEBOOK (camera->priv->notebook), widget, NULL);
		g_hash_table_insert (camera->priv->hash_table, g_strdup (path), file);
	} else {
		widget = gnocam_file_get_widget (file);
	}

	e_shell_folder_title_bar_set_title (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), path);
	if ((pixbuf = util_pixbuf_file ()))
		e_shell_folder_title_bar_set_icon (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), pixbuf);

	/* Hide the old menu */
	if (camera->priv->subcomponent) bonobo_ui_component_unset_container (camera->priv->subcomponent);
	camera->priv->subcomponent = gnocam_file_get_ui_component (file);

	/* Display the menu */
	gnocam_file_set_ui_container (file, camera->priv->container);

	notebook_page = gtk_notebook_page_num (GTK_NOTEBOOK (camera->priv->notebook), widget);
	gtk_notebook_set_page (GTK_NOTEBOOK (camera->priv->notebook), notebook_page);
}

static void
on_directory_selected (GnoCamStorageView* storage_view, const gchar* path, void* data)
{
	GnoCamCamera*		camera;
	GtkWidget*		widget;
	GnoCamFolder*		folder;
	gint			notebook_page;
	GdkPixbuf*		pixbuf;

	camera = GNOCAM_CAMERA (data);

	if (camera->priv->storage_view_mode == GNOCAM_CAMERA_STORAGE_VIEW_MODE_TRANSIENT)
		popdown_transient_folder_bar (camera);

	folder = g_hash_table_lookup (camera->priv->hash_table, path);
	if (!folder) {
		folder = gnocam_folder_new (camera->priv->camera, camera->priv->storage, path, camera->priv->container, camera->priv->client);
		g_return_if_fail (folder);

		gtk_widget_show (widget = gnocam_folder_get_widget (folder));
		gtk_notebook_append_page (GTK_NOTEBOOK (camera->priv->notebook), widget, NULL);
		g_hash_table_insert (camera->priv->hash_table, g_strdup (path), folder);
	} else {
		widget = gnocam_folder_get_widget (folder);
	}
	
	e_shell_folder_title_bar_set_title (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), path);
	if ((pixbuf = util_pixbuf_folder ()))
		e_shell_folder_title_bar_set_icon (E_SHELL_FOLDER_TITLE_BAR (camera->priv->title_bar), pixbuf);

	/* Hide the old menu */
	if (camera->priv->subcomponent) bonobo_ui_component_unset_container (camera->priv->subcomponent);
	camera->priv->subcomponent = gnocam_folder_get_ui_component (folder);

	/* Display the menu */
	gnocam_folder_set_ui_container (folder, camera->priv->container);

	notebook_page = gtk_notebook_page_num (GTK_NOTEBOOK (camera->priv->notebook), widget);
	gtk_notebook_set_page (GTK_NOTEBOOK (camera->priv->notebook), notebook_page);
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

/**********************/
/* Internal functions */
/**********************/

static int 
gp_frontend_status (Camera* camera, char* status)
{
	GnoCamCamera*	c;

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

/*************************/
/* Bonobo-X-Object stuff */
/*************************/

GnoCamCamera*
gnocam_camera_new (const gchar* url, BonoboUIContainer* container, GtkWindow* parent, GConfClient* client, CORBA_Environment* ev)
{
	GnoCamCamera*		new;
	gchar*			name;
	gint			position;
	Camera*			camera;
	Bonobo_Storage		storage;
	GtkWidget*		scroll_frame;
	GtkWidget*		label;

	g_return_val_if_fail (url, NULL);
	g_return_val_if_fail (container, NULL);
	g_return_val_if_fail (parent, NULL);
	g_return_val_if_fail (ev, NULL);

	/* Try to get a camera */
	CHECK_RESULT (gp_camera_new_from_gconf (&camera, url), ev);
	if (BONOBO_EX (ev)) return (NULL);
	g_return_val_if_fail (camera, NULL);
	
	/* Try to get a storage */
	storage = bonobo_get_object (url, "IDL:Bonobo/Storage:1.0", ev);
	if (BONOBO_EX (ev)) {
		gp_camera_unref (camera);
		return (NULL);
	}
	g_return_val_if_fail (storage != CORBA_OBJECT_NIL, NULL);

	new = gtk_type_new (GNOCAM_TYPE_CAMERA);
	new->priv->window = parent;
	new->priv->storage = storage;
	gp_camera_ref (new->priv->camera = camera);
	gtk_object_ref (GTK_OBJECT (new->priv->client = client));
	new->priv->configuration = NULL;
	new->priv->container = container;
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
	gtk_widget_show (new->priv->storage_view = gnocam_storage_view_new (storage));
	gtk_container_add (GTK_CONTAINER (scroll_frame), new->priv->storage_view);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "directory_selected", GTK_SIGNAL_FUNC (on_directory_selected), new);
	gtk_signal_connect (GTK_OBJECT (new->priv->storage_view), "file_selected", GTK_SIGNAL_FUNC (on_file_selected), new);

        /* Create the menu */
	new->priv->component = bonobo_ui_component_new (PACKAGE "Camera");
	bonobo_ui_component_set_container (new->priv->component, BONOBO_OBJREF (new->priv->container));
//	bonobo_object_unref (BONOBO_OBJECT (new->priv->container));
	bonobo_ui_component_set_translate (new->priv->component, "/menu", GNOCAM_CAMERA_UI, NULL);
        bonobo_ui_component_add_verb (new->priv->component, "Manual", on_manual_clicked, new);

        /* Camera Configuration? */
        if (new->priv->camera->abilities->config) {
                gint            result;

                result = gp_camera_config_get (new->priv->camera, &(new->priv->configuration));
		if (result == GP_OK) menu_setup (new->priv->component, new->priv->camera, new->priv->configuration, "/menu/Camera/Camera", NULL, NULL);
        }

        /* Capture? */
        if (new->priv->camera->abilities->capture != GP_CAPTURE_NONE) {
		bonobo_ui_component_set_translate (new->priv->component, "/menu/Camera/Camera/separator", "<separator name=\"separator\"/>", NULL);
        }
        if (new->priv->camera->abilities->capture & GP_CAPTURE_IMAGE) {
		bonobo_ui_component_set_translate (new->priv->component, "/menu/Camera/Camera/CaptureImage", CAPTURE_IMAGE, NULL);
                bonobo_ui_component_add_verb (new->priv->component, "CaptureImage", on_capture_image_clicked, new);
        }
        if (new->priv->camera->abilities->capture & GP_CAPTURE_VIDEO) {
		bonobo_ui_component_set_translate (new->priv->component, "/menu/Camera/Camera/CaptureVideo", CAPTURE_VIDEO, NULL);
                bonobo_ui_component_add_verb (new->priv->component, "CaptureVideo", on_capture_video_clicked, new);
        }
        if (new->priv->camera->abilities->capture & GP_CAPTURE_PREVIEW) {
		bonobo_ui_component_set_translate (new->priv->component, "/menu/Camera/Camera/CapturePreview", CAPTURE_PREVIEW, NULL);
                bonobo_ui_component_add_verb (new->priv->component, "CapturePreview", on_capture_preview_clicked, new);
        }

	/* Select the selected file/folder */
	name = (gchar*) url + 9;
	for (; *name != 0; name++) if (*name == '/') break;
	gtk_signal_emit_by_name (GTK_OBJECT (new->priv->storage_view), "directory_selected", name);
//The menu doesn't get added. This seems to be because it is added during a signal execution. The code below proves all...
//	{
//		GnoCamFolder*	folder = gnocam_folder_new (new->priv->camera, new->priv->storage, name, new->priv->container, new->priv->client);
//		gint		page;
//		gtk_widget_show (label = gnocam_folder_get_widget (folder));
//		gtk_notebook_append_page (GTK_NOTEBOOK (new->priv->notebook), label, NULL);
//		page = gtk_notebook_page_num (GTK_NOTEBOOK (new->priv->notebook), label);
//	        gtk_notebook_set_page (GTK_NOTEBOOK (new->priv->notebook), page);
//	}

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

	Bonobo_Storage_unref (camera->priv->storage, NULL);
	bonobo_object_unref (BONOBO_OBJECT (camera->priv->component));
	gtk_object_unref (GTK_OBJECT (camera->priv->client));
	if (camera->priv->configuration) gp_widget_unref (camera->priv->configuration);
	gp_camera_unref (camera->priv->camera);

	g_hash_table_destroy (camera->priv->hash_table);

	g_free (camera->priv);
}

static void
gnocam_camera_init (GnoCamCamera* camera)
{
	camera->priv = g_new (GnoCamCameraPrivate, 1);
        camera->priv->camera = NULL;
        camera->priv->configuration = NULL;
        camera->priv->hpaned_position = 0;
        camera->priv->title_bar = NULL;
        camera->priv->hpaned = NULL;
	camera->priv->notebook = NULL;
        camera->priv->storage_view_vbox = NULL;
        camera->priv->storage_view_title_bar = NULL;
        camera->priv->storage_view = NULL;
        camera->priv->storage_view_mode = GNOCAM_CAMERA_STORAGE_VIEW_MODE_HIDDEN;
	camera->priv->subcomponent = NULL;
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



