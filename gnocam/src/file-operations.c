#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <bonobo.h>
#include "cameras.h"
#include "file-operations.h"
#include "frontend.h"

/***************/
/* Definitions */
/***************/

#define OPERATION_FILE_UPLOAD		0
#define OPERATION_FILE_SAVE		1
#define OPERATION_GALLERY_OPEN		2
#define OPERATION_GALLERY_SAVE_AS	3

/**********************/
/* External Variables */
/**********************/

extern GConfClient*	gconf_client;
extern GtkWindow*	main_window;

/**************/
/* Prototypes */
/**************/

void on_fileselection_ok_button_clicked 	(GtkButton* button, gpointer user_data);
void on_fileselection_cancel_button_clicked 	(GtkButton* button, gpointer user_data);

void gallery_common (GtkWidget* window, gint operation);
void upload_common (Camera* camera, gchar* path, gchar* filename);

/*************/
/* Callbacks */
/*************/

void
on_fileselection_ok_button_clicked (GtkButton *button, gpointer user_data)
{
	CameraFile*		file;
	GtkFileSelection*	fileselection;
	GtkWidget*		window;
	GtkWidget*		widget;
	CORBA_Object 		interface;
	CORBA_Environment	ev;
	BonoboObjectClient*	client;

	g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (button), "xml_fileselection"), "fileselection"))));

	switch (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "operation"))) {
	case OPERATION_FILE_UPLOAD:
		upload (gtk_object_get_data (GTK_OBJECT (button), "folder"), gtk_file_selection_get_filename (fileselection));
		break;
	case OPERATION_FILE_SAVE: 
		g_assert ((file = gtk_object_get_data (GTK_OBJECT (button), "file")) != NULL);
		camera_file_save (file,  gnome_vfs_uri_new (gtk_file_selection_get_filename (fileselection)));
		gp_file_unref (file);
		break;
	case OPERATION_GALLERY_OPEN:
		g_assert ((window = gtk_object_get_data (GTK_OBJECT (button), "window")));
		g_assert ((widget = gtk_object_get_data (GTK_OBJECT (window), "editor")));
		g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));
		g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistFile:1.0", NULL)));
        	CORBA_exception_init (&ev);
		Bonobo_PersistFile_load (interface, gtk_file_selection_get_filename (fileselection), &ev);
		if (ev._major != CORBA_NO_EXCEPTION) gnome_error_dialog_parented (_("Could not save gallery."), main_window);
	        CORBA_exception_free (&ev);
		break;
	case OPERATION_GALLERY_SAVE_AS:
		g_assert ((window = gtk_object_get_data (GTK_OBJECT (button), "window")));
		g_assert ((widget = gtk_object_get_data (GTK_OBJECT (window), "editor")));
                g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));
                g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistFile:1.0", NULL)));
                CORBA_exception_init (&ev);
                Bonobo_PersistFile_save (interface, gtk_file_selection_get_filename (fileselection), &ev);
                if (ev._major != CORBA_NO_EXCEPTION) gnome_error_dialog_parented (_("Could not load gallery."), main_window);
		CORBA_exception_free (&ev);
                break;
	default:
		g_assert_not_reached ();
	}
	gtk_widget_destroy (GTK_WIDGET (fileselection));
}

void
on_fileselection_cancel_button_clicked (GtkButton *button, gpointer user_data)
{
	GtkFileSelection*	fileselection;
	CameraFile*		file;
	Camera*			camera;

	g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (button), "xml_fileselection"), "fileselection"))));

	switch (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "operation"))) {
	case OPERATION_FILE_UPLOAD:
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (button), "camera")) != NULL);
                gp_camera_unref (camera);
		break;
	case OPERATION_FILE_SAVE:
		g_assert ((file = gtk_object_get_data (GTK_OBJECT (button), "file")) != NULL);
		gp_file_unref (file);
		break;
	case OPERATION_GALLERY_OPEN:
		break;
	default:
		g_assert_not_reached ();
	}
	gtk_widget_destroy (GTK_WIDGET (fileselection));
}

/*************/
/* Functions */
/*************/

