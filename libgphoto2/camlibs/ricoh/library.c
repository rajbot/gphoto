/* library.c
 *
 * Copyright (C) 2002 Lutz M�ller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gphoto2-library.h>
#include <gphoto2-port-log.h>

#include "ricoh.h"

#define GP_MODULE "ricoh"

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif 
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define CR(result) {int r=(result); if (r<0) return r;}

static struct {
	RicohModel id;
	const char *model;
} models[] = {
	{RICOH_MODEL_1,     "Ricoh RDC-1"},
	{RICOH_MODEL_2,     "Ricoh RDC-2"},
	{RICOH_MODEL_2E,    "Ricoh RDC-2E"},
	{RICOH_MODEL_100G,  "Ricoh RDC-100G"},
	{RICOH_MODEL_300,   "Ricoh RDC-300"},
	{RICOH_MODEL_300Z,  "Ricoh RDC-300Z"},
	{RICOH_MODEL_4200,  "Ricoh RDC-4200"},
	{RICOH_MODEL_4300,  "Ricoh RDC-4300"},
	{RICOH_MODEL_5000,  "Ricoh RDC-5000"},
	{RICOH_MODEL_ESP2,  "Philips ESP2"},
	{RICOH_MODEL_ESP50, "Philips ESP50"},
	{RICOH_MODEL_ESP60, "Philips ESP60"},
	{RICOH_MODEL_ESP70, "Philips ESP70"},
	{RICOH_MODEL_ESP80, "Philips ESP80"},
	{RICOH_MODEL_ESP80, "Philips ESP80SXG"},
	{0, NULL}
};

struct _CameraPrivateLibrary {
	RicohModel model;
};

int
camera_abilities (CameraAbilitiesList *list)
{
	int i;
	CameraAbilities a;

	memset (&a, 0, sizeof (CameraAbilities));
	for (i = 0; models[i].model; i++) {
		strcpy (a.model, models[i].model);
		a.status = GP_DRIVER_STATUS_EXPERIMENTAL;
		a.port = GP_PORT_SERIAL;
		a.operations = GP_OPERATION_CAPTURE_IMAGE |
			       GP_OPERATION_CONFIG;
		a.file_operations = GP_FILE_OPERATION_DELETE |
				    GP_FILE_OPERATION_PREVIEW;
		a.folder_operations = GP_FOLDER_OPERATION_NONE;
		CR (gp_abilities_list_append (list, a));
	}

	return (GP_OK);
}

static int
camera_exit (Camera *camera, GPContext *context)
{
	if (camera->pl) {
		free (camera->pl);
		camera->pl = NULL;
	}

	ricoh_disconnect (camera, context);

	return GP_OK;
}

static int
file_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
		void *data, GPContext *context)
{
	Camera *camera = data;
	unsigned int n, i;
	const char *name;

	CR (ricoh_get_num (camera, context, &n));
	for (i = 0; i < n; i++) {
		CR (ricoh_get_pic_name (camera, context, i + 1, &name));
		CR (gp_list_append (list, name, NULL));
	}

	return (GP_OK);
}

static int
get_file_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileType type, CameraFile *file, void *user_data,
	       GPContext *context)
{
	Camera *camera = user_data;
	int n;
	unsigned int size;
	unsigned char *data;

	CR (n = gp_filesystem_number (fs, folder, filename, context));
	n++;

	switch (type) {
	case GP_FILE_TYPE_NORMAL:
		CR (ricoh_get_pic (camera, context, n,
				   RICOH_FILE_TYPE_NORMAL, &data, &size));
		gp_file_set_mime_type (file, GP_MIME_JPEG);
		break;
	case GP_FILE_TYPE_PREVIEW:
		CR (ricoh_get_pic (camera, context, n,
				   RICOH_FILE_TYPE_PREVIEW, &data, &size));
		gp_file_set_mime_type (file, GP_MIME_TIFF);
		break;		
	default:
		return (GP_ERROR_NOT_SUPPORTED);
	}

	gp_file_set_data_and_size (file, data, size);

	return (GP_OK);
}

static int
del_file_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       void *user_data, GPContext *context)
{
	Camera *camera = user_data;
	int n;

	CR (n = gp_filesystem_number (fs, folder, filename, context));
	n++;

	CR (ricoh_del_pic (camera, context, n));

	return (GP_OK);
}

static int
camera_about (Camera *camera, CameraText *about, GPContext *context)
{
	GP_DEBUG ("camera_about()");

	strcpy (about->text,
		_("Ricoh / Philips driver by \n"
		  "Lutz M�ller <lutz@users.sourceforge.net>, \n"
		  "Martin Fischer <martin.fischer@inka.de>, \n"
		  "based on Bob Paauwe's driver\n" )
		);

	return GP_OK;
}

int
camera_id (CameraText *id)
{
	strcpy (id->text, "Ricoh");

	return (GP_OK);
}

static int
camera_summary (Camera *camera, CameraText *about, GPContext *context)
{
	int avail_mem, total_mem;
	char model[128];
	unsigned int i;

	CR (ricoh_get_cam_amem (camera, context, &avail_mem));
	CR (ricoh_get_cam_mem  (camera, context, &total_mem));

	memset (model, 0, sizeof (model));
	for (i = 0; models[i].model; i++)
		if (models[i].id == camera->pl->model)
			break;
	if (models[i].model)
		strncpy (model, models[i].model, sizeof (model) - 1);
	else
		snprintf (model, sizeof (model) - 1, _("unknown (0x%02x)"),
			  camera->pl->model);

	sprintf (about->text, _("Model: %s\n"
			        "Memory: %d byte(s) of %d available"),
		model, avail_mem, total_mem);

	return (GP_OK);
}

static int
camera_capture (Camera *camera, CameraCaptureType type,
		CameraFilePath *path, GPContext *context)
{
	unsigned int n;

	if (type != GP_CAPTURE_IMAGE)
		return (GP_ERROR_NOT_SUPPORTED);

	CR (ricoh_get_num (camera, context, &n));
	CR (ricoh_take_pic (camera, context));

	sprintf (path->name, "rdc%04i.jpg", n + 1);
	strcpy (path->folder, "/");
	CR (gp_filesystem_append (camera->fs, path->folder,
				  path->name, context));

	return (GP_OK);
}

static int
camera_get_config (Camera *camera, CameraWidget **window, GPContext *context)
{
	CameraWidget *section, *widget;
	const char *copyright;
	time_t time;
	RicohResolution resolution;
	RicohExposure exposure;

	CR (gp_widget_new (GP_WIDGET_WINDOW, _("Configuration"), window));

	/* General settings */
	CR (gp_widget_new (GP_WIDGET_SECTION, _("General"), &section));
	CR (gp_widget_append (*window, section));

	/* Copyright */
	CR (gp_widget_new (GP_WIDGET_TEXT, _("Copyright"), &widget));
	CR (gp_widget_set_name (widget, "copyright"));
	CR (gp_widget_set_info (widget, _("Copyright (max. 20 characters")));
	CR (gp_widget_append (section, widget));
	CR (ricoh_get_copyright (camera, context, &copyright));
	CR (gp_widget_set_value (widget, (void *) copyright));

	/* Date */
	CR (gp_widget_new (GP_WIDGET_DATE, _("Date & Time"), &widget));
	CR (gp_widget_set_name (widget, "date"));
	CR (gp_widget_set_info (widget, _("Date & Time")));
	CR (gp_widget_append (section, widget));
	CR (ricoh_get_date (camera, context, &time));
	CR (gp_widget_set_value (widget, &time));

	/* Picture related settings */
	CR (gp_widget_new (GP_WIDGET_SECTION, _("Pictures"), &section));
	CR (gp_widget_append (*window, section));

	/* Resolution */
	CR (gp_widget_new (GP_WIDGET_RADIO, _("Resolution"), &widget));
	CR (gp_widget_set_name (widget, "resolution"));
	CR (gp_widget_set_info (widget, _("Resolution")));
	CR (gp_widget_append (section, widget));
	CR (gp_widget_add_choice (widget, "640x480"));
	CR (gp_widget_add_choice (widget, "1280x960"));
	CR (ricoh_get_resolution (camera, context, &resolution));
	switch (resolution) {
	case RICOH_RESOLUTION_640_480:
		CR (gp_widget_set_value (widget, "640x480"));
		break;
	case RICOH_RESOLUTION_1280_960:
		CR (gp_widget_set_value (widget, "1280x960"));
		break;
	default:
		CR (gp_widget_set_value (widget, "unknown"));
		break;
	}

	/* Exposure */
	CR (gp_widget_new (GP_WIDGET_RADIO, _("Exposure"), &widget));
	CR (gp_widget_set_name (widget, "exposure"));
	CR (gp_widget_set_info (widget, _("Exposure")));
	CR (gp_widget_append (section, widget));
	CR (gp_widget_add_choice (widget, "-2.0"));
	CR (gp_widget_add_choice (widget, "-1.5"));
	CR (gp_widget_add_choice (widget, "0.0"));
	CR (gp_widget_add_choice (widget, "+1.5"));
	CR (gp_widget_add_choice (widget, "+2.0"));
	CR (gp_widget_add_choice (widget, _("Auto")));
	CR (ricoh_get_exposure (camera, context, &exposure));
	switch (exposure) {
	case RICOH_EXPOSURE_M20:
		CR (gp_widget_set_value (widget, "-2.0"));
		break;
	case RICOH_EXPOSURE_M15:
		CR (gp_widget_set_value (widget, "-1.5"));
		break;
	case RICOH_EXPOSURE_M10:
		CR (gp_widget_set_value (widget, "-1.0"));
		break;
	case RICOH_EXPOSURE_M05:
		CR (gp_widget_set_value (widget, "-0.5"));
		break;
	case RICOH_EXPOSURE_00:
		CR (gp_widget_set_value (widget, "0.0"));
		break;
	case RICOH_EXPOSURE_05:
		CR (gp_widget_set_value (widget, "+0.5"));
		break;
	case RICOH_EXPOSURE_10:
		CR (gp_widget_set_value (widget, "+1.0"));
		break;
	case RICOH_EXPOSURE_15:
		CR (gp_widget_set_value (widget, "+1.5"));
		break;
	case RICOH_EXPOSURE_20:
		CR (gp_widget_set_value (widget, "+2.0"));
		break;
	case RICOH_EXPOSURE_AUTO:
		CR (gp_widget_set_value (widget, _("Auto")));
		break;
	default:
		CR (gp_widget_set_value (widget, _("unknown")));
		break;
	}

	return (GP_OK);
}

