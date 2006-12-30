/* gphoto2-ftp-conn.c
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
#include "gphoto2-ftp-conn.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>

#include <netinet/ip.h>
#include <arpa/inet.h>

#include <syslog.h>
#include <errno.h>

GFConn *
gf_conn_open (GFParams *params)
{
	GFConn *conn = NULL;
	struct sockaddr_in sai;
	int n;

	if (!params || !params->fd)
		goto ExitError;

	conn = malloc (sizeof (GFConn));
	if (!conn)
		goto ExitError;
	memset (conn, 0, sizeof (GFConn));
	conn->idletime = params->idletime;

	if (params->passive) {
		socklen_t sai_size = sizeof (sai);

		conn->fd = accept (params->fd,
				   (struct sockaddr *) &sai, &sai_size);
		if (conn->fd < 0)
			goto ExitError;
	} else {
		sai.sin_addr.s_addr = params->sai_peer.sin_addr.s_addr;
		sai.sin_port = htons (params->port);
		sai.sin_family = AF_INET;

		if (connect (params->fd, (struct sockaddr *) &sai,
			     sizeof (sai)))
			goto ExitError;

		conn->fd = params->fd;
		params->fd = 0;
	}

	n = IPTOS_THROUGHPUT;
	setsockopt (conn->fd, SOL_IP, IP_TOS, (char *) &n, sizeof (int));

	fprintf (stdout, "150 Connecting to %s:%d\r\n",
		 inet_ntoa (sai.sin_addr),
		 ntohs ((unsigned short) sai.sin_port));
	fflush (stdout);

	return (conn);

ExitError:
	free (conn);
	fprintf (stdout, "425 Can't open data connection.\r\n");
	fflush (stdout);
	return (NULL);
}

static int
gf_conn_write_now (GFConn *conn, const char *buf, unsigned int len)
{
	int r = 0, retries;

	if (!conn->fd)
		return 0;

	for (retries = 0; retries < 5; retries++) {
		r = write (conn->fd, buf, len);
		if (r >= 0)
			break;

		if (errno == EAGAIN) {
			fd_set s;
			struct timeval tv;

			FD_ZERO (&s);
			FD_SET (conn->fd, &s);
			tv.tv_sec = conn->idletime;
			tv.tv_usec = 0;
			select (conn->fd + 1, &s, NULL, NULL, &tv);
			if (!FD_ISSET (conn->fd, &s)) {
				return (-1);
			}
		} else
			return (-1);
	}
	syslog (LOG_INFO, "Wrote %i byte(s).", r);

	return (r);
}

int
gf_conn_write (GFConn *conn, const char *buf, unsigned int len)
{
	int w1 = 0, w2 = 0;

	if (!conn)
		return (-1);

	/* Do we have enough data to write? */
	if (len + conn->len < sizeof (conn->buf)) {
		memcpy (conn->buf + conn->len, buf, len);
		conn->len += len;
		return len;
	}

	/* Write the buffer. */
	while (conn->len) {
		w1 = gf_conn_write_now (conn, conn->buf, conn->len);
		if (w1 < 0)
			return (w1);
		memmove (conn->buf, conn->buf + w1, conn->len - w1);
		conn->len -= w1;
	}

	/* Write the data. */
	while (len) {
		w2 = gf_conn_write_now (conn, buf, len);
		if (w2 < 0)
			return (w2);
		buf += w2;
		len -= w2;
	}

	syslog (LOG_INFO, "Wrote %i byte(s).", w1 + w2);
	return (w1 + w2);
}

void
gf_conn_flush (GFConn *conn)
{
	if (!conn)
		return;

	if (conn->len) {
		gf_conn_write_now (conn, conn->buf, conn->len);
		conn->len = 0;
	}
}

void
gf_conn_close (GFConn *conn)
{
	if (!conn)
		return;

	if (conn->fd) {
		if (conn->len)
			gf_conn_flush (conn);
		close (conn->fd);
		conn->fd = 0;
	}

	free (conn);
}
