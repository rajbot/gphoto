#include <config.h>

#include <libgknc/gknc-preview.c>

#include <libgpknc/gpknc-cntrl.h>

#include <gtk/gtkwindow.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkcontainer.h>

int
main (int argc, char **argv)
{
	KncCntrl *c;
	GkncPreview *p;
	GtkWidget *d;

	gtk_init (&argc, &argv);

	c = gpknc_cntrl_new_from_path ("serial:/dev/ttyS0");
	if (!c) g_error ("Could not get control!");

	d = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	p = gknc_preview_new ();
	knc_cntrl_unref (c);
	gtk_widget_show (GTK_WIDGET (p));
	gtk_container_add (GTK_CONTAINER (d), GTK_WIDGET (p));
	g_signal_connect (d, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (d);

	gknc_preview_start (p, c);
	knc_cntrl_unref (c);

	gtk_main ();

	return 0;
}
