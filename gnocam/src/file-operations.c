#include <config.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include <gphoto2.h>
#ifdef GNOCAM_USES_GTKHTML
#  include <bonobo.h>
#endif
#include "information.h"
#include "cameras.h"
#include "file-operations.h"
#include "frontend.h"

/***************/
/* Definitions */
/***************/

#define OPERATION_FILE_UPLOAD	0
#define OPERATION_FILE_SAVE	1
#define OPERATION_GALLERY_OPEN	2
#define OPERATION_GALLERY_SAVE	3

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

void
on_fileselection_ok_button_clicked (GtkButton *button, gpointer user_data)
{
	CameraFile*		file;
	GtkFileSelection*	fileselection;
#ifdef GNOCAM_USES_GTKHTML
	GtkWidget*		window;
	GtkWidget*		widget;
	CORBA_Object 		interface;
	CORBA_Environment	ev;
	BonoboObjectClient*	client;
#endif

	g_assert ((fileselection = GTK_FILE_SELECTION (glade_xml_get_widget (gtk_object_get_data (GTK_OBJECT (button), "xml_fileselection"), "fileselection"))));

	switch (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (button), "operation"))) {
	case OPERATION_FILE_UPLOAD:
		upload (gtk_object_get_data (GTK_OBJECT (button), "folder"), gtk_file_selection_get_filename (fileselection));
		break;
	case OPERATION_FILE_SAVE: 
		g_assert ((file = gtk_object_get_data (GTK_OBJECT (button), "file")) != NULL);
		camera_file_save (file,  gtk_file_selection_get_filename (fileselection));
		gp_file_unref (file);
		break;
#ifdef GNOCAM_USES_GTKHTML
	case OPERATION_GALLERY_OPEN:
		g_assert ((window = gtk_object_get_data (GTK_OBJECT (button), "window")));
		g_assert ((widget = gtk_object_get_data (GTK_OBJECT (window), "editor")));
		g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));
		g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistFile:1.0", NULL)));
        	CORBA_exception_init (&ev);
		Bonobo_PersistFile_load (interface, gtk_file_selection_get_filename (fileselection), &ev);
		if (ev._major != CORBA_NO_EXCEPTION) dialog_information (_("Could not save gallery."));
	        CORBA_exception_free (&ev);
		break;
	case OPERATION_GALLERY_SAVE:
		g_assert ((window = gtk_object_get_data (GTK_OBJECT (button), "window")));
		g_assert ((widget = gtk_object_get_data (GTK_OBJECT (window), "editor")));
                g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));
                g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistFile:1.0", NULL)));
                CORBA_exception_init (&ev);
                Bonobo_PersistFile_save (interface, gtk_file_selection_get_filename (fileselection), &ev);
                if (ev._major != CORBA_NO_EXCEPTION) dialog_information (_("Could not load gallery."));
		CORBA_exception_free (&ev);
                break;
#endif
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
        GnomeVFSURI*            uri;
        GnomeVFSHandle*         handle;
        GnomeVFSFileSize        bytes_read;
        GnomeVFSResult          result;
        guint8                  data [1025];
	CameraFile*		file;

	g_assert (folder != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (folder), "camera")) != NULL);
	g_assert ((path = gtk_object_get_data (GTK_OBJECT (folder), "path")) != NULL);

	if (filename) {

		/* Read the data and upload the file. */
		file = gp_file_new ();
	        file->data = NULL;

		/* Quick hack to get the filename excluding path. */
		for (k = strlen (filename) - 1; k >= 0; k--) {
			if (filename[k] == '/') {
				k++;
				break;
			}
		}
		strcpy (file->name, &filename[k]);

	        uri = gnome_vfs_uri_new (filename);
	        if ((result = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ)) != GNOME_VFS_OK) {
	                dialog_information (_("An error occurred while trying to open file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
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
				dialog_information (_("An error occurred while trying to read file '%s' (%s)."), filename, gnome_vfs_result_to_string (result));
	                        gp_file_free (file);
			}
		}
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
        if (gp_camera_file_put (camera, file, path) != GP_OK) dialog_information (_("Could not upload file '%s' into folder '%s'!"), file->name, path);

	/* Clean up. */
        gp_file_free (file);
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

        g_assert (file != NULL);

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
			gp_file_free (file);
			file = NULL;
		} else {
			if (preview) gtk_object_set_data (GTK_OBJECT (item), "preview", file);
			else gtk_object_set_data (GTK_OBJECT (item), "file", file);
		}
	}

	/* Save the file. */
	if (file) {
		gp_file_ref (file);
		if (save_as) camera_file_save_as (file);
		else {
			camera_file_save (file, filename_user);
			gp_file_unref (file);
		}
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
	else {
		if (strcmp ("/", path) == 0) dialog_information (_("Could not delete '/%s'!"), filename);
		else dialog_information (_("Could not delete '%s/%s'!"), path, filename);
	}
}

void
gallery_open (GtkWidget* window)
{
	GladeXML*	xml_fileselection;
	GtkObject*	object;

	g_assert (window);

        /* Pop up the file selection dialog. */
        g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")));

        /* Store some data in the ok button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))));
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
	gtk_object_set_data (object, "window", window);
        gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_GALLERY_OPEN));

        /* Store some data in the cancel button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))));
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
        gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_GALLERY_OPEN));

        /* Connect the signals. */
        glade_xml_signal_autoconnect (xml_fileselection);
}

void
gallery_save_as (GtkWidget* window)
{
        GladeXML*       xml_fileselection;
        GtkObject*      object;

        g_assert (window);

        /* Pop up the file selection dialog. */
        g_assert ((xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection")));

        /* Store some data in the ok button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"))));
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
        gtk_object_set_data (object, "window", window);
        gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_GALLERY_SAVE));

        /* Store some data in the cancel button. */
        g_assert ((object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_cancel_button"))));
        gtk_object_set_data (object, "xml_fileselection", xml_fileselection);
        gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_GALLERY_SAVE));

        /* Connect the signals. */
        glade_xml_signal_autoconnect (xml_fileselection);
}




