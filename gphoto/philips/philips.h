/*  $Id$ */

/* 
 * Philips Digital Camera interface library
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * This is the function prototypes for all the functions in the
 * Philips Digital Camera interface library. This can be included in
 * any code that wishes to use the library.
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

#ifndef PHILIPS_H
#define PHILIPS_H

/* Camera Return Codes */

#define P_OK   "\x00\x00"       /* command completed OK */
#define P_BUSY "\x00\x04\xff"   /* camera busy, please wait */
#define P_INC  "\x03\x00"       /* incomplete command */
#define P_INV  "\x04\x00"       /* invalid parameter */
#define P_ERR  "\x05\x00"       /* Error ???? */
#define P_MODE "\x06\x00"       /* command not valid in this mode */
#define P_ERR1 "\x07\x00"       /* don't know what yet */
#define P_ERR2 "\x18\x00"       /* don't know what yet */

typedef struct	PHILIPS_CFG {
	long	picts;		/* number of pictures in camera memory */
	int		a_memory;	/* available camera memory */
	int		memory;		/* total camera memory */
	time_t	date;		/* camera date & time */
	char	copyright[21];	/* picture copyright string */
	int		resolution;	/* camera resolution setting */
	int		compression;	/* camera compression setting */
	int		white;		/* camera white balance setting */
	int		exposure;	/* camera exposure setting */
	int		mode;		/* camera recording mode */
	int		flash;		/* camera flash mode */
	int		macro;		/* camera macro mode */
	int		zoom;		/* camera zoom setting */
	} PhilipsCfgInfo;

static char *philips_description_string = "Philips ESP80 gPhoto Library\nBob Paauwe <bpaauwe@bobsplace.com>\nhttp://www.bobsplace.com/philips/\nMay work with Ricoh cameras such as the 4300 and 4200.\nKnown Issues:\n  1) The configuration dialog box crashes gPhoto when closed.\n  2) Thumbnails are only displayed in black & white.\n  3) The code that interfaces to gPhoto needs some work.\n";

char *philips_model ( int camera_identifier );
int philips_open ( char *serial_port, int baudrate, long *camera_identifier );
int philips_close ();
int philips_get_mode ();
int philips_gettotalbytes ( int *totalbytes );
int philips_getavailbytes ( int *availbytes );
char *philips_getthumb ( int picture_number, int *sizeofthumb );
int philips_getexposure ( int *exposure_setting );
int philips_getwhitelevel ( int *white_level );
int philips_getzoom ( int *zoom_level );
int philips_getflash ( int *flash_setting );
int philips_getrecordmode ( int *record_mode );
int philips_getcompression ( int *compression_setting );
int philips_getresolution ( int *resolution_setting );
int philips_getcopyright ( char *copyright_string );
int philips_getmacro ( int *macro_setting );
int philips_getnpicts ( long *number_of_pictures_in_camera );
int philips_getpictnum ( long *current_picture_number );
int philips_getpictname ( int picture_number, char *picture_name );
int philips_getpictmemo ( int picture_number, char *picture_memo );
int philips_getpictsize ( int picture_number, int *picture_size );
//int philips_getpictdate ( int picture_number, u_char *picture_timestamp);
int philips_getcamdate ( time_t *date ); 
int philips_getpict ( int picture_number, char *picture_data );

int philips_set_mode ( int camera_mode );
int philips_setexposure ( int exposure_setting );
int philips_setwhitelevel ( int white_level );
int philips_setzoom ( int zoom_level );
int philips_setflash ( int flash_setting );
int philips_setrecordmode ( int recording_mode );
int philips_setcompression ( int compression_setting );
int philips_setresolution ( int resolution_setting );
int philips_setcopyright ( char *copyright_string );
int philips_setmacro ( int macro_setting );
int philips_setcamdate ( time_t date ); 
int philips_putpict ( char *picture_data, long picture_size, int *picture_number );
int philips_takepicture ();
int philips_deletepict ( int picture_number );
int philips_bye ();
PhilipsCfgInfo *philips_getcfginfo ( int *err );
int philips_setcfginfo ( PhilipsCfgInfo *cfginfo );

#endif /* PHILIPS_H */
