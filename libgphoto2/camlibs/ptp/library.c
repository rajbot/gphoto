/* library.c
 *
 * Copyright (C) 2001 Mariusz Woloszyn <emsi@ipartners.pl>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <gphoto2-library.h>
#include <gphoto2-debug.h>
#include <gphoto2-port-log.h>

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

#include "ptp.h"

#define GP_MODULE "PTP"

#define CR(result) {int r=(result);if(r<0) return (r);}

static struct {
	short n;
	const char *txt;
} ptp_errors[] = {
	{PTP_RC_Undefined, 		N_("PTP Udefined Error")},
	{PTP_RC_OK, 			N_("PTP OK!")},
	{PTP_RC_GeneralError, 		N_("PTP General Error")},
	{PTP_RC_SessionNotOpen, 	N_("PTP Session Not Open")},
	{PTP_RC_InvalidTransactionID, 	N_("PTP Invalid Transaction ID")},
	{PTP_RC_OperationNotSupported, 	N_("PTP Operation Not Supported")},
	{PTP_RC_ParameterNotSupported, 	N_("PTP Parameter Not Supported")},
	{PTP_RC_IncompleteTransfer, 	N_("PTP Incomplete Transfer")},
	{PTP_RC_InvalidStorageId, 	N_("PTP Invalid Storage ID")},
	{PTP_RC_InvalidObjectHandle, 	N_("PTP Invalid Object Handle")},
	{PTP_RC_DevicePropNotSupported, N_("PTP Device Prop Not Supported")},
	{PTP_RC_InvalidObjectFormatCode, N_("PTP Invalid Object Format Code")},
	{PTP_RC_StoreFull, 		N_("PTP Store Full")},
	{PTP_RC_ObjectWriteProtected, 	N_("PTP Object Write Protected")},
	{PTP_RC_StoreReadOnly, 		N_("PTP Store Read Only")},
	{PTP_RC_AccessDenied,		N_("PTP Access Denied")},
	{PTP_RC_NoThumbnailPresent, 	N_("PTP No Thumbnail Present")},
	{PTP_RC_SelfTestFailed, 	N_("PTP Self Test Failed")},
	{PTP_RC_PartialDeletion, 	N_("PTP Partial Deletion")},
	{PTP_RC_StoreNotAvailable, 	N_("PTP Store Not Available")},
	{PTP_RC_SpecyficationByFormatUnsupported,
				N_("PTP Specyfication By Format Unsupported")},
	{PTP_RC_NoValidObjectInfo, 	N_("PTP No Valid Object Info")},
	{PTP_RC_InvalidCodeFormat, 	N_("PTP Invalid Code Format")},
	{PTP_RC_UnknownVendorCode, 	N_("PTP Unknown Vendor Code")},
	{PTP_RC_CaptureAlreadyTerminated,
					N_("PTP Capture Already Terminated")},
	{PTP_RC_DeviceBusy, 		N_("PTP Device Bus")},
	{PTP_RC_InvalidParentObject, 	N_("PTP Invalid Parent Object")},
	{PTP_RC_InvalidDevicePropFormat, N_("PTP Invalid Device Prop Format")},
	{PTP_RC_InvalidDevicePropValue, N_("PTP Invalid Device Prop Value")},
	{PTP_RC_InvalidParameter, 	N_("PTP Invalid Parameter")},
	{PTP_RC_SessionAlreadyOpened, 	N_("PTP Session Already Opened")},
	{PTP_RC_TransactionCanceled, 	N_("PTP TransactionCanceled")},
	{PTP_RC_SpecificationOfDestinationUnsupported,
			N_("PTP Specification Of Destination Unsupported")},

	{PTP_ERROR_IO,		  N_("PTP I/O error")},
	{PTP_ERROR_BADPARAM,	  N_("PTP Error: bad parameter")},
	{PTP_ERROR_DATA_EXPECTED, N_("PTP Protocol error, data expected")},
	{PTP_ERROR_RESP_EXPECTED, N_("PTP Protocol error, response expected")},
	{0, NULL}
};

static void
report_result (Camera *camera, short result)
{
	unsigned int i;

	for (i = 0; ptp_errors[i].txt; i++)
		if (ptp_errors[i].n == result)
			gp_camera_set_error (camera, ptp_errors[i].txt);
}

static int
translate_ptp_result (short result)
{
	switch (result) {
	case PTP_RC_ParameterNotSupported:
		return (GP_ERROR_BAD_PARAMETERS);
	case PTP_RC_OK:
		return (GP_OK);
	default:
		return (GP_ERROR);
	}
}

static short
translate_gp_result (int result)
{
	switch (result) {
	case GP_OK:
		return (PTP_RC_OK);
	case GP_ERROR:
		return (PTP_RC_GeneralError);
	default:
		return (PTP_RC_GeneralError);
	}
}

#define CPR(camera,result) {short r=(result); if (r!=PTP_RC_OK) {report_result ((camera), r); return (translate_ptp_result (r));}}
#define CHECK(result) {int ret=(result);if(ret<0){return(ret);}}


static struct {
	const char *model;
	unsigned short usb_vendor;
	unsigned short usb_product;
} models[] = {
	{"Kodak DC-240 (PTP)",  0x40a, 0x121}, /* Special firmware */
	{"Kodak DC-4800", 0x40a, 0x160},
	{"Kodak DX-3500", 0x40a, 0x500},
	{"Kodak DX-3600", 0, 0},
	{"Kodak DX-3900", 0x40a, 0x170},
	{"Kodak MC3", 0, 0},
	{"Sony DSC-P5", 0, 0},
	{"Sony DSC-F707", 0, 0},
	{"HP PhotoSmart 318", 0x3f0, 0x6302},
	{NULL, 0, 0}
};

