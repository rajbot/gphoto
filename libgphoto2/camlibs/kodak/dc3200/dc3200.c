/****************************************************
 * kodak dc3200 digital camera driver library       *
 * for gphoto2                                      *
 *                                                  *
 * author: donn morrison - dmorriso@gulf.uvic.ca    *
 * date: dec 2000 - feb 2001                        *
 * license: gpl                                     *
 * version: 1.5                                     *
 *                                                  *
 ****************************************************/

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

#include "dc3200.h"
#include "library.h"

int camera_id (CameraText *id) 
{
	strcpy(id->text, "kodak-dc3200");

	return (GP_OK);
}

int camera_abilities (CameraAbilitiesList *list) 
{
	CameraAbilities a;

	memset(&a, 0, sizeof(a));
	strcpy(a.model, "Kodak DC3200");
	a.port     = GP_PORT_SERIAL;
	a.speed[0] = 9600;
	a.speed[1] = 19200;
	a.speed[2] = 38400;
	a.speed[3] = 57600;
	a.speed[4] = 115200;
	a.speed[5] = 0;
	a.operations        = GP_OPERATION_NONE;
	a.file_operations   = GP_FILE_OPERATION_PREVIEW;
	a.folder_operations = GP_FOLDER_OPERATION_NONE;

	gp_abilities_list_append(list, a);

	return (GP_OK);
}

int init(Camera *camera)
{
	GPPortSettings settings;
	int ret, selected_speed;

	ret = gp_port_settings_get (camera->port, &settings);
	if (ret < 0)
		return (ret);

	/* Remember the selected speed 0 == fastest */
	selected_speed = settings.serial.speed == 0 ? 115200 : settings.serial.speed;

	settings.serial.speed    = 9600;
	settings.serial.bits     = 8;
	settings.serial.parity   = 0;
	settings.serial.stopbits = 1;

	ret = gp_port_settings_set (camera->port, settings);
	if (ret < 0)
		return (ret);

	gp_port_timeout_set (camera->port, TIMEOUT);

	if (dc3200_set_speed (camera, selected_speed) == GP_ERROR)
		return GP_ERROR;

	/* Set the new speed */
	settings.serial.speed = selected_speed;
	ret = gp_port_settings_set (camera->port, settings);
	if (ret < 0)
		return (ret);

	/* Wait for it to update */
	sleep(1);

	/* Try to talk after speed change */
	if (dc3200_keep_alive(camera) == GP_ERROR)
		return GP_ERROR;

	/* setup the camera */
	if (dc3200_setup(camera) == GP_ERROR)
		return GP_ERROR;		

	return GP_OK;
}

static int camera_exit (Camera *camera)
{
	if (camera->pl) {
		free (camera->pl);
		camera->pl = NULL;
	}

	return (GP_OK);
}

int check_last_use(Camera *camera)
{
	time_t t;

	time(&t);
	
	if(t - camera->pl->last > 9) {
		/* we have to re-init the camera */
		printf(_("camera inactive for > 9 seconds, re-initing.\n"));
		return init(camera);
	}

	return GP_OK;
}

static int folder_list_func (CameraFilesystem *fs, const char *folder,
			     CameraList *list, void *user_data)
{
	Camera 		*camera = user_data;
	u_char		*data = NULL;
	long		data_len = 0;
	u_char		*ptr_data_buff;
	char		filename[13], *ptr;
	int		res, i;

	if(check_last_use(camera) == GP_ERROR)
		return GP_ERROR;

	/* get file list data */
	res = dc3200_get_data (camera, &data, &data_len, CMD_LIST_FILES, folder,
			       NULL);
	if (res == GP_ERROR)
		return GP_ERROR;

	/* check the data length, each record is 20 bytes */
	if(data_len%20 != 0 || data_len < 1) {
		/* there is a problem */
		return GP_ERROR;
	}
	
	if (data == NULL)
		return GP_ERROR;

	/* add directories to the list */
	ptr_data_buff = data;
	i = 0;

	while(i < data_len) {
		//dump_buffer(ptr_data_buff, 20, "list", 20);
	
		/* directories have 0x10 in their attribute */
		if(!(ptr_data_buff[11] & 0x10)) {
			ptr_data_buff += 20;
			i += 20;
			continue;
		}
		
		/* skip directories starting with . */
		if(ptr_data_buff[0] == '.') {
			ptr_data_buff += 20;
			i += 20;
			continue;
		}
		
		/* copy the filename */
		strncpy(filename, ptr_data_buff, sizeof(filename));

		/* chop off the trailing spaces and null terminate */
		ptr = strchr(filename, 0x20);
		if(ptr) ptr[0] = 0;

		/* in case of long directory */
		filename[12] = 0;
		
		/* append dir to the list */
		gp_list_append(list, filename, NULL);
		
		ptr_data_buff += 20;
		i += 20;
	}

	free (data);
	return (dc3200_keep_alive (camera));
}

