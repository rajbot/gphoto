/*  $Id$ */

/* 
 * Philips Digital Camera interface library
 *
 * Portions of this code were adapted from the ricoh_300z library
 * Copyright (C) 1998,1999 Clifford Wright.  
 * 
 * Portions of this code are from dc3play program Copyright (C)
 * 1997 Jun-ichiro Itoh.
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * This is an I/O library to communicate with Philips digital 
 * cameras. It should work with Ricoh cameras as well.
 *
 *    See the file PROTOCOL.TXT for a description of the serial
 *    protocol used by the camera.
 *
 *    See FUNCTIONS.TXT for a brief description of the functions
 *    provided by this library.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#if HAVE_TERMIOS_H
# include <termios.h>
#else
#  if HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#  endif
#  include <sgtty.h>
# endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "crctab.h" /* crc table for crc calculations */
#include "debug.h"  /* debugging macros */
#include "philips_io.h"  /* functions */

static char philips_buf[1024 * 4];/* buffer for data from the camera */
static size_t philips_len;	/* relative pointer of last valid data
				 * in receive buffer */
static int fd0;
static int close_handler_set = 0;/* flag, close handler set for timer */
static int camera_opened = 0;	/* flag, camera is in connected state */
static int disconnecting = 0;	/* flag, camera is disconnecting */
static int philips_mode  = 0;   /* flag, mode camera is in */

/*  philips_open
 *
 *  Open the serial port, initialize the library and establish
 *  communications with the camera. 
 *
 *  Return the ID of the camera and the current camera mode.
 * 
 *     The returned mode should be 00 00 00 meaning the camera
 *     is ready to send pictures to the computer.
 */

int philips_open ( serial_port, baudrate, cameraid )
char	*serial_port;
int		baudrate;
long	*cameraid;
{
	static struct sigaction close_philips = {
		(void (*)())philips_close_handler,
		0, 
		SA_RESTART
		};
	struct timeval zero_time = {0, 0};
	struct itimerval it_zero = {zero_time, zero_time};

	/* Configure event handler for SIGALRM */
	if (!close_handler_set) {
	    sigaction(SIGALRM, &close_philips, (struct sigaction *)NULL);
	    close_handler_set = 1;
		}

	/* Previouslly opened, turn off close delay timer and return */
	if (camera_opened) {
	    setitimer(ITIMER_REAL, &it_zero, (struct itimerval*)NULL);
	    return 0;
		}

	while(disconnecting); /* wait for any previous disconnect to complete */
	/* camera turns on with an initial baud rate of 2400, so
	 * open the serial port and configure to 2400 baud */
	if ( (fd0 = open (serial_port, O_RDWR|O_NDELAY)) == -1 ) {
	    fprintf(stderr, "philips_open: can't open %s\n", serial_port);
	    return -1;
		}

	if (philips_setbaud (fd0, 2400)) {
	    fprintf(stderr, "philips_open: can't set baudrate to 2400\n");
	    return -1;
		}

	/* initialize receive buffer pointer */
	philips_len = 0;

	/* connect to the camera */
	if ((*cameraid = philips_hello(baudrate)) == -1) {
	    fprintf(stderr, "philips_open: communication with camera failed.\n");
	    return -1;
		}

	/* set the desired baudrate */
	if (philips_setspeed(baudrate) == 1) {
	    fprintf(stderr, "philips_open: unable to set camera to %d baud.\n", baudrate);
	    return -1;
		}

	/* The philips software makes this query when initializing
	 * communications with the camera. The camera returns 00 00 01
	 * We don't know what it means but do it anyway for completness
	 * sake. */
	philips_init_query();

	/* Get the current camera mode */
	philips_mode = philips_get_mode ();

	camera_opened = 1;
	return philips_mode;
}

/*  philips_close 
 *
 *  Disconnect from the camera. 
 */

philips_close()
{
    struct timeval zero_time = {0, 0};
    struct timeval noactivity_time = {10, 0};
    struct itimerval it_noactivity = {zero_time, noactivity_time};

    /* set a close delay timer, that allows a close when no activity
     * occurs for 10 seconds. */
    setitimer(ITIMER_REAL, &it_noactivity, (struct itimerval*)NULL);
    return 0;
}

/*  philips_getnpicts
 *
 *  Get the number of pictures current in the cameras memory
 *
 *  Returns -1 if there is an error, otherwise return the
 *  number of pictures.
 */

philips_getnpicts(n)
long	*n;
{
	struct	CAM_DATA	cam_data;
	int		err = 0;

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, "\x00\x01", 2, 0x00, &cam_data );

	if ( ! err ) {
		*n = (long)cam_data.data[2];
		return ( (int)cam_data.data[2] );
		}
	return ( err );
}

/*  philips_getpictnum
 *
 *  Get the number currently displayed picture.
 *
 *  Returns -1 if there is an error, otherwise return the
 *  number of pictures.
 */

philips_getpictnum(n)
long	*n;
{
	struct	CAM_DATA	cam_data;
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_NUMPICT, NULL, 0, 0x00, &cam_data );

	if ( ! err ) {
		*n = (long)cam_data.data[2];
		return ( (int)cam_data.data[2] );
		}
	return ( err );
}

/*  philips_takepicture
 *
 *  Tell the camera to take a picture using the camera default
 *  settings.
 */

philips_takepicture()
{
	struct	CAM_DATA	cam_data;
	int		err = 0;

	/* Set camera state to take a picture */
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_SETVAL, "\x12\x01", 2, 0x00, &cam_data );
	philips_mode = 1;
	err = philips_execcmd ( P_SNAPPIC, "\x01", 1, 0x00, &cam_data );
	err = philips_execcmd ( P_SETVAL, "\x12\x00", 2, 0x00, &cam_data );
	philips_mode = 0;

	return ( err );
}

/*  philips_getpictsize
 *
 *  Get the size (in bytes) of a picture in the camera's memory
 *
 */

