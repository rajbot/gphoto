#include <config.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gphoto2-library.h>
#include <gphoto2-port-log.h>

#include <libknc/knc-i18n.h>
#include <libknc/knc.h>
#include <libknc/knc-utils.h>

#include <libgpknc/gpknc-cntrl.h>

static int
knc_cntrl2gp (KncCntrlRes r)
{
	switch (r) {
	case KNC_CNTRL_OK:
		return GP_OK;
	case KNC_CNTRL_ERR_CANCEL:
		return GP_ERROR_CANCEL;
	case KNC_CNTRL_ERR_NO_MEMORY:
		return GP_ERROR_NO_MEMORY;
	case KNC_CNTRL_ERR_ILLEGAL_PARAMETER:
		return GP_ERROR_BAD_PARAMETERS;
	case KNC_CNTRL_ERR:
	default: return GP_ERROR;
	}
}

#define CR(result,context) {						\
	KncCntrlRes r = (result);					\
	if (r) {							\
		gp_context_error ((context), knc_cntrl_res_name (r));	\
		return knc_cntrl2gp (r);				\
	}								\
}
#define CCR(result,context) {						\
	KncCamRes r = (result);						\
	if (r) {							\
		gp_context_error ((context), knc_cam_res_name (r));	\
		return GP_ERROR;					\
	}								\
}
#define C(result) {int r=(result); if (r < 0) return r;}
#define C_NULL(r) {if (!(r)) return (GP_ERROR_BAD_PARAMETERS);}
#define PING_TIMEOUT 60

struct _CameraPrivateLibrary {
	KncCntrl *c;
	unsigned int speed, timeout;
};

int
camera_id (CameraText* id)
{
        strcpy (id->text, "konica");

        return (GP_OK);
}

int
camera_abilities (CameraAbilitiesList* list)
{
        int i;
        CameraAbilities a;

        for (i = 0; i < knc_count_devices (); i++) {
                memset(&a, 0, sizeof(a));
                a.status = GP_DRIVER_STATUS_PRODUCTION;
		strcpy (a.model, knc_get_device_manufacturer (i));
		strcat (a.model, ":");
		strcat (a.model, knc_get_device_model (i));
                a.port = GP_PORT_SERIAL;
                a.speed[0]     = 300;
                a.speed[1]     = 600;
                a.speed[2]     = 1200;
                a.speed[3]     = 2400;
                a.speed[4]     = 4800;
                a.speed[5]     = 9600;
                a.speed[6]     = 19200;
                a.speed[7]     = 38400;
                a.speed[8]     = 57600;
                a.speed[9]     = 115200;
                a.speed[10]    = 0;
                a.operations = GP_OPERATION_CONFIG |
                               GP_OPERATION_CAPTURE_IMAGE |
                               GP_OPERATION_CAPTURE_PREVIEW;
                a.file_operations = GP_FILE_OPERATION_DELETE |
                                    GP_FILE_OPERATION_PREVIEW;
                a.folder_operations = GP_FOLDER_OPERATION_DELETE_ALL;
                gp_abilities_list_append (list, a);
        }

        return (GP_OK);
}

static KncCntrlRes
data_func (const unsigned char *buf, unsigned int size, void *data)
{
        CameraFile *file = data;

        gp_file_append (file, buf, size);

        return KNC_CNTRL_OK;
}

static int
timeout_func (Camera *camera, GPContext *context)
{
        CR (knc_get_status (camera->pl->c, NULL, NULL), context);

        return (GP_OK);
}

static int
camera_capture_preview (Camera* camera, CameraFile* file, GPContext *context)
{
	KncCamRes cr;

	knc_cntrl_set_func_data (camera->pl->c, data_func, file);
        CR (knc_get_preview (camera->pl->c, &cr, KNC_PREVIEW_YES), context);
        C (gp_file_set_mime_type (file, GP_MIME_JPEG));

        return (GP_OK);
}

