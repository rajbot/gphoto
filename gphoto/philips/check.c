
/*  $Id$ */

/* 
 * Philips Digital Camera Sample Code
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * This program will test most of the camera functions.
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

	philips_debugflag = 0;
	philips_dumpflag = 0;

	if ( argc < 2 ) {
		printf ( "Usage: %s </dev/ttyxx>\n", argv[0] );
		exit (1);
		}

	printf ( "\nTest program for Philips Digital Cameras\n\n" );

    philips_open ( argv[1], baudrate, &cameraID );
	printf ( "Camera ID: %ld\n", cameraID );
	philips_set_mode ( 0 ); /* put camera in picture view mode */
	philips_gettotalbytes(&size);
	printf ( "Camera has %d bytes of memory\n", size );
	philips_getavailbytes(&size);
	printf ( "Camera has %d bytes of memory available\n", size );

	picData = test_getpicture ( 1, &size );
	if ( picData ) {
		test_putpicture ( picData, size );
		free ( picData );
		}

	printf ( "Set memo on picture #1 to \"Lucy, I'm home\"\n" );
	philips_setmemo ( 1, "Lucy, I'm home." );
		philips_getmemo ( 1, memo );
		printf ( "MEMO: [%s]\n", memo );


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
		}

		
	philips_set_mode ( 1 ); /* put camera in picture take mode */
	exposure_test ();
	white_test ();
	zoom_test ();
	flash_test ();
	record_mode_test ();
	compression_test ();
	resolution_test ();
	macro_test ();
	copyright_test ();
	date_test ();

	/* try getting all the configuration information.... */
	printf ( "Reading all configuration information from camera..." );
	if ( (cfginfo = philips_getcfginfo ( &rtn )) != NULL ) {
		printf ( "Ok\n" );
		printf ( "memory: %d/%d\n", cfginfo->a_memory, cfginfo->memory );
		printf ( "zoom: %d\n", cfginfo->zoom );
		free ( cfginfo );
		}
	else
		printf ( "failed (%d)\n", rtn );


	printf ( "Disconnecting from camera....\n" );
    philips_bye();
    sleep(6);
}

date_test ()
{
    time_t camtime;
    char *atime;

    rtn = philips_getcamdate(&camtime);
    atime = ctime(&camtime);
    printf("Camera time test [%s] (%d)\n",atime, rtn);
}

copyright_test ()
{
	char	tmpstr[50];
	char	origstr[50];

	rtn = philips_getcopyright(origstr);
	printf ( "Camera copyright test [%s] (%d)\n", origstr, rtn );

	printf ( "  Setting copyright text to \"Copyright Test\"..." );
	strcpy ( tmpstr, "Copyright Test" );
	rtn = philips_setcopyright(tmpstr);
	philips_getcopyright(tmpstr);
	printf ( "set to %s (%d)\n", tmpstr, rtn );
	sleep(1);

	printf ( "  Setting copyright text to \"%s\"...", origstr );
	rtn = philips_setcopyright(origstr);
	philips_getcopyright(tmpstr);
	printf ( "set to %s (%d)\n", tmpstr, rtn );
	sleep(1);
}

macro_test ()
{
	int	size;

	rtn = philips_getmacro(&size);
	printf ( "Camera macro test [%x] (%d)\n", size, rtn );

	printf ( "  Setting macro mode to 1 (on)..." );
	rtn = philips_setmacro(1);
	philips_getmacro(&size);
	printf ( "set to %x (%d)\n", size, rtn );
	sleep(6);

	printf ( "  Setting macro mode to 0 (off)..." );
	rtn = philips_setmacro(0);
	philips_getmacro(&size);
	printf ( "set to %x (%d)\n", size, rtn );
	sleep(6);
}

compression_test ()
{
	int	size;

	rtn = philips_getcompression(&size);
	printf ( "Camera compression test [%x] (%d)\n", size, rtn );

	printf ( "  Setting compression to 0 (none)..." );
	rtn = philips_setcompression(0);
	philips_getcompression(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep(3);

	printf ( "  Setting compression to 1 (economy)..." );
	rtn = philips_setcompression(1);
	philips_getcompression(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep(3);

	printf ( "  Setting compression to 2 (normal)..." );
	rtn = philips_setcompression(2);
	philips_getcompression(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep(3);

	printf ( "  Setting compression to 3 (???)..." );
	rtn = philips_setcompression(3);
	philips_getcompression(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep(3);

	printf ( "  Setting compression to 4 (fine)..." );
	rtn = philips_setcompression(4);
	philips_getcompression(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep(3);

}

resolution_test ()
{
	int	size;

	rtn = philips_getresolution(&size);
	printf ( "Camera resolution test [%x] (%d)\n", size, rtn );

	printf ( "  Setting resolution to 0 (???)..." );
	rtn = philips_setresolution(0);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);

	printf ( "  Setting resolution to 1 (640 x 480)..." );
	rtn = philips_setresolution(1);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);

	printf ( "  Setting resolution to 2 (???)..." );
	rtn = philips_setresolution(2);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);

	printf ( "  Setting resolution to 3 (???)..." );
	rtn = philips_setresolution(3);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);

	printf ( "  Setting resolution to 4 (1280 x 960)..." );
	rtn = philips_setresolution(4);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);

	printf ( "  Setting resolution to 5 (900 x 600)..." );
	rtn = philips_setresolution(5);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);

	printf ( "  Setting resolution to 6 (1800 x 1200)..." );
	rtn = philips_setresolution(6);
	philips_getresolution(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);
}


