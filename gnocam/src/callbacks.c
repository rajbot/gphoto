#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gphoto2.h>
#include <gconf/gconf-client.h>
#include "preferences.h"
#include "save.h"
#include "gnocam.h"
#include "callbacks.h"
#include "information.h"
#include "cameras.h"
#include "preview.h"
#include "gphoto-extensions.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;
extern GConfClient*	client;

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_button_save_previews_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_previews_as_clicked	(GtkButton* button, gpointer user_data);
void on_button_save_files_clicked 	(GtkButton* button, gpointer user_data);
void on_button_save_files_as_clicked 	(GtkButton* button, gpointer user_data);
void on_button_delete_clicked 		(GtkButton* button, gpointer user_data);

void on_save_previews_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_save_previews_as_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_save_files_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_save_files_as_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_delete_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_exit_activate 			(GtkMenuItem* menuitem, gpointer user_data);
void on_preferences_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_about_activate 			(GtkMenuItem* menuitem, gpointer user_data);

//void on_clist_files_drag_data_get (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer data);

void on_camera_tree_popup_file_delete_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_file_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_file_as_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_preview_as_activate (GtkMenuItem* menuitem, gpointer user_data);
void on_camera_tree_popup_file_save_preview_activate 	(GtkMenuItem* menuitem, gpointer user_data);

void on_properties_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_capture_preview_activate	(GtkMenuItem* menuitem, gpointer user_data);
void on_capture_image_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_capture_video_activate 		(GtkMenuItem* menuitem, gpointer user_data);

void on_camera_tree_popup_folder_upload_file_activate		(GtkMenuItem* menuitem, gpointer user_data);

void on_tree_item_deselect (GtkTreeItem* item, gpointer user_data);
void on_tree_item_select (GtkTreeItem* item, gpointer user_data);

void on_duration_reply (gchar *string, gpointer user_data);

void on_app_preview_close_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_save_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_save_as_activate 	(GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_refresh_activate (GtkMenuItem* menuitem, gpointer user_data);

void on_app_preview_button_refresh_clicked 	(GtkButton* button, gpointer user_data);
void on_app_preview_button_save_clicked 	(GtkButton* button, gpointer user_data);
void on_app_preview_button_save_as_clicked 	(GtkButton* button, gpointer user_data);

/**************/
/* Callbacks. */
/**************/

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
on_preferences_activate (GtkMenuItem *menuitem, gpointer user_data)
{
        preferences ();
}

void
on_about_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	g_assert (glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "about") != NULL);
}

//void
//on_clist_files_drag_data_get (
//	GtkWidget *widget, 
//	GdkDragContext *context, 
//	GtkSelectionData *selection_data, 
//	guint info, 
//	guint time, 
//	gpointer data)
//{
//	GladeXML *xml;
//	gchar *filenames, *filenames_old;
//	GList *selection;
//	GtkCList *clist;
//	gchar *file_name;
//	gint i, row;

//	xml = gtk_object_get_data (GTK_OBJECT (widget), "xml");
//	g_assert (xml != NULL);
//	clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_files"));
//	g_assert (clist != NULL);

        /* Check which files have been selected. */
//	selection = g_list_first (clist->selection);
//	filenames = g_strdup ("");
//      for (i = 0; i < g_list_length (selection); i++) {
//	row = GPOINTER_TO_INT (g_list_nth_data (selection, i));
//	      gtk_clist_get_text (clist, row, 2, &file_name);
//		g_assert (file_name != NULL);
//		filenames_old = filenames;
//		if (i == 0) filenames = g_strdup_printf ("file://tmp/%s\n", g_strdup (file_name));
//		else filenames = g_strdup_printf ("%sfile://tmp/%s\n", filenames_old, g_strdup (file_name));
//		g_free (filenames_old);
//	}

	/* Calculate size of variable filenames. */
//	for (i =0; ; i++) if (filenames[i] == 0) break;

	//FIXME: Getting files takes too much time. Previews work...