static int
camera_capture (Camera* camera, CameraCaptureType type, CameraFilePath* path,
                GPContext *context)
{
        int r;
        CameraFile *file = NULL;
        CameraFileInfo info;
	KncCamRes cr;
	KncCntrlRes cntrl_res;
	KncImageInfo i;

        C_NULL (camera && path);

        /* We only support capturing of images */
        if (type != GP_CAPTURE_IMAGE) return (GP_ERROR_NOT_SUPPORTED);

        /* Stop the timeout, take the picture, and restart the timeout. */
        gp_camera_stop_timeout (camera, camera->pl->timeout);
	gp_file_new (&file);
	knc_cntrl_set_func_data (camera->pl->c, data_func, file);
        cntrl_res = knc_take_picture (camera->pl->c, &cr, KNC_SOURCE_CARD, &i);
        camera->pl->timeout = gp_camera_start_timeout (camera, PING_TIMEOUT,
                                                       timeout_func);
	if (cntrl_res) gp_file_unref (file);
        CR (cntrl_res, context);

        sprintf (path->name, "%06i.jpeg", (int) i.id);
        strcpy (path->folder, "/");
        r = gp_filesystem_append (camera->fs, path->folder,
				  path->name, context);
	if (r < 0) {
		gp_file_unref (file);
		return r;
	}

        info.preview.fields = GP_FILE_INFO_SIZE | GP_FILE_INFO_TYPE;
	gp_file_get_data_and_size (file, NULL, &info.preview.size);
        strcpy (info.preview.type, GP_MIME_JPEG);

        info.file.fields = GP_FILE_INFO_SIZE | GP_FILE_INFO_PERMISSIONS |
                            GP_FILE_INFO_TYPE | GP_FILE_INFO_NAME;
        info.file.size = i.size;
        info.file.permissions = GP_FILE_PERM_READ;
        if (!i.prot) info.file.permissions |= GP_FILE_PERM_DELETE;
        strcpy (info.file.type, GP_MIME_JPEG);
        snprintf (info.file.name, sizeof (info.file.name),
                  "%06i.jpeg", (int) i.id);
        gp_filesystem_set_info_noop (camera->fs, path->folder, info, context);

        gp_file_set_name (file, info.file.name);
        gp_file_set_mime_type (file, GP_MIME_JPEG);
        gp_file_set_type (file, GP_FILE_TYPE_EXIF);
        gp_filesystem_set_file_noop (camera->fs, path->folder, file, context);
        gp_file_unref (file);

        return (GP_OK);
}

static int
get_file_func (CameraFilesystem *fs, const char *folder, const char *filename,
               CameraFileType type, CameraFile *file, void *data,
               GPContext *context)
{
        Camera *camera = data;
        unsigned long image_id;
        char image_id_string[] = {0, 0, 0, 0, 0, 0, 0};
        unsigned int size;
        CameraFileInfo info;
        int r;
	KncCamRes cr;
	KncCntrlRes cntrl_res = KNC_CNTRL_OK;

        if (strlen (filename) != 11) return (GP_ERROR_FILE_NOT_FOUND);
        if (strcmp (folder, "/")) return (GP_ERROR_DIRECTORY_NOT_FOUND);

        /* Check if we can get the image id from the filename. */
        strncpy (image_id_string, filename, 6);
        image_id = atol (image_id_string);

        /* Get information about the image */
        C (gp_filesystem_get_info (camera->fs, folder,
                                        filename, &info, context));

        /*
         * Remove the timeout, get the image and start the timeout
         * afterwards.
         */
        gp_camera_stop_timeout (camera, camera->pl->timeout);
	knc_cntrl_set_func_data (camera->pl->c, data_func, file);
        switch (type) {
        case GP_FILE_TYPE_PREVIEW:
                size = 2048;
                cntrl_res = knc_get_image (camera->pl->c, &cr,
                        image_id, KNC_SOURCE_CARD, KNC_IMAGE_THUMB);
                break;
        case GP_FILE_TYPE_NORMAL:
                size = info.file.size;
                cntrl_res = knc_get_image (camera->pl->c, &cr,
                        image_id, KNC_SOURCE_CARD, KNC_IMAGE_EXIF);
                break;
        default:
                r = GP_ERROR_NOT_SUPPORTED;
        }
        camera->pl->timeout = gp_camera_start_timeout (camera, PING_TIMEOUT,
                                                       timeout_func);
        CR (cntrl_res, context);
	CCR (cr, context);

        C (gp_file_set_mime_type (file, GP_MIME_JPEG));

        return (GP_OK);
}

