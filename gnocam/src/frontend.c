#include <gnome.h>
#include <glade/glade.h>
#include <gphoto2.h>
#include "frontend.h"
#include "information.h"
#include "gnocam.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;

/*************/
/* Functions */
/*************/

int gp_frontend_status (Camera *camera, char *status) 
{
        gnome_appbar_set_status (GNOME_APPBAR (glade_xml_get_widget (xml, "appbar")), status);
        return (GP_OK);
}

int gp_frontend_progress (Camera *camera, CameraFile *file, float percentage)
{
        gnome_appbar_set_progress (GNOME_APPBAR (glade_xml_get_widget (xml, "appbar")), percentage / 100);
        return (GP_OK);
}

int gp_frontend_message (Camera *camera, char *message)
{
        dialog_information (message);
        return (GP_OK);
}

int gp_frontend_confirm (Camera *camera, char *message)
{
        //FIXME
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
                camera_properties (xml, camera, window);
                return (GP_PROMPT_CANCEL);
        }
}


