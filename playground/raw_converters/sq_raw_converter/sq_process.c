/*
 * sq_process.c
 *
 * Part of a processor program for raw data from SQ905C, SQ9050, and 
 * SQ913D cameras. 
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include "gamma.h"
#include "bayer.h"
#include "sq_process.h"


int
get_image_info (image_info *info, unsigned char *photo, int size)
{

	printf("Running get_image_info\n");


	fprintf(stderr,"Size and compression code byte is 0x%x\n", 
						photo[size-0x10]);
	switch (photo[size-0x10]) {
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x76: 
		info->compression=1;
		break;
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x52:
	case 0x53:
	case 0x56: 
	case 0x72: 
		info->compression=0;
		break;
	default:
		fprintf(stderr,"Your camera has unknown resolution settings.\n");
			return -1;
	}
	switch (photo[size-0x10]) {  
	case 0x41:
	case 0x52:
	case 0x61: 
		info->width =352;
		info->height=288;
		break;
	case 0x42:
	case 0x62:
	case 0x72:
		info->width =176;
		info->height=144;
		break;
	case 0x43:
	case 0x53:
	case 0x63:
		info->width =320;
		info->height=240;
		break;
	case 0x56:
	case 0x76:
		info->width =640;
		info->height=480;
		break;
	default:
		fprintf(stderr, "Your pictures have unknown width.\n");
	}
	info->lighting=photo[size-5];
	fprintf(stderr, "info->lighting is %#x\n", info->lighting);
	info->gamma = 0.70;
	fprintf(stderr, "info->gamma is %f\n", info->gamma);
	return 0;
	
}


static int
sq_first_decompress (unsigned char *output, unsigned char *input,
					    unsigned int outputsize)
{
	unsigned char parity = 0;
	unsigned char nibble_to_keep[2];
	unsigned char temp1 = 0, temp2 = 0;
	unsigned char input_byte;
	unsigned char lookup = 0;
	unsigned int i = 0;
	unsigned int bytes_used = 0;
	unsigned int bytes_done = 0;
	unsigned int bit_counter = 8;
	unsigned int cycles = 0;
	int table[9] = { -1, 0, 2, 6, 0x0e, 0x0e, 0x0e, 0x0e, 0xfb};
	unsigned char lookup_table[16]
		     ={0, 2, 6, 0x0e, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,
		           0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb};
	unsigned char translator[16] = {8,7,9,6,10,11,12,13,14,15,5,4,3,2,1,0};

	fprintf (stderr,"Running first_decompress.\n");
	nibble_to_keep[0] = 0;
	nibble_to_keep[1] = 0;

        while (bytes_done < outputsize) {
        	while (parity < 2 ) {
			while ( lookup > table[cycles]) {
				if (bit_counter == 8) {
        				input_byte = input[bytes_used];
        				bytes_used ++;
					temp1 = input_byte;
        				bit_counter = 0;
        			}
				input_byte = temp1;
				temp2 = (temp2 << 1) & 0xFF;
				input_byte = input_byte >> 7;
				temp2 = temp2 | input_byte;
				temp1 = (temp1 <<1)&0xFF;
				bit_counter ++ ;
    				cycles ++ ;
        			if (cycles > 9) {
					fprintf (stderr,"Too many cycles?\n");
					return -1; 
    				}
        			lookup = temp2 & 0xff;        
			}
			temp2 = 0;
			for (i=0; i < 17; i++ ) {
				if (lookup == lookup_table[i] ) {
					nibble_to_keep[parity] = translator[i];
					break;	
				}
				if (i == 16) {
					fprintf(stderr, 
					"Illegal lookup value during decomp\n");
					return -1;
				}	
			}		
        		cycles = 0;
        		parity ++ ;
        	} 
            	output[bytes_done] = (nibble_to_keep[0]<<4)|nibble_to_keep[1];
            	bytes_done ++ ;
        	parity = 0; 
        }
	fprintf(stderr,"bytes_used = 0x%x = %i\n", bytes_used, bytes_used);        
        return 0;
}

static int
sq_second_decompress (unsigned char *uncomp, unsigned char *in, 
						    int width, int height)
{
	int diff = 0;
	int tempval = 0;
	int i, m, parity;
	unsigned char delta_left = 0;
	unsigned char delta_right = 0;
	int input_counter = 0;
	int delta_table[] = {-144, -110, -77, -53, -35, -21, -11, -3,
				2, 10, 20, 34, 52, 76, 110, 144};
	unsigned char *templine_red;
	unsigned char *templine_green;
	unsigned char *templine_blue;
	templine_red = malloc(width);
	if (!templine_red) {
		free(templine_red);
		return -1;
	}	
	for(i=0; i < width; i++){
	    templine_red[i] = 0x80;
	}
	templine_green = malloc(width);	
	if (!templine_green) {
		free(templine_green);
		return -1;
	}	
	for(i=0; i < width; i++){
	    templine_green[i] = 0x80;
	}
	templine_blue = malloc(width);	
	if (!templine_blue) {
		free(templine_blue);
		return -1;
	}	
	for(i=0; i < width; i++){
	    templine_blue[i] = 0x80;
	}
	fprintf(stderr,"Running second_decompress.\n");
	for (m=0; m < height/2; m++) {
		/* First we do an even-numbered line */
		for (i=0; i< width/2; i++) {
			parity = i&1;
	    		delta_right = in[input_counter] &0x0f;
			delta_left = (in[input_counter]>>4)&0xff;
			input_counter ++;
			/* left pixel (red) */
			diff = delta_table[delta_left];
			if (!i) 
				tempval = templine_red[0] + diff;
			else 
				tempval = (templine_red[i]
				        + uncomp[2*m*width+2*i-2])/2 + diff;
			tempval = MIN(tempval, 0xff);
			tempval = MAX(tempval, 0);
			uncomp[2*m*width+2*i] = tempval;
			templine_red[i] = tempval;
			/* right pixel (green) */
			diff = delta_table[delta_right];
			if (!i) 
				tempval = templine_green[1] + diff;
			else if (2*i == width - 2 ) 
				tempval = (templine_green[i]
						+ uncomp[2*m*width+2*i-1])/2 
							+ diff;
			else
				tempval = (templine_green[i+1]
						+ uncomp[2*m*width+2*i-1])/2 
							+ diff;
			tempval = MIN(tempval, 0xff);
			tempval = MAX(tempval, 0);
			uncomp[2*m*width+2*i+1] = tempval;
			templine_green[i] = tempval;
		}
		/* then an odd-numbered line */
		for (i=0; i< width/2; i++) {
			delta_right = in[input_counter] &0x0f;
			delta_left = (in[input_counter]>>4)&0xff;
			input_counter ++;
			/* left pixel (green) */
			diff = delta_table[delta_left];
			if (!i) 
				tempval = templine_green[0] + diff;
			else 
				tempval = (templine_green[i]
				    	    + uncomp[(2*m+1)*width+2*i-2])/2 
						+ diff;
			tempval = MIN(tempval, 0xff);
			tempval = MAX(tempval, 0);
			uncomp[(2*m+1)*width+2*i] = tempval;
			templine_green[i] = tempval;
			/* right pixel (blue) */
			diff = delta_table[delta_right];
			if (!i) 
				tempval = templine_blue[0] + diff;
			else 
				tempval = (templine_blue[i]
					    + uncomp[(2*m+1)*width+2*i-1])/2 
						+ diff;
			tempval = MIN(tempval, 0xff);
			tempval = MAX(tempval, 0);
			uncomp[(2*m+1)*width+2*i+1] = tempval;
			templine_blue[i] = tempval;
		}
	}
	free(templine_green);
	free(templine_red);
	free(templine_blue);
	return 0;
}

