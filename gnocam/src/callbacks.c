#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gphoto2.h>
#include "preferences.h"
#include "save.h"
#include "gnocam.h"

/******************************************************************************/
/* Prototypes                                                                 */
/******************************************************************************/

void on_reply (gint reply, gpointer data);

void on_button_save_previews_clicked (GtkButton *button, gpointer user_data);
void on_button_save_preview_as_clicked (GtkButton *button, gpointer user_data);
void on_button_save_files_clicked (GtkButton *button, gpointer user_data);
void on_button_save_file_as_clicked (GtkButton *button, gpointer user_data);
void on_button_delete_clicked (GtkButton *button, gpointer user_data);

void on_save_previews_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save_preview_as_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save_files_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save_file_as_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_delete_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_exit_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_preferences_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_about_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_clist_files_drag_data_get (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer data);
gboolean on_clist_files_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);

void on_tree_cameras_selection_changed (GtkWidget *tree);
gboolean on_tree_cameras_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_tree_cameras_popup_capture_image_activate (GtkMenuItem *menu_item, gpointer user_data);
void on_tree_cameras_popup_capture_video_activate (GtkMenuItem *menu_item, gpointer user_data);
void on_duration_reply (gchar *string, gpointer user_data);

/*****************************************/
/* Lots of lines only for save / delete. */
/*****************************************/

void
on_button_save_previews_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);

	save (xml, FALSE, FALSE);
}

void
on_button_save_preview_as_clicked (GtkButton *button, gpointer user_data)
{
        GladeXML *xml;

        xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
        g_assert (xml != NULL);

	g_warning ("Not yet implemented!");
}

void
on_button_save_files_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);

	save (xml, TRUE, FALSE);
}

void
on_button_save_file_as_clicked (GtkButton *button, gpointer user_data)
{
        GladeXML *xml;

        xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
        g_assert (xml != NULL);

	g_warning ("Not yet implemented!");
}

void
on_button_delete_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);

	delete (xml);
}

void
on_save_previews_activate (GtkMenuItem *menuitem, gpointer user_data)
{
        GladeXML *xml;

        xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
        g_assert (xml != NULL);

	save (xml, FALSE, FALSE);
}

void
on_save_preview_as_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
        g_assert (xml != NULL);

	g_warning ("Not yet implemented!");
}

void
on_save_files_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
	g_assert (xml != NULL);

	save (xml, TRUE, FALSE);
}

void
on_save_file_as_activate (GtkMenuItem *menuitem, gpointer user_data)
{
        GladeXML *xml;

        xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
        g_assert (xml != NULL);

	g_warning ("Not yet implemented!");
}

void
on_delete_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml;

        xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
        g_assert (xml != NULL);

	delete (xml);
}

void
on_exit_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml;
	
	xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
        g_assert (xml != NULL);

	/* Set the preferences. */
	preferences_set (xml);

	gtk_main_quit ();
}

void
on_preferences_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
	g_assert (xml != NULL);
        preferences (xml);
}

void
on_about_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml, *xml_about;
	GnomeApp *app;

	xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));

	xml_about = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "about");
	if (xml_about == NULL) gnome_app_error (app, _("Could not find " GNOCAM_GLADEDIR "gnocam.glade. Check if GnoCam was installed correctly."));
}

void
on_clist_files_drag_data_get (
	GtkWidget *widget, 
	GdkDragContext *context, 
	GtkSelectionData *selection_data, 
	guint info, 
	guint time, 
	gpointer data)
{
	GladeXML *xml;
	gchar *filenames, *filenames_old;
	GList *selection;
	GtkCList *clist;
	gchar *file_name;
	gint i, row;

	xml = gtk_object_get_data (GTK_OBJECT (widget), "xml");
	g_assert (xml != NULL);
        clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_files"));
        g_assert (clist != NULL);

        /* Check which files have been selected. */
        selection = g_list_first (clist->selection);
	filenames = g_strdup ("");
        for (i = 0; i < g_list_length (selection); i++) {
                row = GPOINTER_TO_INT (g_list_nth_data (selection, i));
                gtk_clist_get_text (clist, row, 2, &file_name);
                g_assert (file_name != NULL);
		filenames_old = filenames;
		if (i == 0) filenames = g_strdup_printf ("file://tmp/%s\n", g_strdup (file_name));
		else filenames = g_strdup_printf ("%sfile://tmp/%s\n", filenames_old, g_strdup (file_name));
		g_free (filenames_old);
	}

	/* Calculate size of variable filenames. */
	for (i =0; ; i++) if (filenames[i] == 0) break;

	//FIXME: Getting files takes too much time. Previews work...
	save (xml, FALSE, TRUE);
	gtk_selection_data_set (selection_data, selection_data->target, 8, filenames, i + 1);
}

