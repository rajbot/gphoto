/* gphoto2-ftp-retr.c
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
#include "gphoto2-ftp-retr.h"

#include <stdio.h>
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
	int r;
	GFConn *conn;
	const char *data;
	unsigned long int size;
	unsigned int i;

	if (!filename || !*filename) {
		fprintf (stdout, "501 No file name.\r\n");
		fflush (stdout);
		return (0);
	}

	gp_file_new (&file);
	r = gp_camera_file_get (params->camera, params->folder, filename,
				GP_FILE_TYPE_NORMAL, file, NULL);
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
