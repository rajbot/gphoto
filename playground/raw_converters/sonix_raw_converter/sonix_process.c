/*
 * sonix_process.c
 *
 * Part of a processor program for raw data from Sonix SN9C2028 cameras. 
 * 
 * Copyright (c) 2008 Theodore Kilgore <kilgota@auburn.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * A previous version of the white_balance() function intended for use in 
 * libgphoto2/camlibs/aox is copyright (c) 2008 Amauri Magagna.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gamma.h"
#include "bayer.h"
#include "sonix_process.h"

/*********************************************************/

int get_image_info( image_info *info, unsigned char *photo, int filesize, 
						unsigned char qvga_setting) {

	int w, h;

	info->compression = 0;

	fprintf (stderr,"filesize is 0x%x=%i\n",filesize,filesize);
	switch (photo[filesize-5]&0x0f) {
	case  0x02: 
	case  0x03:	if (qvga_setting) {
				w = 176; 
				h = 144; 
			} else {
				w = 320;
				h = 240;
			}
			break;
	case  0x00:
	case  0x01:	if (qvga_setting) {
				w = 352; 
				h = 288; 
			} else {
				w = 640;
				h = 480;
			}
			break;
	default: return -1;
	}
	info->reverse=0;
	
	fprintf (stderr,"size code is %02X\n",photo[filesize-5]);
	info->width = w;
	info->height= h;
	info->compression = 0;
	if (photo[filesize-5]&0x01) {
		info->compression = 1;
		info->bayer= BAYER_TILE_RGGB;
	}  else 
		info->bayer= BAYER_TILE_RGGB;
	fprintf (stderr,"compression is %i\n",info->compression);
	if (photo[filesize - 6] > 0x60) {
		info->gamma = .60;	
		info->outdoors	  = 0; 
	} else {
		info->gamma = .65;
		info->outdoors	  = 1; 
	}
	printf("reverse is %i\n", info->reverse);
	printf("gamma is %f\n", info->gamma);
	return 0;
}


int reverse_bytes(unsigned char *some_data, int size) {

	int i;
	unsigned char temp;
	for (i=0; i < size/2; ++i) {
                temp = some_data[i];
                some_data[i] = some_data[size - 1 - i];
                some_data[size - 1 - i] = temp;
	}
	return 0;
}


/* 
 * The decoding algorithm originates with Bertrik Sikken. This version adapted
 * from the webcam-osx (macam) project. See README for details.
 */

/* Four defines for bitstream operations, for the decode function */

#define PEEK_BITS(num,to) {\
    	if (bitBufCount<num){\
    		do {\
    			bitBuf=(bitBuf<<8)|(*(src++));\
    			bitBufCount+=8; \
		}\
		while(bitBufCount<24);\
	}\
	to=bitBuf>>(bitBufCount-num);\
}

/*
 * PEEK_BITS puts the next <num> bits into the low bits of <to>. 
 * when the buffer is empty, it is completely refilled. 
 * This strategy tries to reduce memory access. Note that the high bits 
 * are NOT set to zero!
 */

#define EAT_BITS(num) { bitBufCount-=num; bits_eaten += num;}

/*
 * EAT_BITS consumes <num> bits (PEEK_BITS does not consume anything, 
 * it just peeks)
 */
  
#define PARSE_PIXEL(val) {\
    PEEK_BITS(10,bits);\
    if ((bits&0x200)==0) { \
	EAT_BITS(1); \
    } \
    else if ((bits&0x380)==0x280) { \
	EAT_BITS(3); \
	val+=3; \
	if (val>255) val=255; \
    }\
    else if ((bits&0x380)==0x300) { \
	EAT_BITS(3); \
	val-=3; \
	if (val<0) val=0; \
    }\
    else if ((bits&0x3c0)==0x200) { \
	EAT_BITS(4); \
	val+=8; \
	if (val>255) val=255;\
    }\
    else if ((bits&0x3c0)==0x240) { \
	EAT_BITS(4); \
	val-=8; \
	if (val<0) val=0;\
    }\
    else if ((bits&0x3c0)==0x3c0) { \
	EAT_BITS(4); \
	val-=20; \
	if (val<0) val=0;\
    }\
    else if ((bits&0x3e0)==0x380) { \
	EAT_BITS(5); \
	val+=20; \
	if (val>255) val=255;\
    }\
    else { \
	EAT_BITS(10); \
	val=8*(bits&0x1f)+0; \
    }\
}