static int
camera_set_config (Camera *camera, CameraWidget *window, GPContext *context)
{
	CameraWidget *widget;
	const char *v_char;
	time_t time;
	RicohResolution resolution;
	RicohExposure exposure;

	/* Copyright */
	CR (gp_widget_get_child_by_name (window, "copyright", &widget));
	if (gp_widget_changed (widget)) {
		CR (gp_widget_get_value (widget, &v_char));
		CR (ricoh_set_copyright (camera, context, v_char));
	}

	/* Date */
	CR (gp_widget_get_child_by_name (window, "date", &widget));
	if (gp_widget_changed (widget)) {
		CR (gp_widget_get_value (widget, &time));
		CR (ricoh_set_date (camera, context, time));
	}

	/* Resolution */
	CR (gp_widget_get_child_by_name (window, "resolution", &widget));
	if (gp_widget_changed (widget)) {
		CR (gp_widget_get_value (widget, &v_char));
		if (!strcmp (v_char, "640x480"))
			resolution = RICOH_RESOLUTION_640_480;
		else if (!strcmp (v_char, "1280x960"))
			resolution = RICOH_RESOLUTION_1280_960;
		else 
			resolution = 0;
		CR (ricoh_set_resolution (camera, context, resolution));
	}

	/* Exposure */
	CR (gp_widget_get_child_by_name (window, "exposure", &widget));
	if (gp_widget_changed (widget)) {
		CR (gp_widget_get_value (widget, &v_char));
		if (!strcmp (v_char, "-2.0"))
			exposure = RICOH_EXPOSURE_M20;
		else if (!strcmp (v_char, "-1.5"))
			exposure = RICOH_EXPOSURE_M15;
		else if (!strcmp (v_char, "-1.0"))
			exposure = RICOH_EXPOSURE_M10;
		else if (!strcmp (v_char, "-0.5"))
			exposure = RICOH_EXPOSURE_M05;
		else if (!strcmp (v_char, "0.0"))
			exposure = RICOH_EXPOSURE_00;
		else if (!strcmp (v_char, "+0.5"))
			exposure = RICOH_EXPOSURE_05;
		else if (!strcmp (v_char, "+1.0"))
			exposure = RICOH_EXPOSURE_10;
		else if (!strcmp (v_char, "+1.5"))
			exposure = RICOH_EXPOSURE_15;
		else if (!strcmp (v_char, "+2.0"))
			exposure = RICOH_EXPOSURE_20;
		else if (!strcmp (v_char, _("Auto")))
			exposure = RICOH_EXPOSURE_AUTO;
		else
			exposure = 0;
		CR (ricoh_set_exposure (camera, context, exposure));
	}

	return (GP_OK);
}

