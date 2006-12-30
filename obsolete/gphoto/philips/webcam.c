/*  $Id$ */

/* 
 * Philips Digital Camera Sample Code
 *
 * Copyright (c) 1999 Bob Paauwe
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

/******************************************************************
 *
 *  Sample program to turn the camera into a simple WebCam. 
 *
 *  This porgram will take a picture, save it, and then 
 *  delete it from the camera. It takes the picture using
 *  the lowest resolution and largest amount of compression
 *  to keep the image size small.  Even so, it takes about
 *  52 seconds for the Philips ESP80 camera to take a picture,
 *  save it, and delete it.
 *
 *  It takes the following arguments:
 *
 *     Filename:  File name to save image as
 *     Flash:     On, Off, Auto
 *     Exposure:  Auto, -2.0 to +2.0
 *     Delay:     Number of seconds to delay before taking picture
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "philips.h"

extern int	philips_debugflag;
extern int	philips_dumpflag;

int exposurevalue ( float e );

main( argc, argv )
int		argc;
char	*argv[];
{
    int		baudrate = 115200;
    int		size, rtn, i;
	long	cameraID, picture_number;
	char	*picData;
	FILE	*fp;

	char	serial_port[128];
	char	filename[2048];
	char	name[20];
	int		flash = 0;         /* automatic */
	int		exposure = 0xff;   /* automatic */
	int		white = 0;         /* automatic */
	int		delay = 0;

	strcpy ( serial_port, "/dev/ttyS0" );
	strcpy ( filename, "webcam.jpg" );

	philips_debugflag = 0;
	philips_dumpflag = 0;

	for ( i = 1; i < argc; i++ ) {
		if ( argv[i][0] == '-' ) {
			switch ( argv[i][1] ) {
				case 'p':
				case 'P':
					i++;
					strcpy ( filename, argv[i] );
					break;
				case 'f':
				case 'F':
					i++;
					if ( strcasecmp ( argv[i], "on" ) == 0 )
						flash = 2;
					else if ( strcasecmp ( argv[i], "off" ) == 0 )
						flash = 1;
					else
						flash = 0;
					break;
				case 'w':
				case 'W':
					i++;
					if ( strncasecmp ( argv[i], "o", 1 ) == 0 )
						white = 1;
					else if ( strncasecmp ( argv[i], "f", 1 ) == 0 )
						white = 2;
					else if ( strncasecmp ( argv[i], "i", 1 ) == 0 )
						white = 3;
					else
						white = 0;
					break;
				case 'e':
				case 'E':
					i++;
					exposure = exposurevalue ( atof ( argv[i] ) );
					break;
				case 'd':
				case 'D':
					i++;
					delay = atoi ( argv[i] );
					break;
				case 't':
				case 'T':
					i++;
					strcpy ( serial_port, argv[i] );
					break;
				default:
					Usage ( argv[0] );
					break;
				}
			}
		else
			Usage ( argv[0] );
		}

					
	printf ( "\nWebCam for Philips Digital Cameras\n\n" );

	sleep ( delay );

    philips_open ( serial_port, baudrate, &cameraID );
	philips_set_mode ( 1 ); /* put camera in picture take mode */
	rtn = philips_setcompression(1);   /* Economy */
	rtn = philips_setresolution(1);    /* 640 x 480 */
	rtn = philips_setflash(flash);
	rtn = philips_setexposure(exposure);
	rtn = philips_setwhitelevel(white);
	rtn = philips_takepicture();
	rtn = philips_getpictnum ( &picture_number );
	rtn = philips_getpictsize ( picture_number, &size );
	if ( ! rtn ) {
		picData = (char *)malloc ( size );
		rtn = philips_getpict ( picture_number, picData, name );

		if ( ! rtn ) {
			if ( (fp = fopen ( filename, "w" )) != NULL ) {
				fwrite ( picData, size, 1, fp );
				fclose ( fp );
				}
			else
				fprintf ( stderr, "%s: Can't write %s.\n", argv[0], filename );
			}
		}
	
	rtn = philips_deletepict ( picture_number );
    philips_bye();
    sleep(2);
}



Usage ( char *progname ) {
	
	fprintf ( stderr, "Usage: %s [-tty ttyport] [-d delay] [-p picturefile]\n", progname );
	fprintf ( stderr, "       [-flash on|off|auto] [-exposure -2.0...+2.0]\n" );
	fprintf ( stderr, "       [-white outdoor|fluorescent|incadescent]\n" );
	exit (1);
}


int 	exposurevalue ( float e ) {
	
	if ( e == -2.0 ) return ( 1 );
	if ( e == -1.5 ) return ( 2 );
	if ( e == -1.0 ) return ( 3 );
	if ( e == -0.5 ) return ( 4 );
	if ( e ==  0.0 ) return ( 5 );
	if ( e ==  0.5 ) return ( 6 );
	if ( e ==  1.0 ) return ( 7 );
	if ( e ==  1.5 ) return ( 8 );
	if ( e ==  2.0 ) return ( 9 );
	return ( 0xff );
}
