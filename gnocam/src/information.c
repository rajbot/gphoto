#include <config.h>
#include <gnome.h>
#include <glade/glade.h>
#include "information.h"

/* Static Variables */

static GladeXML*	xml_information = NULL;
static gchar*		message = NULL;

/* Prototypes */

void on_dialog_information_button_clear_clicked	(GtkButton* button, gpointer user_data);
void on_dialog_information_button_close_clicked (GtkButton* button, gpointer user_data);

/* Callbacks */

void on_dialog_information_button_clear_clicked (GtkButton* button, gpointer user_data) 
{
	g_free (message);
	message = NULL;
	gnome_less_clear (GNOME_LESS (glade_xml_get_widget (xml_information, "less")));
}

void on_dialog_information_button_close_clicked (GtkButton* button, gpointer user_data)
{
	gtk_widget_destroy (glade_xml_get_widget (xml_information, "dialog_information"));
	xml_information = NULL;
}

/* Functions */

void 
dialog_information (const gchar* formatted_message, ...)
{
	GnomeLess*	gnome_less;
	gchar* 		old_message;
	gchar*		new_message;
	va_list		va;

	/* Get the new message. */
	va_start (va, formatted_message);
	new_message = g_strdup_vprintf (formatted_message, va);
	va_end (va);

	if (xml_information) {
		g_assert ((gnome_less = GNOME_LESS (glade_xml_get_widget (xml_information, "less"))) != NULL);
		if (message) {
			old_message = message;
			message = g_strdup_printf ("%s\n--\n%s", old_message, new_message);
			g_free (old_message);
		} else message = g_strdup (new_message);
	} else {
		/* Pop up the information dialog. */
		g_assert ((xml_information = glade_xml_new (GNOCAM_GLADEDIR "gnocam.glade", "dialog_information")) != NULL);
		g_assert ((gnome_less = GNOME_LESS (glade_xml_get_widget (xml_information, "less"))) != NULL);
		message = g_strdup (new_message);
		glade_xml_signal_autoconnect (xml_information);
	}
	gtk_object_set_data (GTK_OBJECT (gnome_less), "message", message);
	gnome_less_show_string (gnome_less, message);
	g_free (new_message);
}

