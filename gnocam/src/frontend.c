#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "frontend.h"
#include "gnocam.h"
#include "properties.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml_main;
extern GtkWindow*	main_window;

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera *camera, char *status) 
{
	GtkStatusbar*	statusbar;

	g_assert ((statusbar = (GTK_STATUSBAR (glade_xml_get_widget (xml_main, "main_statusbar")))));

	gtk_statusbar_pop (statusbar, gtk_statusbar_get_context_id (statusbar, "gphoto2"));
	gtk_statusbar_push (statusbar, gtk_statusbar_get_context_id (statusbar, "gphoto2"), status);
        return (GP_OK);
}

int gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
	gtk_progress_set_percentage (GTK_PROGRESS (glade_xml_get_widget (xml_main, "main_progressbar")), percentage/100.0);
        return (GP_OK);
}

int gp_frontend_message (Camera *camera, char *message)
{
	gnome_ok_dialog_parented (message, main_window);
        return (GP_OK);
}

int gp_frontend_confirm (Camera *camera, char *message)
{
        //FIXME: How do we handle that?
        return (GP_CONFIRM_NO);
}

int gp_frontend_prompt (Camera *camera, CameraWidget *window)
{
        frontend_data_t*        frontend_data;
        
        g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);

        /* Is the propertybox opened? */        
        if (frontend_data->xml_properties) {
                values_set (frontend_data->xml_properties, window);
                return (GP_PROMPT_OK);
        } else {
                camera_properties (camera, window);
                return (GP_PROMPT_CANCEL);
        }
}


