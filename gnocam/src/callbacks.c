#include <config.h>
#include <gnome.h>
#ifdef GNOCAM_USES_GTKHTML
#  include <pspell/pspell.h>
#  include <gtkhtml/gtkhtml.h>
#endif
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include "preferences.h"
#include "file-operations.h"
#include "gnocam.h"
#include "callbacks.h"
#include "information.h"
#include "cameras.h"
#include "frontend.h"
#include "preview.h"

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
void on_preferences_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_about_activate			(GtkMenuItem* menuitem, gpointer user_data);
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
#ifdef GNOCAM_USES_GTKHTML
	GladeXML*	xml_gallery;
	GtkTargetEntry	target_table[] = {{"text/uri-list", 0, 0}};

	g_assert ((xml_gallery = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app_gallery")) != NULL);

	/* Store some data. */
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_gallery, "app_gallery_close")), "xml_gallery", xml_gallery);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_gallery, "app_gallery_open")), "xml_gallery", xml_gallery);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_gallery, "app_gallery_save_as")), "xml_gallery", xml_gallery);
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_gallery, "app_gallery_button_save_as")), "xml_gallery", xml_gallery);

	/* Connect the signals. */
	glade_xml_signal_autoconnect (xml_gallery);

	/* Drag and Drop. */
	gtk_drag_dest_set (glade_xml_get_widget (xml_gallery, "editor"), GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
#else
	dialog_information (_("You need to compile " PACKAGE " with gtkhtml enabled in order to use this feature."));
#endif
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

/***********/
/* Pop ups */
/***********/

gboolean
on_tree_item_file_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
        GladeXML*	xml_popup;
        Camera*		camera;
	gchar*		path;
	gchar*		filename;

        g_assert (event != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (widget), "camera")) != NULL);
	g_assert ((filename = gtk_object_get_data (GTK_OBJECT (widget), "filename")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (widget), "path")) != NULL);

        /* Did the user right-click? */
        if (event->button == 3) {

                /* Create the dialog. */
                g_assert ((xml_popup = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "camera_tree_popup_file")) != NULL);

                /* Store some data. */
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_file_save_preview")), "item", widget);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_file_save_preview_as")), "item", widget);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_file_save_file")), "item", widget);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_file_save_file_as")), "item", widget);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_file_delete")), "item", widget);

                /* Connect the signals. */
                glade_xml_signal_autoconnect (xml_popup);

                /* Pop up the dialog. */
                gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "camera_tree_popup_file")), NULL, NULL, NULL, NULL, event->button, event->time);

                return (TRUE);

        } else return (FALSE);
}

gboolean
on_tree_item_folder_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
        GladeXML*       xml_popup;
        Camera*         camera;
        gchar*          path;

        g_assert (event != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (widget), "camera")) != NULL);
        g_assert ((path = gtk_object_get_data (GTK_OBJECT (widget), "path")) != NULL);

        /* Did the user right-click? */
        if (event->button == 3) {

                /* Create the dialog. */
		g_assert ((xml_popup = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "camera_tree_popup_folder")) != NULL);

                /* Store some data. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_folder_upload_file")), "item", widget);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_folder_refresh")), "item", widget);
                
		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_popup);

		/* Pop up the dialog. */
		gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "camera_tree_popup_folder")), NULL, NULL, NULL, NULL, event->button, event->time);

		return (TRUE);

	} else return (FALSE);
}

gboolean
on_tree_item_camera_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
        GladeXML*       xml_popup;
        Camera*         camera;
        gchar*          path;

        g_assert (event != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (widget), "camera")) != NULL);
        g_assert ((path = gtk_object_get_data (GTK_OBJECT (widget), "path")) != NULL);

        /* Did the user right-click? */
        if (event->button == 3) {

                /* Create the dialog. */
                g_assert ((xml_popup = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "camera_tree_popup_camera")) != NULL);

                /* Store some data. */
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_capture_video")), "item", widget);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_capture_image")), "item", widget);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_capture_preview")), "item", widget);
		gtk_object_set_data  (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_manual")), "camera", camera);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_properties")), "camera", camera);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_upload_file")), "item", widget);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_refresh")), "item", widget);

                /* Connect the signals. */
                glade_xml_signal_autoconnect (xml_popup);

                /* Pop up the dialog. */
                gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera")), NULL, NULL, NULL, NULL, event->button, event->time);

                return (TRUE);

        } else return (FALSE);
}

void
on_tree_item_expand (GtkTreeItem* tree_item, gpointer user_data)
{
	if (!gtk_object_get_data (GTK_OBJECT (tree_item), "populated")) camera_tree_folder_populate (tree_item);
}

void
on_tree_item_collapse (GtkTreeItem* tree_item, gpointer user_data)
{
	/* We currently don't do anything here. */
}

