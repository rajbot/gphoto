//This file should be called file-operations.c
//I'll change the name some day...

#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "save.h"
#include "gnocam.h"
#include "information.h"
#include "cameras.h"
#include "gphoto-extensions.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;
extern GConfClient*	client;

/**************/
/* Prototypes */
/**************/

void on_fileselection_ok_button_clicked 	(GtkButton* button, gpointer user_data);
void on_fileselection_cancel_button_clicked 	(GtkButton* button, gpointer user_data);

void upload_common (Camera* camera, gchar* path, gchar* filename);

/*************/
/* Callbacks */
/*************/

//void
//on_reply (gint reply, gpointer data)
//{
//        GtkWidget *app;
//
//        app = glade_xml_get_widget (xml, "app");
//        g_assert (app != NULL);
//        gtk_object_set_data (GTK_OBJECT (app), "reply", GINT_TO_POINTER (reply));
//}

void
on_fileselection_ok_button_clicked (GtkButton *button, gpointer user_data)
{
	CameraFile*		file;
	Camera*			camera;
	gchar*			path;
	gchar* 			filename_user;
	GtkFileSelection*	fileselection;

	g_assert ((fileselection = GTK_FILE_SELECTION (gtk_object_get_data (GTK_OBJECT (button), "fileselection"))) != NULL);

	filename_user = g_strdup (gtk_file_selection_get_filename (fileselection));

	/* Save or upload? */
	if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "save")) == 1) {
		g_assert ((file = gtk_object_get_data (GTK_OBJECT (button), "file")) != NULL);
		camera_file_save (file, filename_user);
	} else {
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (button), "camera")) != NULL);
		g_assert ((path = gtk_object_get_data (GTK_OBJECT (button), "path")) != NULL);
		upload (camera, path, filename_user);

		/* Clean up. */
		gp_camera_unref (camera);
	}

	/* Clean up. */
	g_free (filename_user);
	gtk_widget_destroy (GTK_WIDGET (fileselection));
}

void
on_fileselection_cancel_button_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget*	widget;
	Camera*		camera;

	g_assert ((widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "fileselection"))) != NULL);

	/* Clean up. */
	if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "save")) == 0) {
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (button), "camera")) != NULL);
		gp_camera_unref (camera);
	}
	gtk_widget_destroy (widget);
}

/*************/
/* Functions */
/*************/

void
upload (Camera* camera, gchar* path, gchar* filename)
{
	GladeXML*		xml_fileselection;
	GtkFileSelection*	fileselection;
	GtkObject*		object;
        guint                   j, k;
        GnomeVFSURI*            uri;
        GnomeVFSHandle*         handle;
        GnomeVFSFileSize        bytes_read;
        GnomeVFSResult          result;
        guint8                  data [1025];
	CameraFile*		file;

	g_assert (camera != NULL);
	g_assert (path != NULL);

	if (filename) {

		/* Read the data and upload the file. */
		file = gp_file_new ();
	        file->data = g_new (gchar, 1025);
		strcpy (file->name, filename);
	        uri = gnome_vfs_uri_new (filename);
	        if ((result = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ)) != GNOME_VFS_OK) {
	                dialog_information (_("An error occurred while trying to open file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
	        } else {
	                j = 0;
	                while (TRUE) {
	                        if ((result = gnome_vfs_read (handle, data, 1024, &bytes_read)) != GNOME_VFS_OK) break;
	                        if (bytes_read == 0) break;
	                        else if (bytes_read <= 1024) data [bytes_read] = '\0';
	                        else g_assert_not_reached ();
	                        for (k = 0; k < bytes_read; k++) file->data [k + j] = data [k];
	                        file->size = j + bytes_read;
	                        file->data = g_renew (gchar, file->data, j + 1024);
	                }
	                if (result == GNOME_VFS_OK) camera_file_upload (camera, path, file);
			else {
				dialog_information (_("An error occurred while trying to read file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
	                        gp_file_free (file);
			}
		}
	} else {

	        /* Ask the user for a filename. Pop up the file selection dialog. */
	        g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")) != NULL);
	        g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (xml_fileselection, "fileselection"))) != NULL);
	
	        /* Store some data in the ok button. */
	        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))) != NULL);
	        gtk_object_set_data (object, "camera", camera);
	        gtk_object_set_data (object, "path", path);
	        gtk_object_set_data (object, "fileselection", fileselection);
	        gtk_object_set_data (object, "save", GINT_TO_POINTER (0));
	
	        /* Store some data in the cancel button. */
	        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))) != NULL);
		gtk_object_set_data (object, "camera", camera);
	        gtk_object_set_data (object, "fileselection", fileselection);
		gtk_object_set_data (object, "save", GINT_TO_POINTER (0));
	
	        /* Connect the signals. */
	        glade_xml_signal_autoconnect (xml_fileselection);

                /* Reference the camera. */
                gp_camera_ref (camera);
	}
}
		