static int
delete_file_func (CameraFilesystem *fs, const char *folder,
                  const char *filename, void *data, GPContext *context)
{
        Camera *camera = data;
        char tmp[] = {0, 0, 0, 0, 0, 0, 0};
        unsigned long image_id;
	KncCamRes cr;

        C_NULL (camera && folder && filename);

        /* We don't support folders */
        if (strcmp (folder, "/")) return (GP_ERROR_DIRECTORY_NOT_FOUND);

        /* Extract the image id from the filename */
        strncpy (tmp, filename, 6);
        image_id = atol (tmp);

        CR (knc_erase_image (camera->pl->c, &cr, image_id, KNC_SOURCE_CARD),
	    context);
	CCR (cr, context);

        return (GP_OK);
}

static int
set_speed (Camera *camera, int speed, GPContext *context)
{
        GPPortSettings s;
	KncCamRes cr;
        KncBitRate br;
        KncBitFlag bf;
        int i;
        int speeds[] = {300, 600, 1200, 2400, 4800, 9600, 19200,
                                 38400, 57600, 115200};

        C (gp_port_get_settings (camera->port, &s));
        if ((s.serial.speed == speed) || (s.serial.speed == 115200))
                return (GP_OK);

        switch (speed) {
        case 0:

                /* Set the highest possible speed */
                CR (knc_get_io_pref (camera->pl->c, &cr, &br, &bf), context);
		CCR (cr, context);
                for (i = 9; i >= 0; i--) if ((1 << i) & br) break;
                if (i < 0) return (GP_ERROR_IO_SERIAL_SPEED);
		speed = speeds[i];
                     br = 1 << i;  break;
        case 300   : br = 1 << 0; break;
        case 600   : br = 1 << 1; break;
        case 1200  : br = 1 << 2; break;
        case 2400  : br = 1 << 3; break;
        case 4800  : br = 1 << 4; break;
        case 9600  : br = 1 << 5; break;
        case 19200 : br = 1 << 6; break;
        case 38400 : br = 1 << 7; break;
        case 57600 : br = 1 << 8; break;
        case 115200: br = 1 << 9; break;
        default: return (GP_ERROR_IO_SERIAL_SPEED);
        }

        /* Request the new speed */
        bf = KNC_BIT_FLAG_8_BITS;
        CR (knc_set_io_pref (camera->pl->c, &cr, &br, &bf), context);
	CCR (cr, context);
        s.serial.speed = speed;
        C (gp_port_set_settings (camera->port, s));

        return (GP_OK);
}

static int
delete_all_func (CameraFilesystem *fs, const char* folder, void *data,
                 GPContext *context)
{
        Camera *camera = data;
        unsigned int not_erased = 0;
	KncCamRes cr;

        if (strcmp (folder, "/")) return (GP_ERROR_DIRECTORY_NOT_FOUND);

        CR (knc_erase_all (camera->pl->c, &cr, KNC_SOURCE_CARD, &not_erased),
	    context);
	CCR (cr, context);

        if (not_erased) {
                gp_context_error (context, _("%i pictures could not be "
                        "deleted because they are protected"), not_erased);
                gp_filesystem_reset (camera->fs);
                return (GP_ERROR);
        }

        return (GP_OK);
}

static int
get_info (Camera *camera, unsigned int n, CameraFileInfo *info,
          CameraFile *file, GPContext *context)
{
        unsigned char *buffer = NULL;
	KncCamRes cr;
	KncCntrlRes cntrl_res;
	KncImageInfo i;

        /*
         * Remove the timeout, get the information and restart the
         * timeout afterwards.
         */
        gp_camera_stop_timeout (camera, camera->pl->timeout);
	knc_cntrl_set_func_data (camera->pl->c, data_func, file);
        cntrl_res = knc_get_image_info (camera->pl->c, &cr, n, &i);
        camera->pl->timeout = gp_camera_start_timeout (camera, PING_TIMEOUT,
                                                       timeout_func);
        CR (cntrl_res, context);

        info->audio.fields = GP_FILE_INFO_NONE;

        info->preview.fields = GP_FILE_INFO_TYPE;
        strcpy (info->preview.type, GP_MIME_JPEG);

        info->file.fields = GP_FILE_INFO_SIZE | GP_FILE_INFO_PERMISSIONS |
                            GP_FILE_INFO_TYPE | GP_FILE_INFO_NAME;
        info->file.size = i.size * 1000;
        info->file.permissions = GP_FILE_PERM_READ;
        if (!i.prot) info->file.permissions |= GP_FILE_PERM_DELETE;
        strcpy (info->file.type, GP_MIME_JPEG);
        snprintf (info->file.name, sizeof (info->file.name),
                  "%06i.jpeg", (int) i.id);

        if (file) {
                gp_file_set_type (file, GP_FILE_TYPE_EXIF);
                gp_file_set_name (file, info->file.name);
        } else
                free (buffer);

        return (GP_OK);
}

