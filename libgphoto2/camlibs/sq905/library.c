/* library.c
 *
 * Copyright (C) 2003 Theodore Kilgore <kilgota@auburn.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <bayer.h>
#include <gamma.h>

#include <gphoto2.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "sq905.h"

#include <gphoto2-port.h>

#define GP_MODULE "sq905"

struct _CameraPrivateLibrary {
	SQData data[0x400];
};

struct {
   	char *name;
	CameraDriverStatus status;
   	unsigned short idVendor;
   	unsigned short idProduct;
   	char serial;
} models[] = {
        {"SQ chip camera", GP_DRIVER_STATUS_EXPERIMENTAL,0x2770,0x9120,0},
	/* Developed on the following two cameras: */
        {"Argus DC-1510",GP_DRIVER_STATUS_PRODUCTION,0x2770,0x9120,0},
	{"Gear to go", GP_DRIVER_STATUS_EXPERIMENTAL,0x2770,0x9120,0},
	/* Reported to work also on */
	{"Mitek CD10" , GP_DRIVER_STATUS_EXPERIMENTAL,0x2770,0x9120,0},
	/* Reputed to have the same chipset */
	{"GTW Electronics",GP_DRIVER_STATUS_EXPERIMENTAL,0x2770,0x9120,0},
	{"Concord Eye-Q Easy",GP_DRIVER_STATUS_EXPERIMENTAL,0x2770,0x9120,0},
	{"Che-ez Snap", GP_DRIVER_STATUS_EXPERIMENTAL,0x2770,0x9120,0},
	{NULL,0,0,0}
};

int
camera_id(CameraText *id)
{
    	strcpy (id->text, "SQ chipset camera");
        
    	return GP_OK;
}


int
camera_abilities(CameraAbilitiesList *list)
{
    	int i;    
    	CameraAbilities a;

    	for (i = 0; models[i].name; i++) {
        	memset (&a, 0, sizeof(a));
       		strcpy (a.model, models[i].name);
       		a.status = models[i].status;
       		a.port   = GP_PORT_USB;
       		a.speed[0] = 0;
       		a.usb_vendor = models[i].idVendor;
       		a.usb_product= models[i].idProduct;
       		if (a.status == GP_DRIVER_STATUS_EXPERIMENTAL)
			a.operations = GP_OPERATION_NONE;
		else
			a.operations = GP_OPERATION_CAPTURE_IMAGE;
       		a.folder_operations = GP_FOLDER_OPERATION_NONE;
       		a.file_operations   = GP_FILE_OPERATION_NONE; 
       		gp_abilities_list_append (list, a);
    	}

    	return GP_OK;
}

static int
camera_summary(Camera *camera, CameraText *summary, GPContext *context)
{
	int num_pics = sq_get_num_pics (camera->pl->data);

    	sprintf (summary->text,_("Your USB camera has a S&Q chipset.\n" 
    			"The number of pictures taken is %i\n"), num_pics);  

    	return GP_OK;
}


static int
camera_about (Camera *camera, CameraText *about, GPContext *context)
{
    	strcpy (about->text, _("sq905 generic driver\n"
			    "Theodore Kilgore <kilgota@auburn.edu>\n"));

    	return GP_OK;
}

/*************** File and Downloading Functions *******************/

static int
file_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
                void *data, GPContext *context)
{
        Camera *camera = data; 
    	int n = sq_get_num_pics (camera->pl->data);

    	gp_list_populate (list, "pict%02i.ppm", n);

    	return GP_OK;
}