void
camera_file_upload (Camera* camera, gchar* path, CameraFile* file)
{
        if (gp_camera_file_put (camera, file, path) != GP_OK) dialog_information (_("Could not upload file '%s' into folder '%s'!"), file->name, path);

	/* Clean up. */
        gp_file_free (file);
	g_free (path);
	gp_camera_unref (camera);
}

void
camera_file_save (CameraFile* file, gchar* filename)
{
        GnomeVFSResult          result;
        GnomeVFSHandle*         handle;
        GnomeVFSURI*            uri;
        GnomeVFSFileSize        file_size;

        g_assert (file != NULL);
        g_assert (filename != NULL);

        /* Let gnome-vfs save the file. */
        uri = gnome_vfs_uri_new (filename);
        if ((result = gnome_vfs_create_uri (&handle, uri, GNOME_VFS_OPEN_WRITE, FALSE, 0644)) != GNOME_VFS_OK) {
                dialog_information (_("An error occurred while trying to open file '%s' for write access (%s)."), filename, gnome_vfs_result_to_string (result));
        } else {
                if ((result = gnome_vfs_write (handle, file->data, file->size, &file_size)) != GNOME_VFS_OK) {
                        dialog_information (_("An error occurred while trying to write into file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
                }
                if ((result = gnome_vfs_close (handle)) != GNOME_VFS_OK) {
                        dialog_information (_("An error occurred while trying to close the file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
                }
        }
}

void
camera_file_save_as (CameraFile* file)
{
	GladeXML*		xml_fileselection;
        GtkObject*              object;
        GtkFileSelection*       fileselection;

        g_assert (file != NULL);

        /* Pop up the file selection dialog. */
        g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")) != NULL);
        g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (xml_fileselection, "fileselection"))) != NULL);

        /* Suggest the filename. */
        gtk_file_selection_set_filename (fileselection, file->name);

        /* Store some data in the ok button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))) != NULL);
        gtk_object_set_data (object, "file", file);
        gtk_object_set_data (object, "fileselection", fileselection);
	gtk_object_set_data (object, "save", GINT_TO_POINTER (1));

        /* Store some data in the cancel button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))) != NULL);
        gtk_object_set_data (object, "file", file);
        gtk_object_set_data (object, "fileselection", fileselection);
	gtk_object_set_data (object, "save", GINT_TO_POINTER (1));

        /* Connect the signals. */
        glade_xml_signal_autoconnect (xml_fileselection);
}

void 
save_all_selected (GtkTree* tree, gboolean preview, gboolean save_as, gboolean temporary)
{
        gint            i;
        Camera*         camera;
        gchar*          path;
        gchar*          filename;
	GtkTreeItem*	item;

        g_assert (tree != NULL);
	g_assert (!(save_as && temporary));

	/* Look into folders first. */
	for (i = 0; i < g_list_length (tree->children); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
                g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
		
		/* Is this item a folder? */
		if (!(filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {

			/* Save selected files in subfolders. */
			if (item->subtree) save_all_selected (GTK_TREE (item->subtree), preview, save_as, temporary);
		}
	}

	/* Files. */
	for (i = 0; i < g_list_length (tree->selection); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->selection, i));
		if ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {
			save (item, preview, save_as, temporary);
		}
	}
}

void
save (GtkTreeItem* item, gboolean preview, gboolean save_as, gboolean temporary)
{
	gchar*		path;
	gchar*		filename;
	gchar*		filename_user;
	CameraFile*	file;
	Camera*		camera;
	GConfValue*	value;
	gint		return_status;
	gboolean	b;

	g_assert (item != NULL);
	g_assert (!(save_as && temporary));
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename")) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
        g_assert ((value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL)));
        g_assert (value->type == GCONF_VALUE_STRING);

	if (temporary) filename_user = g_strdup_printf ("file:/tmp/%s", filename);
	else filename_user = g_strdup_printf ("%s/%s", gconf_value_get_string (value), filename);

	/* Check if we already have the file. */
	b = TRUE;
	if (preview) file = gtk_object_get_data (GTK_OBJECT (item), "preview");
	else file = gtk_object_get_data (GTK_OBJECT (item), "file");
	if (!file) {

		/* Get the file/preview from gphoto backend. */
		file = gp_file_new ();
		if (preview) return_status = gp_camera_file_get_preview (camera, file, path, filename);
		else return_status = gp_camera_file_get (camera, file, path, filename);
		if (return_status != GP_OK) {
			if (preview) dialog_information (_("Could not get preview of file '%s/%s' from camera!"), path, filename);
			else dialog_information (_("Could not get file '%s/%s' from camera!"), path, filename);
			b = FALSE;
		} else {
			if (preview) gtk_object_set_data (GTK_OBJECT (item), "preview", file);
			else gtk_object_set_data (GTK_OBJECT (item), "file", file);
		}
	}
	if (b) {
		if (save_as) camera_file_save_as (file);
		else camera_file_save (file, filename_user);
	}

	/* Clean up. */
	gp_frontend_progress (camera, NULL, 0.0);
	g_free (filename_user);
}