//	save_all_selected (xml, FALSE, FALSE, TRUE);
//	gtk_selection_data_set (selection_data, selection_data->target, 8, filenames, i + 1);
//}

void 
on_camera_tree_popup_folder_upload_file_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        GtkTreeItem*    item;
	Camera*		camera;
	gchar*		path;

        g_assert ((item = GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item"))) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);

	upload (camera, path, NULL);
}

void 
on_camera_tree_popup_file_delete_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	GtkTreeItem*	item;

	g_assert ((item = GTK_TREE_ITEM (gtk_object_get_data (GTK_OBJECT (menuitem), "item"))) != NULL);

	delete (item);
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
                
		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_popup);

		/* Pop up the dialog. */
		gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "camera_tree_popup_folder")), NULL, NULL, NULL, NULL, event->button, event->time);

		return (TRUE);

	} else return (FALSE);
}

void
on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time)
{
        GList*			filenames;
        guint 			i;
        gchar*			path;
	Camera*			camera;

	g_assert ((path = gtk_object_get_data (GTK_OBJECT (widget), "path")) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (widget), "camera")) != NULL);

        filenames = gnome_uri_list_extract_filenames (selection_data->data);
        for (i = 0; i < g_list_length (filenames); i++) {
		upload (camera, path, g_list_nth_data (filenames, i));
        }
        gnome_uri_list_free_strings (filenames);
}

void
on_tree_item_expand (GtkTreeItem* tree_item, gpointer user_data)
{
	CameraList		folder_list, file_list;
	CameraListEntry*	folder_list_entry;
	CameraListEntry*	file_list_entry;
	Camera*			camera;
	gchar*			path;
	gchar*			new_path;
	gint			folder_list_count, file_list_count;
	gint			i;
	GtkWidget*		item;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (tree_item), "camera")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (tree_item), "path")) != NULL);
	g_assert (tree_item->subtree);
	g_assert (GTK_OBJECT (tree_item->subtree)->ref_count > 0);

	/* Get file and folder list. */
	if (gp_camera_folder_list (camera, &folder_list, path) != GP_OK) {
		dialog_information ("Could not get folder list for folder '%s'!", path);
		return;
	}
	if (gp_camera_file_list (camera, &file_list, path) != GP_OK) {
		dialog_information ("Could not get file list for folder '%s'!", path);
		return;
	}

	/* Add folders to tree. */
        folder_list_count = gp_list_count (&folder_list);
	if (folder_list_count > 0) {
		for (i = 0; i < folder_list_count; i++) {
			folder_list_entry = gp_list_entry (&folder_list, i);
			
                        /* Construct the new path. */
                        if (strcmp (path, "/") == 0) new_path = g_strdup_printf ("/%s", folder_list_entry->name);
                        else new_path = g_strdup_printf ("%s/%s", path, folder_list_entry->name);

			/* Add the folder to the tree. */
			camera_tree_folder_add (GTK_TREE (tree_item->subtree), camera, new_path);
			
			/* Clean up. */
			g_free (new_path);
		}
	}

	/* Add files to tree. */
	file_list_count = gp_list_count (&file_list);
	if (file_list_count > 0) {
		for (i = 0; i < file_list_count; i++) {
			file_list_entry = gp_list_entry (&file_list, i);

			/* Add the file to the tree. */
			item = gtk_tree_item_new_with_label (file_list_entry->name);
			gtk_widget_show (item);
			gtk_tree_append (GTK_TREE (tree_item->subtree), item);

			/* Store some data. */
			gtk_object_set_data (GTK_OBJECT (item), "camera", camera);
			gtk_object_set_data (GTK_OBJECT (item), "path", g_strdup (path));
			gtk_object_set_data (GTK_OBJECT (item), "filename", g_strdup (file_list_entry->name));
			gtk_object_set_data (GTK_OBJECT (item), "file_list_count", GINT_TO_POINTER (file_list_count));

			/* Connect the signals. */
			gtk_signal_connect (GTK_OBJECT (item), "select", GTK_SIGNAL_FUNC (on_tree_item_select), NULL);
			gtk_signal_connect (GTK_OBJECT (item), "deselect", GTK_SIGNAL_FUNC (on_tree_item_deselect), NULL);
			gtk_signal_connect (GTK_OBJECT (item), "button_press_event", GTK_SIGNAL_FUNC (on_tree_item_file_button_press_event), NULL);
		}
	}
}