#define PUT_PIXEL_PAIR {\
    long pp;\
    pp=(c1val<<8)+c2val;\
    *((unsigned short *) (dst+dst_index))=pp;\
    dst_index+=2; }


/* Now the decode function itself */


int 
sonix_decode(unsigned char * dst, unsigned char * src, int width, int height) 
{
	long dst_index = 0;
	int starting_row = 0;
	unsigned short bits;
	short c1val, c2val; 
	int x, y;
	unsigned long bitBuf = 0;
	unsigned long bitBufCount = 0;
	unsigned long bits_eaten = 0;

	
	for (y = starting_row; y < height; y++) {
		PEEK_BITS(8, bits);
		EAT_BITS(8);
		c2val = (bits & 0xff);
		PEEK_BITS(8, bits);
		EAT_BITS(8);
		c1val = (bits & 0xff);

		PUT_PIXEL_PAIR;

		for (x = 2; x < width ; x += 2) {
			/* columns are reversed for most Sonix cameras. */
			PARSE_PIXEL(c2val);
			PARSE_PIXEL(c1val);
    			PUT_PIXEL_PAIR;
		}
	}

	fprintf(stderr, "Bytes used = %lu = %#x\n", bits_eaten/8,(unsigned int)bits_eaten/8);
	fprintf(stderr, "Bits remaining = %lu\n", bits_eaten%8);
	return 0;
}


/*	========= White Balance / Color Enhance / Gamma adjust (experimental) ==========

	Get histogram for each color plane
	Expand to reach 0.5% of white dots in image

	Get new histogram for each color plane
	Expand to reach 0.5% of black dots in image

	Get new histogram
	Calculate and apply gamma correction

	if not a dark image:
	For each dot, increases color separation

	================================================================================== */

int
histogram (unsigned char *data, unsigned int size, int *htable_r, int *htable_g, int *htable_b)
{
	int x;
	/* Initializations */
	for (x = 0; x < 256; x++) { 
		htable_r[x] = 0; 
		htable_g[x] = 0; 
		htable_b[x] = 0; 
	}
	/* Building the histograms */
	for (x = 0; x < (size * 3); x += 3)
	{
		htable_r[data[x+0]]++;	/* red histogram */
		htable_g[data[x+1]]++;	/* green histogram */
		htable_b[data[x+2]]++;	/* blue histogram */
	}
	return 0;
}