static struct {
	unsigned short format_code;
	const char *txt;
} object_formats[] = {
	{PTP_OFC_Undefined,	"application/x-unknown"},
	{PTP_OFC_Association,	"application/x-association"},
	{PTP_OFC_Script,	"application/x-script"},
	{PTP_OFC_Executable,	"application/octet-stream"},
	{PTP_OFC_Text,		"text/plain"},
	{PTP_OFC_HTML,		"text/html"},
	{PTP_OFC_DPOF,		"text/plain"},
	{PTP_OFC_AIFF,		"audio/x-aiff"},
	{PTP_OFC_WAV,		"audio/x-wav"},
	{PTP_OFC_MP3,		"audio/basic"},
	{PTP_OFC_AVI,		"video/x-msvideo"},
	{PTP_OFC_MPEG,		"video/mpeg"},
	{PTP_OFC_ASF,		"vide/x-asf"},
	{PTP_OFC_EXIF_JPEG,	"image/jpeg"},
	{PTP_OFC_TIFF_EP,	"image/x-tiffep"},
	{PTP_OFC_FlashPix,	"image/x-flashpix"},
	{PTP_OFC_BMP,		"image/x-bitmap"},
	{PTP_OFC_CIFF,		"image/x-ciff"},
	{PTP_OFC_Undefined_0x3806, "application/x-unknown"},
	{PTP_OFC_GIF,		"image/gif"},
	{PTP_OFC_JFIF,		"image/jpeg"},
	{PTP_OFC_PCD,		"image/x-pcd"},
	{PTP_OFC_PICT,		"image/x-pict"},
	{PTP_OFC_PNG,		"image/png"},
	{PTP_OFC_Undefined_0x380C, "application/x-unknown"},
	{PTP_OFC_TIFF,		"image/tiff"},
	{PTP_OFC_TIFF_IT,	"image/x-tiffit"},
	{PTP_OFC_JP2,		"image/x-jpeg2000bff"},
	{PTP_OFC_JPX,		"image/x-jpeg2000eff"},
	{0,			NULL}
};

static int
set_mimetype (Camera *camera, CameraFile *file, unsigned short ofc)
{
	int i;

	for (i = 0; object_formats[i].format_code; i++)
		if (object_formats[i].format_code == ofc)
		{
			CHECK (gp_file_set_mime_type (file, object_formats[i].txt));
			return (GP_OK);
		}

	CHECK (gp_file_set_mime_type (file, "application/x-unknown"));
	return (GP_OK);
}