record_mode_test ()
{
	int	size;

	rtn = philips_getrecordmode(&size);
	printf ( "Camera recording test [%x] (%d)\n", size, rtn );

	printf ( "  Setting record mode to 1 (character)..." );
	rtn = philips_setrecordmode(1);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting record mode to 2 (multi-shot [RDC-5000 only])..." );
	rtn = philips_setrecordmode(2);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting record mode to 3 (sound)..." );
	rtn = philips_setrecordmode(3);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting record mode to 4 (image+sound)..." );
	rtn = philips_setrecordmode(4);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting record mode to 5 (???)..." );
	rtn = philips_setrecordmode(5);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting record mode to 6 (character+sound)..." );
	rtn = philips_setrecordmode(6);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting record mode to 0 (image)..." );
	rtn = philips_setrecordmode(0);
	philips_getrecordmode(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);
}

flash_test ()
{
	int	size;

	rtn = philips_getflash(&size);
	printf ( "Camera flash test [%x] (%d)\n", size, rtn );

	printf ( "  Setting flash to 1 (off)..." );
	rtn = philips_setflash(1);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting flash to 2 (on or synchro/RDC-5000)..." );
	rtn = philips_setflash(2);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting flash to 3 (on [RDC-5000])..." );
	rtn = philips_setflash(3);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting flash to 4 (auto w/ redeye [RDC-5000])..." );
	rtn = philips_setflash(4);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting flash to 5 (synchro w/ redeye [RDC-5000])..." );
	rtn = philips_setflash(5);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting flash to 6 (on w/ redeye [RDC-5000])..." );
	rtn = philips_setflash(6);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);

	printf ( "  Setting flash to 0 (auto)..." );
	rtn = philips_setflash(0);
	philips_getflash(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (2);
}


zoom_test ()
{
	int	size;

	rtn = philips_getzoom(&size);
	printf ( "Camera zoom level test [%x] (%d)\n", size, rtn );

	printf ( "  Setting zoom to 1..." );
	rtn = philips_setzoom(1);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 2..." );
	rtn = philips_setzoom(2);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 3..." );
	rtn = philips_setzoom(3);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 4..." );
	rtn = philips_setzoom(4);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 5..." );
	rtn = philips_setzoom(5);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 6..." );
	rtn = philips_setzoom(6);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 7..." );
	rtn = philips_setzoom(7);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 8..." );
	rtn = philips_setzoom(8);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting zoom to 0..." );
	rtn = philips_setzoom(0);
	philips_getzoom(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (3);
}


white_test ()
{
	int size;

	rtn = philips_getwhitelevel(&size);
	printf ( "Camera white level test [%x] (%d)\n", size, rtn );
	
	printf ( "  Setting white level to 1 (outdoor)..." );
	rtn = philips_setwhitelevel(1);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
	
	printf ( "  Setting white level to 2 (fluorescent)..." );
	rtn = philips_setwhitelevel(2);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
	
	printf ( "  Setting white level to 3 (incadescent)..." );
	rtn = philips_setwhitelevel(3);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
	
	printf ( "  Setting white level to 4 (B&W - Ricoh 4200/5000 only)..." );
	rtn = philips_setwhitelevel(4);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
	
	printf ( "  Setting white level to 5 (Sepia - Ricoh 4200/5000 only)..." );
	rtn = philips_setwhitelevel(5);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
	
	printf ( "  Setting white level to 6 (Overcast - Ricoh 5000 only)..." );
	rtn = philips_setwhitelevel(6);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting white level to 0 (automatic)..." );
	rtn = philips_setwhitelevel(0);
	philips_getwhitelevel(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
}

exposure_test ()
{
	int size;

	rtn = philips_getexposure(&size);
	printf ( "Camera exposure test [%x] (%d)\n", size, rtn );

	printf ( "  Setting exposure to 0 (bad setting)..." );
	rtn = philips_setexposure(0);
	philips_getexposure(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting exposure to 2 (-1.5)..." );
	rtn = philips_setexposure(2);
	philips_getexposure(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting exposure to 4 (-0.5)..." );
	rtn = philips_setexposure(4);
	philips_getexposure(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting exposure to 6 (+0.5)..." );
	rtn = philips_setexposure(6);
	philips_getexposure(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);

	printf ( "  Setting exposure to 8 (+1.5)..." );
	rtn = philips_setexposure(8);
	philips_getexposure(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
	
	printf ( "  Setting exposure to ff (auto)..." );
	rtn = philips_setexposure(0xff);
	philips_getexposure(&size);
	printf ( "set to %x, (%d)\n", size, rtn );
	sleep (1);
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
