#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>
#include <bonobo/bonobo-stream-memory.h>
#include <libgnomevfs/gnome-vfs.h>
#include "information.h"
#include "gnocam.h"
#include "preview.h"
#include "file-operations.h"
#include "cameras.h"
#include "utils.h"

/***************/
/* Definitions */
/***************/

#define EOG_IMAGE_VIEWER_ID "OAFIID:eog_image_viewer:a30dc90b-a68f-4ef8-a257-d2f8ab7e6c9f"

/**********************/
/* External Variables */
/**********************/

extern GConfClient* 	gconf_client;
extern GList*		preview_list;

/**************/
/* Prototypes */
/**************/

void on_preview_close_activate      (GtkWidget* widget, gpointer user_data);
void on_preview_save_activate       (GtkWidget* widget, gpointer user_data);
void on_preview_save_as_activate    (GtkWidget* widget, gpointer user_data);
void on_preview_refresh_activate    (GtkWidget* widget, gpointer user_data);

void on_preview_capture_image_activate 	(GtkWidget* widget, gpointer user_data);
void on_preview_capture_video_activate 	(GtkWidget* widget, gpointer user_data);

void on_preview_properties_activate 	(GtkWidget* widget, gpointer user_data);

void preview_refresh 	(GtkWidget* preview);
void preview_save 	(GtkWidget* preview);
void preview_save_as 	(GtkWidget* preview);

/*************/
/* Callbacks */
/*************/

void
on_preview_capture_image_activate (GtkWidget* widget, gpointer user_data)
{
	capture_image (gtk_object_get_data (GTK_OBJECT (user_data), "camera"));
}

void
on_preview_capture_video_activate (GtkWidget* widget, gpointer user_data)
{
	capture_video (gtk_object_get_data (GTK_OBJECT (user_data), "camera"));
}

void
on_preview_properties_activate (GtkWidget* widget, gpointer user_data)
{
	if (gp_camera_config (gtk_object_get_data (GTK_OBJECT (user_data), "camera")) != GP_OK) dialog_information (_("Could not get camera properties!"));
}

void
on_preview_close_activate (GtkWidget* widget, gpointer user_data)
{
	preview_free (GTK_WIDGET (user_data));
}

void
on_preview_save_activate (GtkWidget* widget, gpointer user_data)
{
        preview_save (GTK_WIDGET (user_data));
}

void
on_preview_save_as_activate (GtkWidget* widget, gpointer user_data)
{
        preview_save_as (GTK_WIDGET (user_data));
}

void
on_preview_refresh_activate (GtkWidget* widget, gpointer user_data)
{
        preview_refresh (GTK_WIDGET (user_data));
}

/*************/
/* Functions */
/*************/

void
preview_refresh (GtkWidget* preview)
{
	Camera*			camera;
        CameraFile*             file;
        CameraFile*             old_file;
        CameraCaptureInfo       info;
	CORBA_Environment	ev;
	CORBA_Object		interface;
	BonoboStream*		stream;
	BonoboObjectClient*	client;

	g_assert (preview);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (preview), "camera")));

        /* Prepare the image. */
        file = gp_file_new ();
        info.type = GP_CAPTURE_PREVIEW;
        info.duration = 0;

        /* Capture. */
        if (gp_camera_capture (camera, file, &info) == GP_OK) {
		if ((old_file = gtk_object_get_data (GTK_OBJECT (preview), "file"))) gp_file_free (old_file);
		gtk_object_set_data (GTK_OBJECT (preview), "file", file);
		if ((client = gtk_object_get_data (GTK_OBJECT (preview), "client"))) {
			CORBA_exception_init (&ev);
			interface = bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistStream:1.0", &ev);
			if (ev._major != CORBA_NO_EXCEPTION) 
				dialog_information (_("Could not connect to the eog image viewer! (%s)"), bonobo_exception_get_text (&ev));
			else {
				g_assert ((stream = bonobo_stream_mem_create (file->data, file->size, FALSE, TRUE)));
				Bonobo_PersistStream_load (interface, (Bonobo_Stream) bonobo_object_corba_objref (BONOBO_OBJECT (stream)), file->type, &ev);
				if (ev._major != CORBA_NO_EXCEPTION) dialog_information (_("Could not display the file! (%s)"), bonobo_exception_get_text (&ev));
				bonobo_object_unref (BONOBO_OBJECT (stream));
				Bonobo_Unknown_unref (interface, &ev);
				CORBA_Object_release (interface, &ev);
			}
			CORBA_exception_free (&ev);
		}
        } else {
                dialog_information (_("Could not get preview from camera!"));
                gp_file_unref (file);
        }
}