void
on_tree_item_select (GtkTreeItem* item, gpointer user_data)
{
	GladeXML*		xml_page = NULL;
	GtkNotebook*		notebook;
	gchar*			filename;
	gchar*			path;
	gchar*			count;
	GtkWidget*		page;
	GtkWidget*		label;
	CameraFile*		file;
	Camera*			camera;
	CameraText		cameratext;
	GtkPixmap*		pixmap;

	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert (gtk_object_get_data (GTK_OBJECT (item), "page") == NULL);

	/* Folder or file? */
	if ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {
		
		/* We've got a file. */
		g_assert ((xml_page = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "table_file")) != NULL);
		g_assert ((page = glade_xml_get_widget (xml_page, "table_file")) != NULL);
		g_assert ((pixmap = GTK_PIXMAP (glade_xml_get_widget (xml_page, "pixmap_preview"))) != NULL);
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_page, "entry_filename")), filename);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_zoom_in")), "pixmap", pixmap);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_zoom_out")), "pixmap", pixmap);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_zoom_1")), "pixmap", pixmap);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_zoom_fit")), "pixmap", pixmap);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_close_page")), "item", item);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_save_file")), "item", item);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_save_file_as")), "item", item);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_save_preview")), "item", item);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_page, "button_save_preview_as")), "item", item);

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_page);

		/* This is because libglade takes the GNOME default for the toolbar style. We don't want that. */
		gtk_toolbar_set_style (GTK_TOOLBAR (glade_xml_get_widget (xml_page, "toolbar_close")), GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_style (GTK_TOOLBAR (glade_xml_get_widget (xml_page, "toolbar_save")), GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_style (GTK_TOOLBAR (glade_xml_get_widget (xml_page, "toolbar_zoom")), GTK_TOOLBAR_ICONS);

		/* Do we already have the preview? */
		if (!(file = gtk_object_get_data (GTK_OBJECT (item), "preview"))) {
			file = gp_file_new ();
		        if (gp_camera_file_get_preview (camera, file, path, filename) != GP_OK) {
				if (strcmp ("/", path) == 0) dialog_information (_("Could not get preview of file '/%s' from the camera!"), filename);
				else dialog_information (_("Could not get preview of file '%s/%s' from the camera!"), path, filename);
				gp_file_free (file);
				file = NULL;
			} else {
				gtk_object_set_data (GTK_OBJECT (item), "preview", file);
			}
			gp_frontend_progress (camera, NULL, 0.0);
		}

		pixmap_set (GTK_PIXMAP (glade_xml_get_widget (xml_page, "pixmap_preview")), file);
		label = gtk_label_new (filename);

	} else {
	
		/* We've got a folder. */
		if (strcmp ("/", path) == 0) {

			/* This is the root folder. */
			g_assert ((xml_page = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "table_camera")) != NULL);
			g_assert ((page = glade_xml_get_widget (xml_page, "table_camera")) != NULL);
			if (gp_camera_summary (camera, &cameratext) != GP_OK) strcpy ("?", (gchar*) &cameratext);
			gnome_less_show_string (GNOME_LESS (glade_xml_get_widget (xml_page, "less")), (gchar*) &cameratext);
		} else {
			
			/* This is a non-root folder. */
			g_assert ((xml_page = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "table_folder")) != NULL);
	                g_assert ((page = glade_xml_get_widget (xml_page, "table_folder")) != NULL);
		}
                count = g_strdup_printf ("%i", GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "folder_list_count")));
                gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_page, "entry_folders")), count);
		g_free (count);
		count = g_strdup_printf ("%i", GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "file_list_count")));
                gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_page, "entry_files")), count);
		g_free (count);
		label = gtk_label_new (path);
	}

	/* Common for all pages. */
	gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_page, "entry_camera")), ((frontend_data_t*) camera->frontend_data)->name);
	gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (xml_page, "entry_path")), "/");
	gtk_notebook_append_page (notebook, page, label);
	gtk_notebook_set_page (notebook, gtk_notebook_page_num (notebook, page));
	gtk_object_set_data (GTK_OBJECT (item), "xml_page", xml_page);
}

void
on_tree_item_deselect (GtkTreeItem* item, gpointer user_data)
{
	GladeXML*	xml_page;

	if ((xml_page = gtk_object_get_data (GTK_OBJECT (item), "xml_page"))) {
		gtk_object_set_data (GTK_OBJECT (item), "xml_page", NULL);
		page_remove (xml_page);
	}
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
	GtkTreeItem*		item;
	GladeXML*		xml_preview;
	Camera*			camera;
	frontend_data_t*	frontend_data;

	g_assert (menuitem != NULL);
	g_assert ((item = GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item"))) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	if (!(frontend_data->xml_preview)) {
		
		/* Open the preview window. */
		g_assert ((xml_preview = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app_preview")) != NULL);
		frontend_data->xml_preview = xml_preview;

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_capture_image")), "item", item);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_capture_video")), "item", item);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_properties")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_refresh")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_save")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_save_as")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_close")), "xml_preview", xml_preview);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_exit")), "xml_preview", xml_preview);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_refresh")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_save")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_save_as")), "camera", camera);

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_preview);

		/* Reference the camera. */
		gp_camera_ref (camera);

		/* Get a preview. */
		preview_refresh (camera);
	}
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



