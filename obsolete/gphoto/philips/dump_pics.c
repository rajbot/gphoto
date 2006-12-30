/*  $Id$ */

/* 
 * Philips Digital Camera Sample Code
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * This program will get picture information from the camera and
 * display it. It will also get picture data and save it to files.
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "philips.h"

extern int	philips_debugflag;
extern int	philips_dumpflag;
char *test_getpicture();
int		rtn;

main( argc, argv )
int		argc;
char	*argv[];
{
    int		baudrate = 115200;
    long	camera_picts = 0;
    int		size, i, thumbSize;
    unsigned char date[6];
	char	tmpstr[128];
	char	memo[257];
	long	cameraID;
	char	*picData, *thumbData;
	PhilipsCfgInfo	*cfginfo;

	philips_debugflag = 1;
	philips_dumpflag = 0;

	if ( argc < 2 ) {
		printf ( "Usage: %s </dev/ttyxx>\n", argv[0] );
		exit (1);
		}

	printf ( "\nDumping Pictures....\n\n" );

    philips_open ( argv[1], baudrate, &cameraID );
	printf ( "Camera ID: %ld\n", cameraID );
	philips_set_mode ( 0 ); /* put camera in picture view mode */
	philips_getnpicts ( &camera_picts );

	for ( i = 1; i <= camera_picts; i++ ) {
		philips_getpictname ( i, tmpstr );
		philips_getpictsize ( i, &size );
		printf ( "Picture #%d %s was taken on ", i, tmpstr );
		philips_getpictdate ( i, date );
		printf ( "%02x/%02x/19%02x %02x:%02x:%02x and is ", date[1], date[2], date[0], date[3], date[4], date[5], date[6]);
		printf ( "%d bytes\n", size );
		philips_getmemo ( i, memo );
		printf ( "MEMO: [%s]\n", memo );
/*		thumbData = philips_getthumb ( i, &thumbSize ); */
		picData = test_getpicture ( i, &size );
		if ( picData ) {
			free ( picData );
			}
		}

		
	printf ( "Disconnecting from camera....\n" );
    philips_bye();
    sleep(6);
}


char *test_getpicture ( n, size )
int		n;
int		*size;
{
	int		rtn;
	char	*picData;
	FILE	*fp;

	picData = NULL;

	printf ( "  Getting picture #%d...", n );
	rtn = philips_getpictsize ( n, size );
	if ( ! rtn ) {
		picData = (char *)malloc ( *size );
		rtn = philips_getpict ( n, picData );

/*  TODO  Use the camera's file name & date for the picture filename */
		if ( ! rtn ) {
			fp = fopen ( "junk.jpg", "w" );
			fwrite ( picData, *size, 1, fp );
			fclose ( fp );
			}
		}
	printf ( "%d bytes, (%d)\n", *size, rtn );
	return ( picData );
}
