#include <gnome.h>
#include <gphoto2.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include "gnocam.h"
#include "file-operations.h"
#include "cameras.h"
#include "preview.h"

/**************/
/* Prototypes */
/**************/

void on_app_preview_destroy	(GtkObject* object, gpointer user_data);

void on_app_preview_close_activate      (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_exit_activate       (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_save_activate       (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_save_as_activate    (GtkMenuItem* menuitem, gpointer user_data);
void on_app_preview_refresh_activate    (GtkMenuItem* menuitem, gpointer user_data);

void on_app_preview_button_refresh_clicked      (GtkButton* button, gpointer user_data);
void on_app_preview_button_save_clicked         (GtkButton* button, gpointer user_data);
void on_app_preview_button_save_as_clicked      (GtkButton* button, gpointer user_data);

/**************/
/* Callbacks. */
/**************/

void
on_app_preview_destroy		(GtkObject* object, gpointer user_data)
{
	Camera*			camera;
	frontend_data_t*	frontend_data;

	g_assert ((camera = gtk_object_get_data (GTK_OBJECT (object), "camera")) != NULL);
        g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

	frontend_data->xml_preview = NULL;
	gp_camera_unref (camera);
}

void
on_app_preview_close_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	GladeXML*	xml_preview;

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
        preview_save (gtk_object_get_data (GTK_OBJECT (menuitem), "camera"));
}

void
on_app_preview_save_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        preview_save_as (gtk_object_get_data (GTK_OBJECT (menuitem), "camera"));
}

void
on_app_preview_refresh_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        preview_refresh (gtk_object_get_data (GTK_OBJECT (menuitem), "camera"));
}

void
on_app_preview_button_refresh_clicked (GtkButton* button, gpointer user_data)
{
        preview_refresh (gtk_object_get_data (GTK_OBJECT (button), "camera"));
}

void
on_app_preview_button_save_clicked (GtkButton* button, gpointer user_data)
{
        preview_save (gtk_object_get_data (GTK_OBJECT (button), "camera"));
}

void
on_app_preview_button_save_as_clicked (GtkButton* button, gpointer user_data)
{
        preview_save_as (gtk_object_get_data (GTK_OBJECT (button), "camera"));
}



