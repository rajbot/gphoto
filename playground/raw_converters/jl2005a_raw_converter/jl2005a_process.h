/* jl2005a_process.h
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

#ifndef __jl2005a_process_H__
#define __jl2005a_process_H__


#define BYTE	unsigned char
#define CLAMP(x)	((x)<0?0:((x)>255)?255:(x))
#define	MAX(a,b) 	((a)>(b)?(a):(b)) 
#define 	MIN(a,b)	((a)<(b)?(a):(b))
#define	MAX3(a,b,c) 	MAX(MAX(a,b),c)
#define	MIN3(a,b,c) 	MIN(MIN(a,b),c)
#define	MAX4(a,b,c,d) 	MAX(MAX(a,b),MAX(c,d))
#define	MIN4(a,b,c,d) 	MIN(MIN(a,b),MIN(c,d))

#define THIS_CAM_TILE	BAYER_TILE_BGGR 
#define HEADER_LEN	5

typedef struct {
	int width;
	int height;
	unsigned char compression;
	float	gamma;
} image_info;

int get_image_info( image_info *info, int size);


int jl2005a_decompress (unsigned char *inp, unsigned char *outp, int width,
   int height);

#endif