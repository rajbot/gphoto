/*  $Id$ */

/* 
 * Philips Digital Camera Sample Code
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * This program tests the uploading of pictures to the camera.
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
    int		size, i;
    unsigned char date[6];
	char	tmpstr[128];
	char	memo[257];
	long	cameraID;
	char	*picData;
	PhilipsCfgInfo	*cfginfo;
	FILE	*fp;

	philips_debugflag = 1;
	philips_dumpflag = 1;

	if ( argc < 2 ) {
		printf ( "Usage: %s </dev/ttyxx>\n", argv[0] );
		exit (1);
		}

	printf ( "\nPicture Upload Test program for Ricoh/Philips Digital Cameras\n\n" );
	size = 172032;
	picData = (char *)malloc ( size );
	fp = fopen ( "junk.jpg", "r" );
	fread ( picData, size, 1, fp );
	fclose ( fp );

    philips_open ( argv[1], baudrate, &cameraID );
	philips_set_mode ( 0 ); /* put camera in picture view mode */

	if ( picData ) {
		test_putpicture ( picData, size );
		free ( picData );
		}

	philips_set_mode ( 0 ); /* put camera in picture view mode */
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

		if ( ! rtn ) {
			fp = fopen ( "junk.jpg", "w" );
			fwrite ( picData, *size, 1, fp );
			fclose ( fp );
			}
		}
	printf ( "%d bytes, (%d)\n", *size, rtn );
	return ( picData );
}

test_putpicture ( picData, size )
char	*picData;
int		size;
{
	int		rtn, num;

	printf ( "  Sending picture to camera..." );
	rtn = philips_putpict ( picData, size, &num );
	printf ( "as picture #%d (%d)\n", num, rtn );
}
