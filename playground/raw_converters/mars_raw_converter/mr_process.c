/*
 * mr_process.c
 *
 * Part of a processor program for raw data from Mars MR97310 cameras. 
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
#include "mr_process.h"

/*********************************************************/
int get_image_info( image_info *info, unsigned char *photo) {

	int w, h;

	switch (photo[6]&0x0f) {
	case  0x00: w = 176; h = 144; break;
	case  0x02: w = 352; h = 288; break;
	case  0x06: w = 320; h = 240; break;
	case  0x08: w = 640; h  = 480; break;
	default: return -1;
	}
	fprintf (stderr,"%02X\n",photo[6]&0x0f);
	printf("w is %i\n", w);
	info->width = w;
	info->height= h;
	info->compression = (photo[6] >>4) & 0x0f;
	info->lighting	  = photo[7];
	printf("lighting indicator is %#x\n", info->lighting);
	info->gamma = MAX(sqrt((float)photo[7]/100.), .60);
	printf("gamma is %f\n", info->gamma);
	return 0;
}





void precalc_table(code_table_t *table)
{
  int i;
  int is_abs, val, len;

  for (i = 0; i < 256; i++) {
    is_abs = 0;
    val = 0;
    len = 0;
    if ((i & 0x80) == 0) {
      /* code 0 */
      val = 0;
      len = 1;
    }else if ((i & 0xE0) == 0xC0) {
      /* code 110 */
      val = -3;
      len = 3;
    }else if ((i & 0xE0) == 0xA0) {
      /* code 101 */
      val = +3;
      len = 3;
    }else if ((i & 0xF0) == 0x80) {
      /* code 1000 */
      val = +8;
      len = 4;
    }else if ((i & 0xF0) == 0x90) {
      /* code 1001 */
      val = -8;
      len = 4;
    }else if ((i & 0xF0) == 0xF0) {
      /* code 1111 */
      val = -20;
      len = 4;
    }else if ((i & 0xF8) == 0xE0) {
      /* code 11100 */
      val = +20;
      len = 5;
    }

    else if ((i & 0xF8) == 0xE8) {
      /* code 11101 */
      is_abs = 1;
      val = 0;
      len = 5;
    }


    table[i].is_abs = is_abs;
    table[i].val = val;
    table[i].len = len;
  }
}


int mars_decompress (unsigned char *inp, unsigned char *outp, int width,
   int height)
{
	int row, col;
	unsigned char code;
	int val;
	code_table_t table[256];
	unsigned char *addr;
	unsigned long bitpos;
	unsigned char lp=0,tp=0,tlp=0,trp=0,predictor = 0;



	/* First calculate the Huffman table */
	precalc_table(table);
	bitpos = 0;
	#define GET_CODE addr = inp + (bitpos >> 3);\
	code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)))

	/* main decoding loop */
	for (row = 0; row < height; row++) {
		col = 0;

		/* first two pixels in first two rows are stored as raw 8-bit */
		if (row < 2) {
    			GET_CODE;
    			bitpos += 8;
    			*outp++ = code;

    			GET_CODE;
    			bitpos += 8;
    			*outp++ = code;
    			col += 2;
		}

		while (col < width) {
    			/* get bitcode */
    			GET_CODE;
    			/* update bit position */
    			bitpos += table[code].len;

    			/* calculate pixel value */
    			if (table[code].is_abs) {
    			/* get 5 more bits and use them as absolute value */
    				GET_CODE;
    				val = (code & 0xF8);
    				bitpos += 5;
    			} else {
    			/* value is relative to top or left pixel */
				
				predictor=0;
    				val = table[code].val;
    				lp =  outp[-2];
    				if (row > 1) {
        				tp  = outp[-2*width];
        				tlp  = outp[-2*width-2];
        				if(col < width-2)
        					trp = outp[-2*width+2];
    				}
				if (row < 2) {
        			/* top row: relative to left pixel */
					val += lp;
    				}else if (col < 2) {
					val += (tp+trp)/2;
    				}else if (col > width - 3) {
					val += (lp+tp+tlp+1)/3;
    				/* main area: average of left pixel and top pixel */
    				} 
    				else {
    				/* main area */
        				/* initial estimate for predictor */
					tlp>>=1;
					trp>>=1;
					val += (lp + tp + tlp + trp + 1)/3;
				}
    			}
    			/* store pixel */
    			*outp++ = CLAMP(val);
    			col++;
		}
	}
	return 0;
}


