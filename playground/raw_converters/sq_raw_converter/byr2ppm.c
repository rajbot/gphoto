/** byr2ppm.c (equivalent to libgphoto2/libgphoto2/bayer.c)
 * Bayer array conversion routines.
 * 
 * Copyright 2001 Lutz Mller <lutz@users.sf.net>
 * Copyright 2007 Theodore Kilgore <kilgota@auburn.edu>
 *
 * 
 * gp_bayer_accrue() from Theodore Kilgore <kilgota@auburn.edu>
 * contains suggestions by B. R. Harris (e-mail address disappeared) and
 * Werner Eugster <eugster@giub.unibe.ch>
 *
 * License
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * 
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* The tiles will be used in the Bayer interpolation routine. */
#include "bayer.h"
#include "gamma.h"

static int tile_colors[8][4] = {
	{0, 1, 1, 2},
	{1, 0, 2, 1},
	{2, 1, 1, 0},
	{1, 2, 0, 1},
	{0, 1, 1, 2},
	{1, 0, 2, 1},
	{2, 1, 1, 0},
	{1, 2, 0, 1}};

#define RED 0
#define GREEN 1
#define BLUE 2


/* Bayer interpolation routine. Credit to libgphoto2/bayer.c, by Lutz M\"uller.
*/

int
gp_bayer_accrue (unsigned char *image, int w, int h, int x0, int y0,
		int x1, int y1, int x2, int y2, int x3, int y3, int colour) ;

int
gp_bayer_expand (unsigned char *input, int w, int h, unsigned char *output,
		 BayerTile tile)
{
	int x, y, i;
	int colour, bayer;
	unsigned char *ptr = input;

	switch (tile) {

		case BAYER_TILE_RGGB:
		case BAYER_TILE_GRBG:
		case BAYER_TILE_BGGR:
		case BAYER_TILE_GBRG:
			for (y = 0; y < h; ++y)
				for (x = 0; x < w; ++x, ++ptr) {
					bayer = (x&1?0:1) + (y&1?0:2);

					colour = tile_colors[tile][bayer];

					i = (y * w + x) * 3;

					output[i+RED]    = 0;
					output[i+GREEN]  = 0;
					output[i+BLUE]   = 0;
					output[i+colour] = *ptr;
				}
			break;

		case BAYER_TILE_RGGB_INTERLACED:
		case BAYER_TILE_GRBG_INTERLACED:
		case BAYER_TILE_BGGR_INTERLACED:
		case BAYER_TILE_GBRG_INTERLACED:
			for (y = 0; y < h; ++y, ptr+=w)
				for (x = 0; x < w; ++x) {
					bayer = (x&1?0:1) + (y&1?0:2);

					colour = tile_colors[tile][bayer];

					i = (y * w + x) * 3;

					output[i+RED]    = 0;
					output[i+GREEN]  = 0;
					output[i+BLUE]   = 0;
					output[i+colour] = (x&1)? ptr[x>>1]:ptr[(w>>1)+(x>>1)];
				}
			break;
	}

	return 0;
}

#define AD(x, y, w) ((y)*(w)*3+3*(x))

int
gp_bayer_interpolate (unsigned char *image, int w, int h, BayerTile tile)
{
	int x, y, bayer;
	int p0, p1, p2, p3;
	int value, div ;

	switch (tile) {
	default:
	case BAYER_TILE_RGGB:
	case BAYER_TILE_RGGB_INTERLACED:
		p0 = 0; p1 = 1; p2 = 2; p3 = 3;
		break;
	case BAYER_TILE_GRBG:
	case BAYER_TILE_GRBG_INTERLACED:
		p0 = 1; p1 = 0; p2 = 3; p3 = 2;
		break;
	case BAYER_TILE_BGGR:
	case BAYER_TILE_BGGR_INTERLACED:
		p0 = 3; p1 = 2; p2 = 1; p3 = 0;
		break;
	case BAYER_TILE_GBRG:
	case BAYER_TILE_GBRG_INTERLACED:
		p0 = 2; p1 = 3; p2 = 0; p3 = 1;
		break;
	}

	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++) {
			bayer = (x&1?0:1) + (y&1?0:2);

			if ( bayer == p0 ) {

				/* red. green lrtb, blue diagonals */
				image[AD(x,y,w)+GREEN] =
					gp_bayer_accrue(image, w, h, x-1, y, x+1, y, x, y-1, x, y+1, GREEN) ;

				image[AD(x,y,w)+BLUE] =
					gp_bayer_accrue(image, w, h, x+1, y+1, x-1, y-1, x-1, y+1, x+1, y-1, BLUE) ;

			} else if (bayer == p1) {

				/* green. red lr, blue tb */
				div = value = 0;
				if (x < (w - 1)) {
					value += image[AD(x+1,y,w)+RED];
					div++;
				}
				if (x) {
					value += image[AD(x-1,y,w)+RED];
					div++;
				}
				image[AD(x,y,w)+RED] = value / div;

				div = value = 0;
				if (y < (h - 1)) {
					value += image[AD(x,y+1,w)+BLUE];
					div++;
				}
				if (y) {
					value += image[AD(x,y-1,w)+BLUE];
					div++;
				}
				image[AD(x,y,w)+BLUE] = value / div;

			} else if ( bayer == p2 ) {

				/* green. blue lr, red tb */
				div = value = 0;

				if (x < (w - 1)) {
					value += image[AD(x+1,y,w)+BLUE];
					div++;
				}
				if (x) {
					value += image[AD(x-1,y,w)+BLUE];
					div++;
				}
				image[AD(x,y,w)+BLUE] = value / div;

				div = value = 0;
				if (y < (h - 1)) {
					value += image[AD(x,y+1,w)+RED];
					div++;
				}
				if (y) {
					value += image[AD(x,y-1,w)+RED];
					div++;
				}
				image[AD(x,y,w)+RED] = value / div;

			} else {

				/* blue. green lrtb, red diagonals */
				image[AD(x,y,w)+GREEN] =
					gp_bayer_accrue (image, w, h, x-1, y, x+1, y, x, y-1, x, y+1, GREEN) ;

				image[AD(x,y,w)+RED] =
					gp_bayer_accrue (image, w, h, x+1, y+1, x-1, y-1, x-1, y+1, x+1, y-1, RED) ;
			}
		}

	return 0;
}

