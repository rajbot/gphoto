/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* 
 * This filter scales an image into a new image.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcam.h"
#include "qcam_gfx.h"

scanbuf *qc_scale (scanbuf *src, int planes, int src_width, int src_height, int dst_width, int dst_height)
{
  unsigned char *s;
  unsigned char *d;
  float *row, *r;
  int src_row, src_col;
  int b;
  float x_rat, y_rat;
  float x_cum, y_cum;
  float x_last, y_last;
  float *x_frac, y_frac, tot_frac;
  int i, j;
  int frac;
  int advance_dest;
  scanbuf *dst;
  scanbuf *dst_img;

  /*  the data pointers...  */
  dst = malloc (sizeof (scanbuf) * dst_width * dst_height * planes);
  dst_img = dst;

  /*  find the ratios of old x to new x and old y to new y  */
  x_rat = (float) src_width / (float) dst_width;
  y_rat = (float) src_height / (float) dst_height;

  /*  allocate an array to help with the calculations  */
  row    = (float *) malloc (sizeof (float) * dst_width * planes);
  x_frac = (float *) malloc (sizeof (float) * (dst_width + src_width));

  /*  initialize the pre-calculated pixel fraction array  */
  src_col = 0;
  x_cum = (float) src_col;
  x_last = x_cum;

  for (i = 0; i < dst_width + src_width; i++) {
    if (x_cum + x_rat <= src_col + 1) {
      x_cum += x_rat;
      x_frac[i] = x_cum - x_last;
    }
    else {
      src_col ++;
      x_frac[i] = src_col - x_last;
    }
    x_last += x_frac[i];
  }

  /*  clear the "row" array  */
  memset (row, 0, sizeof (float) * dst_width * planes);

  /*  counters...  */
  src_row = 0;
  y_cum = (float) src_row;
  y_last = y_cum;

  /*  Scale the selected region  */
  i = dst_height;
  while (i)
    {
      src_col = 0;
      x_cum = (float) src_col;

      /* determine the fraction of the src pixel we are using for y */
      if (y_cum + y_rat <= src_row + 1)
	{
	  y_cum += y_rat;
	  y_frac = y_cum - y_last;
	  advance_dest = 1;
	}
      else
	{
	  src_row ++;
	  y_frac = src_row - y_last;
	  advance_dest = 0;
	}
      y_last += y_frac;

      s = src;
      r = row;

      frac = 0;
      j = dst_width;

      while (j)
	{
	  tot_frac = x_frac[frac++] * y_frac;

	  for (b = 0; b < planes; b++)
	    r[b] += s[b] * tot_frac;

	  /*  increment the destination  */
	  if (x_cum + x_rat <= src_col + 1)
	    {
	      r += planes;
	      x_cum += x_rat;
	      j--;
	    }

	  /* increment the source */
	  else
	    {
	      s += planes;
	      src_col++;
	    }

	}

      if (advance_dest)
	{
	  tot_frac = 1.0 / (x_rat * y_rat);

	  /*  copy "row" to "dest"  */
	  d = dst;
	  r = row;

	  j = dst_width;
	  while (j--)
	    {
	      b = planes;
	      while (b--)
		*d++ = (unsigned char) (*r++ * tot_frac);
	    }

	  dst += dst_width * planes;

	  /*  clear the "row" array  */
	  memset (row, 0, sizeof (float) * dst_width * planes);

	  i--;
	}
      else
	src += src_width * planes;
      
    }

  /*  free up temporary arrays  */
  free (row);
  free (x_frac);

  return (dst_img);
}
