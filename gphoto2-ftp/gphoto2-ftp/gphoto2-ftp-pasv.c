/* gphoto2-ftp-pasv.c
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
#include "gphoto2-ftp-pasv.h"

#include <stdio.h>
#include <syslog.h>

#include <sys/socket.h>

void
gf_pasv (GFParams *params)
{
	int n;
	struct sockaddr_in sai;
	unsigned int a, p;

	if (!params)
		return;

	if (params->fd) {
		close (params->fd);
		params->fd = 0;
	}

	params->fd = socket (AF_INET, SOCK_STREAM, 0);
	if (params->fd < 0) {
		params->fd = 0;
		syslog (LOG_INFO, "Can't open passive connection.");
		fprintf (stdout, "425 Can't open passive connection.\r\n");
		fflush (stdout);
		return;
	}

	sai = params->sai_sock;
	sai.sin_port = 0;
	if (bind (params->fd, (struct sockaddr *) &sai, sizeof (sai)) < 0)
		goto ExitError;

	n = sizeof (sai);
	if (getsockname (params->fd, (struct sockaddr *) &sai, &n) < 0)
		goto ExitError;

	if (listen (params->fd, 1) < 0)
		goto ExitError;

	a = ntohl (sai.sin_addr.s_addr);
	p = ntohs ((unsigned short int) (sai.sin_port));
	fprintf (stdout, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n",
		 (a >> 24) & 0xff, (a >> 16) & 0xff, (a >> 8) & 0xff, a & 0xff,
		 (p >>  8) & 0xff, p & 0xff);
	fflush (stdout);
	params->passive = 1;
	return;

ExitError:
	syslog (LOG_INFO, "Can't open passive connection.");
	fprintf (stdout, "425 Can't open passive connection.\r\n");
	fflush (stdout);
	close (params->fd);
	params->fd = 0;
	return;
}
