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

void on_button_rotate_clicked (GtkButton* button, gpointer user_data);

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
on_button_save_preview_clicked (GtkButton* button, gpointer user_data)
{
	save (GTK_TREE_ITEM (user_data), TRUE, FALSE, FALSE);
}

void
on_button_save_preview_as_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (user_data), TRUE, TRUE, FALSE);
}

void
on_button_save_file_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (user_data), FALSE, FALSE, FALSE);
}

void
on_button_save_file_as_clicked (GtkButton* button, gpointer user_data)
{
        save (GTK_TREE_ITEM (user_data), FALSE, TRUE, FALSE);
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

	g_assert ((xml_gallery = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app_gallery")) != NULL);

	/* Store some data. */
	gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_gallery, "app_gallery_close")), "xml_gallery", xml_gallery);

	/* Connect the signals. */
	glade_xml_signal_autoconnect (xml_gallery);
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
	GtkNotebook*		notebook;
	gchar*			filename;
	gchar*			path;
	gchar*			text;
	gchar*			contents = NULL;
//	gchar*			list_text[1];
	GtkWidget*		page;
	GtkWidget*		label;
	GtkWidget*		widget;
	GtkWidget*		window;
	GtkWidget*		viewport;
	GtkWidget*		hbox;
	GtkWidget*		vbox;
	GtkWidget*		toolbar;
	GtkWidget*		button;
	CameraFile*		file;
	Camera*			camera;
	GdkPixbuf*		pixbuf;
        GdkPixmap*		pixmap;
        GdkBitmap*		bitmap;
	CameraText*		buffer;
	gint			folder_count, file_count;

	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert (gtk_object_get_data (GTK_OBJECT (item), "page") == NULL);

	/* Folder or file? */
	if ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {

		/* Set up the basic structure of the notebook page. */
                page = gtk_vbox_new (FALSE, 5);
		gtk_container_set_border_width (GTK_CONTAINER (page), 5);
                hbox = gtk_hbox_new (FALSE, 5);
                gtk_container_add (GTK_CONTAINER (page), hbox);

                /* Basic description. */
                label = gtk_label_new ("Camera:\nPath:\nFilename:");
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
                text = g_strdup_printf ("%s\n%s\n%s", ((frontend_data_t*) camera->frontend_data)->name, path, filename);
                label = gtk_label_new (text);
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
                g_free (text);

                /* Widget for preview. */
		vbox = gtk_vbox_new (FALSE, 5);
		gtk_container_add (GTK_CONTAINER (hbox), vbox);
		toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH);
		gtk_box_pack_start (GTK_BOX (vbox), toolbar, TRUE, TRUE, 0);
		button = gtk_toolbar_append_element (
			GTK_TOOLBAR (toolbar), 
			GTK_TOOLBAR_CHILD_BUTTON, 
			NULL, 
			NULL,
			"Rotate Left", 
			NULL, 
			gnome_stock_pixmap_widget (glade_xml_get_widget (xml, "app"), GNOME_STOCK_PIXMAP_REDO), 
			GTK_SIGNAL_FUNC (on_button_rotate_clicked), 
			GINT_TO_POINTER (0));
		gtk_object_set_data (GTK_OBJECT (button), "item", item);
		button = gtk_toolbar_append_element (
                        GTK_TOOLBAR (toolbar), 
                        GTK_TOOLBAR_CHILD_BUTTON, 
                        NULL,
			NULL, 
                        "Rotate Right", 
                        NULL, 
                        gnome_stock_pixmap_widget (glade_xml_get_widget (xml, "app"), GNOME_STOCK_PIXMAP_UNDO), 
                        GTK_SIGNAL_FUNC (on_button_rotate_clicked), 
                        GINT_TO_POINTER (1));
		gtk_object_set_data (GTK_OBJECT (button), "item", item);
		button = gtk_toolbar_append_element (
                        GTK_TOOLBAR (toolbar),
                        GTK_TOOLBAR_CHILD_BUTTON,
                        NULL,
			NULL,
                        "Save Preview",
                        NULL,
                        gnome_stock_pixmap_widget (glade_xml_get_widget (xml, "app"), GNOME_STOCK_PIXMAP_SAVE),
                        GTK_SIGNAL_FUNC (on_button_save_preview_clicked),
			item);
		button = gtk_toolbar_append_element (
                        GTK_TOOLBAR (toolbar),
                        GTK_TOOLBAR_CHILD_BUTTON,
                        NULL,
			NULL,
                        "Save Preview As",
			NULL,
			gnome_stock_pixmap_widget (glade_xml_get_widget (xml, "app"), GNOME_STOCK_PIXMAP_SAVE_AS),
			GTK_SIGNAL_FUNC (on_button_save_preview_as_clicked),
                        item);
		button = gtk_toolbar_append_element (
                        GTK_TOOLBAR (toolbar),
                        GTK_TOOLBAR_CHILD_BUTTON,
                        NULL,
			NULL,
			"Save File",
			NULL,
			gnome_stock_pixmap_widget (glade_xml_get_widget (xml, "app"), GNOME_STOCK_PIXMAP_SAVE),
			GTK_SIGNAL_FUNC (on_button_save_file_clicked),
			item);
		button = gtk_toolbar_append_element (
                        GTK_TOOLBAR (toolbar),
                        GTK_TOOLBAR_CHILD_BUTTON,
                        NULL,
			NULL,
                        "Save File As",
                        NULL,
                        gnome_stock_pixmap_widget (glade_xml_get_widget (xml, "app"), GNOME_STOCK_PIXMAP_SAVE_AS),
			GTK_SIGNAL_FUNC (on_button_save_file_as_clicked),
			item);
                window = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (vbox), window, TRUE, TRUE, 0);
                viewport = gtk_viewport_new (NULL, NULL);
                gtk_container_add (GTK_CONTAINER (window), viewport);

		/* Do we already have the preview? */
		if (!(file = gtk_object_get_data (GTK_OBJECT (item), "preview"))) {
			file = gp_file_new ();
		        if (gp_camera_file_get_preview (camera, file, path, filename) != GP_OK) {
				if (strcmp ("/", path) == 0) dialog_information (_("Could not get preview of file '/%s' from the camera!"), filename);
				else dialog_information (_("Could not get preview of file '%s/%s' from the camera!"), path, filename);
				gtk_container_add (GTK_CONTAINER (viewport), gtk_label_new ("?"));
				gp_file_free (file);
				file = NULL;
			} else {
				gtk_object_set_data (GTK_OBJECT (item), "preview", file);
			}
			gp_frontend_progress (camera, NULL, 0.0);
		}

		if (file) {
			
			/* Create a fake pixmap. */
			pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);
			gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 127);
	                widget = gtk_pixmap_new (pixmap, bitmap);
			gtk_object_set_data (GTK_OBJECT (widget), "pixbuf", pixbuf);
	                gtk_container_add (GTK_CONTAINER (viewport), widget);
			gtk_object_set_data (GTK_OBJECT (item), "pixmap", widget);

			/* Render the preview. */
			update_pixmap (GTK_PIXMAP (widget), file);
	
			/* Clist for exif tags. */
