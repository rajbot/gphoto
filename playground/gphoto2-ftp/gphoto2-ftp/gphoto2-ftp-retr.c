/* gphoto2-ftp-retr.c
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

#include "config.h"
#include "gphoto2-ftp-retr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <syslog.h>

#include <gphoto2-camera.h>

#include "gphoto2-ftp-conn.h"

#undef MAX
#define MAX(a,b) ((a>b)?a:b)
#undef MIN
#define MIN(a,b) ((a<b)?a:b)

#define PACKET_SIZE 262144

int
gf_retr (GFParams *params, const char *filename)
{
	CameraFile *file = NULL;
	int r = GP_OK;
	GFConn *conn;
	const char *data, *name;
	unsigned long int size;
	int i;
	char *folder = NULL;

	if (!filename || !*filename) {
		fprintf (stdout, "501 No file name.\r\n");
		fflush (stdout);
		return (0);
	}

	for (i = strlen (filename) - 1; i >= 0; i--)
		if (filename[i] == '/')
			break;

	folder = malloc (strlen (params->folder) + 1 + strlen (filename) + 1);
	if (!folder) {
		fprintf (stdout, "501 Not enough memory.\r\n");
		fflush (stdout);
		return (0);
	}
	memset (folder, 0, strlen (params->folder) + 1 +
			   strlen (filename) + 1);

	if (i >= 0) {
		if (filename[0] == '/')
			strncpy (folder, filename, i + 1);
		else {
			strcpy (folder, params->folder);
			if (folder[strlen (folder) - 1] != '/')
				strcat (folder, "/");
			strncat (folder, filename, i);
		}
		if (folder[strlen (folder) - 1] == '/')
			folder[strlen (folder) - 1] = '\0';
	} else
		strcpy (folder, params->folder);

	filename += i + 1;

	if (params->file)
		gp_file_get_name (params->file, &name);

	if (name && (!strcmp (folder, "/capture-preview") &&
		     !strcmp (filename, name))) {
		file = params->file;
		gp_file_ref (params->file);
	} else {
		gp_file_new (&file);
		r = gp_camera_file_get (params->camera, folder, filename,
					GP_FILE_TYPE_NORMAL, file, NULL);
	}

	free (folder);
	if (r < 0) {
		gp_file_unref (file);
		return (r);
	}

	conn = gf_conn_open (params);
	if (!conn) {
		gp_file_unref (file);
		return (-1);
	}

	gp_file_get_data_and_size (file, &data, &size);
	for (i = 0; i < size; ) {
		r = gf_conn_write (conn, data + i,
				   MIN (PACKET_SIZE, size - i));
		if (r < 0) {
			gp_file_unref (file);
			gf_conn_close (conn);
			return (0);
		}
		i += r;
		syslog (LOG_INFO, "Now transmitting at %i...", i);
	}

	gp_file_unref (file);
	gf_conn_flush (conn);

	fprintf (stdout, "226 Transfer complete.\r\n");
	fflush (stdout);

	gf_conn_close (conn);

	return (0);
}
