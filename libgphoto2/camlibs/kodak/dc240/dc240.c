#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gphoto2.h>
#include <gphoto2-port.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define _(String) (String)
#    define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "dc240.h"
#include "library.h"

int camera_id (CameraText *id) 
{
	strcpy(id->text, "kodak-dc240");

	return (GP_OK);
}

struct camera_to_usb {
	char *name;
	unsigned short idVendor;
	unsigned short idProduct;
} camera_to_usb[] = {
	{ "Kodak DC240", 0x040A, 0x0120 },
	{ "Kodak DC280", 0x040A, 0x0130 },
	{ "Kodak DC3400", 0x040A, 0x0132 },
	{ "Kodak DC5000", 0x040A, 0x0131 },
        { NULL, 0, 0 }
};

/*
  Abilities are based upon what we can do with a DC240.
  Later cameras have a superset of the DC240 feature and are not
  currently supported.
 */
int camera_abilities (CameraAbilitiesList *list) 
{
	CameraAbilities *a;
        int i;

        for (i = 0; camera_to_usb[i].name; i++)
        {
            gp_abilities_new(&a);
            
            strcpy(a->model, camera_to_usb[i].name);
            a->port     = GP_PORT_SERIAL | GP_PORT_USB;
            a->speed[0] = 9600;
            a->speed[1] = 19200;
            a->speed[2] = 38400;
            a->speed[3] = 57600;
            a->speed[4] = 115200;
            a->speed[5] = 0;
            a->usb_vendor  = camera_to_usb[i].idVendor;
            a->usb_product = camera_to_usb[i].idProduct;
            a->operations        = 	GP_OPERATION_CAPTURE_IMAGE;
            a->file_operations   = 	GP_FILE_OPERATION_DELETE | 
                                        GP_FILE_OPERATION_PREVIEW;
            a->folder_operations = 	GP_FOLDER_OPERATION_NONE;
            
            gp_abilities_list_append(list, a);
        }
	return (GP_OK);
}

static short find_usb_device_id (const char *model, unsigned short *idVendor, 
                             unsigned short *idProduct)
{
    short i;
    for (i = 0; i < sizeof (camera_to_usb) / sizeof (struct camera_to_usb); i++)
    {
        if (strcmp (model, camera_to_usb[i].name) == 0)
        {
            *idVendor = camera_to_usb[i].idVendor;
            *idProduct = camera_to_usb[i].idProduct;
            return 1;
        }
    }

    return 0;
}

int camera_init (Camera *camera) 
{
    unsigned short usb_vendor, usb_product;
    int ret;
    gp_port_settings settings;
    DC240Data *dd;

    if (!camera)
        return (GP_ERROR);

    dd = (DC240Data*)malloc(sizeof(DC240Data));
    if (!dd)
        return (GP_ERROR);

    /* First, set up all the function pointers */
    camera->functions->id 		= camera_id;
    camera->functions->abilities 	= camera_abilities;
    camera->functions->init 	        = camera_init;
    camera->functions->exit 	        = camera_exit;
    camera->functions->folder_list_folders      = camera_folder_list_folders;
    camera->functions->folder_list_files	= camera_folder_list_files;
    camera->functions->file_get 	= camera_file_get;
    camera->functions->file_delete 	= camera_file_delete;
    camera->functions->capture 	        = camera_capture;
    camera->functions->summary	        = camera_summary;
    camera->functions->manual 	        = camera_manual;
    camera->functions->about 	        = camera_about;

    switch (camera->port->type) {
    case GP_PORT_SERIAL:
        if ((ret = gp_port_new(&(dd->dev), GP_PORT_SERIAL)) < 0) {
            free(dd);
            return (GP_ERROR);
        }
        strcpy(settings.serial.port, camera->port_info->path);
        settings.serial.speed    = 9600;
        settings.serial.bits     = 8;
        settings.serial.parity   = 0;
        settings.serial.stopbits = 1;
        break;
    case GP_PORT_USB:
        if ((ret = gp_port_new(&(dd->dev), GP_PORT_USB)) < 0) {
            free(dd);
            return (GP_ERROR);
        }
        if (find_usb_device_id (camera->model, &usb_vendor, &usb_product) == 0) {
            gp_port_free(dd->dev);
            free (dd);
            return (GP_ERROR);
        }
        if (gp_port_usb_find_device(dd->dev, usb_vendor, usb_product) == GP_ERROR) {
            gp_port_free(dd->dev);
            free (dd);
            return (GP_ERROR);
        }
        settings.usb.inep       = 0x82;
        settings.usb.outep      = 0x01;
        settings.usb.config     = 1;
        settings.usb.interface  = 0;
        settings.usb.altsetting = 0;
        break;
    default:
        return (GP_ERROR);
    }

    if (gp_port_settings_set(dd->dev, settings) == GP_ERROR) {
        gp_port_free(dd->dev);
        free(dd);
        return (GP_ERROR);
    }

    if (gp_port_open(dd->dev) == GP_ERROR) {
        gp_port_free(dd->dev);
        free(dd);
        return (GP_ERROR);
    }

    gp_port_timeout_set(dd->dev, TIMEOUT);

    if (camera->port->type == GP_PORT_SERIAL) {
        /* Reset the camera to 9600 */
        gp_port_send_break(dd->dev, 1);

        /* Wait for it to reset */
        GP_SYSTEM_SLEEP(1500);

        if (dc240_set_speed(dd, camera->port_info->speed) == GP_ERROR) {
            gp_port_close(dd->dev);
            gp_port_free(dd->dev);
            free(dd);
            return (GP_ERROR);
        }
    }

    /* Open the CF card */
    if (dc240_open(dd) == GP_ERROR) {
        gp_port_close(dd->dev);
        gp_port_free(dd->dev);
        free(dd);
        return (GP_ERROR);
    }

    if (dc240_packet_set_size(dd, HPBS+2) == GP_ERROR) {
        gp_port_close(dd->dev);
        gp_port_free(dd->dev);
        free(dd);
        return (GP_ERROR);
    }

    camera->camlib_data = dd;

    return (GP_OK);
}

