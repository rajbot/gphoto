/* gphoto2-ftp-list.c
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
#include "gphoto2-ftp-list.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <syslog.h>

#include <gphoto2/gphoto2-list.h>

#include <gphoto2-ftp-conn.h>

typedef enum {
	GF_LIST_OPTION_a = 1 << 0,
	GF_LIST_OPTION_C = 1 << 1,
	GF_LIST_OPTION_d = 1 << 2,
	GF_LIST_OPTION_F = 1 << 3,
	GF_LIST_OPTION_l = 1 << 4,
	GF_LIST_OPTION_r = 1 << 5,
	GF_LIST_OPTION_R = 1 << 6,
	GF_LIST_OPTION_t = 1 << 7,
	GF_LIST_OPTION_S = 1 << 8
} GFListOption;

static unsigned int
gf_list_file (GFParams *params, GFListOption options, GFConn *conn,
	      const char *folder, const char *file)
{
	char buf[1024], m[1024];
	CameraFileInfo info;

	memset (buf, 0, sizeof (buf));
	memset (m, 0, sizeof (m));
	if (options & GF_LIST_OPTION_l) {
		strcpy (m, "----------");
		if ((gp_camera_file_get_info (params->camera, folder, file,
					     &info, NULL) >= 0) &&
		    (info.file.fields & GP_FILE_INFO_PERMISSIONS)) {
			if (info.file.permissions & GP_FILE_PERM_READ)
				m[1] = m[4] = m[7] = 'r';
			if (info.file.permissions & GP_FILE_PERM_DELETE)
				m[2] = m[5] = m[8] = 'w';
		}
	}
	strcat (buf, m);
	if (strlen (buf))
		strcat (buf, " ");
	strcat (buf, file);
	strcat (buf, "\r\n");

	gf_conn_write (conn, buf, strlen (buf));

	return (1);
}

static unsigned int
gf_list_folder (GFParams *params, GFListOption options, GFConn *conn,
		const char *folder)
{
	unsigned int n = 0;
	CameraList list;
	int i, c;
	const char *name;
	char buf[1024];

	memset (buf, 0, sizeof (buf));

	if (folder) {
		strcpy (buf, "\r\n");
		strncat (buf, folder, sizeof (buf) - 1 - 5);
		strcat (buf, ":\r\n");
		gf_conn_write (conn, buf, strlen (buf));
	}

	if (gp_camera_folder_list_files (params->camera,
					 folder ? folder : params->folder,
					 &list, NULL) >= 0) {
		c = gp_list_count (&list);
		if (c < 0)
			c = 0;
		sprintf (buf, "Total %d\r\n", c);
		gf_conn_write (conn, buf, strlen (buf));
		for (i = 0; i < c; i++) {
			gp_list_get_name (&list, i, &name);
			n += gf_list_file (params, options, conn,
					   folder ? folder : params->folder,
					   name);
		}
	}

	return (n);
}

int
gf_list (GFParams *params, const char *args)
{
	GFConn *conn;
	GFListOption options = 0;
	unsigned int n = 0;
	int r;
	CameraFilePath path;

	/* Process options. */
	while (args && (*args == '-')) {
		while (isalnum (*(++args))) {
			switch (*args) {
			case 'a':
				options |= GF_LIST_OPTION_a;
				break;
			case 'C':
				options |= GF_LIST_OPTION_C;
				options &= ~GF_LIST_OPTION_l;
				break;
			case 'd':
				options |= GF_LIST_OPTION_d;
				break;
			case 'F':
				options |= GF_LIST_OPTION_F;
				break;
			case 'l':
				options |= GF_LIST_OPTION_l;
				options &= ~GF_LIST_OPTION_C;
				break;
			case 'r':
				options |= GF_LIST_OPTION_r;
				break;
			case 'R':
				options |= GF_LIST_OPTION_R;
				break;
			case 'S':
				options |= GF_LIST_OPTION_S;
				options &= ~GF_LIST_OPTION_t;
				break;
			case 't':
				options |= GF_LIST_OPTION_t;
				options &= ~GF_LIST_OPTION_S;
				break;
			case '1':
				options &= ~GF_LIST_OPTION_l;
				options &= ~GF_LIST_OPTION_C;
				break;
			default:
				break;
			}
		}
		while (isspace (*args))
			args++;
	}

	conn = gf_conn_open (params);
	if (!conn)
		return (-1);

	if (args && !strcmp (args, "/capture-image")) {
		r = gp_camera_capture (params->camera, GP_CAPTURE_IMAGE,
				       &path, NULL);
		if (r < 0) {
			syslog (LOG_INFO, "Could not capture: '%s'",
				gp_result_as_string (r));
			n = 0;
		} else {
			gf_conn_write (conn, path.folder, strlen (path.folder));
			if (strlen (path.folder) > 1)
				gf_conn_write (conn, "/", 1);
			gf_conn_write (conn, path.name, strlen (path.name));
			gf_conn_write (conn, "\r\n", 2);
			n = 1;
		}
	} else
		n = gf_list_folder (params, options, conn, NULL);

	gf_conn_close (conn);

	switch (n) {
	case 0:
		fprintf (stdout, "550 No files found.\r\n");
		break;
	case 1:
		fprintf (stdout, "226 1 match.\r\n");
		break;
	default:
		fprintf (stdout, "226 %d matches total.\r\n", n);
	}
	fflush (stdout);

	return (0);
}
