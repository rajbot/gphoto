/* gphoto2-ftp-params.h
 *
 * Copyright © 2002 Lutz Müller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef __GPHOTO2_FTP_PARAMS_H__
#define __GPHOTO2_FTP_PARAMS_H__

#include <netinet/in.h>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-port-info-list.h>

typedef enum {
	GF_TYPE_ASCII = 0,
	GF_TYPE_BINARY,
	GF_TYPE_LOCAL
} GFType;

typedef struct _GFParams GFParams;
struct _GFParams {
	Camera *camera;
	CameraAbilitiesList *al;
	GPPortInfoList *il;
	CameraFile *file;
	char *folder;

	int idletime;
	int passive;
	int port;
	int fd;
	GFType type;
	struct sockaddr_in sai_sock, sai_peer;
};

#endif