philips_getpictsize(n, size)
int n;		/* picture number */
int *size;	/* size of picture in bytes */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 4;
	buf[1] = n & 0xff;
	buf[2] = (n >> 8) & 0xff;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETIMG, buf, 3, 0x00, &cam_data );

	if ( ! err ) 
		*size = cam_data.data[5] << 24 | cam_data.data[4] << 16 | cam_data.data[3] << 8 | cam_data.data[2]; 

	return ( err );
}


/*  philips_gettotalbytes
 *
 *  Get the size (in bytes) of the camera's memory
 *
 */

philips_gettotalbytes(size)
int *size;	/* size of picture in bytes */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0x00;
	buf[1] = 0x05;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 2, 0x00, &cam_data );

	if ( ! err ) 
		*size = cam_data.data[5] << 24 | cam_data.data[4] << 16 | cam_data.data[3] << 8 | cam_data.data[2]; 

	return ( err );
}


/*  philips_getavailbytes
 *
 *  Get the size (in bytes) of the camera's memory available
 *
 */

philips_getavailbytes(size)
int *size;	/* size of picture in bytes */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0x00;
	buf[1] = 0x06;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 2, 0x00, &cam_data );

	if ( ! err ) 
		*size = cam_data.data[5] << 24 | cam_data.data[4] << 16 | cam_data.data[3] << 8 | cam_data.data[2]; 

	return ( err );
}


/*  philips_getexposure
 *
 *  Get the current exposure setting.
 *
 *    01 = -2.0
 *    02 = -1.5
 *    05 =  0.0
 *    08 = +1.5
 *    09 = +2.0
 *    FF = automatic
 *
 */

philips_getexposure(exposure)
int *exposure;	/* exposure setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x03;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*exposure = cam_data.data[2];

	return ( err );
}


/*  philips_setexposure
 *
 *  Set the current exposure setting.
 *
 *    01 = -2.0
 *    02 = -1.5
 *    05 =  0.0
 *    08 = +1.5
 *    09 = +2.0
 *    FF = automatic
 *
 */

philips_setexposure(exposure)
int exposure;	/* exposure setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x03;
	buf[1] = exposure & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getwhitelevel
 *
 *  Get the camera's current white level setting.
 *
 */

philips_getwhitelevel(level)
int *level;	/* white level */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x04;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*level = cam_data.data[2];

	return ( err );
}


/*  philips_setwhitelevel
 *
 *  Set the current whitelevel setting.
 *
 *    00 = automatic
 *    01 = outdoors
 *    02 = flouresent
 *    05 = incandescent
 */

philips_setwhitelevel(whitelevel)
int whitelevel;	/* whitelevel setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x04;
	buf[1] = whitelevel & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getzoom
 *
 *  Get the camera's current zoom setting.
 *
 */

philips_getzoom(zoom)
int *zoom;	/* zoom level */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x05;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*zoom = cam_data.data[2];

	return ( err );
}


/*  philips_setzoom
 *
 *  Set the current zoom setting.
 *
 *    00 = no zoom
 *    08 = maximum zoom
 */

philips_setzoom(zoom)
int zoom;	/* zoom setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x05;
	buf[1] = zoom & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getflash
 *
 *  Get the camera's current flash setting.
 *
 */

philips_getflash(flash)
int *flash;	/* flash level */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x06;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*flash = cam_data.data[2];

	return ( err );
}


/*  philips_setflash
 *
 *  Set the current flash setting.
 *
 *    00 = automatic
 *    01 = off
 *    02 = on
 */

philips_setflash(flash)
int flash;	/* flash setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x06;
	buf[1] = flash & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getrecordmode
 *
 *  Get the camera's current record mode setting.
 *
 */

philips_getrecordmode(mode)
int *mode;	/* record mode */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x07;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*mode = cam_data.data[2];

	return ( err );
}


/*  philips_setrecordmode
 *
 *  Set the current record mode setting.
 *
 *    00 = image
 *    01 = character
 *    02 = unknown
 *    03 = sound
 *    04 = image+sound
 *    05 = unknown
 *    06 = character+sound
 */

philips_setrecordmode(mode)
int mode;	/* mode setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x07;
	buf[1] = mode & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getcompression
 *
 *  Get the camera's current compression setting.
 *
 */

philips_getcompression(compression)
int *compression;	/* compression mode */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x08;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*compression = cam_data.data[2];

	return ( err );
}


/*  philips_setcompression
 *
 *  Set the compression setting.
 *
 *    00 = no compression
 *    01 = maximum
 *    02 = normal
 *    03 = mimimal
 */

philips_setcompression(mode)
int mode;	/* mode setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x08;
	buf[1] = mode & 0xff;
	buf[2] = mode ? 0x01 : 0x00;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 3, 0x00, &cam_data ) );
}


/*  philips_getresolution
 *
 *  Get the camera's current resolution setting.
 *
 */

philips_getresolution(resolution)
int *resolution;	/* resolution mode */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x09;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*resolution = cam_data.data[2];

	return ( err );
}


/*  philips_setresolution
 *
 *  Set the resolution setting.
 *
 *    00 = unknown
 *    01 = 640x480
 *    02 = unknown
 *    03 = unknown
 *    04 = 1280x960
 */

philips_setresolution(resolution)
int resolution;	/* resolution setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x09;
	buf[1] = resolution & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getcopyright
 *
 *  Get the camera's current copyright string.
 *
 */

philips_getcopyright(copyright)
char *copyright;	/* copyright string */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x0F;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		{
		memmove ( copyright, &(cam_data.data[2]), (cam_data.length - 2) );
		copyright[cam_data.length - 2] = 0;
		}

	return ( err );
}


/*  philips_setcopyright
 *
 *  Set the copyright string.
 *
 *  Does the length always have to be 20 characters? if so 
 *  we'll need to pad or truncate.
 */

philips_setcopyright(copyright)
char *copyright;	/* copyright setting */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[128];

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0x0F;
	sprintf ( &buf[1], "%-20.20s", copyright );
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 21, 0x00, &cam_data ) );
}


/*  philips_getmacro
 *
 *  Get the camera's current macro setting.
 *
 */

philips_getmacro(macro)
int *macro;	/* macro mode */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x16;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) 
		*macro = cam_data.data[2];

	return ( err );
}