static int
get_info_func (CameraFilesystem *fs, const char *folder, const char *filename,
               CameraFileInfo *info, void *data, GPContext *context)
{
        Camera *camera = data;
        CameraFile *file;
        int n, result;

        /* We need image numbers starting with 1 */
        n = gp_filesystem_number (camera->fs, folder, filename, context);
        if (n < 0) return (n);
        n++;

        gp_file_new (&file);
        result = get_info (camera, n, info, file, context);
        if (result < 0) {
                gp_file_unref (file);
                return (result);
        }
        gp_filesystem_set_file_noop (fs, folder, file, context);
        gp_file_unref (file);

        return (GP_OK);
}

static int
camera_pre_func (Camera *camera, GPContext *context)
{
        /* Set best speed */
        set_speed (camera, 0, context);

        return (GP_OK);
}

static int
camera_post_func (Camera *camera, GPContext *context)
{
        /* Set default speed */
        set_speed (camera, 9600, context);

        return (GP_OK);
}

static int
camera_exit (Camera* camera, GPContext *context)
{
        if (camera->pl) {
                gp_camera_stop_timeout (camera, camera->pl->timeout);
		knc_cntrl_unref (camera->pl->c);
                free (camera->pl);
                camera->pl = NULL;
        }

        return (GP_OK);
}

static int
set_info_func (CameraFilesystem *fs, const char *folder, const char *file,
               CameraFileInfo info, void *data, GPContext *context)
{
        Camera *camera = data;
        char tmp[7];
        int p;
        unsigned long image_id;
	KncCamRes cr;

        /* Permissions? */
        if (info.file.fields & GP_FILE_INFO_PERMISSIONS) {
                strncpy (tmp, file, 6);
                tmp[6] = '\0';
                image_id = atol (tmp);
                if (info.file.permissions & GP_FILE_PERM_DELETE) p = FALSE;
                else p = TRUE;
                CR (knc_set_prot (camera->pl->c, &cr, image_id,
				  KNC_SOURCE_CARD, p), context);
		CCR (cr, context);
        }

        /* Name? */
        if (info.file.fields & GP_FILE_INFO_NAME) {
                gp_context_error (context, _("Your camera does not support "
                        "changing filenames."));
                return (GP_ERROR_NOT_SUPPORTED);
        }

        return (GP_OK);
}

static int
file_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
                void *data, GPContext *context)
{
        CameraFile *file;
        CameraFileInfo info;
        KncStatus status;
        unsigned int i, id;
        Camera *camera = data;
        int result;
	KncCamRes cr;

        /*
         * We can't get the filename from the camera.
         * But we decide to call the images %6i.jpeg', with the image id as
         * parameter. Therefore, let's get the image ids.
         */
        CR (knc_get_status (camera->pl->c, &cr, &status), context);
	CCR (cr, context);

        id = gp_context_progress_start (context, status.pictures,
                                        _("Getting file list..."));
        for (i = 0; i < status.pictures; i++) {

                /* Get information */
                gp_file_new (&file);
                result = get_info (camera, i + 1, &info, file, context);
                if (result < 0) {
                        gp_file_unref (file);
                        return (result);
                }

                /*
                 * Append directly to the filesystem instead of to the list,
                 * because we have additional information.
                 */
                gp_filesystem_append (camera->fs, folder, info.file.name,
                                      context);
                gp_filesystem_set_info_noop (camera->fs, folder, info, context);
                gp_filesystem_set_file_noop (camera->fs, folder, file, context);
                gp_file_unref (file);

                gp_context_idle (context);
                gp_context_progress_update (context, id, i + 1);
                if (gp_context_cancel (context) == GP_CONTEXT_FEEDBACK_CANCEL)
                        return (GP_ERROR_CANCEL);
        }
        gp_context_progress_stop (context, id);

        return (GP_OK);
}

