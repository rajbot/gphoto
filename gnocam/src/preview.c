#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gconf/gconf-client.h>
#include "information.h"
#include "gnocam.h"
#include "preview.h"
#include "save.h"

void
preview_refresh (Camera* camera)
{
	GladeXML*		xml_preview;
        frontend_data_t*        frontend_data;
        CameraFile*             file;
        CameraFile*             old_file;
        CameraCaptureInfo       info;
        GdkPixbuf*              pixbuf;
        GdkPixbufLoader*        loader;
        GdkPixmap*              pixmap;
        GdkBitmap*              bitmap;
        GtkWidget*              widget;

	g_assert (camera != NULL);
        g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

        xml_preview = frontend_data->xml_preview;

        /* Prepare the image. */
        file = gp_file_new ();
        info.type = GP_CAPTURE_PREVIEW;
        info.duration = 0;

        /* Capture. */
        if (gp_camera_capture (camera, file, &info) == GP_OK) {
		if ((old_file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "file"))) gp_file_free (old_file);
		gtk_object_set_data (GTK_OBJECT (glade_xml_get_widget (xml_preview, "app_preview")), "file", file);

		/* Load the image. */
                g_assert ((loader = gdk_pixbuf_loader_new ()) != NULL);
                if (gdk_pixbuf_loader_write (loader, file->data, file->size)) {
                        gdk_pixbuf_loader_close (loader);
                        pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
                        gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, 127);
                        gdk_pixbuf_unref (pixbuf);
                        g_assert ((widget = glade_xml_get_widget (xml_preview, "app_preview_pixmap")) != NULL);
                        gtk_pixmap_set (GTK_PIXMAP (widget), pixmap, bitmap);
                } else dialog_information (_("Could not load image!"));
        } else {
                dialog_information (_("Could not get preview from camera '%s'!"), frontend_data->name);
                gp_file_free (file);
        }
}

void
preview_save (Camera* camera)
{
	CameraFile*		file;
	frontend_data_t*	frontend_data;
	GConfClient*		client;
	GConfValue*		value;
	gchar*			filename;

	g_assert (camera != NULL);
        g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);
	g_assert ((client = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (frontend_data->xml, "app")), "client")) != NULL);

        if ((file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (frontend_data->xml_preview, "app_preview")), "file"))) {
		g_assert ((value = gconf_client_get (client, "/apps/" PACKAGE "/prefix", NULL)));
		g_assert (value->type == GCONF_VALUE_STRING);
		filename = g_strdup_printf ("%s/%s", gconf_value_get_string (value), file->name);
		camera_file_save (file, filename);
		g_free (filename);
        }
}

void
preview_save_as (Camera* camera)
{
	CameraFile*		file;
	frontend_data_t*        frontend_data;

	g_assert (camera != NULL);
	g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	if ((file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (frontend_data->xml_preview, "app_preview")), "file"))) {
		camera_file_save_as (file);
	}
}

