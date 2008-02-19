/* mr_process.h
 *
 * Header file for image processor for raw files from Mars MR97310 cameras.
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

#ifndef __MR_PROCESS_H__
#define __MR_PROCESS_H__

#define BYTE	unsigned char
#define CLAMP(x)	((x)<0?0:((x)>255)?255:(x))
#define	MAX(a,b) 	((a)>(b)?(a):(b)) 
#define 	MIN(a,b)	((a)<(b)?(a):(b))
#define MINMAX(a,min,max) { (min)=MIN(min,a); (max)=MAX(max,a); }

#define THIS_CAM_TILE	BAYER_TILE_RGGB 
#define HEADER_LEN	12

typedef struct {
	int width;
	int height;
	unsigned char lighting;
	unsigned char compression;
	float	gamma;
} image_info;

int get_image_info( image_info *info, unsigned char *photo);


typedef struct {
	int is_abs;
	int len;
	int val;
} code_table_t;

void precalc_table(code_table_t *table);
int mars_decompress (unsigned char *inp, unsigned char *outp, int width,
					int height);
int normalize(int width, int height, unsigned char* rgb);
int histogram (unsigned char *data, unsigned int size, int *htable_r, 
                                        int *htable_g, int *htable_b);
int white_balance (unsigned char *data, unsigned int size, float saturation, 
					image_info *info);
#endif
