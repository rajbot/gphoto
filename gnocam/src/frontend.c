#include <gnome.h>
#include <bonobo.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "frontend.h"
#include "gnocam.h"
#include "properties.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*		main_window;
extern BonoboUIComponent*	component;

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera *camera, char *status) 
{
	bonobo_ui_component_set_status (component, status, NULL);
        return (GP_OK);
}

int gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
	//FIXME: Add a progress bar to the UI.
        return (GP_OK);
}

int gp_frontend_message (Camera *camera, char *message)
{
	gnome_ok_dialog_parented (message, main_window);
        return (GP_OK);
}