static struct {
	unsigned int speed;
	RicohSpeed rspeed;
} speeds[] = {
	{  2400, RICOH_SPEED_2400},
	{115200, RICOH_SPEED_115200},
	{  4800, RICOH_SPEED_4800},
	{ 19200, RICOH_SPEED_19200},
	{ 38400, RICOH_SPEED_38400},
	{ 57600, RICOH_SPEED_57600},
	{     0, 0}
};

int
camera_init (Camera *camera, GPContext *context)
{
	GPPortSettings settings;
	unsigned int speed, i;
	int result;
	RicohModel model = 0;

	/* Try to contact the camera. */
	CR (gp_port_set_timeout (camera->port, 5000));
	CR (gp_port_get_settings (camera->port, &settings));
	speed = (settings.serial.speed ? settings.serial.speed : 115200);
	for (i = 0; speeds[i].speed; i++) {
		GP_DEBUG ("Trying speed %i...", speeds[i].speed);
		settings.serial.speed = speeds[i].speed;
		CR (gp_port_set_settings (camera->port, settings));

		/*
		 * Note that ricoh_connect can only be called to 
		 * initialize the connection at 2400 bps. At other
		 * speeds, a different function needs to be used.
		 */
		result = (speeds[i].rspeed == RICOH_SPEED_2400) ? 
				ricoh_connect (camera, NULL, &model) :
				ricoh_get_mode (camera, NULL, NULL);
		if (result == GP_OK)
			break;
	}

	/* Contact made? If not, report error. */
	if (!speeds[i].speed) {
		gp_context_error (context, _("Could not contact camera."));
		return (GP_ERROR);
	}

	/* Contact made. Do we need to change the speed? */
	if (settings.serial.speed != speed) {
		for (i = 0; speeds[i].speed; i++)
			if (speeds[i].speed == speed)
				break;
		if (!speeds[i].speed) {
			gp_context_error (context, _("Speed %i is not "
				"supported!"), speed);
			return (GP_ERROR);
		}
		CR (ricoh_set_speed (camera, context, speeds[i].rspeed));
		settings.serial.speed = speed;
		CR (gp_port_set_settings (camera->port, settings));

		/* Check if the camera is still there. */
		CR (ricoh_get_mode (camera, context, NULL));
	}

	/* setup the function calls */
	camera->functions->exit = camera_exit;
	camera->functions->summary = camera_summary;
	camera->functions->capture = camera_capture;
	camera->functions->about = camera_about;
	camera->functions->get_config = camera_get_config;
	camera->functions->set_config = camera_set_config;
	
	CR (gp_filesystem_set_list_funcs (camera->fs, file_list_func, NULL,
					  camera));
	CR (gp_filesystem_set_file_funcs (camera->fs, get_file_func,
					  del_file_func, camera));

	/*
	 * Remember the model. It could be that there hasn't been the 
	 * need to call ricoh_connect. Then we don't have a model. Should
	 * we disconnect and reconnect in this case?
	 */
	camera->pl = malloc (sizeof (CameraPrivateLibrary));
	if (!camera->pl)
		return (GP_ERROR_NO_MEMORY);
	memset (camera->pl, 0, sizeof (CameraPrivateLibrary));
	camera->pl->model = model;

	return (GP_OK);
}

