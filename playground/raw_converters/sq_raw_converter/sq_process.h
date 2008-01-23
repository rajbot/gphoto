/*
 * sq_process.h 
 *
 * Header file for image processor for raw files from SQ905C, SQ9050, 
 * and SQ913D cameras. 
 *
 * Copyright (c) 2008 Theodore Kilgore <kilgota@auburn.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __DIGIGR8_H__
#define __DIGIGR8_H__

#define BYTE	unsigned char
#define	MAX(a,b) 	((a)>(b)?(a):(b)) 
#define 	MIN(a,b)	((a)<(b)?(a):(b))
#define CLAMP(x)        ((x)<0?0:((x)>255)?255:(x))
#define THIS_CAM_TILE	BAYER_TILE_BGGR 
#define HEADER_LEN	0x10


typedef struct {
	int width;
	int height;
	unsigned char compression;
	unsigned char lighting;
	float	gamma;
} image_info;

int get_image_info( image_info *info, unsigned char *photo, int size);


int sq_decompress (unsigned char *out_data, unsigned char *data, 
							    int w, int h);
int sq_postprocess	(int width, int height, unsigned char* rgb);

int
histogram (unsigned char *data, unsigned int size, int *htable_r, 
						int *htable_g, int *htable_b);
int
white_balance (unsigned char *data, unsigned int size, float saturation);

#endif