int camera_exit (Camera *camera) 
{
    DC240Data *dd = camera->camlib_data;

    if (!dd)
        return (GP_OK);

    dc240_close(dd);

    if (dd->dev) {
        if (gp_port_close(dd->dev) == GP_ERROR)
                { /* camera did a bad, bad thing */ }
        gp_port_free(dd->dev);
    }
    free(dd);
 
    return (GP_OK);
}

int camera_folder_list_folders (Camera *camera, const char *folder, 
				CameraList *list) 
{
    DC240Data *dd = camera->camlib_data;

    return (dc240_get_folders (dd, list, folder));
}

int camera_folder_list_files (Camera *camera, const char *folder, 
			      CameraList *list) 
{
    DC240Data *dd = camera->camlib_data;

    return (dc240_get_filenames (dd, list, folder));
}

int camera_file_get (Camera *camera, const char *folder, const char *filename, 
		     CameraFileType type, CameraFile *file) 
{
	DC240Data *dd = camera->camlib_data;
	
	switch (type) {
	case GP_FILE_TYPE_NORMAL:
		return (dc240_file_action (dd, DC240_ACTION_IMAGE, file,
					   folder, filename));
	case GP_FILE_TYPE_PREVIEW:
		return (dc240_file_action (dd, DC240_ACTION_PREVIEW, file,
					   folder, (char*) filename));
	default:
		return (GP_ERROR_NOT_SUPPORTED);
	}
}

int camera_file_delete (Camera *camera, const char *folder, 
			const char *filename) 
{
    DC240Data *dd = camera->camlib_data;

    return (dc240_file_action (dd, DC240_ACTION_DELETE, NULL, folder, 
    			       filename));
}

int camera_capture (Camera *camera, int capture_type, CameraFilePath *path) 
{
    DC240Data *dd = camera->camlib_data;

    return dc240_capture(dd, path);
}

int camera_summary (Camera *camera, CameraText *summary) 
{
/*	DC240Data *dd = camera->camlib_data; */

	strcpy(summary->text, _("No summary information."));

	return (GP_OK);
}

int camera_manual (Camera *camera, CameraText *manual) 
{
/*	DC240Data *dd = camera->camlib_data; */

	strcpy(manual->text, _("No Manual Available"));

	return (GP_OK);
}

int camera_about (Camera *camera, CameraText *about) 
{
/*	DC240Data *dd = camera->camlib_data; */

	strcpy (about->text, 
		_("Kodak DC240 Camera Library\n"
		"Scott Fritzinger <scottf@gphoto.net>\n"
		"Camera Library for the Kodak DC240, DC280, DC3400 and DC5000 cameras.\n"
		"Rewritten and updated for gPhoto2."));

	return (GP_OK);
}