void
preview_save (GtkWidget* preview)
{
	Camera*			camera;
	CameraFile*		file;
	GConfValue*		value;
	gchar*			filename;
	GnomeVFSURI*		uri;

	g_assert ((preview));
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (preview), "camera")));

        if ((file = gtk_object_get_data (GTK_OBJECT (preview), "file"))) {
		g_assert ((value = gconf_client_get (gconf_client, "/apps/" PACKAGE "/prefix", NULL)));
		g_assert (value->type == GCONF_VALUE_STRING);
		filename = g_strdup_printf ("%s/%s", gconf_value_get_string (value), file->name);
		uri = gnome_vfs_uri_new (filename);
		g_free (filename);
		camera_file_save (file, uri);
        }
}

void
preview_save_as (GtkWidget* preview)
{
	Camera*			camera;
	CameraFile*		file;

	g_assert ((preview));
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (preview), "camera")));

	if ((file = gtk_object_get_data (GTK_OBJECT (preview), "file"))) {
		gp_file_ref (file);
		camera_file_save_as (file);
	}
}

GtkWidget*
preview_new (Camera* camera)
{
	GtkWidget*		window;
	GtkWidget*		widget;
	BonoboUIComponent*	component;
	BonoboUIContainer*	container;
	BonoboUIVerb		verb [] = {
		BONOBO_UI_UNSAFE_VERB ("Refresh", on_preview_refresh_activate), 
		BONOBO_UI_UNSAFE_VERB ("Save", on_preview_save_activate),
		BONOBO_UI_UNSAFE_VERB ("SaveAs", on_preview_save_as_activate),
		BONOBO_UI_UNSAFE_VERB ("CaptureImage", on_preview_capture_image_activate),
		BONOBO_UI_UNSAFE_VERB ("CaptureVideo", on_preview_capture_video_activate),
		BONOBO_UI_UNSAFE_VERB ("Close", on_preview_close_activate),
		BONOBO_UI_UNSAFE_VERB ("Exit", on_exit_activate),
		BONOBO_UI_UNSAFE_VERB ("Properties", on_preview_properties_activate),
		BONOBO_UI_UNSAFE_VERB ("Preferences", on_preferences_activate),
		BONOBO_UI_UNSAFE_VERB ("Manual", on_manual_activate),
		BONOBO_UI_UNSAFE_VERB ("About", on_about_activate),
		BONOBO_UI_VERB_END};

        g_assert (camera);

	/* Create the interface. */
	window = bonobo_window_new ("Preview", "Preview");
	container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (window));
	component = bonobo_ui_component_new ("Preview");
	bonobo_ui_component_set_container (component, bonobo_object_corba_objref (BONOBO_OBJECT (container)));
	bonobo_ui_component_add_verb_list_with_data (component, verb, window);
	bonobo_ui_util_set_ui (component, "", "gnocam-preview.xml", "Preview");
	if ((widget = bonobo_widget_new_control (EOG_IMAGE_VIEWER_ID, bonobo_object_corba_objref (BONOBO_OBJECT (container))))) {
		bonobo_window_set_contents (BONOBO_WINDOW (window), widget);
		gtk_object_set_data (GTK_OBJECT (window), "client", bonobo_widget_get_server (BONOBO_WIDGET (widget)));
	} else dialog_information (_("Could not start the eog image viewer!"));
	gtk_widget_show_all (window);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (window), "camera", camera);

	/* Ref the camera. */
	gp_camera_ref (camera);

        /* Get a preview. */
        preview_refresh (window);

	return (window);
}

void 
preview_free (GtkWidget* preview)
{
	Camera*		camera;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (preview), "camera")));

	/* Clean up. */
	gp_camera_unref (camera);
	preview_list = g_list_remove (preview_list, preview);
	gtk_widget_destroy (preview);
}



