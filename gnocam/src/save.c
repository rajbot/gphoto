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

/**************/
/* Prototypes */
/**************/

void on_reply (gint reply, gpointer data);

void on_fileselection_ok_button_clicked 	(GtkButton* button, gpointer user_data);
void on_fileselection_cancel_button_clicked 	(GtkButton* button, gpointer user_data);

void save_common (Camera* camera, gchar* path, gchar* filename, gboolean file, gchar* filename_user);
void upload_common (Camera* camera, gchar* path, gchar* filename);

/*************/
/* Callbacks */
/*************/

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
on_fileselection_ok_button_clicked (GtkButton *button, gpointer user_data)
{
	Camera*		camera;
	gchar*		path;
	gchar*		filename;
	gchar* 		filename_user;
	GtkFileSelection*	fileselection;
	gboolean		file;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (button), "camera")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (button), "path")) != NULL);
	g_assert ((fileselection = GTK_FILE_SELECTION (gtk_object_get_data (GTK_OBJECT (button), "fileselection"))) != NULL);

	filename_user = g_strdup (gtk_file_selection_get_filename (fileselection));

	/* Save or upload? */
	if (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "save")) == 1) {
		g_assert ((filename = gtk_object_get_data (GTK_OBJECT (button), "filename")) != NULL);
		file = (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "file")) == 1);
		save_common (camera, path, filename, file, filename_user);
		g_free (path);
		g_free (filename);
	} else {
		upload_common (camera, path, filename_user);
		g_free (path);
	}

	gtk_widget_destroy (GTK_WIDGET (fileselection));
}

void
on_fileselection_cancel_button_clicked (GtkButton *button, gpointer user_data)
{
        gchar*  	path;
        gchar*  	filename;
	GtkWidget*	widget;

        g_assert ((path = gtk_object_get_data (GTK_OBJECT (button), "path")) != NULL);
	g_assert ((widget = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (button), "fileselection"))) != NULL);

	/* Nothing to do. Clean up. */
        g_free (path);
        if ((filename = gtk_object_get_data (GTK_OBJECT (button), "filename"))) g_free (filename);
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

	g_assert (camera != NULL);
	g_assert (path != NULL);

        /* Path will be freed afterwards. */
        path = g_strdup (path);

	if (filename) upload_common (camera, path, g_strdup (filename));
	else {

	        /* Pop up the file selection dialog. */
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
	        gtk_object_set_data (object, "path", path);
	        gtk_object_set_data (object, "fileselection", fileselection);
	
	        /* Connect the signals. */
	        glade_xml_signal_autoconnect (xml_fileselection);
	}
}

