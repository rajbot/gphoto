/* gphoto2-ftp-port.c
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
#include "gphoto2-ftp-port.h"

#include <stdio.h>
#include <errno.h>

#include <netinet/ip.h>

#include <sys/socket.h>

int
gf_port (GFParams *params, const char *args)
{
	int r, on;
	unsigned int a1, a2, a3, a4, p1, p2, a;
	struct sockaddr_in sai;

	r = sscanf (args, "%u,%u,%u,%u,%u,%u", &a1, &a2, &a3, &a4, &p1, &p2 );
	if (r != 6) {
		fprintf (stdout, "501 Syntax error.\r\n");
		fflush (stdout);
		return (-1);
	}
	if ((a1 >= 256) || (a2 >= 256) || (a3 >= 256) || (a4 >= 256) ||
	    (p1 >= 256) || (p2 >= 256)) {
		fprintf (stdout, "501 Syntax error.\r\n");
		fflush (stdout);
		return (-1);
	}

	a = (a1 << 24) | (a2 << 16) | (a3 << 8) | a4,
	params->port = (p1 << 8 ) | p2;

	if (params->fd) {
		close (params->fd);
		params->fd = 0;
	}

	params->fd = socket (AF_INET, SOCK_STREAM, 0);
	if (params->fd < 0) {
		params->fd = 0;
		params->port = 0;
		fprintf (stdout, "415 Can't make data socket.\r\n");
		fflush (stdout);
		return (-1);
	}

	on = 1;
	if (setsockopt (params->fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,
			sizeof (on)) < 0) {
		close (params->fd);
		params->fd = 0;
		params->port = 0;
		fprintf (stdout, "415 Can't set options.\r\n");
		fflush (stdout);
		return (-1);
	}

	sai = params->sai_sock;
	sai.sin_port = htons (ntohs (params->sai_sock.sin_port) - 1);
	sai.sin_family = AF_INET;
	sai.sin_addr = params->sai_sock.sin_addr;
	for (r = 1; r < 10; r++) {
		if (bind (params->fd, (struct sockaddr *) &sai,
			  sizeof (sai)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			r = 10;
			break;
		}
		sleep (r);
	}
	if (r == 10) {
		close (params->fd);
		params->fd = 0;
		params->port = 0;
		fprintf (stdout, "415 Can't bind.\r\n");
		fflush (stdout);
		return (-1);
	}

	params->passive = 0;

	on = IPTOS_THROUGHPUT;
	setsockopt (params->fd, IPPROTO_IP, IP_TOS, (char *) &on, sizeof (int));

	fprintf (stdout, "200 PORT command successful.\r\n");
	fflush (stdout);

	return (0);
}
