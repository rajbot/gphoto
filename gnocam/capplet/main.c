#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <gconf/gconf.h>
#include <capplet-widget.h>

#include "gnocam-capplet-content.h"

/*************/
/* Callbacks */
/*************/

static void
on_try (CappletWidget* widget, gpointer user_data)
{
}

static void
on_revert (CappletWidget* widget, gpointer user_data)
{
}

static void
on_ok (CappletWidget* widget, gpointer user_data)
{
}

static void
on_cancel (CappletWidget* widget, gpointer user_data)
{
}

/*************/
/* Functions */
/*************/

int main (int argc, char** argv)
{
	GError*		error;
	GtkWidget*	content;
	GtkWidget*	capplet;
	
	/* Internationalization */
	bindtextdomain (PACKAGE, GNOME_LOCALEDIR);
	textdomain (PACKAGE);

	if (gnome_capplet_init (PACKAGE, VERSION, argc, argv, NULL, 0, NULL) < 0) return (1);

	if (!gconf_init(argc, argv, &error)) {
		g_assert(error);
		g_warning("GConf init failed: %s!", error->message);
		return (1);
	}

	/* Create the capplet's contents */
	content = gnocam_capplet_content_new ();
	gtk_widget_show (content);

	/* Create the capplet */
	capplet = capplet_widget_new ();
	gtk_widget_show (capplet);
	gtk_container_add (GTK_CONTAINER (capplet), content);

	/* Connect the signals */
	gtk_signal_connect (GTK_OBJECT (capplet), "try", GTK_SIGNAL_FUNC (on_try), content); 
	gtk_signal_connect (GTK_OBJECT (capplet), "revert", GTK_SIGNAL_FUNC (on_revert), content); 
	gtk_signal_connect (GTK_OBJECT (capplet), "ok", GTK_SIGNAL_FUNC (on_ok), content); 
	gtk_signal_connect (GTK_OBJECT (capplet), "cancel", GTK_SIGNAL_FUNC (on_cancel), content);
	
	capplet_gtk_main ();

	return (0);
}