/* Accrue four surrounding values.  If three values are one side of the average value,
	the fourth value is ignored. This will sharpen up boundaries. B.R.Harris 13Nov03*/
int
gp_bayer_accrue (unsigned char *image, int w, int h, int x0, int y0,
		int x1, int y1, int x2, int y2, int x3, int y3, int colour)
{	int x [4] ;
	int y [4] ;
	int value [4] ;
	int above [4] ;
	int counter	; // counter
	int total_value	; // total value
	int average ;
	int i ;
	x[0] = x0 ; x[1] = x1 ; x[2] = x2 ; x[3] = x3 ;
	y[0] = y0 ; y[1] = y1 ; y[2] = y2 ; y[3] = y3 ;

	// special treatment for green
	counter = total_value = 0 ;
	if(colour == GREEN)
	{
	  	// we need to make sure that horizontal or vertical lines become horizontal
	  	// and vertical lines even in this interpolation procedure
	  	// therefore, we determine whether we might have such a line structure
	  	for (i = 0 ; i < 4 ; i++)
	  	{	if ((x[i] >= 0) && (x[i] < w) && (y[i] >= 0) && (y[i] < h))
			{
				value [i] = image[AD(x[i],y[i],w) + colour] ;
				counter++;
			}
			else
			{
				value [i] = -1 ;
			}
	  	}
		if(counter == 4)
		{
			// we must assume that x0,y0 and x1,y1 are on the horizontal axis and
			// x2,y2 and x3,y3 are on the vertical axis
			int hdiff ;
			int vdiff ;
			hdiff = value [1] - value [0] ;
			hdiff *= hdiff ;	// make value positive by squaring it
			vdiff = value [3] - value [2] ;
			vdiff *= vdiff ;	// make value positive by squaring it
			if(hdiff > 2*vdiff)
			{
				// we might have a vertical structure here
				return (value [3] + value [2])/2 ;
			}
			if(vdiff > 2*hdiff)
			{
				// we might have a horizontal structure here
				return (value [1] + value [0])/2 ;
			}
			// else we proceed as with blue and red
		}
		// if we do not have four points then we proceed as we do for blue and red
	}

	// for blue and red as it was before
	counter = total_value = 0 ;
	for (i = 0 ; i < 4 ; i++)
	{	if ((x[i] >= 0) && (x[i] < w) && (y[i] >= 0) && (y[i] < h))
		{	value [i] = image[AD(x[i],y[i],w) + colour] ;
			total_value += value [i] ;
			counter++ ;
		}
	}
	average = total_value / counter ;
	if (counter < 4) return average ;  // Less than four surrounding - just take average
	counter = 0 ;
	for (i = 0 ; i < 4 ; i++)
	{	above[i] = value[i] > average ;
		if (above[i]) counter++ ;
	}
	// Note: counter == 0 indicates all values the same
	if ((counter == 2) || (counter == 0)) return average ;
	total_value = 0 ;
	for (i = 0 ; i < 4 ; i++)
	{	if ((counter == 3) == above[i])
		{	total_value += value[i] ; }
	}
	return total_value / 3 ;
}


int
gp_bayer_decode (unsigned char *input, int w, int h, unsigned char *output,
		 BayerTile tile)
{
	gp_bayer_expand (input, w, h, output, tile);
	gp_bayer_interpolate (output, w, h, tile);
	return 0;
}


/* Gamma correction routine. Credit to libgphoto2/bayer.c, by Lutz M\"uller.
*/


int
gp_gamma_correct_triple (unsigned char *table_red,
			 unsigned char *table_green,
			 unsigned char *table_blue,
			 unsigned char *data, unsigned int size)
{
	int x;

	for (x = 0; x < (size * 3); x += 3) {
		data[x + 0] = table_red  [data[x + 0]];
		data[x + 1] = table_green[data[x + 1]];
		data[x + 2] = table_blue [data[x + 2]];
	}
	return 0;
}

int
gp_gamma_correct_single (unsigned char *table, unsigned char *data,
			 unsigned int size)
{
	return (gp_gamma_correct_triple (table, table, table, data, size));
}


int
gp_gamma_fill_table (unsigned char *table, double g)
{
	int x;
	for (x = 0; x < 256; x++)
		table[x] = 255 * pow ((double) x/255., g);
	return 0;
}