void
upload_common (Camera* camera, gchar* path, gchar* filename)
{
        guint                   j, k;
        CameraFile*             file;
        GnomeVFSURI*            uri;
        GnomeVFSHandle*         handle;
        GnomeVFSFileSize        bytes_read;
        GnomeVFSResult          result;
        guint8                  data [1025];

        g_assert (path != NULL);
        g_assert (camera != NULL);
	g_assert (filename != NULL);

	/* Make sure we own path and filename. */
	path = g_strdup (path);
	filename = g_strdup (filename);

        file = gp_file_new ();
        file->data = g_new (gchar, 1025);
        uri = gnome_vfs_uri_new (filename);
        if ((result = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ)) != GNOME_VFS_OK) {
                dialog_information (_("An error occurred while trying to open file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
        } else {
	        j = 0;
	        while (TRUE) {
	                if ((result = gnome_vfs_read (handle, data, 1024, &bytes_read)) != GNOME_VFS_OK) {
	                        dialog_information (_("An error occurred while trying to read file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
	                        break;
	                }
	                if (bytes_read == 0) break;
	                else if (bytes_read <= 1024) data [bytes_read] = '\0';
	                else g_assert_not_reached ();
	                for (k = 0; k < bytes_read; k++) file->data [k + j] = data [k];
	                file->size = j + bytes_read;
	                file->data = g_renew (gchar, file->data, j + 1024);
	        }
	        if (result == GNOME_VFS_OK) 
		        if (gp_camera_file_put (camera, file, path) != GP_OK) 
				dialog_information (_("Could not upload '%s' into folder '%s'!"), filename, path);
	}

	/* Clean up. */
        gp_file_free (file);
	g_free (filename);
	g_free (path);
}

void
save_common (Camera* camera, gchar* path, gchar* filename, gboolean file, gchar* filename_user)
{
        CameraFile*     	camera_file;
        gint            	return_status;
        GnomeVFSResult          result;
        GnomeVFSHandle*         handle;
        GnomeVFSURI*            uri;
        GnomeVFSFileSize        file_size;

	/* Be sure path and filename are ours. */
	path = g_strdup (path);
	filename = g_strdup (filename);

        /* Get the file/preview from gphoto backend. */
        camera_file = gp_file_new ();
        if (file) {
                return_status = gp_camera_file_get (camera, camera_file, path, filename);
        } else {
                return_status = gp_camera_file_get_preview (camera, camera_file, path, filename);
        }
        if (return_status == GP_ERROR) {
                dialog_information (_("Could not get file '%s/%s' from camera!"), path, filename);
        } else {

                /* Let gnome-vfs save the file. */
                uri = gnome_vfs_uri_new (filename_user);
                if ((result = gnome_vfs_create_uri (&handle, uri, GNOME_VFS_OPEN_WRITE, FALSE, 0644)) != GNOME_VFS_OK) {
                        dialog_information (gnome_vfs_result_to_string (result));
                } else {
                        if ((result = gnome_vfs_write (handle, camera_file->data, camera_file->size, &file_size)) != GNOME_VFS_OK) {
                                dialog_information (gnome_vfs_result_to_string (result));
                        }
                        if ((result = gnome_vfs_close (handle)) != GNOME_VFS_OK) {
                                dialog_information (gnome_vfs_result_to_string (result));
                        }
                }
                g_free (uri);
        }

	/* Clean up. */
        gp_frontend_progress (camera, NULL, 0.0);
        gp_file_free (camera_file);
	g_free (path);
	g_free (filename_user);
	g_free (filename);
}

void 
save (GladeXML* xml, Camera* camera, gchar* path, gchar* filename, gboolean file, gboolean temporary)
{
	GConfValue*	value;
	GConfClient*	client;
	GnomeApp*	app;
        gchar*          filename_user;

	g_assert (xml != NULL);
	g_assert ((app = GNOME_APP (glade_xml_get_widget (xml, "app"))) != NULL);
	g_assert ((client = gtk_object_get_data (GTK_OBJECT (app), "client")) != NULL);

	if (temporary) {
		filename_user = g_strdup_printf ("file:/tmp/%s", filename);
	} else {
		g_assert ((value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL)));
		g_assert (value->type == GCONF_VALUE_STRING);
		filename_user = g_strdup_printf ("%s/%s", gconf_value_get_string (value), filename);
	}
	save_common (camera, path, filename, file, filename_user);
}

void 
save_as (GladeXML* xml, Camera* camera, gchar* path_orig, gchar* filename_orig, gboolean file)
{
	GladeXML*		xml_fileselection;
	GtkObject*		object;
	GtkFileSelection*	fileselection;
	gchar*			path;
	gchar*			filename;

	g_assert (xml != NULL);
	g_assert (camera != NULL);
	g_assert (path_orig != NULL);
	g_assert (filename_orig != NULL);

	/* Filename and path will be freed afterwards. */
	filename = g_strdup (filename_orig);
        path = g_strdup (path_orig);

	/* Pop up the file selection dialog. */
	g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")) != NULL);
	g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (xml_fileselection, "fileselection"))) != NULL);

	/* Suggest the filename. */
	gtk_file_selection_set_filename (fileselection, filename);

	/* Store some data in the ok button. */
	g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))) != NULL);
	gtk_object_set_data (object, "camera", camera);
	gtk_object_set_data (object, "path", path);
	gtk_object_set_data (object, "filename", filename);
	gtk_object_set_data (object, "xml", xml);
	gtk_object_set_data (object, "fileselection", fileselection);
	gtk_object_set_data (object, "save", GINT_TO_POINTER (1));
	if (file) gtk_object_set_data (object, "file", GINT_TO_POINTER (1));
	else gtk_object_set_data (object, "file", GINT_TO_POINTER (0));

	/* Store some data in the cancel button. */
	g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))) != NULL);
	gtk_object_set_data (object, "path", path);
	gtk_object_set_data (object, "filename", filename);
	gtk_object_set_data (object, "fileselection", fileselection);

	/* Connect the signals. */
	glade_xml_signal_autoconnect (xml_fileselection);
}

void 
save_all_selected (GtkTree* tree, gboolean file, gboolean ask_for_filename, gboolean temporary)
{
	GladeXML*	xml;
        gint            i;
        Camera*         camera;
        gchar*          path;
        gchar*          filename;
	GtkTreeItem*	item;

        g_assert (tree != NULL);
	g_assert (!(ask_for_filename && temporary));
	g_assert ((xml = gtk_object_get_data (GTK_OBJECT (tree), "xml")) != NULL);

	/* Look into folders first. */
	for (i = 0; i < g_list_length (tree->children); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
                g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
		
		/* Is this item a folder? */
		if (!(filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {

			/* Save selected files in subfolders. */
			if (item->subtree) save_all_selected (GTK_TREE (item->subtree), file, ask_for_filename, temporary);
		}
	}

	/* Files. */
	for (i = 0; i < g_list_length (tree->selection); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->selection, i));
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (item), "camera")) != NULL);
                g_assert ((path = gtk_object_get_data (GTK_OBJECT (item), "path")) != NULL);
		if ((filename = gtk_object_get_data (GTK_OBJECT (item), "filename"))) {

			/* Save. */
			if (ask_for_filename) save_as (xml, camera, path, filename, file);
			else save (xml, camera, path, filename, file, temporary);
		}
	}
}

void
delete_all_selected (GtkTree* tree)
{
	GladeXML*	xml;
        gchar*          filename;
	gint		i;
	GtkTreeItem*	item;
	GtkNotebook*	notebook;

        g_assert (tree != NULL);
        g_assert ((xml = gtk_object_get_data (GTK_OBJECT (tree), "xml")) != NULL);
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

