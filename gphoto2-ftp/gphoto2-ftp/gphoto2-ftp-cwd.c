/* gphoto2-ftp-cwd.c
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
#include "gphoto2-ftp-cwd.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include <gphoto2/gphoto2-camera.h>

#undef MAX
#define MAX(a,b) ((a>b)?a:b)

void
gf_cwd (GFParams *params, const char *path)
{
	CameraList list;
	char *new_path;
	unsigned int i, n;

	if (!params || !path)
		return;

	if (*path == '/') {
		new_path = malloc (strlen (path) + 1);
		strcpy (new_path, path);
	} else {
		new_path = malloc (strlen (params->folder) + 1 +
				   strlen (path) + 1);
		strcpy (new_path, params->folder);
		if (new_path[strlen (new_path) - 1] != '/')
			strcat (new_path, "/");
		strcat (new_path, path);
	}

	/* Remove '/.'. */
	for (i = 0; new_path[i]; ) {
		if (new_path[i++] != '/')
			continue;
		if (new_path[i] == '.') {
			if (!new_path[i + 1])
				new_path[i] = '\0';
			else if (new_path[i + 1] == '/') {
				memmove (new_path + i, new_path + i + 2, 
					 strlen (new_path) - i - 1);
				i--;
			}
		}
	}

	/* Remove '/..'. */
	for (i = 0; new_path[i]; ) {
		if (new_path[i++] != '/')
			continue;
		if (!new_path[i] || !new_path[i + 1])
			break;
		if ((new_path[i] == '.') && (new_path[i + 1] == '.')) {
			if (!new_path[i + 2]) {
				for (n = MAX (2, i) - 2; n > 0; n--)
					if (new_path[n] == '/')
						break;
				memmove (new_path + n + 1, new_path + i + 2,
					 strlen (new_path) - i - 1);
				i = n;
			} else if (new_path[i + 2] == '/') {
				for (n = MAX (2, i) - 2; n > 0; n--)
					if (new_path[n] == '/')
						break;
				memmove (new_path + n + 1,
					 new_path + i + 3,
					 strlen (new_path) - i - 2);
				i = n;
			}
		}
	}

	/* Remove trailing '/'. */
	if ((strlen (new_path) > 1) && new_path[strlen (new_path) - 1] == '/')
		new_path[strlen (new_path) - 1] = '\0';

	if (strcmp (new_path, "/capture-image") &&
	    strcmp (new_path, "/capture-preview") &&
	    (gp_camera_folder_list_files (params->camera, new_path,
					  &list, NULL) < 0)) {
		fprintf (stdout, "530 Can not change directory to "
			 "'%s'.\r\n", new_path);
		fflush (stdout);
		free (new_path);
		return;
	}

	free (params->folder);
	params->folder = new_path;
	fprintf (stdout, "250 Changed to directory '%s'.\r\n", params->folder);
	fflush (stdout);
	return;
}