/********************/
/* File list stuff. */
/********************/

gboolean
on_clist_files_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
        GladeXML *xml_popup, *xml;

        g_assert (event != NULL);
        xml = gtk_object_get_data (GTK_OBJECT (widget), "xml");
        g_assert (xml != NULL);

        /* Did the user right-click? */
        if (event->button == 3) {

                /* Create the dialog. */
                xml_popup = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "menu_clist_files_popup");
                g_assert (xml != NULL);

                /* Store some data. */
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "clist_files_popup_save_previews")), "xml", xml);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "clist_files_popup_save_preview_as")), "xml", xml);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "clist_files_popup_save_files")), "xml", xml);
                gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "clist_files_popup_save_file_as")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "clist_files_popup_delete")), "xml", xml);

                /* Connect the signals. */
                glade_xml_signal_autoconnect (xml_popup);

                /* Pop up the dialog. */
                gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "menu_clist_files_popup")), NULL, NULL, NULL, NULL, event->button, event->time);

                return (TRUE);

        } else return (FALSE);
}

/**********************/
/* Camera tree stuff. */
/**********************/

void
on_tree_cameras_selection_changed (GtkWidget *tree)
{
	GList *selection;
	GtkWidget *item;
	Camera *camera;
	CameraList camera_list;
	CameraListEntry *camera_list_entry;
	CameraFile *camera_file;
	gint count, i, j, row;
	GtkCList *clist;
	GdkPixbuf *pixbuf;
	GdkPixbufLoader *loader;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	GladeXML *xml;
	GnomeApp *app;
	gchar *text_dummy[2] = {NULL, NULL};
	gchar *path, *file_name;
	gboolean found;
	
	g_assert (tree != NULL);
	xml = gtk_object_get_data (GTK_OBJECT (tree), "xml");
	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);
	clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_files"));
	g_assert (clist != NULL);
	selection = g_list_first (GTK_TREE_SELECTION (tree));
	gtk_clist_set_row_height (clist, 70); //FIXME: Move that to an appropriate position.

	/* Look for each file in the file list if we still are to display it. */
	/* Ok, we only check if the folder is still selected, not if the file */
	/* is still in there... FIXME?                                        */
	for (row = clist->rows - 1; row >= 0; row--) {
		found = FALSE;
		for (i = 0; i < g_list_length (selection); i++) {
			item = GTK_WIDGET (g_list_nth_data (selection, i));
	
			/* Is camera the same? */
			if (gtk_object_get_data (GTK_OBJECT (item), "camera") != gtk_clist_get_row_data (clist, row)) continue;

			/* Is the path the same? */
			gtk_clist_get_text (clist, row, 1, &path);
			if (strcmp (gtk_object_get_data (GTK_OBJECT (item), "path"), path) != 0) continue;

			found = TRUE;
		}
		if (!found) gtk_clist_remove (clist, row);	
	}

	/* Look for each selected folder if we have displayed all files. */
	for (i = 0; i < g_list_length (selection); i++) {
		item = GTK_WIDGET (g_list_nth_data (selection, i));
		camera = gtk_object_get_data (GTK_OBJECT (item), "camera");
		g_assert (camera != NULL);
		path = gtk_object_get_data (GTK_OBJECT (item), "path");
		g_assert (path != NULL);

		/* Get the files. */
		if (gp_camera_file_list (camera, &camera_list, path) == GP_OK) {
			count = gp_list_count (&camera_list);
			for (j = 0; j < count; j++) {
				camera_list_entry = gp_list_entry (&camera_list, j);

				/* Do we have this file already in the list? */
				found = FALSE;
				for (row = 0; row < clist->rows; row++) {

					/* Is the path the same? */
					gtk_clist_get_text (clist, row, 1, &path);
					if (strcmp (gtk_object_get_data (GTK_OBJECT (item), "path"), path) != 0) continue;

					/* Is the camera the same? */
					if (gtk_object_get_data (GTK_OBJECT (item), "camera") != gtk_clist_get_row_data (clist, row)) continue;

					/* Is the file name the same? */
					gtk_clist_get_text (clist, row, 2, &file_name);
					if (strcmp (file_name, camera_list_entry->name) != 0) continue;

					found = TRUE;
				}
				if (!found) {
					path = gtk_object_get_data (GTK_OBJECT (item), "path");
					g_assert (path != NULL);

					/* Add entry to list. */
					gtk_clist_append (clist, text_dummy);
					gtk_clist_set_text (clist, clist->rows - 1, 1, path);
					gtk_clist_set_row_data (clist, clist->rows - 1, camera);
	
					/* Get the file. */
					camera_file = gp_file_new ();
					if (gp_camera_file_get_preview (camera, camera_file, path, camera_list_entry->name) == GP_OK) {
	
						/* Process the image. */
						loader = gdk_pixbuf_loader_new ();
						g_assert (loader != NULL);
						if (gdk_pixbuf_loader_write (loader, camera_file->data, camera_file->size)) {
							gdk_pixbuf_loader_close (loader);
							pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
							gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 127);
							gdk_pixbuf_unref (pixbuf);
		
							/* Add pixbuf to the entry. */
							gtk_clist_set_pixmap (clist, clist->rows - 1, 0, pixmap, bitmap);
						} else {
							gnome_app_error (app, _("Could not load image!"));
							gtk_clist_set_text (clist, clist->rows - 1, 0, "?");
						}
						gtk_clist_set_text (clist, clist->rows - 1, 2, camera_list_entry->name);
	
					} else {
						gnome_app_error (app, _("Could not get preview from the camera!"));
						gtk_clist_set_text (clist, clist->rows - 1, 0, "?");
						gtk_clist_set_text (clist, clist->rows - 1, 2, "?");
					}
		
					/* We got the file. Clean up. */
					gp_frontend_progress (camera, NULL, 0.0);
					gp_file_free (camera_file);
				}
			}
		} else {
			gnome_app_error (app, _("Could not get the file list from the camera!"));
		}
	}
}