/*  philips_setmacro
 *
 *  Set the macro mode.
 *
 *  0 = off
 *  1 = on
 */

philips_setmacro(macro)
int macro;	/* macro mode */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];

	if ( philips_mode != 1 ) philips_set_mode ( 1 );

	buf[0] = 0x16;
	buf[1] = macro & 0xff;
	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data ) );
}


/*  philips_getpictdate
 *
 *  Get the timestamp for a stored picture.
 */

philips_getpictdate(n, date)
int		n;       	/* picture number */
u_char	*date;		/* date picture was taken */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 3;
	buf[1] = n & 0xff;
	buf[2] = (n >> 8) & 0xff;

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETIMG, buf, 3, 0x00, &cam_data );

	if ( ! err ) 
		memmove(date, &(cam_data.data[3]), 6);

	return ( err );
}

/*  philips_getpict
 *
 *  Get a picture from the camera
 */

philips_getpict(n, image)
	int n;		/* picture number to download */
	char *image;	/* memory pointer to store image */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		totlen;
	int		flen = 0;
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = n & 0xff;
	buf[1] = (n >> 8) & 0xff;

	/* send picture number to retreive to start download */
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETPICT, buf, 2, 0x00, &cam_data );
	if ( err ) return (err);

	totlen = (cam_data.data[16] << 16) | (cam_data.data[15] << 8) | cam_data.data[14];
	while (flen < totlen) {
		err = philips_getpacket ( &cam_data );
		if ( err == 0 ) {
			memmove ( &image[flen], cam_data.data, cam_data.length );
			flen += cam_data.length;

			if (philips_verbose && (cam_data.blockno % philips_echobackrate == 0)) {
				fprintf(stderr, "got block %3d: %d/%d \r", cam_data.blockno, flen, totlen);
				}
			}
		else if ( err == -1 ) {
			fprintf ( stderr, "in philips_getpict, camera NAK'ed use, keep trying...\n" );
			}
		else {
			fprintf ( stderr, "in philips_getpict. error reading packet.... giving up.\n" );
			return ( err );
			}

		}
	if (philips_verbose) {
		fprintf(stderr, "got block %3d: %d/%d ...done%s\n",
			cam_data.blockno, flen, totlen, err ? " with err" : "");
	}

	return ( err );
}

/*  philips_putpict
 *
 *  Send a picture (or sound) to the camera.
 *
 *   TODO: figure out how to handle the picture/sound name.
 *
 *   For now, the program generates it's own, but should it
 *   be the callers responsibility to define this? At the very
 *   lest it needs to let us know if this is picture or sound.
 */

struct	PICINFO  {
	char	name[12];
	long	size;
	};

philips_putpict ( picData, size, picNum )
char	*picData;   /* pointer to the data to send */
long	size;       /* size of picture data */
int		*picNum;    /* send back the picture number that was used */
{
    struct  CAM_DATA    cam_data;
    u_char  buf[132];
	struct	PICINFO		picinfo;
    int     err = 0, i, ptr;

    if ( philips_mode != 0 ) philips_set_mode ( 0 );

	sprintf ( buf, "%8.8s.%3.3s", "RDX00000", "JPG" );
	memmove ( picinfo.name, buf, 12 );
	picinfo.size = size;

	cam_data.ack_only = 0;
    err = philips_execcmd ( P_PUTPICT, (char *)&picinfo, 16, 0x00, &cam_data );
	if ( err ) {
		dprintf ( ( stderr, "Failed to execute P_PUTPICT command with name %s\n", buf ) );
		return ( err );
		}
	*picNum = (cam_data.data[3] << 8) | (cam_data.data[2] & 0xff);

	cam_data.ack_only = 1;
	ptr = 0;
	i = 0;
	while ( ptr < picinfo.size ) {
		if ( (picinfo.size - ptr) > 128 )
			memcpy ( buf, (picData + ptr), 128 );
		else {
			memset ( buf, 0x00, 132 );
			memcpy ( buf, (picData + ptr), (picinfo.size - ptr) );
			}
	    err = philips_execcmd ( P_BLOCK, buf, 128, i, &cam_data );
		ptr += 128;
		i++;
		if ( err ) {
			dprintf ( (stderr, "Failed while sending block %d\n", i) );
			return ( err );
			}
		}

	/*
	 * I think the camera may be waiting for a done sending
	 * image block, maybe with an overall crc. But I'm not 
	 * sure what to send. Just sending a random command seems
	 * to work but the camera only sends back an ACK, no data
	 */

	/* Put the camera back to initial ready mode */
	buf[0] = 0x12;
	buf[1] = 0x00;

	cam_data.ack_only = 1;
	err = philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data );

	if ( !err )
		philips_mode = 0;

	return ( err );
}

/*  philips_getthumb
 *
 *  Get a thumbnail picture from the camera
 */

char *philips_getthumb(n, size)
	int		n;		/* picture number to download */
	int		*size;  /* size of thumbnail */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		totlen;
	int		flen = 0;
	int		err = 0;
	char	*image;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = n & 0xff;
	buf[1] = (n >> 8) & 0xff;

	/* send picture number to retreive to start download */
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETTHUMB, buf, 2, 0x00, &cam_data );
	if ( err ) {
		*size = err;
		return (NULL);
		}

	*size = (cam_data.data[17] << 24) | (cam_data.data[16] << 16) | (cam_data.data[15] << 8) | cam_data.data[14];

	if ( (image = (char *)malloc ( (long)*size )) == NULL ) {
		fprintf ( stderr, "unable to allocate memory for image (%d bytes)\n", *size );
		return (NULL);
		}

	while (flen < *size) {
		err = philips_getpacket ( &cam_data );
		if ( err == 0 ) {
			memmove ( &image[flen], cam_data.data, cam_data.length );
			flen += cam_data.length;
			if (philips_verbose && (cam_data.blockno % philips_echobackrate == 0)) {
				fprintf(stderr, "got block %3d: %d/%d \r", cam_data.blockno, flen, totlen);
				}
			}
		else if ( err == -1 ) {
			fprintf ( stderr, "in philips_getthumb. camera NAK'ed us, keep trying???\n" );
			}
		else {
			fprintf ( stderr, "in philips_getthumb. error reading thumbnail packet.... giving up.\n" );
			free ( image );
			return ( NULL );
			}
		}
	if (philips_verbose) {
		fprintf(stderr, "got block %3d: %d/%d ...done%s\n",
			cam_data.blockno, flen, totlen, err ? " with err" : "");
	}

	return ( image );
}