void
on_tree_item_collapse (GtkTreeItem* tree_item, gpointer user_data)
{
	/* Remove items of subtrees. We do that because we user should 	*/
	/* be able to get an updated tree by collapsing and reexpanding	*/
	/* the tree.							*/
	camera_tree_folder_clean (tree_item);
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
	CameraFile*		file;
	Camera*			camera;
        GdkPixbuf*		pixbuf;
        GdkPixbufLoader*	loader;
        GdkPixmap*		pixmap;
        GdkBitmap*		bitmap;
	CameraText*		buffer;
	gint			folder_count, file_count;
	gint			magnification;
	GdkInterpType		interpolation;
	GConfValue*		value;

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
                window = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (hbox), window, TRUE, TRUE, 0);
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

	                /* Process the image. */
	                g_assert ((loader = gdk_pixbuf_loader_new ()) != NULL);
	                if (gdk_pixbuf_loader_write (loader, file->data, file->size)) {
	                        gdk_pixbuf_loader_close (loader);
	                        pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

				/* Magnification? */
				value = gconf_client_get_without_default (client, "/apps/" PACKAGE "/magnification", NULL);
				if (value) {
					g_assert (value->type = GCONF_VALUE_INT);
					magnification = gconf_value_get_int (value);
				} else magnification = 1;
				
				/* Interpolation? */
				value = gconf_client_get_without_default (client, "/apps/" PACKAGE "/interpolation", NULL);
				if (value) {
					g_assert (value->type = GCONF_VALUE_INT);
					switch (gconf_value_get_int (value)) {
					case 0:
						interpolation = GDK_INTERP_NEAREST;
						break;
					case 1: 
						interpolation = GDK_INTERP_TILES;
						break;
					case 2: 
						interpolation = GDK_INTERP_BILINEAR;
						break;
					case 3:
						interpolation = GDK_INTERP_HYPER;
						break;
					default: 
						g_assert_not_reached ();
					}
				} else interpolation = GDK_INTERP_BILINEAR;
	                        gdk_pixbuf_render_pixmap_and_mask (gdk_pixbuf_scale_simple (
					pixbuf, 
					magnification * gdk_pixbuf_get_width (pixbuf), 
					magnification * gdk_pixbuf_get_height (pixbuf), 
					interpolation), &pixmap, &bitmap, 127);
	                        gdk_pixbuf_unref (pixbuf);
	                        widget = gtk_pixmap_new (pixmap, bitmap);
	                        gtk_container_add (GTK_CONTAINER (viewport), widget);
				gtk_object_set_data (GTK_OBJECT (item), "pixmap", pixmap);
	
				/* Clist for exif tags. */
//				widget = gtk_clist_new (1);
//				gtk_container_add (GTK_CONTAINER (page), widget);
//				list_text[0] = g_strdup ("This list will display exif tags.");
//				gtk_clist_append (GTK_CLIST (widget), list_text);
//				g_free (list_text[0]);
	
	                } else {
	                        if (strcmp ("/", path) == 0) dialog_information (_("Could not load image '/%s'!"), filename);
				else dialog_information (_("Could not load image '%s/%s'!"), path, filename);
				gtk_container_add (GTK_CONTAINER (viewport), gtk_label_new ("?"));
	                }
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

