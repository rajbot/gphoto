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
#define OPERATION_FILE_SAVE		1 /* Using gnome-vfs */
#define OPERATION_FILE_DOWNLOAD		2 /* Using bonobo-storage-module */

/**********************/
/* External Variables */
/**********************/

extern GConfClient*	gconf_client;
extern GtkWindow*	main_window;

/**************/
/* Prototypes */
/**************/

void on_fileselection_ok_button_clicked 	(GtkWidget* widget, gpointer user_data);
void on_fileselection_cancel_button_clicked 	(GtkWidget* widget, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void
on_fileselection_ok_button_clicked (GtkWidget* widget, gpointer user_data)
{
	GnomeVFSURI* uri = gnome_vfs_uri_new (gtk_file_selection_get_filename (GTK_FILE_SELECTION (gtk_widget_get_ancestor (widget, GTK_TYPE_FILE_SELECTION))));

	g_return_if_fail (uri);

	switch (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "operation"))) {
	case OPERATION_FILE_UPLOAD:
		upload (gtk_object_get_data (GTK_OBJECT (widget), "folder"), uri);
		break;
	case OPERATION_FILE_DOWNLOAD:
		download (gtk_object_get_data (GTK_OBJECT (widget), "folder"), uri, (gtk_object_get_data (GTK_OBJECT (widget), "widget") != NULL));
		break;
	case OPERATION_FILE_SAVE: 
		camera_file_save (gtk_object_get_data (GTK_OBJECT (widget), "file"), uri); 
		break;
	default:
		g_assert_not_reached ();
	}

	/* Clean up. */
	gnome_vfs_uri_unref (uri);
	gtk_widget_destroy (gtk_widget_get_ancestor (widget, GTK_TYPE_FILE_SELECTION));
}

void
on_fileselection_cancel_button_clicked (GtkWidget* widget, gpointer user_data)
{
	gtk_widget_destroy (gtk_widget_get_ancestor (widget, GTK_TYPE_FILE_SELECTION));
}

/*************/
/* Functions */
/*************/

void
download (GtkTreeItem* folder, GnomeVFSURI* uri, gboolean preview)
{
	g_return_if_fail (folder);
	
	if (uri) {
        	CORBA_Environment       ev;
	        CORBA_Environment       dummy;
		Bonobo_Storage		corba_storage = gtk_object_get_data (GTK_OBJECT (folder), "corba_storage");
	        Bonobo_Stream           corba_stream_source;
	        BonoboStream*           stream_destination;
	        Bonobo_Stream_iobuf*    buffer;
	        gint                    mode;
        
        	g_return_if_fail (corba_storage);

                /* Init exception. */
                CORBA_exception_init (&ev);
                CORBA_exception_init (&dummy);

                /* Download. */
                if (preview) mode = Bonobo_Storage_READ | Bonobo_Storage_COMPRESSED;
                else mode = Bonobo_Storage_READ;
                corba_stream_source = Bonobo_Storage_openStream (corba_storage, 
			gnome_vfs_uri_get_basename (gtk_object_get_data (GTK_OBJECT (folder), "uri")), mode, &ev);
                if (!BONOBO_EX (&ev)) {
                        Bonobo_Stream_read (corba_stream_source, 4000000, &buffer, &ev);
                        if (!BONOBO_EX (&ev)) {
                                stream_destination = bonobo_stream_open_full (
                                        "fs", gnome_vfs_uri_get_path (uri), Bonobo_Storage_WRITE | Bonobo_Storage_CREATE, 0664, &ev);
                                if (!BONOBO_EX (&ev)) {
                                        Bonobo_Stream_write (bonobo_stream_corba_object_create (BONOBO_OBJECT (stream_destination)), buffer, &ev);
                                        bonobo_object_unref (BONOBO_OBJECT (stream_destination));
                                }
                                CORBA_free (buffer);
                        }
                        Bonobo_Stream_unref (corba_stream_source, &dummy);
                }

                /* Display error message (if any). */
                if (BONOBO_EX (&ev)) {
			gchar* name = gnome_vfs_unescape_string_for_display (gnome_vfs_uri_get_basename (uri));
                        gchar* tmp = g_strdup_printf (_("Could not download the file '%s'!\n(%s)"), name, bonobo_exception_get_text (&ev));
			g_free (name);
                        gnome_error_dialog_parented (tmp, main_window);
                        g_free (tmp);
                }

                /* Free exception. */
                CORBA_exception_free (&ev);
                CORBA_exception_free (&dummy);

        } else {
	
		/* Ask the user for a filename. Pop up the file selection dialog. */
		GladeXML* xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection");

                /* Store some data in the ok button. */
        	GtkObject* object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"));
                gtk_object_set_data (object, "folder", folder);
		if (preview) gtk_object_set_data (object, "preview", GINT_TO_POINTER (1));
                gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_DOWNLOAD));

                /* Connect the signals. */
                glade_xml_signal_autoconnect (xml_fileselection);
        }
}