#define RED(p,x,y,w) *((p)+3*((y)*(w)+(x))  )
#define GREEN(p,x,y,w) *((p)+3*((y)*(w)+(x))+1)
#define BLUE(p,x,y,w) *((p)+3*((y)*(w)+(x))+2)

//#define MINMAX(a,min,max) { (min)=MIN(min,a); (max)=MAX(max,a); }

int normalize(int width, int height, unsigned char* rgb)
{
	int
		x,y,
		red_min=255, red_max=0, 
		blue_min=255, blue_max=0, 
		green_min=255, green_max=0;
	double
		min, max, amplify; 

	/* determine min and max per color... */

	for( y=0; y<height; y++){
		for( x=0; x<width; x++ ){
			MINMAX( RED(rgb,x,y,width), red_min,   red_max  );
			MINMAX( GREEN(rgb,x,y,width), green_min, green_max);
			MINMAX( BLUE(rgb,x,y,width), blue_min,  blue_max );
		}
	}

	/* determine min and max per color... */

	for( y=0; y<height; y++){
		for( x=0; x<width; x++ ){
			MINMAX( RED(rgb,x,y,width), red_min,   red_max  );
			MINMAX( GREEN(rgb,x,y,width), green_min, green_max);
			MINMAX( BLUE(rgb,x,y,width), blue_min,  blue_max );
		}
	}

	/* Normalize brightness ... */

	max = MAX( MAX( red_max, green_max ), blue_max);
	min = MIN( MIN( red_min, green_min ), blue_min);
	amplify = 255.0/(max-min);


	printf("min=%f max=%f amplify=%f\n",min,max,amplify);
	if (max == 255)
		printf("max is already max'ed at 255. Exiting from normalize()\n");
		return 0;



	for( y=0; y<height; y++){
		for( x=0; x<width; x++ ){
			RED(rgb,x,y,width)= MIN(amplify*(double)(RED(rgb,x,y,width)-min),255);
			GREEN(rgb,x,y,width)= MIN(amplify*(double)(GREEN(rgb,x,y,width)-min),255);
			BLUE(rgb,x,y,width)= MIN(amplify*(double)(BLUE(rgb,x,y,width)-min),255);
		}
	}

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
	for (x = 0; x < 0x100; x++) { 
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
white_balance (unsigned char *data, unsigned int size, float saturation, 
				    image_info *info)
{
	int x, r, g, b, max, d;
	double r_factor, g_factor, b_factor, max_factor;
	int htable_r[0x100], htable_g[0x100], htable_b[0x100];
	BYTE gtable[0x100];
	double new_gamma, gamma=1.0;

	/* ------------------- GAMMA CORRECTION ------------------- */

	histogram(data, size, htable_r, htable_g, htable_b);
	x = 1;
	for (r = 48; r < 208; r++)
	{
		x += htable_r[r]; 
		x += htable_g[r];
		x += htable_r[r]; 
	}
	new_gamma = sqrt((double) (x * 1.5) / (double) (size * 3));
	x=0;
	printf("Provisional gamma correction = %1.2f\n", new_gamma);
	/* Recalculate saturation factor for later use. */
	saturation=saturation*new_gamma*new_gamma;
	printf("saturation = %1.2f\n", saturation);
	if(new_gamma >= 1.0)
		gamma = new_gamma;
	else
		gamma = info->gamma;

	printf("Gamma correction = %1.2f\n", gamma);
	gp_gamma_fill_table(gtable, gamma);

	/* ---------------- BRIGHT DOTS ------------------- */
	max = size / 200; 
	histogram(data, size, htable_r, htable_g, htable_b);

	for (r=0xfe, x=0; (r > 32) && (x < max); r--)  
		x += htable_r[r]; 
	for (g=0xfe, x=0; (g > 32) && (x < max); g--) 
		x += htable_g[g];
	for (b=0xfe, x=0; (b > 32) && (x < max); b--) 
		x += htable_b[b];
	r_factor = (double) 0xfd / r;
	g_factor = (double) 0xfd / g;
	b_factor = (double) 0xfd / b;

	max_factor = r_factor;
	if (g_factor > max_factor) max_factor = g_factor;
	if (b_factor > max_factor) max_factor = b_factor;

	if (max_factor >= 2.5) {
		r_factor = (r_factor / max_factor) * 2.5;
		g_factor = (g_factor / max_factor) * 2.5;
		b_factor = (b_factor / max_factor) * 2.5;
	}
	printf("White balance (bright): r=%1d, g=%1d, b=%1d, fr=%1.3f, fg=%1.3f, fb=%1.3f\n", r, g, b, r_factor, g_factor, b_factor);
	if (max_factor <= 2.5) {
		for (x = 0; x < (size * 3); x += 3)
		{
			d = (data[x+0]<<8) * r_factor;
			d >>=8;
			if (d > 0xff) { d = 0xff; }
			data[x+0] = d;
			d = (data[x+1]<<8) * g_factor;
			d >>=8;
			if (d > 0xff) { d = 0xff; }
			data[x+1] = d;
			d = (data[x+2]<<8) * b_factor;
			d >>=8;
			if (d > 0xff) { d = 0xff; }
			data[x+2] = d;
		}
	}
	/* ---------------- DARK DOTS ------------------- */
	max = size / 200;  /*  1/200 = 0.5%  */
	histogram(data, size, htable_r, htable_g, htable_b);

	for (r=0, x=0; (r < 96) && (x < max); r++)  
		x += htable_r[r]; 
	for (g=0, x=0; (g < 96) && (x < max); g++) 
		x += htable_g[g];
	for (b=0, x=0; (b < 96) && (x < max); b++) 
		x += htable_b[b];

	r_factor = (double) 0xfe / (0xff-r);
	g_factor = (double) 0xfe / (0xff-g);
	b_factor = (double) 0xfe / (0xff-b);

	max_factor = r_factor;
	if (g_factor > max_factor) max_factor = g_factor;
	if (b_factor > max_factor) max_factor = b_factor;

	if (max_factor >= 1.15) {
		r_factor = (r_factor / max_factor) * 1.15;
		g_factor = (g_factor / max_factor) * 1.15;
		b_factor = (b_factor / max_factor) * 1.15;
	}
	printf(
	"White balance (dark): r=%1d, g=%1d, b=%1d, fr=%1.3f, fg=%1.3f, fb=%1.3f\n", 
				r, g, b, r_factor, g_factor, b_factor);

	for (x = 0; x < (size * 3); x += 3)
	{
		d = (int) 0xff08-(((0xff-data[x+0])<<8) * r_factor);
		d >>= 8;
		if (d < 0) { d = 0; }
		data[x+0] = d;
		d = (int) 0xff08-(((0xff-data[x+1])<<8) * g_factor);
		d >>= 8;
		if (d < 0) { d = 0; }
		data[x+1] = d;
		d = (int) 0xff08-(((0xff-data[x+2])<<8) * b_factor);
		d >>= 8;
		if (d < 0) { d = 0; }
		data[x+2] = d;
	}

	/* ------------------ COLOR ENHANCE ------------------ */

	if(saturation > 0.0) {
		for (x = 0; x < (size * 3); x += 3)
		{
			r = data[x+0]; g = data[x+1]; b = data[x+2];
			d = (int) (r + g + b) /3.;
			if ( r > d )
				r = r + (int) ((r - d) * (0xff-r)/(0x100-d) * saturation);
			else 
				r = r + (int) ((r - d) * (0xff-d)/(0x100-r) * saturation);
			if (g > d)
				g = g + (int) ((g - d) * (0xff-g)/(0x100-d) * saturation);
			else 
				g = g + (int) ((g - d) * (0xff-d)/(0x100-g) * saturation);
			if (b > d)
				b = b + (int) ((b - d) * (0xff-b)/(0x100-d) * saturation);
			else 
				b = b + (int) ((b - d) * (0xff-d)/(0x100-b) * saturation);
			data[x+0] = CLAMP(r);
			data[x+1] = CLAMP(g);
			data[x+2] = CLAMP(b);
		}
	}
	return 0;
}