gboolean
on_tree_item_camera_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GladeXML*	xml_popup;
	Camera*		camera;

	g_assert (event != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (widget), "camera")) != NULL);

	/* Did the user right-click? */
	if (event->button == 3) {
		
		/* Create the dialog. */
		g_assert ((xml_popup = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "camera_tree_popup_camera")) != NULL);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_capture_video")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_capture_image")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_capture_preview")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_properties")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera_upload_file")), "item", widget);

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_popup);

		/* Pop up the dialog. */
		gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "camera_tree_popup_camera")), NULL, NULL, NULL, NULL, event->button, event->time);

		/* Reference the camera. */
		gp_camera_ref (camera);

		return (TRUE);

	} else return (FALSE);
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
	
	        /* Get the camera properties from the backend. */
	        if (gp_camera_config (camera) != GP_OK) {
			dialog_information (_("Could not get camera properties of camera %s!"), frontend_data->name);
	        }
	}
}

void
on_capture_preview_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	GladeXML*		xml_preview;
	Camera*			camera;
	frontend_data_t*	frontend_data;

	g_assert (menuitem != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	if (!(frontend_data->xml_preview)) {
		
		/* Open the preview window. */
		g_assert ((xml_preview = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app_preview")) != NULL);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_capture_image")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_capture_video")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_properties")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_refresh")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_save")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_save_as")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_close")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_refresh")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_save")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_save_as")), "camera", camera);

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_preview);

		/* Reference the camera. */
		gp_camera_ref (camera);

		frontend_data->xml_preview = xml_preview;

		/* Get a preview. */
		preview_refresh (camera);
	}
}

void
on_capture_image_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	Camera *camera;
	CameraCaptureInfo info;
	CameraFile *file;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);;

	/* Prepare the image. */
	info.type = GP_CAPTURE_IMAGE;
	info.duration = 0;
	file = gp_file_new ();

	/* Capture. */
	gp_camera_capture (camera, file, &info);

	/* Clean up. */
	gp_file_free (file);
}

void
on_duration_reply (gchar *string, gpointer user_data)
{
	Camera *camera;
	CameraCaptureInfo info;
	CameraFile *file;

	camera = (Camera *) user_data;
	g_assert (camera != NULL);

	if (string) {
		
		/* Prepare the video. */
		info.type = GP_CAPTURE_VIDEO;
		info.duration = atoi (string);
		file = gp_file_new ();
		
		/* Capture. */
		gp_camera_capture (camera, file, &info);
		
		/* Clean up. */
		gp_file_free (file);
	}
}

void
on_capture_video_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	GnomeApp*	app;
        Camera*		camera;

	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);
        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);

	/* Ask for duration. */
	gnome_app_request_string (app, _("How long should the video be (in seconds)?"), on_duration_reply, camera);
}

/************************/
/* app_preview specific */
/************************/

void
on_app_preview_close_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	Camera*			camera;
	frontend_data_t*	frontend_data;
	
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (menuitem), "camera")) != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	/* Clean up. */
	gtk_widget_destroy (glade_xml_get_widget (frontend_data->xml_preview, "app_preview"));
	frontend_data->xml_preview = NULL;
	gp_camera_unref (camera);
}

void
on_app_preview_save_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	preview_save (gtk_object_get_data (GTK_OBJECT (menuitem), "camera")); 
}

void
on_app_preview_save_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	preview_save_as (gtk_object_get_data (GTK_OBJECT (menuitem), "camera"));
}

void
on_app_preview_refresh_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	preview_refresh (gtk_object_get_data (GTK_OBJECT (menuitem), "camera"));
}

void
on_app_preview_button_refresh_clicked (GtkButton* button, gpointer user_data)
{
	preview_refresh (gtk_object_get_data (GTK_OBJECT (button), "camera"));
}

void
on_app_preview_button_save_clicked (GtkButton* button, gpointer user_data)
{
	preview_save (gtk_object_get_data (GTK_OBJECT (button), "camera"));
}

void
on_app_preview_button_save_as_clicked (GtkButton* button, gpointer user_data)
{
	preview_save_as (gtk_object_get_data (GTK_OBJECT (button), "camera"));
}


