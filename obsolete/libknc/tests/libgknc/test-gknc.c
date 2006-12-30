#include <config.h>

#include <libknc/knc-cntrl.h>

#include <libgknc/gknc-preview.c>

#include <libgpknc/gpknc-cntrl.h>

#include <gtk/gtkwindow.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkvbox.h>

static KncCntrlRes
func_lock (void *d)
{
	g_mutex_lock ((GMutex *) d);
	return KNC_CNTRL_OK;
}

static void
func_unlock (void *d)
{
	g_mutex_unlock ((GMutex *) d);
}

int
main (int argc, char **argv)
{
	KncCntrl *c;
	GkncPreview *p;
	GtkWidget *vbox, *d, *b;
	GMutex *m;

	g_thread_init (NULL);
	gtk_init (&argc, &argv);

	d = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_show (vbox = gtk_vbox_new (FALSE, 5));
	gtk_container_add (GTK_CONTAINER (d), vbox);
	p = gknc_preview_new ();
	gtk_widget_show (GTK_WIDGET (p));
	gtk_container_add (GTK_CONTAINER (vbox), GTK_WIDGET (p));
	g_signal_connect (d, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (d);
	gtk_widget_show (b = gtk_button_new_with_label ("Nothing"));
	gtk_container_add (GTK_CONTAINER (vbox), b);

	c = gpknc_cntrl_new_from_path ("serial:/dev/ttyS0");
	if (!c) g_error ("Could not get control!");
	m  = g_mutex_new ();
	knc_cntrl_set_func_lock (c, func_lock, func_unlock, m);
	gknc_preview_start (p, c);
	knc_cntrl_unref (c);

	gtk_main ();

	g_mutex_free (m);

	return 0;
}
