#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "cameras.h"

#ifdef GNOCAM_USES_GTKHTML

/**************/
/* Prototypes */
/**************/

void on_app_gallery_close_activate      (GtkMenuItem* menuitem, gpointer user_data);
void on_app_gallery_exit_activate       (GtkMenuItem* menuitem, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void
on_app_gallery_close_activate (GtkMenuItem* menuitem, gpointer user_data)
{
        GladeXML*       xml_gallery;

        g_assert ((xml_gallery = gtk_object_get_data (GTK_OBJECT (menuitem), "xml_gallery")) != NULL);

        gtk_widget_destroy (glade_xml_get_widget (xml_gallery, "app_gallery"));
}

void
on_app_gallery_exit_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	app_clean_up ();
        gtk_main_quit ();
}

#endif