static void
strcpy_mime(char * dest, unsigned short ofc) {
	int i;

	for (i = 0; object_formats[i].format_code; i++)
		if (object_formats[i].format_code == ofc)
			strcpy(dest, object_formats[i].txt);

}
	
struct _CameraPrivateLibrary {
	PTPParams params;
};

static short
ptp_read_func (unsigned char *bytes, unsigned int size, void *data)
{
	Camera *camera = data;
	int result;

	/*
	 * gp_port_read returns (in case of success) the number of bytes
	 * read. libptp doesn't need that.
	 */
	result = gp_port_read (camera->port, bytes, size);
	if (result >= 0)
		return (PTP_RC_OK);
	else
		return (translate_gp_result (result));
}

static short
ptp_write_func (unsigned char *bytes, unsigned int size, void *data)
{
	Camera *camera = data;
	int result;

	/*
	 * gp_port_write returns (in case of success) the number of bytes
	 * write. libptp doesn't need that.
	 */
	result = gp_port_write (camera->port, bytes, size);
	if (result >= 0)
		return (PTP_RC_OK);
	else
		return (translate_gp_result (result));
}

static void
ptp_debug_func (void *data, const char *format, va_list args)
{
	gp_logv (GP_LOG_DEBUG, "ptp", format, args);
}

static void
ptp_error_func (void *data, const char *format, va_list args)
{
	Camera *camera = data;
	char buf[2048];

	vsnprintf (buf, sizeof (buf), format, args);
	gp_camera_set_error (camera, "%s", buf);
}

int
camera_abilities (CameraAbilitiesList *list)
{
	int i;
	CameraAbilities a;

	for (i = 0; models[i].model; i++) {
		strcpy (a.model, models[i].model);
		a.status = GP_DRIVER_STATUS_EXPERIMENTAL;
		a.port   = GP_PORT_USB;
		a.speed[0] = 0;
		a.usb_vendor = models[i].usb_vendor;
		a.usb_product= models[i].usb_product;
		a.operations        = GP_OPERATION_NONE;
		a.file_operations   = GP_FILE_OPERATION_PREVIEW|
					GP_FILE_OPERATION_DELETE;
		a.folder_operations = GP_FOLDER_OPERATION_NONE;
		CR (gp_abilities_list_append (list, a));
	}

	return (GP_OK);
}

int
camera_id (CameraText *id)
{
	strcpy (id->text, "PTP");

	return (GP_OK);
}

static int
camera_exit (Camera *camera)
{
	if (camera->pl) {
		ptp_closesession (&camera->pl->params);
		free (camera->pl);
		camera->pl = NULL;
	}

	return (GP_OK);
}

static int
camera_about (Camera *camera, CameraText *text)
{
	strncpy (text->text,
		 _("Written by Mariusz Woloszyn <emsi@ipartners.pl>. "
		   "Enjoy!"), sizeof (text->text));
	return (GP_OK);
}

static int
file_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
		void *data)
{
	Camera *camera = data;
	PTPObjectInfo objectinfo;
	int i;
	char filename[MAXFILELEN];


	for (i = 0; i < camera->pl->params.handles.n; i++) {
		CPR (camera, ptp_getobjectinfo(&camera->pl->params,
		camera->pl->params.handles.handler[i], &objectinfo));
		ptp_getobjectfilename (&objectinfo, filename);
		CR (gp_list_append (list, filename, NULL));
	}

	return (GP_OK);
}