int
sq_decompress (unsigned char *out_data, unsigned char *data,
	    int w, int h)
{
	int size;
	unsigned char *temp_data;
	size = w*h/2;
	temp_data = malloc(size);
	if (!temp_data) 
		return(-1);	
	sq_first_decompress (temp_data, data, size);
	fprintf(stderr,"Stage one done\n");
	sq_second_decompress (out_data, temp_data, w, h);
	fprintf(stderr,"Stage two done\n");
	free(temp_data);
	return(0);
}

/* Brightness correction routine adapted from 
 * camlibs/polaroid/jd350e.c, copyright © 2001 Michael Trawny 
 * <trawny99@users.sourceforge.net>
 */


#define RED(p,x,y,w) *((p)+3*((y)*(w)+(x))  )
#define GREEN(p,x,y,w) *((p)+3*((y)*(w)+(x))+1)
#define BLUE(p,x,y,w) *((p)+3*((y)*(w)+(x))+2)

#define MINMAX(a,min,max) { (min)=MIN(min,a); (max)=MAX(max,a); }

int sq_postprocess(int width, int height, unsigned char* rgb)
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
white_balance (unsigned char *data, unsigned int size, float saturation)
{
	int x, r, g, b, max, d;
	double r_factor, g_factor, b_factor, max_factor;
	int htable_r[0x100], htable_g[0x100], htable_b[0x100];
	BYTE gtable[0x100];
	double new_gamma, gamma=1.0;

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
	saturation=saturation*new_gamma*new_gamma;
	printf("saturation = %1.2f\n", saturation);
	gamma = new_gamma;
	if (new_gamma < .70) gamma = 0.70;
	if (new_gamma > 1.2) gamma = 1.2;
	printf("Gamma correction = %1.2f\n", gamma);
	gp_gamma_fill_table(gtable, gamma);
	gp_gamma_correct_single(gtable,data,size);
	if (saturation < .5 ) /* If so, exit now. */ 
		return 0;

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
	if (max_factor >= 4.0) {
	/* We need a little bit of control, here. If max_factor > 5 the photo
	 * was very dark, after all. 
	 */
		if (2.0*b_factor < max_factor)
			b_factor = max_factor/2.;
		if (2.0*r_factor < max_factor)
			r_factor = max_factor/2.;
		if (2.0*g_factor < max_factor)
			g_factor = max_factor/2.;
		r_factor = (r_factor / max_factor) * 4.0;
		g_factor = (g_factor / max_factor) * 4.0;
		b_factor = (b_factor / max_factor) * 4.0;
	}

	if (max_factor > 1.5) 
		saturation = 0;
	printf("White balance (bright): r=%1d, g=%1d, b=%1d, fr=%1.3f, fg=%1.3f, fb=%1.3f\n", r, g, b, r_factor, g_factor, b_factor);
	if (max_factor <= 1.4) {
		for (x = 0; x < (size * 3); x += 3)
		{
			d = (data[x+0]<<8) * r_factor+8;
			d >>=8;
			if (d > 0xff) { d = 0xff; }
			data[x+0] = d;
			d = (data[x+1]<<8) * g_factor+8;
			d >>=8;
			if (d > 0xff) { d = 0xff; }
			data[x+1] = d;
			d = (data[x+2]<<8) * b_factor+8;
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
			d = (int) (r + g + b) / 3.;
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
