#include <config.h>
#include <gphoto2.h>
#include <gnome.h>
#include <bonobo.h>
#include <glade/glade.h>

#include "frontend.h"

/**********************/
/* External Variables */
/**********************/

extern GtkWindow*		main_window;
extern BonoboUIComponent*	main_component;

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera* camera, char* status) 
{
	bonobo_ui_component_set_status (main_component, status, NULL);
        return (GP_OK);
}

int gp_frontend_progress (Camera* camera, CameraFile* file, float percentage)
{
	//FIXME: Add a progress bar to the UI.
        return (GP_OK);
}

int gp_frontend_message (Camera* camera, char* message)
{
	gnome_ok_dialog_parented (message, main_window);
        return (GP_OK);
}

int gp_frontend_confirm (Camera* camera, char* message)
{
	GtkWidget*	widget;
	gint		result;

	widget = gnome_dialog_new (message, GNOME_STOCK_BUTTON_YES, GNOME_STOCK_BUTTON_NO, NULL);
	gnome_dialog_set_parent (GNOME_DIALOG (widget), main_window);
	result = gnome_dialog_run_and_close (GNOME_DIALOG (widget));
	gtk_widget_destroy (widget);
	
	if (result == 1) return (GP_PROMPT_CANCEL);
	return (GP_PROMPT_OK);
}