static int file_list_func (CameraFilesystem *fs, const char *folder,
			   CameraList *list, void *user_data)
{
	Camera		*camera = user_data;
	u_char		*data = NULL;
	long		data_len = 0;
	u_char		*ptr_data_buff;
	char		filename[13];
	int		res, i;

	if(check_last_use(camera) == GP_ERROR)
		return GP_ERROR;

	/* get file list data */
	res = dc3200_get_data (camera, &data, &data_len, CMD_LIST_FILES, folder,
			       NULL);
	if (res == GP_ERROR)
		return GP_ERROR;

	/* check the data length */
	if(data_len%20 != 0 || data_len < 1) {
		/* there is a problem */
		return GP_ERROR;
	}

	if(data == NULL)
		return GP_ERROR;

	/* add files to the list */
	ptr_data_buff = data;
	i = 0;
	
	while(i < data_len) {
		/* files don't have 0x10 in their attribute */
		if(ptr_data_buff[11] & 0x10) {
			ptr_data_buff += 20;
			i += 20;
			continue;
		}
		
		/* copy the first 8 bytes of filename */
		strncpy(filename, ptr_data_buff, 8);
		filename[8] = 0;
		/* add dot */
		strcat(filename, ".");
		/* copy extension, last 3 bytes*/
		strncat(filename, ptr_data_buff+8, 3);
		
		if(!strstr(filename, ".JPG") && !strstr(filename, ".jpg")) {
			ptr_data_buff += 20;
			i += 20;
			continue;
		}

		/* append file to the list */
		gp_list_append(list, filename, NULL);
		
		ptr_data_buff += 20;
		i += 20;
	}

	free(data);
	return (dc3200_keep_alive(camera));
}

static int get_file_func (CameraFilesystem *fs, const char *folder,
			  const char *filename, CameraFileType type,
			  CameraFile *file, void *user_data)
{
	Camera		*camera = user_data;
	u_char		*data = NULL;
	long		data_len = 0;
	int		res;

	if(check_last_use(camera) == GP_ERROR)
		return GP_ERROR;

	switch (type) {
	case GP_FILE_TYPE_PREVIEW:
		res = dc3200_get_data (camera, &data, &data_len,
				       CMD_GET_PREVIEW, folder, filename);
		break;
	case GP_FILE_TYPE_NORMAL:
		res = dc3200_get_data (camera, &data, &data_len, CMD_GET_FILE,
				       folder, filename);
		break;
	default:
		return (GP_ERROR_NOT_SUPPORTED);
	}
	if (res < 0)
		return (res);

	if (data == NULL || data_len < 1)
		return (GP_ERROR);

	gp_file_append (file, data, data_len);

	free(data);
	return (dc3200_keep_alive(camera));
}

static int camera_manual (Camera *camera, CameraText *manual)
{
	strcpy (manual->text, 
		_("Known problems:\n"
		"\n"
		"1. If the Kodak DC3200 does not receive a command at least "
		"every 10 seconds, it will time out, and will have to be "
		"re-initialized. If you notice the camera does not respond, "
		"simply re-select the camera. This will cause it to "
		"reinitialize.\n"
		"\n"
		"2. If you cancel a picture transfer, the driver will be left "
		"in an unknown state, and will most likely need to be "
		"re-initialized."));
	return (GP_OK);
}

static int camera_about (Camera *camera, CameraText *about)
{
	strcpy	(about->text, 
		_("Kodak DC3200 Driver\n"
		"Donn Morrison <dmorriso@gulf.uvic.ca>\n"
		"\n"
		"Questions and comments appreciated."));
	return (GP_OK);
}

int camera_init (Camera *camera) 
{
	int ret;

	camera->pl = malloc (sizeof (CameraPrivateLibrary));
	if (!camera->pl)
		return (GP_ERROR_NO_MEMORY);

        /* First, set up all the function pointers */
        camera->functions->exit                 = camera_exit;
        camera->functions->manual               = camera_manual;
        camera->functions->about                = camera_about;

	/* Set up the CameraFilesystem */
	gp_filesystem_set_list_funcs (camera->fs, file_list_func,
				      folder_list_func, camera);
	gp_filesystem_set_file_funcs (camera->fs, get_file_func, NULL, camera);
        
        /* initialize the camera */
	ret = init (camera);
	if (ret < 0) {
		free (camera->pl);
		camera->pl = NULL;
                return (ret);
        }

        ret = dc3200_keep_alive (camera);
	if (ret < 0) {
		free (camera->pl);
		camera->pl = NULL;
		return (ret);
	}

	return (GP_OK);
}

