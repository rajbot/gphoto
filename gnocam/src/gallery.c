#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <HTMLparser.h>
#include <HTMLtree.h>
#include <parser.h>
#include <pspell/pspell.h>
#include <gtkhtml/gtkhtml.h>
#include <bonobo.h>
#include <bonobo/bonobo-stream-memory.h>
#include <libgnomevfs/gnome-vfs.h>
#include "gnocam.h"
#include "file-operations.h"
#include "gallery.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*	main_window;

/***************/
/* Definitions */
/***************/

#define HTML_EDITOR_CONTROL_ID "OAFIID:control:html-editor:63c5499b-8b0c-475a-9948-81ec96a9662c"

/**************/
/* Prototypes */
/**************/

void on_editor_drag_data_received	(GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time);

void on_window_gallery_open_activate 		(GtkWidget* widget, gpointer user_data);
void on_window_gallery_save_as_activate		(GtkWidget* widget, gpointer user_data);
void on_window_gallery_clear_activate 		(GtkWidget* widget, gpointer user_data);
void on_window_gallery_close_activate      	(GtkWidget* widget, gpointer user_data);
void on_window_gallery_exit_activate       	(GtkWidget* widget, gpointer user_data);

void on_window_gallery_button_clear_clicked 	(GtkWidget* widget, gpointer user_data);
void on_window_gallery_button_save_as_clicked	(GtkWidget* widget, gpointer user_data);

/*************/
/* Callbacks */
/*************/

void on_editor_drag_data_received (GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* selection_data, guint info, guint time)
{
        CORBA_Object            interface;
        CORBA_Environment       ev;
        BonoboObjectClient*     client;
	BonoboStream*		stream;
	BonoboStreamMem*	stream_mem;
	xmlDocPtr		doc;
	xmlNodePtr		node;
	xmlNodePtr		node_new;
	GList*			filenames;
	gint			i;
	gchar*			buffer;
	gchar*			message;
	gint			buffer_size;

	/* Preliminary stuff. */
	CORBA_exception_init (&ev);
	g_assert ((client = bonobo_widget_get_server (BONOBO_WIDGET (widget))));

	/* Get the editor's contents. */
	g_assert ((interface = bonobo_object_client_query_interface (client, "IDL:Bonobo/PersistStream:1.0", NULL)));
	stream = bonobo_stream_mem_create (NULL, 0, FALSE, TRUE);
	Bonobo_PersistStream_save (interface, (Bonobo_Stream) bonobo_object_corba_objref (BONOBO_OBJECT (stream)), "text/html", &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		message = g_strdup_printf (_("Could not get the editor's contents! (%s)"), bonobo_exception_get_text (&ev));
		gnome_error_dialog_parented (message, main_window);
		g_free (message);
	} else {
		stream_mem = BONOBO_STREAM_MEM (stream);
		buffer = g_malloc (stream_mem->pos + 1);
		memcpy (buffer, stream_mem->buffer, stream_mem->pos);
		buffer[stream_mem->pos] = 0;
		bonobo_object_unref (BONOBO_OBJECT (stream));
		g_assert ((doc = htmlParseDoc (buffer, NULL)));
		g_free (buffer);

		/* Modify the gallery (now as xml file in 'doc'). */
		filenames = gnome_uri_list_extract_filenames (selection_data->data);
		node = xmlDocGetRootElement (doc);
		node = xmlGetLastChild (node);
		for (i = 0; i < g_list_length (filenames); i++) {
			node_new = xmlNewChild (node, NULL, "img", NULL);
			xmlSetProp (node_new, "src", g_strdup_printf ("file:%s", (gchar*) g_list_nth_data (filenames, i)));
		}
		gnome_uri_list_free_strings (filenames);
	
		/* Load the new data into the html-editor. */
		xmlDocDumpMemory (doc, (xmlChar**) &buffer, &buffer_size);
		xmlFreeDoc (doc);
		stream = bonobo_stream_mem_create (buffer, strlen (buffer), FALSE, TRUE);
		g_free (buffer);
		Bonobo_PersistStream_load (interface, (Bonobo_Stream) bonobo_object_corba_objref (BONOBO_OBJECT (stream)), "text/html", &ev);
		if (ev._major != CORBA_NO_EXCEPTION) {
			message = g_strdup_printf (_("Could not set the editor's contents! (%s)"), bonobo_exception_get_text (&ev));
			gnome_error_dialog_parented (message, main_window);
			g_free (message);
		}
		bonobo_object_unref (BONOBO_OBJECT (stream));
	}

	/* Clean up. */
	Bonobo_Unknown_unref (interface, &ev);
	CORBA_Object_release (interface, &ev);
	CORBA_exception_free (&ev);
}

