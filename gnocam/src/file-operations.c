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

void upload_common (Camera* camera, gchar* path, gchar* filename);

/*************/
/* Callbacks */
/*************/

void
on_fileselection_ok_button_clicked (GtkButton *button, gpointer user_data)
{
	CameraFile*		file;
	GtkFileSelection*	fileselection;

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

	g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (button), "xml_fileselection"), "fileselection"))));

	switch (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "operation"))) {
	case OPERATION_FILE_UPLOAD:
		break;
	case OPERATION_FILE_SAVE:
		g_assert ((file = gtk_object_get_data (GTK_OBJECT (button), "file")) != NULL);
		gp_file_unref (file);
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
	GladeXML*		xml_fileselection;
	GtkObject*		object;
	GnomeVFSURI*		uri_source;
	gchar*			tmp;
	CORBA_Environment	ev;
	Bonobo_Stream		corba_stream_source;
	Bonobo_Stream		corba_stream_destination;
	Bonobo_Stream_iobuf*	buffer;

	g_return_if_fail (folder);

	if (filename) {

		/* Init exception. */
		CORBA_exception_init (&ev);

		/* Upload. */
		uri_source = gnome_vfs_uri_new (filename);
		tmp = gnome_vfs_uri_to_string (uri_source, GNOME_VFS_URI_HIDE_NONE);
		gnome_vfs_uri_unref (uri_source);
		corba_stream_source = bonobo_get_object (tmp, "IDL:Bonobo/Stream:1.0", &ev);
		g_free (tmp);
		if (!BONOBO_EX (&ev)) {
			Bonobo_Stream_read (corba_stream_source, 4000000, &buffer, &ev);
			if (!BONOBO_EX (&ev)) {
				Bonobo_Stream_unref (corba_stream_source, &ev);
				if (!BONOBO_EX (&ev)) {
					corba_stream_destination = Bonobo_Storage_openStream (
						gtk_object_get_data (GTK_OBJECT (folder), "corba_storage"), filename, Bonobo_Storage_WRITE, &ev);
					if (!BONOBO_EX (&ev)) {
						Bonobo_Stream_write (corba_stream_destination, buffer, &ev);
						if (!BONOBO_EX (&ev)) {
							Bonobo_Stream_commit (corba_stream_destination, &ev);
							if (!BONOBO_EX (&ev)) {
								Bonobo_Stream_unref (corba_stream_destination, &ev);
							}
						}
					}
					CORBA_free (buffer);
				}
			}
		}

		/* Display error message (if any). */
		if (BONOBO_EX (&ev)) {
			tmp = g_strdup_printf (_("Could not upload the file!\n(%s)"), bonobo_exception_get_text (&ev));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}

		/* Free exception. */
		CORBA_exception_free (&ev);

		/* Display the new file. */
		camera_tree_folder_refresh (folder);
		
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
	        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
		gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_UPLOAD));
	
	        /* Connect the signals. */
	        glade_xml_signal_autoconnect (xml_fileselection);
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
	gchar*		folder;
	gchar*		filename;
	gchar*		filename_user;
	gchar*		message;
	CameraFile*	file = NULL;
	Camera*		camera;
	GConfValue*	value;
	gint		result;
	GnomeVFSURI*	uri = NULL;

	g_return_if_fail (item);
	g_return_if_fail (!(save_as && temporary));
	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));
	g_return_if_fail (filename = (gchar*) gnome_vfs_uri_get_basename (uri));
	g_return_if_fail (folder = gnome_vfs_uri_extract_dirname (uri));
	g_return_if_fail (camera = gtk_object_get_data (GTK_OBJECT (item), "camera"));
        g_return_if_fail (value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL));
        g_return_if_fail (value->type == GCONF_VALUE_STRING);

	/* Assemble the uri. */
	if (temporary) filename_user = g_strdup_printf ("file:%s/%s", g_get_tmp_dir (), filename);
	else filename_user = g_strdup_printf ("%s/%s", gconf_value_get_string (value), filename);
	uri = gnome_vfs_uri_new (filename_user);
	g_free (filename_user);

	/* Get the file/preview from gphoto backend. */
	file = gp_file_new ();
	if (preview) result = gp_camera_file_get_preview (camera, file, folder, filename);
	else result = gp_camera_file_get (camera, file, folder, filename);
	if (result != GP_OK) {
		if (preview) message = g_strdup_printf (
			_("Could not get preview of file '%s/%s' from camera!\n(%s)"), 
			folder, 
			filename, 
			gp_camera_result_as_string (camera, result));
		else message = g_strdup_printf (
			_("Could not get file '%s/%s' from camera!\n(%s)"), 
			folder, 
			filename, 
			gp_camera_result_as_string (camera, result));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
		g_free (folder);
		gp_file_free (file);
		gnome_vfs_uri_unref (uri);
		gp_frontend_progress (camera, NULL, 0.0);
		return;
	} 

	/* Save the file. */
	if (save_as) camera_file_save_as (file);
	else camera_file_save (file, uri);
	gp_file_unref (file);

	/* Clean up. */
	g_free (folder);
	gnome_vfs_uri_unref (uri);
	gp_frontend_progress (camera, NULL, 0.0);
}

void
delete (GtkTreeItem* item) 
{
	CORBA_Environment	ev;
	gchar*			path;
	gchar* 			tmp;

	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Delete the file. */
	path = gnome_vfs_uri_to_string (gtk_object_get_data (GTK_OBJECT (item), "uri"), GNOME_VFS_URI_HIDE_NONE);
	Bonobo_Storage_erase (gtk_object_get_data (GTK_OBJECT (item), "corba_storage"), path, &ev);
	if (BONOBO_EX (&ev)) {
	        tmp = g_strdup_printf (_("Could not erase '%s'!\n(%s)"), path, bonobo_exception_get_text (&ev));
	        gnome_error_dialog_parented (tmp, main_window);
	        g_free (tmp);
	} else camera_tree_item_remove (item);

	/* Clean up. */
	g_free (path);
	CORBA_exception_free (&ev);
}



