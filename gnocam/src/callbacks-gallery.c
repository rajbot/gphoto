#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <HTMLparser.h>
#include <HTMLtree.h>
#include <parser.h>
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
	xmlDocPtr		doc;
	xmlNodePtr		node;
	xmlNodePtr		node_new;
	GList*			filenames;
	gint			i;

	/***********************************************************************/
	/* This is an ugly hack. I have _no_ idea how to edit a bonobo_stream. */
	/* And, it's ugly code. I wrote it only to show you that it works.     */
	/* Rewrite it. Do it. Now!                                             */
	/***********************************************************************/

	/* Preliminary stuff. */
	CORBA_exception_init (&ev);
	g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));

	/* Get the current content and save it to "/tmp/editor". */
	g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistFile:1.0", NULL)));
	Bonobo_PersistFile_save (interface, "/tmp/editor", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	Bonobo_Unknown_unref (interface, &ev);
	CORBA_Object_release (interface, &ev);

	/* Modify the gallery. */
	filenames = gnome_uri_list_extract_filenames (selection_data->data);
	g_assert ((doc = htmlParseFile ("/tmp/editor", NULL)));
	printf ("\nBefore modification:\n");
	htmlDocDump (NULL, doc);
	node = xmlDocGetRootElement (doc);
	node = xmlGetLastChild (node);
	for (i = 0; i < g_list_length (filenames); i++) {
		node_new = xmlNewChild (node, NULL, "img", NULL);
		xmlSetProp (node_new, "src", g_strdup_printf ("file:%s", (gchar*) g_list_nth_data (filenames, i)));
	}
	printf ("\nAfter modification:\n");
	htmlDocDump (NULL, doc);
	htmlSaveFile ("/tmp/editor", doc);
	gnome_uri_list_free_strings (filenames);

	/* Load "/tmp/editor" into the html-editor. */
	g_assert ((interface =  bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistFile:1.0", NULL)));
	Bonobo_PersistFile_load (interface, "/tmp/editor", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	Bonobo_Unknown_unref (interface, &ev);
	CORBA_Object_release (interface, &ev);

	/* Clean up. */
	CORBA_exception_free (&ev);
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
	dialog_information (_("Not yet implemented!"));
}

#endif

