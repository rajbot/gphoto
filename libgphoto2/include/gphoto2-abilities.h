/* gphoto2-abilities.h
 *
 * Copyright (C) 2000 Scott Fritzinger
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

#ifndef __GPHOTO2_ABILITIES_H__
#define __GPHOTO2_ABILITIES_H__

#include <gphoto2-port.h>

#define SERIAL_SUPPORTED(p)     (p & (1 << 0))
#define PARALLEL_SUPPORTED(p)   (p & (1 << 1))
#define USB_SUPPORTED(p)        (p & (1 << 2))
#define IEEE1394_SUPPORTED(p)   (p & (1 << 3))
#define NETWORK_SUPPORTED(p)    (p & (1 << 4))

typedef gp_port_type CameraPortType; /* GP_PORT_SERIAL, GP_PORT_USB, etc. */

typedef enum {
	GP_OPERATION_NONE               = 0,
	GP_OPERATION_CAPTURE_IMAGE      = 1 << 0,
	GP_OPERATION_CAPTURE_VIDEO      = 1 << 1,
	GP_OPERATION_CAPTURE_AUDIO      = 1 << 2,
	GP_OPERATION_CAPTURE_PREVIEW    = 1 << 3,
	GP_OPERATION_CONFIG             = 1 << 4
} CameraOperation;

typedef enum {
	GP_FILE_OPERATION_NONE          = 0,
	GP_FILE_OPERATION_DELETE        = 1 << 1,
	GP_FILE_OPERATION_CONFIG        = 1 << 2,
	GP_FILE_OPERATION_PREVIEW       = 1 << 3
} CameraFileOperation;

typedef enum {
	GP_FOLDER_OPERATION_NONE        = 0,
	GP_FOLDER_OPERATION_DELETE_ALL  = 1 << 0,
	GP_FOLDER_OPERATION_PUT_FILE    = 1 << 1,
	GP_FOLDER_OPERATION_CONFIG      = 1 << 2
} CameraFolderOperation;

typedef struct {
	char model [128];

	/* Supported ports and speeds (latter terminated with a value of 0) */
	CameraPortType port;
	int speed [64];

	/* Supported operations */
	CameraOperation       operations;
	CameraFileOperation   file_operations;
	CameraFolderOperation folder_operations;

	int usb_vendor, usb_product;

	/* For core use */
	char library [1024];
	char id [1024];
} CameraAbilities;

CameraAbilities *gp_abilities_new   (void);
int              gp_abilities_free  (CameraAbilities *abilities);
int              gp_abilities_dump  (CameraAbilities *abilities);
int              gp_abilities_clear (CameraAbilities *abilities);

#endif /* __GPHOTO2_ABILITIES_H__ */