static int
get_file_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileType type, CameraFile *file, void *data)
{
	Camera *camera = data;
	unsigned char *fdata = NULL;
	unsigned long image_id;
	PTPObjectInfo ptp_objectinfo;


	if (strcmp (folder, "/"))
		return (GP_ERROR_DIRECTORY_NOT_FOUND);


	// Get file number
	image_id = gp_filesystem_number (fs, folder, filename);

	switch (type) {
	case GP_FILE_TYPE_NORMAL:
		CPR (camera, ptp_getobjectinfo(&camera->pl->params,
		camera->pl->params.handles.handler[image_id],&ptp_objectinfo));
		fdata=malloc(ptp_objectinfo.ObjectCompressedSize);
		CPR (camera, ptp_getobject(&camera->pl->params,
		camera->pl->params.handles.handler[image_id],
		ptp_objectinfo.ObjectCompressedSize, fdata));
		CHECK (gp_file_set_data_and_size (file, fdata,
		ptp_objectinfo.ObjectCompressedSize));
		break;

	case GP_FILE_TYPE_PREVIEW:
		CPR (camera, ptp_getobjectinfo(&camera->pl->params,
		camera->pl->params.handles.handler[image_id],&ptp_objectinfo));
		fdata=malloc(ptp_objectinfo.ThumbCompressedSize);
		CPR (camera, ptp_getthumb(&camera->pl->params,
		camera->pl->params.handles.handler[image_id],
		ptp_objectinfo.ThumbCompressedSize, fdata));
		CHECK (gp_file_set_data_and_size (file, fdata,
		ptp_objectinfo.ThumbCompressedSize));
		break;
		

	default:

	}

	CHECK (set_mimetype (camera, file, ptp_objectinfo.ObjectFormat));

	return (GP_OK);
}

static int
delete_file_func (CameraFilesystem *fs, const char *folder,
			const char *filename, void *data)
{
	Camera *camera = data;

	unsigned long image_id;

	if (strcmp (folder, "/"))
		return (GP_ERROR_DIRECTORY_NOT_FOUND);

	// Get file number
	image_id = gp_filesystem_number (fs, folder, filename);

	CPR (camera, ptp_deleteobject(&camera->pl->params,
		camera->pl->params.handles.handler[image_id],0));

	return (GP_OK);
}

static int
get_info_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileInfo *info, void *data)
{
	Camera *camera = data;
	PTPObjectInfo oi;
	int n;
	char capture_date[MAXFILELEN];
	struct tm tm;
	char tmp[16];

	CR (n = gp_filesystem_number (fs, folder, filename));
	CPR (camera, ptp_getobjectinfo(&camera->pl->params,
		     camera->pl->params.handles.handler[n], &oi));
/*	GP_DEBUG ("ObjectInfo for '%s':");
	GP_DEBUG ("  StorageID: %d", oi.StorageID);
	GP_DEBUG ("  ObjectFormat: %d", oi.ObjectFormat);
	GP_DEBUG ("  ObjectCompressedSize: %d", oi.ObjectCompressedSize);
	GP_DEBUG ("  ThumbFormat: %d", oi.ThumbFormat);
	GP_DEBUG ("  ThumbCompressedSize: %d", oi.ThumbCompressedSize);
	GP_DEBUG ("  ThumbPixWidth: %d", oi.ThumbPixWidth);
	GP_DEBUG ("  ThumbPixHeight: %d", oi.ThumbPixHeight);
	GP_DEBUG ("  ImagePixWidth: %d", oi.ImagePixWidth);
	GP_DEBUG ("  ImagePixHeight: %d", oi.ImagePixHeight);
	GP_DEBUG ("  ImageBitDepth: %d", oi.ImageBitDepth);
	GP_DEBUG ("  ParentObject: %d", oi.ParentObject);
	GP_DEBUG ("  AssociationType: %d", oi.AssociationType);
	GP_DEBUG ("  AssociationDesc: %d", oi.AssociationDesc);
	GP_DEBUG ("  SequenceNumber: %d", oi.SequenceNumber);
*/
	info->file.fields = GP_FILE_INFO_SIZE|GP_FILE_INFO_TYPE;

	info->file.size   = oi.ObjectCompressedSize;
	strcpy_mime (info->file.type, oi.ObjectFormat);

	if ((oi.ObjectFormat & 0x0800) != 0) {
		info->preview.fields = GP_FILE_INFO_SIZE|GP_FILE_INFO_WIDTH
				|GP_FILE_INFO_HEIGHT|GP_FILE_INFO_TIME|GP_FILE_INFO_TYPE;
		strcpy_mime(info->preview.type, oi.ThumbFormat);
		info->preview.size   = oi.ThumbCompressedSize;
		info->preview.width  = oi.ThumbPixWidth;
		info->preview.height = oi.ThumbPixHeight;

		info->file.fields = info->file.fields|GP_FILE_INFO_WIDTH|GP_FILE_INFO_HEIGHT|
				GP_FILE_INFO_TIME;

		info->file.size   = oi.ObjectCompressedSize;
		info->file.width  = oi.ImagePixWidth;
		info->file.height = oi.ImagePixHeight;
		
		ptp_getobjectcapturedate (&oi, capture_date);
		strncpy (tmp, capture_date, 4);
		tmp[4] = 0;
		tm.tm_year=atoi (tmp) - 1900;
		strncpy (tmp, capture_date + 4, 2);
		tmp[2] = 0;
		tm.tm_mon = atoi (tmp) - 1;
		strncpy (tmp, capture_date + 6, 2);
		tmp[2] = 0;
		tm.tm_mday = atoi (tmp);
		strncpy (tmp, capture_date + 9, 2);
		tmp[2] = 0;
		tm.tm_hour = atoi (tmp);
		strncpy (tmp, capture_date + 11, 2);
		tmp[2] = 0;
		tm.tm_min = atoi (tmp);
		strncpy (tmp, capture_date + 13, 2);
		tmp[2] = 0;
		tm.tm_sec = atoi (tmp);
		info->file.time = mktime (&tm);
	}

		return (GP_OK);
}