gboolean
on_tree_cameras_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GladeXML *xml_popup, *xml;
	Camera *camera;

	g_assert (event != NULL);
	xml = gtk_object_get_data (GTK_OBJECT (widget), "xml");
	g_assert (xml != NULL);
	camera = gtk_object_get_data (GTK_OBJECT (widget), "camera");
	g_assert (camera != NULL);

	/* Did the user right-click? */
	if (event->button == 3) {
		
		/* Create the dialog. */
		xml_popup = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "menu_tree_cameras_popup");
		g_assert (xml != NULL);

		/* Store some data. */
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "tree_cameras_popup_capture_video")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "tree_cameras_popup_capture_video")), "camera", camera);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "tree_cameras_popup_capture_image")), "xml", xml);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_popup, "tree_cameras_popup_capture_image")), "camera", camera);

		/* Connect the signals. */
		glade_xml_signal_autoconnect (xml_popup);

		/* Pop up the dialog. */
		gtk_menu_popup (GTK_MENU (glade_xml_get_widget (xml_popup, "menu_tree_cameras_popup")), NULL, NULL, NULL, NULL, event->button, event->time);

		return (TRUE);

	} else return (FALSE);
}

void
on_tree_cameras_popup_capture_image_activate (GtkMenuItem *menu_item, gpointer user_data)
{
	GladeXML *xml;
	Camera *camera;
	CameraCaptureInfo info;
	CameraFile *file;

	xml = gtk_object_get_data (GTK_OBJECT (menu_item), "xml");
	g_assert (xml != NULL);
	camera = gtk_object_get_data (GTK_OBJECT (menu_item), "camera");
	g_assert (camera != NULL);

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

	if (strcmp (string, "") != 0) {
		
		/* Prepare the video. */
		info.type = GP_CAPTURE_VIDEO;
		info.duration = atoi (string);
		file = gp_file_new ();
		
		/* Capture. */
		gp_camera_capture (camera, file, &info);
		
		/* Clean up. */
		gp_file_free (file);
	}
	g_free (string);
}

void
on_tree_cameras_popup_capture_video_activate (GtkMenuItem *menu_item, gpointer user_data)
{
	GladeXML *xml;
        Camera *camera;

        xml = gtk_object_get_data (GTK_OBJECT (menu_item), "xml");
        g_assert (xml != NULL);
        camera = gtk_object_get_data (GTK_OBJECT (menu_item), "camera");
        g_assert (camera != NULL);

	/* Ask for duration. */
	gnome_app_request_string (GNOME_APP (glade_xml_get_widget (xml, "app")), _("How long should the video be?"), on_duration_reply, camera);
}

