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
void on_button_save_files_clicked (GtkButton *button, gpointer user_data);
void on_button_delete_files_clicked (GtkButton *button, gpointer user_data);
void on_button_delete_files_clicked (GtkButton *button, gpointer user_data);
void on_save_previews_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save_files_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_exit_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_preferences_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_about_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_clist_files_drag_data_get (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer data);
void on_tree_cameras_selection_changed (GtkWidget *tree);

/******************************************************************************/
/* Callbacks                                                                  */
/******************************************************************************/

void
on_reply (gint reply, gpointer data)
{
	GladeXML *xml;
	GtkWidget *app;

	xml = data;
	g_assert (xml != NULL);
	app = glade_xml_get_widget (xml, "app");
	g_assert (app != NULL);
	gtk_object_set_data (GTK_OBJECT (app), "reply", GINT_TO_POINTER (reply));
}

void
on_button_save_previews_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);

	save (xml, FALSE, FALSE);
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
on_button_delete_files_clicked (GtkButton *button, gpointer user_data)
{
	GladeXML *xml;
	GList *selection;
	Camera *camera;
	GtkCList *clist;
	GnomeApp *app;
	gchar *path = NULL;
	gchar *file_name = NULL;
	gchar *message;
	gint reply;
	guint row;

	xml = gtk_object_get_data (GTK_OBJECT (button), "xml");
	g_assert (xml != NULL);
	clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_files"));
	g_assert (clist != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);

	/* Check which files have been selected. */
	selection = g_list_first (clist->selection);
	if (selection != NULL) {
		{
			if (g_list_length (selection) > 1)
				message = g_strdup_printf (_("Do you really want to delete the %i selected files?"), g_list_length (selection));
			else
				message = g_strdup_printf (_("Do you really want to delete the selected file?"));
			gnome_dialog_run_and_close (GNOME_DIALOG (gnome_app_question_modal (app, message, on_reply, xml)));
			reply = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (app), "reply"));
			if (reply == GNOME_YES) {
				while (selection != NULL) {

					/* Retrieve some data we need. */
					row = GPOINTER_TO_INT (selection->data);
					camera = gtk_clist_get_row_data (clist, row);
					g_assert (camera != NULL);
					gtk_clist_get_text (clist, row, 1, &path);
					g_assert (path != NULL);
					gtk_clist_get_text (clist, row, 2, &file_name);
					g_assert (file_name != NULL);

					/* Delete the file. */
					if (gp_camera_file_delete (camera, path, file_name) == GP_OK) {

						/* Update the file list. */
						gtk_clist_remove (clist, row);

					} else {

						/* Unselect the file we could not delete. */
						gtk_clist_unselect_row (clist, row, 0);
						gnome_app_error (app, _("Could not delete file!"));
					}
					selection = g_list_first (clist->selection); 
				}
			}
		}
	} else {
		gnome_app_error (app, _("No files selected!"));
	}
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
on_save_files_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GladeXML *xml;

	xml = gtk_object_get_data (GTK_OBJECT (menuitem), "xml");
	g_assert (xml != NULL);

	save (xml, TRUE, FALSE);
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
	guint count, i, j, row;
	GtkCList *clist;
	GdkPixbuf *pixbuf;
	GdkPixbufLoader *loader;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	GladeXML *xml;
	GnomeApp *app;
	gchar *text_dummy[2] = {NULL, NULL};
	gchar *path;
	
	xml = gtk_object_get_data (GTK_OBJECT (tree), "xml");
	g_assert (xml != NULL);
	app = GNOME_APP (glade_xml_get_widget (xml, "app"));
	g_assert (app != NULL);
	clist = GTK_CLIST (glade_xml_get_widget (xml, "clist_files"));
	g_assert (clist != NULL);

	/* Clear the file list. */
	gtk_clist_clear (clist);
	gtk_clist_set_row_height (clist, 70); //FIXME: Move that to an appropriate position.

	/* Check which folders are selected. */	
	selection = g_list_first (GTK_TREE_SELECTION (tree));
	row = 0;
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

				/* Add entry to list. */
				gtk_clist_append (clist, text_dummy);
				gtk_clist_set_text (clist, row, 1, path);
				gtk_clist_set_row_data (clist, row, camera);

				/* Get the file. */
				camera_list_entry = gp_list_entry (&camera_list, j);
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
						gtk_clist_set_pixmap (clist, row, 0, pixmap, bitmap);
					} else {
						gnome_app_error (app, _("Could not load image!"));
						gtk_clist_set_text (clist, row, 0, "?");
					}
					gtk_clist_set_text (clist, row, 2, camera_list_entry->name);

				} else {
					gnome_app_error (app, _("Could not get preview from the camera!"));
					gtk_clist_set_text (clist, row, 0, "?");
					gtk_clist_set_text (clist, row, 2, "?");
				}

				/* We got the file. Clean up. */
				gp_frontend_progress (camera, NULL, 0.0);
				row++;
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