void
upload (GtkTreeItem* folder, GnomeVFSURI* uri)
{
	g_return_if_fail (folder);

	if (uri) {
		gchar*			tmp;
		CORBA_Environment	ev;
		CORBA_Environment	dummy;
		Bonobo_Storage		corba_storage = gtk_object_get_data (GTK_OBJECT (folder), "corba_storage");
		Bonobo_Stream		corba_stream_source;
		Bonobo_Stream		corba_stream_destination;
		Bonobo_Stream_iobuf*	buffer;

		g_return_if_fail (corba_storage);

		/* Init exception. */
		CORBA_exception_init (&ev);
		CORBA_exception_init (&dummy);

		/* Upload. */
		tmp = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
		corba_stream_source = bonobo_get_object (tmp, "IDL:Bonobo/Stream:1.0", &ev);
		g_free (tmp);
		if (!BONOBO_EX (&ev)) {
			Bonobo_Stream_read (corba_stream_source, 4000000, &buffer, &ev);
			if (!BONOBO_EX (&ev)) {
				if (!BONOBO_EX (&ev)) {
					corba_stream_destination = Bonobo_Storage_openStream (corba_storage,
						gnome_vfs_uri_get_basename (uri), Bonobo_Storage_WRITE, &ev);
					if (!BONOBO_EX (&ev)) {
						Bonobo_Stream_write (corba_stream_destination, buffer, &ev);
						if (!BONOBO_EX (&ev)) Bonobo_Stream_commit (corba_stream_destination, &ev);
						Bonobo_Stream_unref (corba_stream_destination, &dummy);
					}
					CORBA_free (buffer);
				}
			}
			Bonobo_Stream_unref (corba_stream_source, &dummy);
		}

		/* Display error message (if any). */
		if (BONOBO_EX (&ev)) {
			tmp = g_strdup_printf (_("Could not upload the file!\n(%s)"), bonobo_exception_get_text (&ev));
			gnome_error_dialog_parented (tmp, main_window);
			g_free (tmp);
		}

		/* Free exception. */
		CORBA_exception_free (&ev);
		CORBA_exception_free (&dummy);

		/* Add the new file to the tree. */
		camera_tree_file_add (
			GTK_TREE (folder->subtree), 
			gnome_vfs_uri_append_file_name (gtk_object_get_data (GTK_OBJECT (folder), "uri"), gnome_vfs_uri_get_basename (uri)));
		
	} else {

	        /* Ask the user for a filename. Pop up the file selection dialog. */
		GladeXML* xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection");
	
	        /* Store some data in the ok button. */
		GtkObject* object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"));
	        gtk_object_set_data (object, "folder", folder);
	        gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_UPLOAD));
	
	        /* Connect the signals. */
	        glade_xml_signal_autoconnect (xml_fileselection);
	}
}

