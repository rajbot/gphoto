/*  $Id$ */

/* 
 * Philips Digital Camera gPhoto interface 
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * This code glues the Philips Digital Camera Library to the gPhoto
 * digital camera program. 
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
#include <time.h>
#include <jpeglib.h>
#include "../src/gphoto.h"
#include "../src/util.h"

/* prototypes for io library calls */
#include "philips.h"


extern char *Philips_models[];
long cameraid; /* this should be global or returned */
char	*philips_processThumb ( char *thumbdata, int *Size );

extern philips_configure ();


/* #include <gif_lib.h> */

/*  philips_open_camera
 *
 *  Make a connection to the camera and set up the serial 
 *  communications link.
 */

int philips_open_camera () {
	
	if ( philips_open(serial_port, 115200, &cameraid) == -1 )
		return (0);
	return (1);
}

/*  philips_close_camera
 *
 *  Shut down the connection to the camera.
 */

void philips_close_camera() {

	philips_close();
}

/*  philips_number_of_pictures
 *
 *  Get the number of pictures currently stored in
 *  the camera's memory.
 */

int philips_number_of_pictures () {

	long num_pictures_taken = 0;

	if ( philips_open_camera() == 0 ) {
		error_dialog("Could not open camera.");
		return (0);
        }

	if ( philips_getnpicts ( &num_pictures_taken ) == -1)
	    num_pictures_taken = 0;

	philips_close_camera();

	return ((int)num_pictures_taken);
}

/*  philips_take_picture
 *
 *  Take a picture using the current camera default settings.
 */

int philips_take_picture () {

	long picture_number = 0;

	if ( philips_open_camera() == 0 ) {
        error_dialog("Could not open camera.");
        return (0);
        }

	if ( philips_takepicture() == 0 ) {
		philips_getpictnum ( &picture_number );
		}

	philips_close_camera();

	return ((int)picture_number);
}

/*  philips_get_picture
 *
 *  Read a picture or thumbnail from the camera's memory
 *  and return an Image structure filled in with the data.
 */

struct Image *philips_get_picture (int picNum, int thumbnail) {

	int 	Size;
	char	*picData, *thumbData, header[14];
	struct	Image	*image;


	if ( picNum == 0 ) /* no such picture, abort... */
		return ( NULL );

	if (philips_open_camera() == 0) {
		error_dialog("Could not open camera.");
		return ( NULL );
		}

	if ( (image = (struct Image *)malloc ( sizeof(struct Image) )) == NULL ) {
		error_dialog("Could not allocate memory for image structure.");
		return ( NULL );
		}

	if ( thumbnail ) {
        picData = philips_getthumb ( picNum, &Size );

		if ( ! picData ) {
			error_dialog ( "Could not read thumbnail." );
			return ( NULL );
			}
		
		image->image_size = Size;
		image->image_info = NULL;
		image->image_info_size = 0;

		if ( cameraid != 5000 ) { /* thumbnail format unknown, guess */
			thumbData = philips_processThumb ( picData, &Size );
			free ( picData );

			image->image = thumbData;
			strcpy ( image->image_type, "pgm" );
			}
		else {   /* RDC-5000 uses JPEG thumbnails */
			image->image = picData;
			strcpy ( image->image_type, "jpg" );
			}
		}
	
	else { /* Not a thumbnail */
		philips_getpictsize ( picNum, &Size );
        image->image = (char *)malloc ( Size );
		image->image_size = Size;
		image->image_info_size = 0;
		strcpy ( image->image_type, "jpg" );
	    philips_getpict ( picNum, (char *)image->image );
		}

	philips_close_camera();
	return ( image );
}



/*  philips_get_preview
 *
 *  Show a live image. Well, since the Philips camera doesn't
 *  have a mode to show live images, this simply takes a picture
 *  at the lowest resolution/maximum compression setting, downloads
 *  it, and then deletes it from the camera. It takes a fairly
 *  long time to get the image so it isn't 'real-time'.
 */

struct Image *philips_get_preview () {

	long	picNum = 0;
	int		Size;
	struct	Image	*image;