void
upload (GtkTreeItem* folder, gchar* filename)
{
	Camera*			camera;
	gchar*			path;
	GladeXML*		xml_fileselection;
	GtkObject*		object;
        gint			k;
	glong			j;
	GnomeVFSHandle*         handle;
        GnomeVFSFileSize        bytes_read;
        GnomeVFSResult          result;
	GnomeVFSURI*		uri = NULL;
        guint8                  data [1025];
	CameraFile*		file;
	gchar*			message;

	g_return_if_fail (folder);
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (folder), "camera"));
	g_return_if_fail (path = (gchar*) gnome_vfs_uri_get_path (gtk_object_get_data (GTK_OBJECT (folder), "uri")));

	if (filename) {
		uri = gnome_vfs_uri_new (filename);
	
		/* Read the data and upload the file. */
		file = gp_file_new ();
	        file->data = NULL;
		strcpy (file->name, gnome_vfs_uri_get_basename (uri));

	        if ((result = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ)) != GNOME_VFS_OK) {
			message = g_strdup_printf (_("An error occurred while trying to open file '%s'.\n(%s)."), filename, gnome_vfs_result_to_string (result));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
	        } else {
	                j = 0;
	                while (TRUE) {
				result = gnome_vfs_read (handle, data, 1024, &bytes_read);
	                        if ((result != GNOME_VFS_OK) && (result != GNOME_VFS_ERROR_EOF)) break;
	                        if (bytes_read == 0) break;
				g_assert (bytes_read <= 1024);

				/* Write the data into the CameraFile. */
				file->data = g_renew (gchar, file->data, j + bytes_read + 1);
	                        for (k = 0; k < bytes_read; k++) {
					file->data [k + j] = data [k];
				}
				file->data [bytes_read] = '\0';
				j+= bytes_read;
	                        file->size = j;
	                }
	                if ((result == GNOME_VFS_OK) || (result == GNOME_VFS_ERROR_EOF)) {
				camera_file_upload (camera, path, file);
				camera_tree_folder_refresh (folder);
			} else {
				message = g_strdup_printf (
					_("An error occurred while trying to read file '%s'.\n(%s)."), 
					filename, 
					gnome_vfs_result_to_string (result));
				gnome_error_dialog_parented (message, main_window);
				g_free (message);
			}
		}
		
		/* Clean up. */
		gp_camera_unref (camera);
		gp_file_unref (file);
		gnome_vfs_uri_unref (uri);
		
	} else {

	        /* Ask the user for a filename. Pop up the file selection dialog. */
	        g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")) != NULL);
	
	        /* Store some data in the ok button. */
	        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))) != NULL);
	        gtk_object_set_data (object, "folder", folder);
	        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
	        gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_UPLOAD));
	
	        /* Store some data in the cancel button. */
	        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))) != NULL);
		gtk_object_set_data (object, "camera", camera);
	        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
		gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_UPLOAD));
	
	        /* Connect the signals. */
	        glade_xml_signal_autoconnect (xml_fileselection);

                /* Reference the camera. */
                gp_camera_ref (camera);
	}
}
		
void
camera_file_upload (Camera* camera, gchar* path, CameraFile* file)
{
	gint	result;
	gchar*	message = NULL;
	
	g_return_if_fail (camera);
	g_return_if_fail (path);
	g_return_if_fail (file);
	
        if ((result = gp_camera_file_put (camera, file, path)) != GP_OK) {
		message = g_strdup_printf (_("Could not upload file '%s' into folder '%s'!\n(%s)"), file->name, path, gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
	}
}

void
camera_file_save (CameraFile* file, GnomeVFSURI* uri)
{
        GnomeVFSResult          result;
        GnomeVFSHandle*         handle;
        GnomeVFSFileSize        file_size;
	gchar*			message;

        g_return_if_fail (file);
        g_return_if_fail (uri);

	/* Save the file. */
        if ((result = gnome_vfs_create_uri (&handle, uri, GNOME_VFS_OPEN_WRITE, FALSE, 0644)) != GNOME_VFS_OK) {
                message = g_strdup_printf (
			_("An error occurred while trying to open file '%s' for write access (%s)."), 
			gnome_vfs_uri_get_basename (uri), 
			gnome_vfs_result_to_string (result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
        } else {
                if ((result = gnome_vfs_write (handle, file->data, file->size, &file_size)) != GNOME_VFS_OK) {
                        message = g_strdup_printf (
				_("An error occurred while trying to write into file '%s' (%s)."), 
				gnome_vfs_uri_get_basename (uri),
				gnome_vfs_result_to_string (result));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
                }
                if ((result = gnome_vfs_close (handle)) != GNOME_VFS_OK) {
                        message = g_strdup_printf (
				_("An error occurred while trying to close the file '%s' (%s)."), 
				gnome_vfs_uri_get_basename (uri), 
				gnome_vfs_result_to_string (result));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
                }
        }
}

void
camera_file_save_as (CameraFile* file)
{
	GladeXML*		xml_fileselection;
        GtkObject*              object;

        g_return_if_fail (file);

        /* Pop up the file selection dialog. */
        g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")) != NULL);

        /* Suggest the filename. */
        gtk_file_selection_set_filename (GTK_FILE_SELECTION (glade_xml_get_widget (xml_fileselection, "fileselection")), file->name);

        /* Store some data in the ok button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))) != NULL);
        gtk_object_set_data (object, "file", file);
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
	gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_SAVE));

        /* Store some data in the cancel button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))) != NULL);
        gtk_object_set_data (object, "file", file);
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
	gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_SAVE));

        /* Connect the signals. */
        glade_xml_signal_autoconnect (xml_fileselection);

	/* The file is ours. */
	gp_file_ref (file);
}

void 
save_all_selected (GtkTree* tree, gboolean preview, gboolean save_as, gboolean temporary)
{
        gint            i;
	GtkTreeItem*	item;

        g_return_if_fail (tree);
	g_return_if_fail (!(save_as && temporary));

	/* Look into folders first. */
	for (i = 0; i < g_list_length (tree->children); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
		
		/* Is this item a folder? */
		if (item->subtree) save_all_selected (GTK_TREE (item->subtree), preview, save_as, temporary);
	}

	/* Save files. */
	for (i = 0; i < g_list_length (tree->selection); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->selection, i));
		if (!item->subtree) save (item, preview, save_as, temporary);
	}
}

void
save (GtkTreeItem* item, gboolean preview, gboolean save_as, gboolean temporary)
{
	gchar*		path = NULL;
	gchar*		filename;
	gchar*		filename_user;
	gchar*		message;
	CameraFile*	file;
	Camera*		camera;
	GConfValue*	value;
	gint		result;
	GnomeVFSURI*	uri = NULL;

	g_return_if_fail (item);
	g_return_if_fail (!(save_as && temporary));
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));
	g_return_if_fail (filename = (gchar*) gnome_vfs_uri_get_basename (uri));
	g_return_if_fail (path = (gchar*) gnome_vfs_uri_get_path (uri));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
        g_return_if_fail (value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL));
        g_return_if_fail (value->type == GCONF_VALUE_STRING);

	/* Assemble the uri. */
	if (temporary) filename_user = g_strdup_printf ("file:%s/%s", g_get_tmp_dir (), filename);
	else filename_user = g_strdup_printf ("%s/%s", gconf_value_get_string (value), filename);
	uri = gnome_vfs_uri_new (filename_user);
	g_free (filename_user);

	/* Check if we already have the file. */
	if (preview) file = gtk_object_get_data (GTK_OBJECT (item), "preview");
	else file = gtk_object_get_data (GTK_OBJECT (item), "file");
	if (!file) {

		/* Get the file/preview from gphoto backend. */
		file = gp_file_new ();
		if (preview) result = gp_camera_file_get_preview (camera, file, path, filename);
		else result = gp_camera_file_get (camera, file, path, filename);
		if (result != GP_OK) {
			if (preview) message = g_strdup_printf (
				_("Could not get preview of file '%s/%s' from camera!\n(%s)"), 
				path, 
				filename, 
				gp_camera_result_as_string (camera, result));
			else message = g_strdup_printf (
				_("Could not get file '%s/%s' from camera!\n(%s)"), 
				path, 
				filename, 
				gp_camera_result_as_string (camera, result));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
			gp_file_free (file);
			file = NULL;
		} else {
			if (preview) gtk_object_set_data (GTK_OBJECT (item), "preview", file);
			else gtk_object_set_data (GTK_OBJECT (item), "file", file);
		}
	}

	/* Save the file. */
	if (file) {
		if (save_as) camera_file_save_as (file);
		else camera_file_save (file, uri);
	}

	/* Clean up. */
	gnome_vfs_uri_unref (uri);
	gp_frontend_progress (camera, NULL, 0.0);
}