/*  philips_deletepict
 *
 *  Delete a single picture from the camera.
 */

philips_deletepict(n)
int		n;		/* picture number to delete */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[3];
	int		err = 0;

	/* put camera in delete mode */
	if ( philips_mode != 0 ) philips_set_mode ( 0 );
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_DELETE, NULL, 0, 0x00, &cam_data );

	/* find picture to delete */
	buf[0] = n & 0xff;
	buf[1] = (n >> 8) & 0xff;
	err = philips_execcmd ( P_SELECT, buf, 2, 0x00, &cam_data );

	/* send picture number to delete */
	buf[0] = n & 0xff;
	buf[1] = (n >> 8) & 0xff;
	err = philips_execcmd ( P_DELPICT, buf, 2, 0x00, &cam_data );
	return ( err );
}

/*  philips_get_mode
 *
 *  Ask the camera for its current mode.  The known
 *  modes are:
 *
 *        00 00 00 -  ready to send pictures
 *        00 00 01 -  ready to take a picture
 */

philips_get_mode()
{
	struct	CAM_DATA cam_data;
	int		err = 0;

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, "\x12", 1, 0x00, &cam_data );

	if ( err ) 
		return ( -1 );
	else 
		{
		philips_mode = cam_data.data[2];
		return ( philips_mode );
		}
}

/*  philips_set_mode
 *
 *  Put the camera in a particular mode.  The known
 *  modes are:
 *
 *        00 00 00 -  ready to send pictures
 *        00 00 01 -  ready to take a picture
 */

philips_set_mode(n)
int	n;
{
	struct	CAM_DATA cam_data;
	u_char	buf[5];
	int		err = 0;

	buf[0] = 0x12;
	buf[1] = n & 0xff;

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_SETVAL, buf, 2, 0x00, &cam_data );

	if ( !err )
		philips_mode = n;
}



/*  philips_init_query
 *
 *  Sends a query to the camera. The program supplied with
 *  the camera sends this, but it is unknown what it realy 
 *  does at this time.
 */

philips_init_query()
{
	struct	CAM_DATA	cam_data;

	cam_data.ack_only = 0;
	return ( philips_execcmd ( P_GETVAL, "\x00\x00", 2, 0x00, &cam_data ) );
}


/*  philips_getcamdate
 *
 *  Get the time/date from the camera
 */

philips_getcamdate(date)
time_t	*date;	/* camera real time date */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;
	struct	tm	time;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0xa;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETVAL, buf, 1, 0x00, &cam_data );

	if ( ! err ) {
		/* since the camera is unlikely to last > 90 years,
		 * the following y2k conversion should be OK */
		time.tm_year = ((cam_data.data[3] & 0xf0) >> 4) * 10 + (cam_data.data[3] & 0xf);
		if(time.tm_year < 90) time.tm_year += 100;
		time.tm_mon = ((cam_data.data[4] & 0xf0) >> 4) * 10 + (cam_data.data[4] & 0xf) - 1;
		time.tm_mday = ((cam_data.data[5] & 0xf0) >> 4) * 10 + (cam_data.data[5] & 0xf);
		time.tm_hour = ((cam_data.data[6] & 0xf0) >> 4) * 10 + (cam_data.data[6] & 0xf);
		time.tm_min = ((cam_data.data[7] & 0xf0) >> 4) * 10 + (cam_data.data[7] & 0xf);
		time.tm_sec = ((cam_data.data[8] & 0xf0) >> 4) * 10 + (cam_data.data[8] & 0xf);
		time.tm_isdst = -1;
		*date = mktime(&time);
		}

	return ( err );
}

/*  philips_setcamdate
 *
 *  Set the time/date on the camera
 */

#define HEX(x) (x / 10 << 4) + (x % 10)

philips_setcamdate(date)
time_t date;	/* camera real time date */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[10];
	int		err = 0;
	struct	tm	*time;
	int		temp;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0xa;
	time = localtime(&date);
	temp = time->tm_year/100 + 19;
	buf[1] = HEX(temp);
	temp = time->tm_year % 100;
	buf[2] = HEX(temp);
	buf[3] = HEX((time->tm_mon + 1));
	buf[4] = HEX(time->tm_mday);
	buf[5] = HEX(time->tm_hour);
	buf[6] = HEX(time->tm_min);
	buf[7] = HEX(time->tm_sec);
	dprintf((stderr, "set date = %02X %02X %02X %02X %02X %02X %02X\n",
	  buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]));

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_SETVAL, buf, 8, 0x00, &cam_data );

	return ( err );
}

/******   Get information about pictures in the camera's memory *****/


/*  philips_getmemo
 *
 *  Get the memo entry associated with a picture.
 */

philips_getmemo ( n, memo )
int     n;      /* picture number */
char    *memo;  /* string to hold memo entry (256 bytes) */
{
    struct  CAM_DATA    cam_data;
    u_char  buf[10];
    int     err = 0;

    if ( philips_mode != 0 ) philips_set_mode ( 0 );

    buf[0] = 0x02;
    buf[1] = n & 0xff;
    buf[2] = (n >> 8) & 0xff;

	cam_data.ack_only = 0;
    err = philips_execcmd ( P_GETIMG, buf, 3, 0x00, &cam_data );

    if ( err )
        return ( err );
    else {
        memmove ( memo, &(cam_data.data[10]), (cam_data.length - 10) );

        err = philips_getpacket ( &cam_data );
        if ( ! err ) {
            memmove ( &memo[118], cam_data.data, cam_data.length );
        	err = philips_getpacket ( &cam_data );
			}
		}
	return ( err );

}


/*  philips_setmemo
 *
 *  Set the memo entry associated with a picture.
 */