	if ( philips_open_camera() == 0 ) {
        error_dialog("Could not open camera.");
        return ( NULL );
        }

	if ( (image = (struct Image *)malloc ( sizeof(struct Image) )) == NULL ) {
		error_dialog("Could not allocate memory for image structure.");
		return ( NULL );
		}

	philips_set_mode ( 1 );

	/* put camera in lowest resolution mode */
	philips_setresolution ( 1 );
	philips_setcompression ( 1 );

	if ( philips_takepicture() == 0 ) {
		philips_set_mode (0);
		sleep (1);
		philips_getpictnum ( &picNum );
		}

	philips_getpictsize(picNum, &Size);
    image->image = (void *)malloc ( Size );
	image->image_size = Size;
	strcpy ( image->image_type, "jpg" );
	philips_getpict ( picNum, (char *)image->image );
	philips_deletepict(picNum);
	philips_close_camera();
	return ( image );
}


/*  philips_delete_picture
 * 
 *  Delete a single picture from the camera's memory.
 */

int philips_delete_picture (int picNum) {

	if (philips_open_camera() == 0) {
        error_dialog("Could not open camera.");
        return (0);
        }
	philips_deletepict(picNum);
	philips_close_camera();

	return (1);
}

int philips_initialize ()
{
	return ( 1 );
}


static char	philips_summary_string[1024];

char *philips_summary ()
{
    PhilipsCfgInfo  *pcfginfo;
    int         error;
    char        tmp[128];

    /* initialize camera and grab configuration information */

    if (philips_open_camera() == 0) { 
        error_dialog ( "Could not open camera." );
        return ( NULL );
        }


    if ( (pcfginfo = philips_getcfginfo ( &error )) == NULL ) {
        error_dialog ( "Can't get camera configuration." );
        philips_close_camera();
        return ( NULL );
        }
    philips_close_camera();
	sprintf ( philips_summary_string, "%s\n\n", philips_model(cameraid) );
	sprintf ( tmp, "Number of pictures: %ld\n", pcfginfo->picts );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Memory     : %d/%d bytes\n", pcfginfo->a_memory, pcfginfo->memory );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Copyright String  : %s\n", pcfginfo->copyright );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Resolution : %d\n", pcfginfo->resolution );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Compression: %d\n", pcfginfo->compression );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera White level: %d\n", pcfginfo->white );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Exposure   : %d\n", pcfginfo->exposure );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Record Mode: %d\n", pcfginfo->mode );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Flash Mode : %d\n", pcfginfo->flash );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Macro      : %d\n", pcfginfo->macro );
	strcat ( philips_summary_string, tmp );
	sprintf ( tmp, "Camera Zoom Level : %d\n", pcfginfo->zoom );
	strcat ( philips_summary_string, tmp );

	return ( philips_summary_string );
}




/*
 *   ProcessThumb
 *
 *   Process raw thumbnail data into a Portable Gray Map (pgm)
 *   type P5.
 *
 */

 char	*philips_processThumb ( char *thumbdata, int *Size ) {
 	
	char	*postprocess;
	int		index, state, x;

	*Size = 4813;
	postprocess = (char *)malloc ( *Size );

	if ( postprocess == NULL )
		return ( NULL );

	strcpy ( postprocess, "P5\n80 60 255\n" );
	index = 13;

	for ( x = 0; x < 9600; x++ ) {
		switch ( state ) {
			case 0:
				postprocess[index++] = thumbdata[x];
				state++;
				break;
			case 1: 
				postprocess[index++] = thumbdata[x];
				state++;
				break;
			case 2:
				state++;
				break;
			case 3:
				state = 0;
				break;
			}
		}
	
	return ( postprocess );
}


char *philips_description ()
{
	return ( philips_description_string );
}

/* Declare the camera function pointers */

struct _Camera philips = {
			philips_initialize,
			philips_get_picture,
			philips_get_preview,
			philips_delete_picture,
			philips_take_picture,
			philips_number_of_pictures,
			philips_configure,
			philips_summary,
			philips_description
			};

