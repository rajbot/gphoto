#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs.h>
#include "file-operations.h"
#include "gnocam.h"
#include "cameras.h"
#include "preview.h"
#include "utils.h"

/**********************/
/* External Variables */
/**********************/

extern GConfClient*	gconf_client;
extern GtkTree*		main_tree;
extern GtkWindow*	main_window;
extern GnoCamViewMode	view_mode;

/**************/
/* Prototypes */
/**************/

void on_button_save_preview_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_preview_as_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_file_clicked	(GtkButton* button, gpointer user_data);
void on_button_save_file_as_clicked 	(GtkButton* button, gpointer user_data);

void on_camera_tree_popup_file_delete_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_file_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_file_as_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_preview_as_activate (GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_preview_activate 	(GtkMenuItem* menuitem, gpointer user_data);

void on_camera_tree_popup_camera_manual_activate (GtkMenuItem* menuitem, gpointer user_data);

void on_camera_tree_popup_folder_upload_file_activate	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_folder_refresh_activate	(GtkMenuItem* menuitem, gpointer user_data);

void on_properties_activate 				(GtkMenuItem* menuitem, gpointer user_data);

/**************/
/* Callbacks. */
/**************/

void
on_button_save_preview_clicked (GtkButton* button, gpointer user_data)
{
	save (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), TRUE, FALSE);
}

void
on_button_save_preview_as_clicked (GtkButton* button, gpointer user_data)
{
        download (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), NULL, TRUE);
}

void
on_button_save_file_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), FALSE, FALSE);
}

void
on_button_save_file_as_clicked (GtkButton* button, gpointer user_data)
{
        download (GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (button), "item")), NULL, FALSE);
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
	save (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), FALSE, FALSE);
}

void
on_camera_tree_popup_file_save_file_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	download (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), NULL, FALSE);
}

void
on_camera_tree_popup_file_save_preview_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	save (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), TRUE, FALSE);
}

void
on_camera_tree_popup_file_save_preview_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	download (gtk_object_get_data (GTK_OBJECT (menuitem), "item"), NULL, TRUE);
}

void
on_camera_tree_popup_camera_manual_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	Camera*		camera;
	CameraText 	manual;
	gchar*		message;
	gint		result;
	
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);

	if ((result = gp_camera_manual (camera, &manual)) == GP_OK) gnome_ok_dialog_parented (manual.text, main_window);
	else {
		message = g_strdup_printf ("Could not get camera manual!\n(%s)", gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
	}
}

void
on_properties_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	properties (gtk_object_get_data (GTK_OBJECT (menuitem), "camera"));
}


