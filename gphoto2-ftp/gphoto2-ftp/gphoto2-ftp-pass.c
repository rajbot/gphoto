/* gphoto2-ftp-pass.c
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

#include "config.h"
#include "gphoto2-ftp-pass.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <gphoto2/gphoto2-port-info-list.h>
#include <gphoto2/gphoto2-port-result.h>

#define CR(result) {int r=(result); if (r<0) return (r);}

int
gf_pass (GFParams *params, const char *password)
{
	GPPortInfo i;
	int p;

	CR (p = gp_port_info_list_lookup_path (params->il, password));
	CR (gp_port_info_list_get_info        (params->il, p, &i));
	CR (gp_camera_set_port_info           (params->camera, i));
	free (params->folder);
	params->folder = malloc (strlen ("/" + 1));
	if (!params->folder)
		return (GP_ERROR_NO_MEMORY);
	strcpy (params->folder, "/");
	printf ("%3d %s\r\n", 230, "Connected to the camera.");
	fflush (stdout);

	return (0);
}
