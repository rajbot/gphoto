#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "frontend.h"
#include "information.h"
#include "gnocam.h"
#include "properties.h"
#include "information.h"

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera *camera, char *status) 
{
	//FIXME: Put this into the statusbar.
	dialog_information (_("Status: %s"), status);
        return (GP_OK);
}

int gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
	//FIXME: Put this into the statusbar.
	dialog_information (_("Progress: %f"), percentage);
        return (GP_OK);
}

int gp_frontend_message (Camera *camera, char *message)
{
        dialog_information (message);
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


