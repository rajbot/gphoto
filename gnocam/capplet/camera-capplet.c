#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <capplet-widget.h>
#include <gphoto2/gphoto2.h>
#include "preferences.h"
#include "capplet.h"


int 
gp_frontend_status (Camera *camera, char *status)
{
        //FIXME: Yet to come...
        return (0);
}


int 
gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
        //FIXME: Yet to come...
        return (0);
}

int
gp_frontend_message (Camera *camera, char *message)
{
        //FIXME: Do that better...
        gnome_error_dialog (message);
        return (0);
}

int
gp_frontend_confirm (Camera *camera, char *message)
{
        //FIXME: Yet to come...
        return (0);
}

int
gp_frontend_prompt (Camera *camera, CameraWidget *window)
{
        //FIXME: Yet to come...
        return (0);
}

gint
main (gint argc, char *argv[])
{
	gint init_result, token;
	GnomeClient *client = NULL;
	GnomeClientFlags flags;
	gchar *session_args[3];
	GtkWidget *capplet;

//	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	/* Init several libraries. */
	gp_init (GP_DEBUG_NONE);
	gp_frontend_register (gp_frontend_status, gp_frontend_progress, gp_frontend_message, gp_frontend_confirm, gp_frontend_prompt);
	init_result = gnome_capplet_init ("camera-capplet", VERSION, argc, argv, NULL, 0, NULL);

	/* ? */
	//FIXME: Don't ask me what's happening here...
	if (init_result < 0) {
		g_warning (_("An initialization error occurred while starting 'camera-capplet'.\nAborting...\n"));
		exit (1);
	} else if (init_result == 3) {
		printf ("Got 3\n");
		return (0);
	} else if (init_result == 4) {
		printf ("Got 4\n");
		return (0);
	} else {
		client = gnome_master_client ();
		flags = gnome_client_get_flags (client);
		if (flags & GNOME_CLIENT_IS_CONNECTED) {
			token = gnome_startup_acquire_token ("GNOME_CAMERA_PROPERTIES", gnome_client_get_id (client));
			if (token) {
				session_args[0] = argv[0];
				session_args[1] = "--init-session-settings";
				session_args[3] = NULL;
				gnome_client_set_priority (client, 20);
				gnome_client_set_restart_style (client, GNOME_RESTART_ANYWAY);
				gnome_client_set_restart_command (client, 2, session_args);
			} else {
				gnome_client_set_restart_style (client, GNOME_RESTART_NEVER);
				gnome_client_flush (client);
			}
		} else {
			token = 1;
		}
		if (init_result != 1) {

			/* Create the capplet. */
			capplet = camera_capplet_new ();

			/* Fill it with current settings. */
			preferences_read (capplet);

			/* Start main loop. */
			capplet_gtk_main ();
		}
		return (0);
	}
}