void
on_window_gallery_close_activate (GtkWidget* widget, gpointer user_data)
{
        gtk_widget_destroy (user_data);
}

void
on_window_gallery_exit_activate (GtkWidget* widget, gpointer user_data)
{
        gtk_main_quit ();
}

void
on_window_gallery_open_activate (GtkWidget* widget, gpointer user_data)
{
	gallery_open (user_data);
}

void
on_window_gallery_save_as_activate (GtkWidget* widget, gpointer user_data)
{
	gallery_save_as (user_data);
}

void
on_window_gallery_button_save_as_clicked (GtkWidget* widget, gpointer user_data)
{
	gallery_save_as (user_data);
}

void
on_window_gallery_clear_activate (GtkWidget* widget, gpointer user_data)
{
	gnome_ok_dialog_parented (_("Not yet implemented!"), main_window);
}

void
on_window_gallery_button_clear_clicked (GtkWidget* widget, gpointer user_data)
{
	gnome_ok_dialog_parented (_("Not yet implemented!"), main_window);
}

/*************/
/* Funktions */
/*************/

void gallery_new (void)
{
        GtkTargetEntry          target_table[] = {{"text/uri-list", 0, 0}};
        GtkWidget*              window;
        GtkWidget*              widget;
        BonoboUIComponent*      component;
        BonoboUIContainer*      container;
        BonoboUIVerb            verb [] = {
                BONOBO_UI_UNSAFE_VERB ("Open", on_window_gallery_open_activate),
                BONOBO_UI_UNSAFE_VERB ("SaveAs", on_window_gallery_save_as_activate),
                BONOBO_UI_UNSAFE_VERB ("Close", on_window_gallery_close_activate),
                BONOBO_UI_UNSAFE_VERB ("Clear", on_window_gallery_clear_activate),
                BONOBO_UI_UNSAFE_VERB ("Exit", on_window_gallery_exit_activate),
                BONOBO_UI_UNSAFE_VERB ("About", on_about_activate),
                BONOBO_UI_UNSAFE_VERB ("Preferences", on_preferences_activate),
                BONOBO_UI_VERB_END};

        window = bonobo_window_new ("Gallery", "Gallery");
        container = bonobo_ui_container_new ();
        bonobo_ui_container_set_win (container, BONOBO_WINDOW (window));
        component = bonobo_ui_component_new ("Gallery");
        bonobo_ui_component_set_container (component, bonobo_object_corba_objref (BONOBO_OBJECT (container)));
	bonobo_ui_component_add_verb_list_with_data (component, verb, window);
        bonobo_ui_util_set_ui (component, "", "gnocam-gallery.xml", "Gallery");
        widget = bonobo_widget_new_control (HTML_EDITOR_CONTROL_ID, bonobo_object_corba_objref (BONOBO_OBJECT (container)));
        bonobo_window_set_contents (BONOBO_WINDOW (window), widget);
        gtk_widget_show_all (window);

	/* Store some data. */
	gtk_object_set_data (GTK_OBJECT (window), "editor", widget);

        /* Drag and Drop. */
	gtk_signal_connect (GTK_OBJECT (widget), "drag_data_received", GTK_SIGNAL_FUNC (on_editor_drag_data_received), NULL);
	gtk_drag_dest_set (widget, GTK_DEST_DEFAULT_ALL, target_table, 1, GDK_ACTION_COPY);
}


