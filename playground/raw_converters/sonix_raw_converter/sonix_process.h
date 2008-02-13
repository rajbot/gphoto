/* sonix_process.h
 *
 * Header file for image processor for raw files from Sonix SN9C2028 cameras.
 *
 * Copyright (C) 2008 Theodore Kilgore <kilgota@auburn.edu>
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

#ifndef __SONIX_PROCESS_H__
#define __SONIX_PROCESS_H__

#define BYTE	unsigned char
#define HEADER_LEN	8

typedef struct {
	int width;
	int height;
	int bayer;
	unsigned char reverse;
	unsigned char compression;
	float	gamma;
	unsigned char outdoors;
} image_info;

int get_image_info( image_info *info, unsigned char *photo, int filesize, 
						unsigned char qvga_setting);

int reverse_bytes(unsigned char *some_data, int size);


int sonix_decode		(unsigned char * dst, unsigned char * src, 
				    int width, int height);
int
histogram (unsigned char *data, unsigned int size, int *htable_r, int *htable_g, int *htable_b);

int
white_balance (unsigned char *data, unsigned int size, float saturation);


#endif 