philips_setmemo ( n, memo )
int     n;      /* picture number */
char    *memo;  /* string to hold memo entry (256 bytes) */
{
    struct  CAM_DATA    cam_data;
    u_char  buf[132];
    int     err = 0, i, memo_len;

    if ( philips_mode != 0 ) philips_set_mode ( 0 );

	memset ( buf, 0x00, 132 );

    buf[0] = n & 0xff;
    buf[1] = (n >> 8) & 0xff;

	memo_len = strlen ( memo );

	if ( memo_len <= 118 ) {
		memcpy ( &buf[10], memo, memo_len );
		memo_len = 0;
		}
	else {
		memcpy ( &buf[10], memo, 118 );
		memo_len -= 118;
		if ( memo_len > 128 ) 
			memo_len = 128;
		}

	cam_data.ack_only = 1;
    err = philips_execcmd ( P_PUTIMG, buf, 128, 0x00, &cam_data );

	if ( ! err ) {
		memset ( buf, 0x00, 132 );
	
		if ( memo_len )
			memcpy ( buf, (memo + 118), memo_len );

    	err = philips_execcmd ( P_PUTIMG, buf, 128, 0x01, &cam_data );
		if ( ! err ) {
			/* Does this need a CRC????, of what, just the data? */
	/*
	crc = updcrc (cmd & 0xff, crc);
	crc = updcrc (len & 0xff, crc);
	for (i = 0; i < len; i++) {
		crc = updcrc (data[i] & 0xff, crc);
		}
	*/
			sprintf ( buf, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7F\x00\xC0\xDF\x7F\x00" );
			cam_data.ack_only = 0;
    		err = philips_execcmd ( P_PUTIMG, buf, 16, 0x02, &cam_data );
			}
			
		}
	return ( err );
}


/*  philips_pictname
 *
 *  Get the name of a picture in memory.
 */

philips_getpictname(n, name)
int		n;		/* picture number to access */
char	*name;	/* string to return the data */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0x00;
	buf[1] = n & 0xff;
	buf[2] = (n >> 8) & 0xff;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETIMG, buf, 3, 0x00, &cam_data );
	if ( ! err )
		strncpy ( name, &(cam_data.data[2]), 20 );

	return ( err );
}


/*  philips_pictmemo
 *
 *  Get the memo entry of a picture in memory.
 */

philips_getpictmemo(n, memo)
int		n;		/* picture number to access */
char	*memo;	/* string to return the data */
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;

	if ( philips_mode != 0 ) philips_set_mode ( 0 );

	buf[0] = 0x02;
	buf[1] = n & 0xff;
	buf[2] = (n >> 8) & 0xff;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_GETIMG, buf, 3, 0x00, &cam_data );
	if ( ! err )
		strncpy ( memo, &(cam_data.data[2]), (cam_data.length - 2) );

	return ( err );
}


/*  philips_hello
 *
 *  Try to initialize communications with the camera
 */

philips_hello(baudrate)
long	baudrate;
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	u_char	tmpstr[50];
	int		err = 0;

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_HELLO, "\x00\x00\x00", 3, 0x00, &cam_data );

	/*  We got a fatal error, should we retry at a faster baud? */
	if ( err == 1 ) {  
		dpprintf((stderr, "hello: No response, try %ld", baudrate));
		if (philips_setbaud(fd0, baudrate)) {
	   		fprintf(stderr, "can't set baudrate\n");
	    	return 1;
			}
		err = philips_execcmd ( P_HELLO, "\x00\x00\x00", 3, 0x00, &cam_data );
		}

	/* Non fatal error (camera NACK's us), try something different */
	if ( err == -1 ) {
		dpprintf((stderr, "hello: 31 00 00 00 -> NACK "));
		} 
	else if ( err == 0 ) {
		sprintf ( tmpstr, "%d%d%d%d%d%d", cam_data.data[0], cam_data.data[1], cam_data.data[2], cam_data.data[3], cam_data.data[4], cam_data.data[5] );
		err = atoi ( tmpstr );
	/*	dpprintf((stderr, "hello: 31 00 00 00 -> ")); */
		}

	return ( err );
}

/*  philips_bye
 *
 *  Shutdown the link to the camera.
 */

philips_bye()
{
	struct	CAM_DATA	cam_data;
	int		err = 0;

	cam_data.ack_only = 0;
	err = philips_execcmd ( P_DISCONECT, NULL, 0, 0x00, &cam_data );

	return ( err );
}

/*  philips_setspeed
 *
 *  Change the camera's baud rate and the computer's baud rate.
 */

philips_setspeed(baud)
int		baud;
{
	struct	CAM_DATA	cam_data;
	u_char	buf[5];
	int		err = 0;
	u_char	value;

#if HAVE_TERMIOS_H
	/*termios*/
	tcdrain(fd0);
#else
	/*sgtty*/
	ioctl(fd0, TCDRAIN, 0);
#endif

	switch (baud) {
	case -1:
	case 2400:
		value = 0x00;
		break;
	case 4800:
		value = 0x01;
		break;
	case 9600:
		value = 0x02;
		break;
	case 19200:
		value = 0x03;
		break;
	case 38400:
		value = 0x04;
		break;
	case 57600:
		value = 0x05;
		break;
	case 115200:
		value = 0x07;
		break;
	default:
		/* unsupported baud rate. */
		dprintf((stderr, "unsupported baudrate %d\n", baud));
		return 1;
	}

	/* set baudrate */
	buf[0] = value;
	cam_data.ack_only = 0;
	err = philips_execcmd ( P_SETBAUD, buf, 1, 0x00, &cam_data );

#if HAVE_TERMIOS_H
	/*termios*/
	tcdrain(fd0);
#else
	/*sgtty*/
	ioctl(fd0, TCDRAIN, 0);
#endif

	usleep(20*1000);

#if HAVE_TERMIOS_H
	/*termios*/
	tcdrain(fd0);
#else
	/*sgtty*/
	ioctl(fd0, TCDRAIN, 0);
#endif

	usleep(20*1000);

	if (baud == -1)
		err += philips_setbaud(fd0, 2400);
	else
		err += philips_setbaud(fd0, baud);

	usleep(1000*1000);

	return err ? 1 : 0;
}


