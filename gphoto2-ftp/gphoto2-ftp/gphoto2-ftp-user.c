/* gphoto2-ftp-user.c
 *
 * Copyright (C) 2002 Lutz Müller <lutz@users.sourceforge.net>
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

#include <config.h>
#include "gphoto2-ftp-user.h"

#include <stdio.h>
#include <string.h>

#define CR(result) {int r=(result); if (r<0) return r;}

int
gf_user (GFParams *params, const char *user)
{
	CameraAbilities a;
	int m;

	CR (m = gp_abilities_list_lookup_model (params->al, user));
	CR (gp_abilities_list_get_abilities    (params->al, m, &a));
	CR (gp_camera_set_abilities            (params->camera, a));
	if (strcmp (user, "Directory Browse"))
		printf ("%3d %s\r\n", 331, "Please specify the port.");
	else
		printf ("%3d %s\r\n", 230, "Connected to camera.");
	fflush (stdout);

	return (0);
}
