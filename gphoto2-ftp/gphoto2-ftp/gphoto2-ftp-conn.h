/* gphoto2-ftp-conn.h
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

#ifndef __GPHOTO2_FTP_CONN_H__
#define __GPHOTO2_FTP_CONN_H__

#include <gphoto2-ftp-params.h>

typedef struct _GFConn GFConn;
struct _GFConn {
	int idletime;
	int fd;
	char buf[1024];
	unsigned int len;
};


GFConn *gf_conn_open  (GFParams *params);
int     gf_conn_write (GFConn *conn, const char *buf, unsigned int len);
void    gf_conn_flush (GFConn *conn);
void    gf_conn_close (GFConn *conn);

#endif