/*  philips_execcmd
 *
 *  Send a command to the camera and return whatever data
 *  the camera sends back in a CAM_DATA structure.
 *
 *    Returns  0 if OK
 *            -1 if send of command header (with length 0x10) failed
 *            -2 if send of command header failed
 *            -3 if send of command data failed
 *            -4 if send of command CRC failed
 *             1 if something bad happened
 *
 */

philips_execcmd ( cmd, data, len, blkno, cam_data )
u_char	cmd;       /* command class */
u_char	*data;     /* specific command & data */
int		len;       /* length of command & data */
int		blkno;     /* block number */
struct	CAM_DATA	*cam_data;   /* data returned by camera */
{
	u_char	tbuf[9];
	u_short	crc = 0;
	size_t	i, x;
	int		err = 0;
	u_char	send_buff[260];

	philips_flush (); /* make sure camera is ready for command */
	/* generate crc sent at the end of the packet */
	crc = updcrc (cmd & 0xff, crc);
	crc = updcrc (len & 0xff, crc);

	/* send the command type packet header */
	tbuf[0] = 0x10;
	tbuf[1] = 0x02;
	tbuf[2] = cmd;
	tbuf[3] = len & 0xff;
	if ( len == 0x10 ) {
		tbuf[4] = len & 0xff;
		if ( philips_put (tbuf, 5, 0) ) return ( -1 );
		}
	else {
		if ( philips_put (tbuf, 4, 0) ) return ( -2 );
		}

	/* send the command */
	for (i = 0, x = 0; i < len; i++) {
		send_buff[x] = data[i];
		crc = updcrc (data[i] & 0xff, crc); /* calculate crc */
		/* 0x10 must be escaped */
		if (data[i] == 0x10) {
			x++;
			send_buff[x] = data[i];
			}
		x++;
		}
	err = philips_put ( &send_buff[0], x, 0 );
	if ( err ) return ( -3 );

	/* send the crc identifier */
	tbuf[0] = 0x10;
	if ( cam_data->ack_only ) 
		tbuf[1] = 0x17;
	else
		tbuf[1] = 0x03;

	tbuf[2] = crc & 0x00ff;   /* CRC value */
	tbuf[3] = (crc & 0xff00) >> 8; /* CRC value */

	tbuf[4] = len + 2;  /* length of data */
	tbuf[5] = blkno;    /* block number */
	if ( philips_put (tbuf, 6, 0) ) return ( -4 );

	if ( (err = philips_getpacket(cam_data)) ) {
		/* Something went wrong, return error */
		return ( err );
		}
	if ( (! cam_data->ack_only) && (cam_data->class != cmd) ) {
		fprintf ( stderr, "execcmd: returned command class (%x) != sent command class (%x), not good!\n", cam_data->class, cmd );
		fprintf ( stderr, "class = %x\n", cam_data->class );
		fprintf ( stderr, "length = %x\n", cam_data->length );
		fprintf ( stderr, "ack = %x\n", cam_data->ack );
		fprintf ( stderr, "blockno = %x\n", cam_data->blockno );
		fprintf ( stderr, "data = " );
		for ( i = 0; i < cam_data->length; i++ ) {
			fprintf ( stderr, "%02x ", cam_data->data[i] );
			}
		fprintf ( stderr, "\n" );
		return ( 1 );
		}
	
	/* look at what the camera return code and feed that back */
	err = (cam_data->data[1] << 8 | cam_data->data[0]) & 0xffff;
	return ( err );
}


/* philips_flush
 *
 * make sure camera is ready for next command packet
 * by waiting 100 ms after any last data sent.
 */

void philips_flush ()
{
#if HAVE_TERMIOS_H
	/*termios*/
	tcdrain(fd0);
#else
	/*sgtty*/
	ioctl(fd0, TCDRAIN, 0);
#endif
	usleep(100*1000);
}


/*  philips_getpacket
 *
 *  Get a data packet from the camera. Currently it
 *  expects to get three packets:
 *    1) Acknowledgement (10 06)
 *    2) Data (10 02)
 *    3) CRC (10 03)
 *
 *  will there ever be a time when these three packets don't
 *  arrive and it is not an error?  If so, this will need to
 *  be re-written.
 */

philips_getpacket ( data )
struct	CAM_DATA	*data;
{
	enum { PRESYNC, SYNC, SYNCUP, PACKET, CRC, SKIPCRC, DONE, NAK } state;
	int		badcrc = 0, i, len;
	u_char	tbuf[5];

	/* initialize */
	data->class   = 0;
	data->length  = 0;
	data->ack     = 0;
	data->blockno = 0;

	state = PRESYNC;