static int
get_file_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileType type, CameraFile *file, void *user_data,
	       GPContext *context)
{
    	Camera *camera = user_data; 
    	int w, h = 0, buffersize, comp_ratio, k;
    
    	GP_DEBUG("Downloading pictures!\n");    

    	/* Get the number of the photo on the camera */
	k = gp_filesystem_number (camera->fs, "/", filename, context); 

    	/* Resolution must be checked for each individual picture. */
    	comp_ratio = sq_get_comp_ratio (camera->pl->data, k);
    	w = sq_get_picture_width (camera->pl->data, k);
    	switch (w) {
	case 176:
		h = 144;
		break;
	default:
		h = 288;
		break;
	}
    	buffersize = w * h / comp_ratio;

    	unsigned char data[buffersize]; 
    	memset(data,0,buffersize+1); 
    	unsigned char p_data[w * h];

    	switch (type) {
	case GP_FILE_TYPE_NORMAL:
		sq_read_picture_data (camera->port, data, buffersize);
		break;
	case GP_FILE_TYPE_PREVIEW:
		sq_read_picture_data (camera->port, data, buffersize);
		break;
	default:
		return GP_ERROR_NOT_SUPPORTED;
    	}

    	/* There really ain't no such thing as a "preview" 
    	 * type with this camera. The only reason this switch 
    	 * is put here is in order to cure a really nasty 
    	 * timing problem. The effect is to force a tight linkage 
    	 * between the downloaded image data and the picture file 
    	 * into which it is supposed to go. For, otherwise it is 
    	 * possible (for example) to download two data sets for one 
    	 * picture, which causes data corruption. The problem only 
    	 * shows up on some computers. Anyway, the switch turns off 
    	 * the machine which makes the salt!
    	 */
    
    	int m = 0;

    	for (m = 0; m < buffersize; m++) { 
		switch (comp_ratio) {
			/* This is an experiment which does not work. 
			 * But it almost works! 
	 	 	 * By looking at the output, an expert might 
			 * actually be able to 
	 	 	 * see what to instead!
	 	 	 */
		case 2:
			p_data[2 * m] = 0x10 * (data[m]/0x10 );
			p_data[2 * m + 1] = 0x10 * (data[m] % 0x10);
			break;
		default:
			p_data[m] = data[m];
		}
    	}

	/*
	 * Now we want to get our picture into a file on 
     	 * the hard drive. We reverse the data string.
     	 * Otherwise, the picture will be upside down.  
     	 */

    	int i = 0;    
    	for (i = 0; i <= buffersize/2; ++i) {
		unsigned char temp;
        	temp = p_data[i];
        	p_data[i] = p_data[buffersize -1 -i];
        	p_data[buffersize - 1 - i] = temp;
    	}    	

        if (!p_data)
	        return GP_ERROR;

	/* Now put the data into a real PPM picture file! 
	 * The results are quite nice if comp_ratio = 1.
	 * But if comp_ratio=2 this procedure still does 
	 * give the picture back!
	 */

	char ppm[w* h * 3];
	unsigned char gtable[256];
	char *ptr;
	int size = 0;

    	sprintf (ppm,
		"P6\n"
		"# CREATOR: gphoto2, SQ905 library\n"
		"%d %d\n"
		"255\n", w, h);

	ptr = ppm + strlen (ppm);	

	size = strlen (ppm) + (w * h * 3);

	GP_DEBUG ("size = %i\n", size);

	gp_bayer_decode (p_data, w , h , ptr, BAYER_TILE_BGGR ); 

	/*
	 * Unsurprisingly, reversing the data requires changing RGGB to BGGR,
	 * too!
	 */
	gp_gamma_fill_table (gtable, .65);

	/*
	 * gamma of .65 arrived at by experinentation. Maybe could be 
	 * improved? 
	 */
	gp_gamma_correct_single (gtable, ptr, w * h);
        gp_file_set_mime_type (file, GP_MIME_PPM);
        gp_file_set_name (file, filename); 
	gp_file_append (file, ppm, size);	

        return GP_OK;
}

/*************** Exit and Initialization Functions ******************/

static int
camera_exit (Camera *camera, GPContext *context)
{
	GP_DEBUG ("SQ camera_exit");
	sq_reset (camera->port);

	if (camera->pl) {
		free (camera->pl);
		camera->pl = NULL;
	}

	return GP_OK;
}

int
camera_init(Camera *camera, GPContext *context)
{
	GPPortSettings settings;
	int ret = 0;

	/* First, set up all the function pointers */
	camera->functions->summary      = camera_summary;
	camera->functions->about        = camera_about;
	camera->functions->exit	    = camera_exit;
   
	GP_DEBUG ("Initializing the camera\n");

	ret = gp_port_get_settings(camera->port,&settings);
	if (ret < 0) return ret; 
 
	ret = gp_port_set_settings(camera->port,settings);
	if (ret < 0) return ret; 

        /* Tell the CameraFilesystem where to get lists from */
	gp_filesystem_set_list_funcs (camera->fs, file_list_func, NULL, camera);
	gp_filesystem_set_file_funcs (camera->fs, get_file_func, NULL, camera);

	camera->pl = malloc (sizeof (CameraPrivateLibrary));
	if (!camera->pl) return GP_ERROR_NO_MEMORY;
	memset (camera->pl, 0, sizeof (CameraPrivateLibrary));

	/* Connect to the camera */
	sq_init (camera->port, camera->pl->data);

	return GP_OK;
}