void
delete_all_selected (GtkTree* tree)
{
	gint		i;
	GtkTreeItem*	item;

        g_return_if_fail (tree);

        /* Look into folders first. */
        for (i = 0; i < g_list_length (tree->children); i++) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));

                /* Is this item a folder? */
		if (item->subtree) delete_all_selected (GTK_TREE (item->subtree));
        }

        /* Files. */
        for (i = 0; i < g_list_length (tree->selection); i++) {
                item = GTK_TREE_ITEM (g_list_nth_data (tree->selection, i));
                if (!item->subtree) delete (item);
	}
}

void
delete (GtkTreeItem* item)
{
	GnomeVFSURI*	uri;
	gchar*		message;
	Camera*		camera;
	gint		result;
	gchar*		filename;
	gchar*		path;

	g_return_if_fail (item);
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
	g_return_if_fail (path = (gchar*) gnome_vfs_uri_get_path (uri));
	g_return_if_fail (filename = (gchar*) gnome_vfs_uri_get_basename (uri));

	if ((result = gp_camera_file_delete (camera, path, filename)) == GP_OK) camera_tree_item_remove (item);
	else {
		if (strcmp ("/", path) == 0) message = g_strdup_printf (_("Could not delete '/%s'!\n(%s)"), filename, gp_camera_result_as_string (camera, result));
		else message = g_strdup_printf (_("Could not delete '%s/%s'!\n(%s)"), path, filename, gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
	}
}

void
gallery_common (GtkWidget* window, gint operation)
{
        GladeXML*       xml_fileselection;
        GtkObject*      object;

        g_return_if_fail (window);

        /* Pop up the file selection dialog. */
        g_return_if_fail (xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection"));

        /* Store some data in the ok button. */
        g_return_if_fail (object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button")));
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
        gtk_object_set_data (object, "window", window);
        gtk_object_set_data (object, "operation", GINT_TO_POINTER (operation));

        /* Store some data in the cancel button. */
        g_return_if_fail (object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button")));
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
        gtk_object_set_data (object, "operation", GINT_TO_POINTER (operation));

        /* Connect the signals. */
        glade_xml_signal_autoconnect (xml_fileselection);
}

void
gallery_open (GtkWidget* window)
{
	gallery_common (window, OPERATION_GALLERY_OPEN);
}

void
gallery_save_as (GtkWidget* window)
{
	gallery_common (window, OPERATION_GALLERY_SAVE_AS);
}




