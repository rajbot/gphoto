#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#ifdef GNOCAM_USES_GTKHTML
#  include <bonobo.h>
#  include <bonobo/bonobo-stream-memory.h>
#endif
#include "cameras.h"
#include "file-operations.h"
#include "information.h"

#ifdef GNOCAM_USES_GTKHTML

/**************/
/* Prototypes */
/**************/

void on_editor_drag_data_received	(GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time);

void on_app_gallery_open_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_app_gallery_save_as_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_app_gallery_clear_activate 		(GtkMenuItem* menuitem, gpointer user_data);
void on_app_gallery_close_activate      	(GtkMenuItem* menuitem, gpointer user_data);
void on_app_gallery_exit_activate       	(GtkMenuItem* menuitem, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void on_editor_drag_data_received (GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time)
{
        CORBA_Object            interface;
        CORBA_Environment       ev;
        BonoboObjectClient*     client;
	BonoboStream*		stream;
	Bonobo_Stream		bonobo_stream;
	gchar*			text;

	CORBA_exception_init (&ev);
	g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));
	g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistStream:1.0", NULL)));

	/* Get the current content. */
//FIXME:
	g_assert ((stream = bonobo_stream_mem_create ("Sorry, not yet implemented.", strlen ("Sorry, not yet implemented."), FALSE, TRUE)));
	bonobo_stream = bonobo_object_corba_objref (BONOBO_OBJECT (stream));
//	bonobo_stream_client_read_string (bonobo_stream, &text, &ev);
//	g_assert (ev._major == CORBA_NO_EXCEPTION);
//	printf ("TEXT:%s\n", text);
//	bonobo_stream_client_write_string (bonobo_stream, "Sorry, not yet implemented!", TRUE, &ev);
//	g_assert (ev._major == CORBA_NO_EXCEPTION);
//	bonobo_stream_client_read_string (bonobo_stream, &text, &ev);
//	g_assert (ev._major == CORBA_NO_EXCEPTION);
//	printf ("TEXT:%s\n", text);

	/* Add the picture. */
//FIXME: Implement that.

	/* Set the new content. */
//FIXME: Implement that.
	Bonobo_PersistStream_load (interface, bonobo_stream, "text/html", &ev);
	if (ev._major != CORBA_NO_EXCEPTION) dialog_information (_("Could not load the new gallery!"));
	if (ev._major != CORBA_SYSTEM_EXCEPTION) CORBA_Object_release (interface, &ev);
	Bonobo_Unknown_unref (interface, &ev);
	CORBA_exception_free (&ev);
	bonobo_object_unref (BONOBO_OBJECT (stream));
}

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

void
on_app_gallery_open_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	gallery_open (gtk_object_get_data (GTK_OBJECT (menuitem), "xml_gallery"));
}

void
on_app_gallery_save_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{
	gallery_save_as (gtk_object_get_data (GTK_OBJECT (menuitem), "xml_gallery"));
}

void
on_app_gallery_clear_activate (GtkMenuItem* menuitem, gpointer user_data)
{
}

#endif