	while (1) {
		switch (state) {
			case PRESYNC:   /* first character.... */
				if (philips_get(tbuf, 1, 0))
					return (1);
				else if (tbuf[0] == 0x10)
					state = SYNC;
				else {
					dprintf((stderr, "philips_getpacket: Bad sync %02x - retrying\n", tbuf[0]));
					}
				break;

			case SYNC:   /* second character.... */
				if (philips_get(tbuf, 1, 1))
					return ( 1 );
				state = SYNCUP;
				break;

			case SYNCUP: /* check the second character.... */
				/* check packet type, and change state as required */
				switch(tbuf[0]) {
					case 0x06:		/* ACK */
			    		data->ack = 1;
						if ( data->ack_only )
							return ( 0 );  /* hack for multi packet sends */
						state = PRESYNC;
						break;
					case 0x15:		/* NACK */
						state = NAK;
						break;
					case 0x02:		/* data */
			   			state = PACKET;
			   			break;
					case 0x03:		/* crc */
					case 0x17:		/* image crc */
								/* Note the software supplied
								 * by Ricoh failed because it
								 * did not recognize the 0x17
								 * crc flag */
			  		 	if ( data->class )
							state = CRC;
			   		 	else
							state = SKIPCRC;
			   			break;
					case 0x10:		/* should not occur */
					    dprintf((stderr, "philips_getpacket: Dup sync char - ignoring\n"));
						/* keep state as is */
			 		   break;
					default:
					    dprintf((stderr, "philips_getpacket: Bad type %02x - restart.\n", tbuf[0]));
						state = PRESYNC;
						break;
					}
				break;

			case PACKET:    /* Get data from camera... */
				if (philips_get(&(data->class), 1, 2))
					return (1);
				if (philips_get(tbuf, 1, 3))
					return (1);

				len = tbuf[0] & 0xff;

				state = PRESYNC;
				i = 0;
				while (i < len) {
					if (philips_get(&(data->data[i]), 1, 4))
						return (1);
					if ( data->data[i] == 0x10 ) {
						/* is this a new packet or just an escaped 0x10 */
						if (philips_get(&(data->data[i]), 1, 5))
							return (1);
						if(data->data[i] != 0x10) {
							dprintf ( (stderr, "philips_getpacket: Got a %x character after an escape.\n", data->data[i]) );
							dprintf ( (stderr, "     After reading %d of %d bytes in the packet.\n", i, (len - 1)) );
							dprintf ( (stderr, "     Will a %x allways be escaped???\n", data->data[i]) );
					   		state = SYNCUP;
							break;
							}
						}
					i++;
					}
				break;

			case CRC:
				if (philips_get(tbuf, 2, 6))
					return (1);

		    	{
					u_short crc;
					int i;

					crc = 0;
					crc = updcrc(data->class & 0xff, crc);
					crc = updcrc(len & 0xff, crc);
					for (i = 0; i < len; i++) {
						crc = updcrc(data->data[i] & 0xff, crc);
						}

					if ((crc & 0xff) == tbuf[0] && ((crc >> 8) & 0xff) == tbuf[1])
						badcrc = 0;
					else
						{
						badcrc = 1;
						dprintf((stderr, "crc: %04x<->%02x%02x: %s\n", crc, tbuf[1], tbuf[0], badcrc ? "bad" : "good"));
						}

		    	}

				if (philips_get(tbuf, 2, 7))
					return (1);
				if (tbuf[0] != len + 2) {
					dprintf((stderr, "philips_getpacket: Bad crc length (%d %d)\n", tbuf[0], len + 2));
					badcrc++;
					}

				if ( badcrc ) {
					/* send NAK to camera */
					tbuf[0] = 0x10;
					tbuf[1] = 0x15;
					philips_put(tbuf, 2, 1);
					state = PRESYNC;
					}
				else {
					data->blockno = tbuf[1];
					state = DONE;
					}
				break;

			case SKIPCRC:
				if (philips_get(tbuf, 3, 8))
					return (1);
				data->blockno = tbuf[2];
				state = DONE;
				break;

			case DONE:   /* Got packet and CRC, now check */
				/* send ACK camera */
				tbuf[0] = 0x10;
				tbuf[1] = 0x06;
				philips_put(tbuf, 2, 1);
				data->length = len;

				if ( len == 3 && data->data[0] == 0x00 && data->data[1] == 0x04 && data->data[2] == 0xff ) {
					state = PRESYNC;
					data->class = 0;
					}
				else 
					return ( 0 );  /* Yeah, were done */
				break;

			case NAK:  /* camera didn't accept command, resend */
				return ( -1 );
				break;

			} /* end of switch */

		} /* end of while */
}


/* send bytes to the camera */
philips_put(buf, len, wait)
	u_char *buf;
	int len, wait;
{
	int wlen;
	int i;

	wlen = write(fd0, buf, len);
	if (wlen != len) {
	    dprintf((stderr, "failed in philips_put\n"));
	    return 1;
	}
	if ( wait ) {
		/* wait for data to reach the camera before returning */
#if HAVE_TERMIOS_H
	    /*termios*/
	    tcdrain(fd0);
#else
	    /*sgtty*/
	    ioctl(fd0, TCDRAIN, 0);
#endif
		}

	/* print stream debug info if dumpflag is set */
	if ( philips_dumpflag ) philips_dump_stream('<', buf, len);
	return 0;
}

/* get data from the philips receive buffer */
philips_get(buf, len, calledfrom)
	u_char *buf;
	int len, calledfrom;
{
	if ( philips_len < len ) {
		if ( philips_wait ( len ) ) {
			dprintf((stderr, "philips_get: Timed out (at %d)\n", calledfrom));
			if ( !philips_debugflag )
				fprintf(stderr, "philips_get: Camera not ready.\n");
			return 1;
			}
		}

	if (philips_len < len)
		abort();

	memcpy(buf, philips_buf, len);
	if (philips_len - len)
		memmove(philips_buf, philips_buf + len, philips_len - len);
	philips_len -= len;

	return 0;
}

/* If the philips receive buffer does not have enough data to satisfy the
 * length requested, this routine will get data from the camera
 */
philips_wait(rlen)
	int rlen;
{
	fd_set rdfd;
	int maxfd;
	int len;
	struct timeval t;

	while (philips_len < rlen) {
		/* obtain chars from the camera */
		FD_ZERO(&rdfd);
		FD_SET(fd0, &rdfd);
		maxfd = fd0;
		/* timeout: 4sec */
		t.tv_sec = 4;
		t.tv_usec = 0;
		switch (select(maxfd + 1, &rdfd, NULL, NULL, &t)) {
		case -1:
			if (errno == EINTR)
				break;
			perror("select");
			exit(1);
		case 0:
			dprintf((stderr, "read timeout.\n"));
			return 1;
		default:
			break;
		}

		if (FD_ISSET(fd0, &rdfd)) {
			len = read(fd0, philips_buf + philips_len,
				sizeof(philips_buf) - philips_len);
			philips_len += len;
		} else {
			dprintf((stderr, "something wrong in philips_get\n"));
			return 1;
		}
	}
	if ( philips_dumpflag ) philips_dump_stream('>', philips_buf, philips_len);

	return 0;
}

/*****  Utility commands   *******************************************/

/*  philips_setbaud
 *
 *  Set the tty port's baud rate
 *
 */