int
camera_init (Camera *camera)
{
	GPPortSettings settings;
	short ret;

	/* Make sure our port is a USB port */
	if (camera->port->type != GP_PORT_USB) {
		gp_camera_set_error (camera, _("PTP is only possible for "
			"USB cameras."));
		return (GP_ERROR_UNKNOWN_PORT);
	}

	camera->functions->about = camera_about;
	camera->functions->exit = camera_exit;

	/* We need some data that we pass around */
	camera->pl = malloc (sizeof (CameraPrivateLibrary));
	if (!camera->pl)
		return (GP_ERROR_NO_MEMORY);
	camera->pl->params.write_func = ptp_write_func;
	camera->pl->params.read_func  = ptp_read_func;
	camera->pl->params.debug_func = ptp_debug_func;
	camera->pl->params.error_func = ptp_error_func;
	camera->pl->params.data = camera;
	camera->pl->params.transaction_id=0x01;

	/* Configure the port */
	CR (gp_port_set_timeout (camera->port, 2000));
	CR (gp_port_get_settings (camera->port, &settings));
	settings.usb.inep = 0x01;
	settings.usb.outep = 0x01;
	settings.usb.config = 1;
	settings.usb.interface = 0;
	settings.usb.altsetting = 0;
	CR (gp_port_set_settings (camera->port, settings));

	/* Establish a connection to the camera */
	ret=ptp_opensession (&camera->pl->params, 1);
	if (ret!=PTP_RC_SessionAlreadyOpened && ret!=PTP_RC_OK) {
		report_result(camera, ret);
	}
	/* Get file handles array for filesystem */
	CPR (camera, ptp_getobjecthandles (&camera->pl->params, &camera->pl->params.handles, 0xffffffff)); // XXX return from all stores

	GP_DEBUG ("ptp_getobjecthandles done");

	/* Configure the CameraFilesystem */
	CR (gp_filesystem_set_list_funcs (camera->fs, file_list_func,
					  NULL, camera));
	CR (gp_filesystem_set_info_funcs (camera->fs, get_info_func, NULL,
					  camera));
	CR (gp_filesystem_set_file_funcs (camera->fs, get_file_func,
					  delete_file_func, camera));
	CR (gp_filesystem_set_folder_funcs (camera->fs, NULL,
					    NULL, NULL,
					    NULL, camera));

	return (GP_OK);
}
