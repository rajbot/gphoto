#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include "preferences.h"
#include "file-operations.h"
#include "gnocam.h"
#include "information.h"
#include "cameras.h"
#include "preview.h"
#include "gallery.h"
#include "callbacks.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;
extern GConfClient*	client;

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_app_destroy	(GtkObject* object, gpointer user_data);

void on_button_close_page_clicked	(GtkButton* button, gpointer user_data);

void on_button_zoom_in_clicked 		(GtkButton* button, gpointer user_data);
void on_button_zoom_out_clicked		(GtkButton* button, gpointer user_data);
void on_button_zoom_1_clicked		(GtkButton* button, gpointer user_data);
void on_button_zoom_fit_clicked		(GtkButton* button, gpointer user_data);

void on_button_save_preview_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_preview_as_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_file_clicked	(GtkButton* button, gpointer user_data);
void on_button_save_file_as_clicked 	(GtkButton* button, gpointer user_data);

void on_button_save_previews_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_previews_as_clicked	(GtkButton* button, gpointer user_data);
void on_button_save_files_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_files_as_clicked 	(GtkButton* button, gpointer user_data);
void on_button_delete_clicked 		(GtkButton* button, gpointer user_data);

void on_new_gallery_activate		(GtkMenuItem* menuitem, gpointer user_data);
void on_save_previews_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_save_previews_as_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_save_files_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_save_files_as_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_delete_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_exit_activate 			(GtkMenuItem* menuitem, gpointer user_data);
void on_manual_activate			(GtkMenuItem* menuitem, gpointer user_data);

void on_camera_tree_popup_file_delete_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_file_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_file_as_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_preview_as_activate (GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_preview_activate 	(GtkMenuItem* menuitem, gpointer user_data);

void on_camera_tree_popup_camera_manual_activate (GtkMenuItem* menuitem, gpointer user_data);

void on_properties_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_capture_preview_activate	(GtkMenuItem* menuitem, gpointer user_data);
void on_capture_image_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_capture_video_activate 		(GtkMenuItem* menuitem, gpointer user_data);

void on_camera_tree_popup_folder_upload_file_activate	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_folder_refresh_activate	(GtkMenuItem* menuitem, gpointer user_data);

void on_duration_reply (gchar *string, gpointer user_data);

/**************/
/* Callbacks. */
/**************/

void
on_app_destroy (GtkObject* object, gpointer user_data)
{
	app_clean_up ();
        gtk_main_quit ();
}

void
on_button_close_page_clicked (GtkButton* button, gpointer user_data)
{
	GtkTreeItem*	item;

	g_assert ((item = (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")))) != NULL);

	page_remove (gtk_object_get_data (GTK_OBJECT (item), "xml_page"));
        gtk_object_set_data (GTK_OBJECT (item), "xml_page", NULL);
}

void
on_button_zoom_in_clicked (GtkButton* button, gpointer user_data)
{
	gfloat	magnification = 1.1;

        pixmap_rescale (gtk_object_get_data (GTK_OBJECT (button), "pixmap"), &magnification);
}

void
on_button_zoom_out_clicked (GtkButton* button, gpointer user_data)
{
	gfloat magnification = 0.9;

        pixmap_rescale (gtk_object_get_data (GTK_OBJECT (button), "pixmap"), &magnification);
}

void
on_button_zoom_1_clicked (GtkButton* button, gpointer user_data)
{
	pixmap_rescale (gtk_object_get_data (GTK_OBJECT (button), "pixmap"), NULL);
}

void
on_button_zoom_fit_clicked (GtkButton* button, gpointer user_data)
{
	dialog_information (_("Not yet implemented!"));
}

void
on_button_save_preview_clicked (GtkButton* button, gpointer user_data)
{
	save (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), TRUE, FALSE, FALSE);
}

void
on_button_save_preview_as_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), TRUE, TRUE, FALSE);
}

void
on_button_save_file_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), FALSE, FALSE, FALSE);
}

void
on_button_save_file_as_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), FALSE, TRUE, FALSE);
}

void
on_button_save_previews_clicked (GtkButton *button, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), TRUE, FALSE, FALSE);
}

void
on_button_save_previews_as_clicked (GtkButton *button, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), TRUE, TRUE, FALSE);
}

void
on_button_save_files_clicked (GtkButton *button, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), FALSE, FALSE, FALSE);
}

