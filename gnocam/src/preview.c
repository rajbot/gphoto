#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gconf/gconf-client.h>
#include "information.h"
#include "gnocam.h"
#include "preview.h"
#include "file-operations.h"
#include "cameras.h"

/**********************/
/* External Variables */
/**********************/

extern GConfClient* 	client;

/**************/
/* Prototypes */
/**************/

void on_app_preview_destroy     (GtkObject* object, gpointer user_data);

void on_app_preview_close_activate      (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_exit_activate       (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_save_activate       (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_save_as_activate    (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_refresh_activate    (GtkMenuItem* menuitem, gpointer user_data);

void on_app_preview_button_refresh_clicked      (GtkButton* button, gpointer user_data);
void on_app_preview_button_save_clicked         (GtkButton* button, gpointer user_data);
void on_app_preview_button_save_as_clicked      (GtkButton* button, gpointer user_data);

void preview_refresh 	(GladeXML* xml_preview);
void preview_save 	(GladeXML* xml_preview);
void preview_save_as 	(GladeXML* xml_preview);

/*************/
/* Callbacks */
/*************/

void
on_app_preview_destroy (GtkObject* object, gpointer user_data)
{
        Camera*                 camera;

        g_assert ((camera = gtk_object_get_data (GTK_OBJECT (object), "camera")) != NULL);

        gp_camera_unref (camera);
}

void
on_app_preview_close_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        GladeXML*       xml_preview;

        g_assert ((xml_preview = gtk_object_get_data (GTK_OBJECT (menuitem), "xml_preview")) != NULL);

        gtk_widget_destroy (glade_xml_get_widget (xml_preview, "app_preview"));
}

void
on_app_preview_exit_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        app_clean_up ();
        gtk_main_quit ();
}

void
on_app_preview_save_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        preview_save (gtk_object_get_data (GTK_OBJECT (menuitem), "xml_preview"));
}

void
on_app_preview_save_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        preview_save_as (gtk_object_get_data (GTK_OBJECT (menuitem), "xml_preview"));
}

void
on_app_preview_refresh_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        preview_refresh (gtk_object_get_data (GTK_OBJECT (menuitem), "xml_preview"));
}

void
on_app_preview_button_refresh_clicked (GtkButton* button, gpointer user_data)
{
        preview_refresh (gtk_object_get_data (GTK_OBJECT (button), "xml_preview"));
}

void
on_app_preview_button_save_clicked (GtkButton* button, gpointer user_data)
{
        preview_save (gtk_object_get_data (GTK_OBJECT (button), "xml_preview"));
}

void
on_app_preview_button_save_as_clicked (GtkButton* button, gpointer user_data)
{
        preview_save_as (gtk_object_get_data (GTK_OBJECT (button), "xml_preview"));
}

/*************/
/* Functions */
/*************/

void
preview_refresh (GladeXML* xml_preview)
{
	Camera*			camera;
        CameraFile*             file;
        CameraFile*             old_file;
        CameraCaptureInfo       info;

	g_assert (xml_preview != NULL);
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "camera")));

        /* Prepare the image. */
        file = gp_file_new ();
        info.type = GP_CAPTURE_PREVIEW;
        info.duration = 0;

        /* Capture. */
        if (gp_camera_capture (camera, file, &info) == GP_OK) {
		if ((old_file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "file"))) gp_file_free (old_file);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "file", file);
		pixmap_set (GTK_PIXMAP (glade_xml_get_widget (xml_preview, "app_preview_pixmap")), file);
        } else {
                dialog_information (_("Could not get preview from camera!"));
                gp_file_free (file);
        }
}

void
preview_save (GladeXML* xml_preview)
{
	Camera*			camera;
	CameraFile*		file;
	GConfValue*		value;
	gchar*			filename;

	g_assert ((xml_preview));
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "camera")));

        if ((file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "file"))) {
		g_assert ((value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL)));
		g_assert (value->type == GCONF_VALUE_STRING);
		filename = g_strdup_printf ("%s/%s", gconf_value_get_string (value), file->name);
		gp_file_ref (file);
		camera_file_save (file, filename);
		g_free (filename);
        }
}

void
preview_save_as (GladeXML* xml_preview)
{
	Camera*			camera;
	CameraFile*		file;

	g_assert ((xml_preview));
	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "camera")));

	if ((file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "file"))) {
		gp_file_ref (file);
		camera_file_save_as (file);
	}
}

void
preview_new (Camera* camera)
{
	GladeXML*		xml_preview;
	GtkTreeItem*		item;

        g_assert (camera);
	g_assert ((item = ((frontend_data_t*) camera->frontend_data)->item));
        g_assert ((xml_preview = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "app_preview")) != NULL);

        /* Store some data. */
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_capture_image")), "item", item);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_capture_video")), "item", item);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_properties")), "camera", camera);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_refresh")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_save")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_save_as")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_close")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_exit")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_refresh")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_save")), "xml_preview", xml_preview);
        gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview_button_save_as")), "xml_preview", xml_preview);

        /* Connect the signals. */
        glade_xml_signal_autoconnect (xml_preview);

        /* Reference the camera. */
        gp_camera_ref (camera);

        /* Get a preview. */
        preview_refresh (xml_preview);
}