int
white_balance (unsigned char *data, unsigned int size, float saturation)
{
	int x, r, g, b, max, d;
	double r_factor, g_factor, b_factor, max_factor;
	int htable_r[256], htable_g[256], htable_b[256];
	BYTE gtable[256];
	double new_gamma;

	/* ------------------- GAMMA CORRECTION ------------------- */

	histogram(data, size, htable_r, htable_g, htable_b);
	x = 1;
	for (r = 64; r < 192; r++)
	{
		x += htable_r[r]; 
		x += htable_g[r];
		x += htable_b[r];
	}
	new_gamma = sqrt((double) (x * 1.5) / (double) (size * 3));
	printf("Provisional gamma correction = %1.2f\n", new_gamma);
	/* Recalculate saturation factor for later use. */
	if (new_gamma < 0.75) new_gamma = 0.75;
	if (new_gamma > 1.2) new_gamma = 1.2;
	printf("Gamma correction = %1.2f\n", new_gamma);
	gp_gamma_fill_table(gtable, new_gamma);
	gp_gamma_correct_single(gtable,data,size);

	/* ---------------- BRIGHT DOTS ------------------- */
	max = size / 200; 
	histogram(data, size, htable_r, htable_g, htable_b);

	for (r=254, x=0; (r > 64) && (x < max); r--)  
		x += htable_r[r]; 
	for (g=254, x=0; (g > 64) && (x < max); g--) 
		x += htable_g[g];
	for (b=254, x=0; (b > 64) && (x < max); b--) 
		x += htable_b[b];

	r_factor = (double) 254 / r;
	g_factor = (double) 254 / g;
	b_factor = (double) 254 / b;
	max_factor = r_factor;
	if (g_factor > max_factor) max_factor = g_factor;
	if (b_factor > max_factor) max_factor = b_factor;

	if (max_factor > 3.0) {
		r_factor = (r_factor / max_factor) * 3.0;
		g_factor = (g_factor / max_factor) * 3.0;
		b_factor = (b_factor / max_factor) * 3.0;
	}

	printf("White balance (bright): r=%1d, g=%1d, b=%1d, fr=%1.3f, fg=%1.3f, fb=%1.3f\n", r, g, b, r_factor, g_factor, b_factor);

	for (x = 0; x < (size * 3); x += 3)
	{
		d = (int) data[x+0] * r_factor;
		if (d > 255) { d = 255; }
		data[x+0] = d;
		d = (int) data[x+1] * g_factor;
		if (d > 255) { d = 255; }
		data[x+1] = d;
		d = (int) data[x+2] * b_factor;
		if (d > 255) { d = 255; }
		data[x+2] = d;
	}
	/* ---------------- DARK DOTS ------------------- */


	max = size / 200;  /*  1/200 = 0.5%  */

	histogram(data, size, htable_r, htable_g, htable_b);

	for (r=0, x=0; (r < 64) && (x < max); r++)  
		x += htable_r[r]; 
	for (g=0, x=0; (g < 64) && (x < max); g++) 
		x += htable_g[g];
	for (b=0, x=0; (b < 64) && (x < max); b++) 
		x += htable_b[b];

	r_factor = (double) 254 / (255-r);
	g_factor = (double) 254 / (255-g);
	b_factor = (double) 254 / (255-b);

	printf("White balance (dark): r=%1d, g=%1d, b=%1d, fr=%1.3f, fg=%1.3f, fb=%1.3f\n", r, g, b, r_factor, g_factor, b_factor);

	for (x = 0; x < (size * 3); x += 3)
	{
		d = (int) 255-((255-data[x+0]) * r_factor);
		if (d < 0) { d = 0; }
		data[x+0] = d;
		d = (int) 255-((255-data[x+1]) * g_factor);
		if (d < 0) { d = 0; }
		data[x+1] = d;
		d = (int) 255-((255-data[x+2]) * b_factor);
		if (d < 0) { d = 0; }
		data[x+2] = d;
	}

	/* ------------------ COLOR ENHANCE ------------------ */


	for (x = 0; x < (size * 3); x += 3)
	{
		r = data[x+0]; g = data[x+1]; b = data[x+2];
		d = (int) (r + 2*g + b) / 4.;
		if ( r > d )
			r = r + (int) ((r - d) * (255-r)/(256-d) * saturation);
		else 
			r = r + (int) ((r - d) * (255-d)/(256-r) * saturation);
		if (g > d)
			g = g + (int) ((g - d) * (255-g)/(256-d) * saturation);
		else 
			g = g + (int) ((g - d) * (255-d)/(256-g) * saturation);
		if (b > d)
			b = b + (int) ((b - d) * (255-b)/(256-d) * saturation);
		else 
			b = b + (int) ((b - d) * (255-d)/(256-b) * saturation);

		if (r < 0) { r = 0; }
		if (r > 255) { r = 255; }
		data[x+0] = r;
		if (g < 0) { g = 0; }
		if (g > 255) { g = 255; }
		data[x+1] = g;
		if (b < 0) { b = 0; }
		if (b > 255) { b = 255; }
		data[x+2] = b;
	}

	
	return 0;
}
