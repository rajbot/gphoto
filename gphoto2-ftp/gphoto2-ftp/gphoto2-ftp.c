/* gphoto2-ftp.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <syslog.h>
#include <unistd.h>
#include <limits.h>

#include <netinet/ip.h>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-abilities-list.h>

#include "gphoto2-ftp-cwd.h"
#include "gphoto2-ftp-list.h"
#include "gphoto2-ftp-params.h"
#include "gphoto2-ftp-pass.h"
#include "gphoto2-ftp-pasv.h"
#include "gphoto2-ftp-port.h"
#include "gphoto2-ftp-retr.h"
#include "gphoto2-ftp-type.h"
#include "gphoto2-ftp-user.h"

int
main (int argc, char **argv)
{
	fd_set rs;
	struct timeval tv;
	int n, cmd_len;
	char cmd[PATH_MAX + 32];
	const char *arg;
	GFParams params;

	memset (&params, 0, sizeof (GFParams));
	if (gp_camera_new (&(params.camera)) < 0)
		goto ExitError;
	if (gp_abilities_list_new (&(params.al)) < 0)
		goto ExitError;
	if (gp_abilities_list_load (params.al, NULL) < 0)
		goto ExitError;
	if (gp_port_info_list_new (&params.il) < 0)
		goto ExitError;
	if (gp_port_info_list_load (params.il) < 0)
		goto ExitError;
	params.folder = malloc (strlen ("/") + 1);
	if (!params.folder)
		goto ExitError;
	strcpy (params.folder, "/");
	params.idletime = 1800;

	n = sizeof (struct sockaddr_in);
	if (getsockname (0, (struct sockaddr *) &params.sai_sock, &n)) {
		fprintf (stdout, "421 Can not get name of sock.\r\n");
		fflush (stdout);
		goto ExitError;
	}
	n = sizeof (struct sockaddr_in);
	if (getpeername (0, (struct sockaddr *) &params.sai_peer, &n)) {
		fprintf (stdout, "421 Can not get name of peer.\r\n");
		fflush (stdout);
		goto ExitError;
	}
	n = IPTOS_LOWDELAY;
	setsockopt (0, IPPROTO_IP, IP_TOS, (char *) &n, sizeof (int));

	fprintf (stdout, "220 FTP server ready.\r\n");
	fflush (stdout);

	while (1) {

		/* Read something. */
		syslog (LOG_INFO, "Reading...");
		FD_ZERO (&rs);
		tv.tv_sec = 1800;
		tv.tv_usec = 0;
		memset (cmd, 0, sizeof (cmd));
		cmd_len = n = 0;
		while (1) {
			if (!n) {
				FD_SET (0, &rs);
				select (1, &rs, NULL, NULL, &tv);
				if (ioctl (0, FIONREAD, &n) < 0)
					n = 0;
			}
			if (FD_ISSET (0, &rs)) {
				if (read (0, cmd + cmd_len, 1) <= 0)
					goto ExitError;
				if (n)
					n--;
				if (cmd[cmd_len] == '\n') {
					cmd[cmd_len + 1] = '\0';
					break;
				}
				if (cmd_len < sizeof (cmd) - 2)
					cmd_len++;
			} else
				goto ExitError;
		}

		syslog (LOG_INFO, "Got '%s'...", cmd);
		for (n = 0; isalpha (cmd[n]) && n < sizeof (cmd); n++)
			cmd[n] = tolower (cmd[n]);
		if (!n) {
			printf ("%3d %s\r\n", 221, "Goodbye.");
			fflush (stdout);
			goto ExitOk;
		}
		while (isspace (cmd[n]) && n < sizeof (cmd))
			cmd[n++] = '\0';
		arg = cmd + n;
		while (cmd[n] && n < sizeof (cmd))
			n++;
		n--;
		while (isspace (cmd[n]))
			cmd[n--] = '\0';
		syslog (LOG_INFO, "Processing '%s' - '%s'...", cmd, arg);
		if (!strcasecmp (cmd, "cwd")) {
			gf_cwd (&params, arg);
		} else if (!strcasecmp (cmd, "list")) {
			if (gf_list (&params, arg) < 0)
				goto ExitError;
		} else if (!strcasecmp (cmd, "noop")) {
			fprintf (stdout, "200 NOOP command successful.\r\n");
			fflush (stdout);
		} else if (!strcasecmp (cmd, "pass")) {
			if (gf_pass (&params, arg) < 0)
				goto ExitError;
		} else if (!strcasecmp (cmd, "pasv")) {
			gf_pasv (&params);
		} else if (!strcasecmp (cmd, "port")) {
			if (gf_port (&params, arg) < 0)
				goto ExitError;
		} else if (!strcasecmp (cmd, "pwd") ||
			   !strcasecmp (cmd, "xpwd")) {
			fprintf (stdout, "257 \"%s\"\r\n", params.folder);
			fflush (stdout);
		} else if (!strcasecmp (cmd, "quit")) {
			printf ("%3d %s\r\n", 221, "Goodbye.");
			fflush (stdout);
			syslog (LOG_INFO, "Quit.");
			return (0);
		} else if (!strcasecmp (cmd, "retr")) {
			if (gf_retr (&params, arg) < 0)
				goto ExitError;
		} else if (!strcasecmp (cmd, "syst")) {
			printf ("215 UNIX Type: L8\r\n");
			fflush (stdout);
		} else if (!strcasecmp (cmd, "type")) {
			if (!arg)
				goto ExitError;
			gf_type (&params, *arg);
		} else if (!strcasecmp (cmd, "user")) {
			if (gf_user (&params, arg) < 0)
				goto ExitError;
		} else {
			syslog (LOG_INFO, "Command '%s'...", cmd);
			printf ("%3d %s\r\n", 550, "Unknown command.");
			fflush (stdout);
		}
	}

ExitError:
	gp_abilities_list_free (params.al);
	gp_port_info_list_free (params.il);
	gp_camera_unref (params.camera);
	free (params.camera);
	syslog (LOG_INFO, "Error: 1");
	return (1);

ExitOk:
	gp_abilities_list_free (params.al);
	gp_port_info_list_free (params.il);
	gp_camera_unref (params.camera);
	free (params.camera);
	return (0);
}