void
delete_all_selected (GtkTree* tree)
{
        gchar*          filename;
	gint		i;
	GtkTreeItem*	item;
	GtkNotebook*	notebook;

        g_assert (tree != NULL);
	g_assert ((notebook = GTK_NOTEBOOK (glade_xml_get_widget (xml, "notebook_files"))) != NULL);

        /* Look into folders first. */
        for (i = 0; i < g_list_length (tree->children); i++) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));

                /* Is this item a folder? */
                if (!(filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {

			/* Delete in subfolder. */
			if (item->subtree) delete_all_selected (GTK_TREE (item->subtree));
                }
        }

        /* Files. */
        for (i = 0; i < g_list_length (tree->selection); i++) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->selection, i));
                if ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {

			/* Delete. */
			delete (item);
		}
	}
}

//FIXME: Ask the user before deletion?
//        if (g_list_length (selection) > 1)
//                message = g_strdup_printf (_("Do you really want to delete the %i selected files?"), g_list_length (selection));
//        else
//                message = g_strdup_printf (_("Do you really want to delete the selected file?"));
//        gnome_dialog_run_and_close (GNOME_DIALOG (gnome_app_question_modal (app, message, on_reply, xml)));
//        reply = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (app), "reply"));
//	if (reply == GNOME_YES) {
//                        while (selection != NULL) {


void
delete (GtkTreeItem* item)
{
	gchar*	path;
	gchar*	filename;
	Camera*	camera;

	g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
	g_assert ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename")) != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);

	if (gp_camera_file_delete (camera, path, filename) == GP_OK) camera_tree_item_remove (item);
	else dialog_information (_("Could not delete '%s/%s'!"), path, filename);
}