philips_setbaud(fd, baud)
	int fd;
	int baud;
{
#if HAVE_TERMIOS_H
	/* termios */
	struct termios tio;

	if (tcgetattr(fd, &tio) < 0) {
		perror("tcgetattr");
		return 1;
	}
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8 | CREAD | CLOCAL;
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 5;
	cfsetispeed(&tio, philips_baudconv(baud));
	cfsetospeed(&tio, philips_baudconv(baud));
	if (tcsetattr(fd, TCSANOW, &tio) < 0) {
		perror("tcsetattr");
		return 1;
	}
# else
	/* sgtty */
	struct sgttyb ttyb;

	if (ioctl(fd, TIOCGETP, &ttyb) < 0) {
		perror("ioctl(TIOCGETP)");
		return 1;
	}
	ttyb.sg_ispeed = baud;
	ttyb.sg_ospeed = baud;
	ttyb.sg_flags = 0;
	if (ioctl(fd, TIOCSETP, &ttyb) < 0) {
		perror("ioctl(TIOCSETP)");
		return 1;
	}
# endif
	dprintf((stderr, "baudrate set to %d\n", baud));
	return (0);
}

/*  philips_baudconv
 *
 *  Convert a baud rate number into a value that can be used
 *  in the set baud rate system call.
 */
speed_t
philips_baudconv(int baud)
{
#define	BAUDCASE(x)	case (x): { ret = B##x; break; }
	speed_t ret;

	ret = (speed_t) baud;
	switch (baud) {
	/* POSIX defined baudrates */
	BAUDCASE(0);	/*is it meaningful? */
	BAUDCASE(50);
	BAUDCASE(75);
	BAUDCASE(110);
	BAUDCASE(134);
	BAUDCASE(150);
	BAUDCASE(200);
	BAUDCASE(300);
	BAUDCASE(600);
	BAUDCASE(1200);
	BAUDCASE(1800);
	BAUDCASE(2400);
	BAUDCASE(4800);
	BAUDCASE(9600);
	BAUDCASE(19200);
	BAUDCASE(38400);

	/* non POSIX values */
#ifdef B7200
	BAUDCASE(7200);
#endif
#ifdef B14400
	BAUDCASE(14400);
#endif
#ifdef B28800
	BAUDCASE(28800);
#endif
#ifdef B57600
	BAUDCASE(57600);
#endif
#ifdef B115200
	BAUDCASE(115200);
#endif
#ifdef B230400
	BAUDCASE(230400);
#endif

	/* last resort */
	default:
		fprintf(stderr, "no defined value for baudrate %d found, "
			"use the value without conversion\n", baud);
	}

	return ret;
#undef BAUDCASE
}


/*  philips_close_handler
 *
 *  Shutdown the communication channel to the camera. This
 *  is typiclly down before exiting the program or possibly
 *  when changing modes.
 */

static void*
philips_close_handler(n)
int n;		/* signal number. should only be SIGALRM */
{
    struct timeval zero_time = {0, 0};
    struct timeval wait_disconnect = {10, 0};
    struct itimerval it_wait_disc = {zero_time, wait_disconnect};

    if (disconnecting)
		disconnecting = 0;
    else {
		philips_bye();
		close(fd0);
		setitimer(ITIMER_REAL, &it_wait_disc, (struct itimerval*)NULL);
		disconnecting = 1;
		camera_opened = 0;
    	}
}

/*  philips_model
 *
 *  return the camera model
 */

char *philips_model ( int id )
{
	static char errorstr[25];

	sprintf ( errorstr, "Unknown model %d", id );

	switch ( id ) {
		case 4200: return ( "Ricoh RCD-4200" );
		case 4300: return ( "Ricoh RCD-4300" );
		case 4000: return ( "Philips ESP80SXG" );
		case 3100: return ( "Ricoh RCD-300Z" );
		case 3000: return ( "Ricoh RCD-300" );
		default:   return ( errorstr );
		}
}


/*  philips_getcfginfo
 *
 *  Collect the camera's current configuration and return
 *  a pointer to the data.
 *
 *  Return codes: 
 *       0 = no failures
 *       x = failure code returned by camera for last command.
 *
 *  Assumes that camera has already been initialized.... probably
 *  not a good idea, but....
 */

PhilipsCfgInfo *philips_getcfginfo ( int *rtn )
{
	PhilipsCfgInfo	*cfginfo;

	if ( (cfginfo = (PhilipsCfgInfo *)malloc ( sizeof(PhilipsCfgInfo) )) == NULL ) {
		return ( NULL );
		}

	if ( (*rtn = philips_gettotalbytes ( &(cfginfo->memory) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getavailbytes ( &(cfginfo->a_memory) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getexposure ( &(cfginfo->exposure) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getwhitelevel ( &(cfginfo->white) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getzoom ( &(cfginfo->zoom) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getflash ( &(cfginfo->flash) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getrecordmode ( &(cfginfo->mode) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getcompression ( &(cfginfo->compression) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getresolution ( &(cfginfo->resolution) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getmacro ( &(cfginfo->macro) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getnpicts ( &(cfginfo->picts) )) == -1 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getcopyright ( cfginfo->copyright )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	if ( (*rtn = philips_getcamdate ( &(cfginfo->date) )) != 0 ) {
		free ( cfginfo );
		return ( NULL );
		}
	
	return ( cfginfo );
}



/*  philips_setcfginfo
 *
 *  Set the camera's current configuration.  Free the data 
 *  structure when done [is this a good idea?]
 *
 *  Return codes: 
 *       0 = no failures
 *       x = failure code returned by camera for last command.
 *
 *  Assumes that camera has already been initialized.... probably
 *  not a good idea, but....
 */

int philips_setcfginfo ( PhilipsCfgInfo *cfginfo )
{
	int		rtn;

	if ( (rtn = philips_setexposure ( cfginfo->exposure )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setwhitelevel ( cfginfo->white )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setzoom ( cfginfo->zoom )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setflash ( cfginfo->flash )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setrecordmode ( cfginfo->mode )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setcompression ( cfginfo->compression )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setresolution ( cfginfo->resolution )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setmacro ( cfginfo->macro )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setcopyright ( cfginfo->copyright )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	if ( (rtn = philips_setcamdate ( cfginfo->date )) != 0 ) {
		free ( cfginfo );
		return ( rtn );
		}
	
	free ( cfginfo );
	return ( 0 );
}