static int
camera_summary (Camera* camera, CameraText* summary, GPContext *context)
{
        KncInfo info;
	KncCamRes cr;

        CR (knc_get_info (camera->pl->c, &cr, &info), context);
	CCR (cr, context);

        snprintf (summary->text, sizeof (summary->text),
                _("Model: %s\n"
                "Serial Number: %s,\n"
                "Hardware Version: %i.%i\n"
                "Software Version: %i.%i\n"
                "Testing Software Version: %i.%i\n"
                "Name: %s,\n"
                "Manufacturer: %s\n"),
                info.model, info.serial_number,
                info.hardware.major, info.hardware.minor,
                info.software.major, info.software.minor,
                info.testing.major, info.testing.minor,
                info.name, info.manufacturer);

        return (GP_OK);
}

static int
camera_about (Camera* camera, CameraText* about, GPContext *context)
{
        C_NULL (camera && about);

        /* Translators: please write 'M"uller' (that is, with u-umlaut)
           if your charset allows it.  If not, use "Mueller". */
        strcpy (about->text, _("Konica library\n"
                "Lutz Mueller <lutz@users.sourceforge.net>\n"
                "Support for all Konica and several HP cameras."));

        return (GP_OK);
}

int
camera_init (Camera *camera, GPContext *context)
{
	int i;
	GPPortSettings s;
	unsigned int speeds[] = {115200, 9600, 57600, 38400, 19200,
				 4800, 2400, 1200, 600, 300};

	/* We only support serial cameras. */
	if (camera->port->type != GP_PORT_SERIAL)
		return GP_ERROR_NOT_SUPPORTED;

	/* First, set up all the function pointers. */
        camera->functions->pre_func             = camera_pre_func;
        camera->functions->post_func            = camera_post_func;
        camera->functions->exit                 = camera_exit;
#if 0
        camera->functions->get_config           = camera_get_config;
        camera->functions->set_config           = camera_set_config;
#endif
        camera->functions->capture              = camera_capture;
        camera->functions->capture_preview      = camera_capture_preview;
        camera->functions->summary              = camera_summary;
        camera->functions->about                = camera_about;

        /* Store some data we constantly need. */
        camera->pl = malloc (sizeof (CameraPrivateLibrary));
        memset (camera->pl, 0, sizeof (CameraPrivateLibrary));

        /* Initiate the connection */
        C (gp_port_get_settings (camera->port, &s));
	s.serial.bits = 8;
	s.serial.parity = 0;
	s.serial.stopbits = 1;
	C (gp_port_set_settings (camera->port, s));
	for (i = 0; i < 10; i++) {
		s.serial.speed = speeds[i];
		C (gp_port_set_settings (camera->port, s));
		if ((camera->pl->c = gpknc_cntrl_new_from_port (camera->port)))
			break;
	}
	if (i == 10) {
		gp_context_error (context, _("The camera could not be "
			"contacted. Please make sure it is conntected to the "
			"computer and turned on."));
		return (GP_ERROR_IO);
	}
#if 0
/* Ideally, we need to reset the speed to the speed that we encountered
   after each operation (multiple programs accessing the camera). However,
   that takes quite a bit of time for HP cameras... */
                camera->pl->speed = speeds[i];
#endif

        /* Set up the filesystem */
        C(gp_filesystem_set_info_funcs (camera->fs, get_info_func,
                                        set_info_func, camera));
        C(gp_filesystem_set_list_funcs (camera->fs, file_list_func,
                                        NULL, camera));
        C(gp_filesystem_set_file_funcs (camera->fs, get_file_func,
                                        delete_file_func, camera));
        C(gp_filesystem_set_folder_funcs (camera->fs, NULL, delete_all_func,
                                        NULL, NULL, camera));

        /* Ping the camera every minute to prevent shut-down. */
        camera->pl->timeout = gp_camera_start_timeout (camera, PING_TIMEOUT,
                                                       timeout_func);

        return (GP_OK);
}