void
on_button_save_files_as_clicked (GtkButton *button, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), FALSE, TRUE, FALSE);
}

void
on_button_delete_clicked (GtkButton *button, gpointer user_data)
{
	delete_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")));
}

void
on_new_gallery_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gallery_new ();
}

void
on_save_previews_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), TRUE, FALSE, FALSE);
}

void
on_save_previews_as_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), TRUE, TRUE, FALSE);
}

void
on_save_files_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), FALSE, FALSE, FALSE);
}

void
on_save_files_as_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	save_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")), FALSE, TRUE, FALSE);
}

void
on_delete_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	delete_all_selected (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")));
}

void
on_exit_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	app_clean_up ();
	gtk_main_quit ();
}

void
on_preferences_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        preferences ();
}

void
on_about_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	g_assert (glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "about") != NULL);
}

void
on_manual_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	gchar*	manualfile;

	if ((manualfile = gnome_help_file_find_file ("gnocam", "index.html"))) {
		gchar* url = g_strconcat ("file:", manualfile, NULL);
		gnome_help_goto (NULL, url);
		g_free (url);
		g_free (manualfile);
	} else {
		dialog_information (
			"Could not find the manual for " PACKAGE ". "
			"Check if it has been installed correctly in "
			"$PREFIX/share/gnome/help/gnocam.");
	}
}

void 
on_camera_tree_popup_folder_upload_file_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	upload (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), NULL);
}

void
on_camera_tree_popup_folder_refresh_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	camera_tree_folder_refresh (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item")));
}

void 
on_camera_tree_popup_file_delete_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	delete (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item")));
}

void
on_camera_tree_popup_file_save_file_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	save (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), FALSE, FALSE, FALSE);
}

void
on_camera_tree_popup_file_save_file_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	save (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), FALSE, TRUE, FALSE);
}

void
on_camera_tree_popup_file_save_preview_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	save (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), TRUE, FALSE, FALSE);
}

void
on_camera_tree_popup_file_save_preview_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	save (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), TRUE, TRUE, FALSE);
}

void
on_camera_tree_popup_camera_manual_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	Camera*		camera;
	CameraText 	manual;
	
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);

	if (gp_camera_manual (camera, &manual) == GP_OK) gnome_app_message (GNOME_APP (glade_xml_get_widget (xml, "app")), manual.text);
	else dialog_information (_("Could not get camera manual!"));
}

void
on_properties_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        Camera*         	camera;
	frontend_data_t*	frontend_data;

        g_assert (menuitem != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	if (!(frontend_data->xml_properties)) {

                /* Reference the camera. */
                gp_camera_ref (camera);
	
	        /* Get the camera properties from the backend. */
	        if (gp_camera_config (camera) != GP_OK) {
			dialog_information (_("Could not get camera properties of camera %s!"), frontend_data->name);
	        }
	}
}

void
on_capture_preview_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        GtkTreeItem*            item;
        Camera*                 camera;

        g_assert ((item = GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item"))) != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);

	preview_new (camera);
}

void
on_capture_image_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkTreeItem*		item;
	Camera*			camera;
	CameraCaptureInfo 	info;
	CameraFile*		file;

	g_assert ((item = GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item"))) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);

	/* Prepare the image. */
	info.type = GP_CAPTURE_IMAGE;
	info.duration = 0;
	file = gp_file_new ();

	/* Capture. */
	gp_camera_capture (camera, file, &info);

	/* Clean up. */
	gp_file_free (file);
	camera_tree_folder_refresh (item);
}

void
on_duration_reply (gchar *string, gpointer user_data)
{
	GtkTreeItem*		item;
	Camera*			camera;
	CameraCaptureInfo 	info;
	CameraFile*		file;

        g_assert ((item = GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (user_data), "item"))) != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);;

	if (string) {
		
		/* Prepare the video. */
		info.type = GP_CAPTURE_VIDEO;
		info.duration = atoi (string);
		file = gp_file_new ();
		
		/* Capture. */
		gp_camera_capture (camera, file, &info);
		
		/* Clean up. */
		gp_file_free (file);
		camera_tree_folder_refresh (item);
	}
}

void
on_capture_video_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	GnomeApp*	app;

	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);

	/* Ask for duration. */
	gnome_app_request_string (app, _("How long should the video be (in seconds)?"), on_duration_reply, menuitem);
}