void
camera_file_save (CameraFile* file, GnomeVFSURI* uri)
{
	g_return_if_fail (file);

	if (uri) {
	        GnomeVFSResult          result;
	        GnomeVFSHandle*         handle;
	        GnomeVFSFileSize        file_size;

		/* Save the file. */
	        if ((result = gnome_vfs_create_uri (&handle, uri, GNOME_VFS_OPEN_WRITE, FALSE, 0644)) == GNOME_VFS_OK) {
	                if ((result = gnome_vfs_write (handle, file->data, file->size, &file_size)) == GNOME_VFS_OK) result = gnome_vfs_close (handle);
			else gnome_vfs_close (handle);
	        }

		/* Report errors (if any). */
		if (result != GNOME_VFS_OK) {
			gchar* message = g_strdup_printf (_("Could not save file!\n(%s)"), gnome_vfs_result_to_string (result));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
		}
		
	} else {

	        /* Pop up the file selection dialog. */
		GladeXML* xml_fileselection = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "fileselection");

	        /* Store some data in the ok button. */
		GtkObject* object = GTK_OBJECT (glade_xml_get_widget (xml_fileselection, "fileselection_ok_button"));
		gtk_object_set_data (object, "operation", GINT_TO_POINTER (OPERATION_FILE_SAVE));

		/* Suggest the filename. */
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (glade_xml_get_widget (xml_fileselection, "fileselection")), file->name);
		
		/* Ref the file. */
		gp_file_ref (file);
	        gtk_object_set_data_full (object, "file", file, (GtkDestroyNotify) gp_file_unref);
	
	        /* Connect the signals. */
	        glade_xml_signal_autoconnect (xml_fileselection);
	}
}

void 
save_all_selected (GtkTree* tree, gboolean preview, gboolean save_as)
{
        gint            i;
	GtkTreeItem*	item;

        g_return_if_fail (tree);

	/* Look into folders first. */
	for (i = 0; i < g_list_length (tree->children); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->children, i));
		
		/* Is this item a folder? */
		if (item->subtree) save_all_selected (GTK_TREE (item->subtree), preview, save_as);
	}

	/* Save files. */
	for (i = 0; i < g_list_length (tree->selection); i++) {
		item = GTK_TREE_ITEM (g_list_nth_data (tree->selection, i));
		if (save_as) download (item, NULL, preview);
		else save (item, preview);
	}
}

void
save (GtkTreeItem* item, gboolean preview)
{
	gchar*		tmp;
	GConfValue*	value;
	GnomeVFSURI*	uri;

	g_return_if_fail (item);
        g_return_if_fail (value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL));
        g_return_if_fail (value->type == GCONF_VALUE_STRING);

	/* Save the file in the 'tmp' directory. */
	tmp = g_strdup_printf ("%s/%s", gconf_value_get_string (value), gnome_vfs_uri_get_basename (gtk_object_get_data (GTK_OBJECT (item), "uri")));
	uri = gnome_vfs_uri_new (tmp);
	g_free (tmp);
	download (item, uri, preview);
	gnome_vfs_uri_unref (uri);
}

void
delete (GtkTreeItem* item) 
{
	CORBA_Environment	ev;
	gchar*			file;
	Bonobo_Storage		corba_storage;
	GnomeVFSURI*		uri;

	g_return_if_fail (uri = gtk_object_get_data (GTK_OBJECT (item), "uri"));
	g_return_if_fail (corba_storage = gtk_object_get_data (GTK_OBJECT (item), "corba_storage"));
		
	/* Init exception. */
	CORBA_exception_init (&ev);

	/* Delete the file. */
	file = gnome_vfs_unescape_string_for_display (gnome_vfs_uri_get_basename (uri));
	Bonobo_Storage_erase (corba_storage, file, &ev);
	if (BONOBO_EX (&ev)) {
		gchar* tmp = g_strdup_printf (_("Could not erase '%s'!\n(%s)"), file, bonobo_exception_get_text (&ev));
	        gnome_error_dialog_parented (tmp, main_window);
	        g_free (tmp);
	} else gtk_container_remove (GTK_CONTAINER (GTK_WIDGET (item)->parent), GTK_WIDGET (item));

	/* Clean up. */
	CORBA_exception_free (&ev);
	g_free (file);
}



