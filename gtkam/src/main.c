/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gphoto2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "frontend.h"
#include "interface.h"
#include "support.h"

#include "gtkam-main.h"
#include "gtkam-error.h"

/* The Globals */
GtkWidget *gp_gtk_progress_window	= NULL;
int	   gp_gtk_debug			= 0;

int
main (int argc, char *argv[])
{
	GtkWidget *m, *dialog;
	char buf[1024], port[1024], speed[1024], model[1024];
	int x, y, result;
	Camera *camera;
	gchar *msg;

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
#endif

	g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

	gtk_set_locale ();
	gtk_init (&argc, &argv);

	if (argc > 1) {
		if (strcmp(argv[1], "-d")==0)
			gp_gtk_debug = GP_DEBUG_HIGH;
		   else
			gp_gtk_debug = GP_DEBUG_NONE;
	}

	/* Register my callbacks for interaction */
	gp_frontend_register(frontend_status, frontend_progress,
		frontend_message, frontend_confirm, frontend_prompt);

	add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");

	gp_gtk_progress_window = create_progress_window();

	/* Create the main window */
	m = gtkam_main_new ();
        if (gp_setting_get ("gtkam", "width", buf) == GP_OK) {
		x = atoi (buf);
		if (gp_setting_get ("gtkam", "height", buf) == GP_OK) {
			y = atoi (buf);
		        gtk_widget_set_usize (GTK_WIDGET (m), x, y);
		}
	}
	gtk_widget_show (m);
	gtk_signal_connect (GTK_OBJECT (m), "delete_event",
			    GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	/* Retrieve the last camera used by gtkam */
	if (((gp_setting_get ("gtkam", "camera", model) == GP_OK) ||
	     (gp_setting_get ("gtkam", "model", model) == GP_OK)) && 
	    ((gp_setting_get ("gtkam", "port", port) == GP_OK) ||
	     (gp_setting_get ("gtkam", "port name", port) == GP_OK)) && 
	    (gp_setting_get ("gtkam", "speed", speed) == GP_OK)) {
		gp_camera_new (&camera);
		gp_camera_set_model (camera, model);
		if (strcmp (port, "None"))
			gp_camera_set_port_name (camera, port);
		if (atoi (speed))
			gp_camera_set_port_speed (camera, atoi (speed));

		result = gp_camera_init (camera);
		if (result < 0) {
			msg = g_strdup_printf ("Could not initialize '%s' "
					       "on port '%s'", model, port);
			dialog = gtkam_error_new (msg, result, camera, NULL);
			g_free (msg);
			gtk_widget_show (dialog);
		} else
			gtkam_main_set_camera (GTKAM_MAIN (m), camera);
		gp_camera_unref (camera);
	}

	gtk_main ();

	sprintf (buf, "%i", m->allocation.width);
	gp_setting_set ("gtkam", "width", buf);
	sprintf (buf, "%i", m->allocation.height);
	gp_setting_set ("gtkam", "height", buf);

	gp_exit ();

	return 0;
}
