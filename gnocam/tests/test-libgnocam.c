#include <config.h>

#include <gtk/gtkmain.h>

#include <bonobo/bonobo-main.h>

#include <libgnocam/gnocam-chooser.h>

int
main (int argc, char **argv)
{
	GnocamChooser *c;

	bonobo_init (&argc, argv);
	gtk_init (&argc, &argv);

	g_log_set_fatal_mask (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL);

	c = gnocam_chooser_new ();
	gtk_widget_show (GTK_WIDGET (c));
	g_signal_connect (c, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();

	return 0;
}