//			widget = gtk_clist_new (1);
//			gtk_container_add (GTK_CONTAINER (page), widget);
//			list_text[0] = g_strdup ("This list will display exif tags.");
//			gtk_clist_append (GTK_CLIST (widget), list_text);
//			g_free (list_text[0]);

		}
		label = gtk_label_new (filename);

	} else {
	
		/* We've got a folder. */
		folder_count = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "folder_list_count"));
		file_count = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "file_list_count"));
		if ((folder_count == 0) && (file_count == 0)) 
			contents = g_strdup_printf (_("Folder '%s' does neither contain folders nor files."), path);
		else if ((folder_count == 1) && (file_count == 0)) 
			contents = g_strdup_printf (_("Folder '%s' contains 1 folder."), path);
		else if ((folder_count > 1) && (file_count == 0))
			contents = g_strdup_printf (_("Folder '%s' contains %i folders."), path, folder_count);
		else if ((folder_count == 0) && (file_count == 1))
			contents = g_strdup_printf (_("Folder '%s' contains 1 file."), path);
		else if ((folder_count == 1) && (file_count == 1))
			contents = g_strdup_printf (_("Folder '%s' contains 1 folder and 1 file."), path);
		else if ((folder_count > 1) && (file_count == 1))
			contents = g_strdup_printf (_("Folder '%s' contains %i folders and 1 file."), path, folder_count);
		else if ((folder_count == 0) && (file_count > 1))
			contents = g_strdup_printf (_("Folder '%s' contains %i files."), path, file_count);
		else if ((folder_count == 1) && (file_count > 1))
			contents = g_strdup_printf (_("Folder '%s' contains 1 folder and %i files."), path, file_count);
		else if ((folder_count > 1) && (file_count > 1))
			contents = g_strdup_printf (_("Folder '%s' contains %i folders and %i files. "), path, folder_count, file_count);
		else g_assert_not_reached ();
		if (strcmp ("/", path) == 0) {
			buffer = g_new0 (CameraText, 1);
			if (gp_camera_summary (camera, buffer) != GP_OK) strcpy ("?", (gchar*) buffer);
			text = g_strdup_printf (_("%s\n\nCamera summary:\n%s"), contents, buffer);
			g_free (buffer);
			g_free (contents);
		} else {
			text = contents;
		}
		page = gtk_label_new (text);
		gtk_label_set_justify (GTK_LABEL (page), GTK_JUSTIFY_LEFT);
		g_free (text);
		
		label = gtk_label_new (path);
	}

	gtk_widget_show_all (page);
	gtk_notebook_append_page (notebook, page, label);
	gtk_notebook_set_page (notebook, gtk_notebook_page_num (notebook, page));
	gtk_object_set_data (GTK_OBJECT (item), "page", page);
}

void
on_tree_item_deselect (GtkTreeItem* item, gpointer user_data)
{
        GtkNotebook*    notebook;
	GtkWidget*	page;

	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);
	g_assert ((page = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (item), "page"))) != NULL);

	gtk_object_set_data (GTK_OBJECT (item), "page", NULL);
	gtk_notebook_remove_page (notebook, gtk_notebook_page_num (notebook, page));
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

void
on_button_rotate_clicked (GtkButton* button, gpointer user_data)
{
	GtkTreeItem*	item;
	GtkPixmap*	widget;
	GdkPixbuf*	pixbuf;
	GdkPixbuf*	pixbuf_old;

//FIXME: Where is gdk_pixbuf_rotate? Not implemented. 
	dialog_information (_("Not yet implemented!"));
	g_assert ((item = gtk_object_get_data (GTK_OBJECT (button), "item")) != NULL);
	if ((widget = gtk_object_get_data (GTK_OBJECT (item), "pixmap"))) {
		g_assert ((pixbuf = gtk_object_get_data (GTK_OBJECT (widget), "pixbuf")) != NULL);
		pixbuf_old = pixbuf;
//		if (user_data) pixbuf = gdk_pixbuf_rotate (pixbuf, 95.0);
//		else pixbuf = gdk_pixbuf_rotate (pixbuf, -95.0);
//		gdk_pixbuf_unref (pixbuf);
//		gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 127);
//		gtk_pixmap_set (GTK_PIXMAP (widget), pixmap, bitmap);
	}
}


