#include <config.h>

#include <gtk/gtkmain.h>

#include <libgnome/gnome-program.h>
#include <libgnomeui/gnome-ui-init.h>

#include "gnocam-applet.h"
	
int
main (int argc, char *argv[])
{
	GnoCamApplet *a;

	gnome_program_init ("gnocam-applet", VERSION, LIBGNOMEUI_MODULE,
			    argc, argv, NULL);

	a = gnocam_applet_new ();
	gtk_widget_show (GTK_WIDGET (a));

	gtk_main ();

	return 0;
}
