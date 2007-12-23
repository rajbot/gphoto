/* jl2005a_process.c
 *
 * Copyright (C) 2007 Theodore Kilgore <kilgota@auburn.edu>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gamma.h"
#include "bayer.h"
#include "jl2005a_process.h"

/*********************************************************/
int get_image_info( image_info *info, int size) {

	int w, h, comp_ratio;

	switch(size) {
 		case 0x318e:
			w = 176;
			h = 72;
			comp_ratio = 2;
			break;
		case 0x630e:
			w = 176;
			h = 144;
			comp_ratio = 1;
			break;
 		case 0xc60e:
			w = 352;
			h = 144;
			comp_ratio = 2;
			break;
		case 0x18c0e:
			w = 352;
			h = 288;
			comp_ratio = 1;
			break;

		case 0x960e:
			w = 320;
			h = 240;
			comp_ratio = 2;
			break;
		case 0x12c0e:
			w = 320;
			h = 240;
			comp_ratio = 1;
			break;
		case 0x2580e:
			w = 640;
			h = 480;
			comp_ratio = 2;
			break;
		case 0x4b00e:
			w = 640;
			h = 480;
			comp_ratio = 1;
			break;
		default: ;
			printf("Not a JL2005A photo. Exiting.\n");
			return -1;
	}

	printf("w is %i\n", w);
	info->width = w;
	info->height= h;
	info->compression = comp_ratio;
	info->gamma = .5;
	printf("gamma is %f\n", info->gamma);
	return 0;
}


int jl2005a_decompress (unsigned char *inp, unsigned char *outp, int width,
   int height)
{
	int i,j;
	int hdiff,vdiff;
	for (i=0; i < height/2; i+=2) {
		memcpy(outp+2*i*width,inp+i*width, 2*width);
	}
	memcpy(outp+(height-2)*width,outp+(height-4)*width, 2*width);

	for (i=0; i < height/4-1; i++) {
		for (j=0; j < width; j++) {
			outp[(4*i+2)*width+j]=(inp[(2*i)*width+j]+
						inp[(2*i+2)*width+j])/2;
				outp[(4*i+3)*width+j]=(outp[(4*i+1)*width+j]+
						outp[(4*i+5)*width+j])/2;
		}
	}
	if (width == 176) 
		memmove(outp+6*width, outp, (height-6)*width);

	return 0;
}

