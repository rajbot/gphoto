/* fuji.h
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

#ifndef __FUJI_H__
#define __FUJI_H__

#include <gphoto2-context.h>
#include <gphoto2-camera.h>

typedef enum _FujiCmd FujiCmd;
enum _FujiCmd {
	FUJI_CMD_PIC_GET	= 0x00,
	FUJI_CMD_PIC_GET_THUMB  = 0x02,
	FUJI_CMD_SPEED		= 0x07,
        FUJI_CMD_VERSION        = 0x09,
	FUJI_CMD_PIC_NAME	= 0x0a,
	FUJI_CMD_PIC_COUNT	= 0x0b,
        FUJI_CMD_PIC_SIZE       = 0x17,
        FUJI_CMD_PIC_DEL        = 0x19,
        FUJI_CMD_TAKE           = 0x27,
        FUJI_CMD_CHARGE_FLASH   = 0x34,
        FUJI_CMD_CMDS_VALID     = 0x4C,
        FUJI_CMD_PREVIEW        = 0x64
};

int fuji_get_cmds  (Camera *camera, unsigned char *cmds, GPContext *context);

int fuji_ping      (Camera *camera, GPContext *context);
int fuji_pic_count (Camera *camera, unsigned int *n, GPContext *context);

/* Operations on pictures */
int fuji_pic_name      (Camera *camera, unsigned int i, const char **name,
		        GPContext *context);
int fuji_pic_size      (Camera *camera, unsigned int i, unsigned int *size,
		        GPContext *context);
int fuji_pic_del       (Camera *camera, unsigned int i, GPContext *context);
int fuji_pic_get       (Camera *camera, unsigned int i, unsigned char **data,
		        unsigned int *size, GPContext *context);
int fuji_pic_get_thumb (Camera *camera, unsigned int i, unsigned char **data,
			unsigned int *size, GPContext *context);

typedef enum _FujiSpeed FujiSpeed;
enum _FujiSpeed {
	FUJI_SPEED_9600		= 0,
	FUJI_SPEED_19200	= 5,
	FUJI_SPEED_38400	= 6,
	FUJI_SPEED_57600	= 7,
	FUJI_SPEED_115200	= 8
};

int fuji_set_speed (Camera *camera, FujiSpeed speed, GPContext *context);

#endif /* __FUJI_H__ */
