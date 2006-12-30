/* histogram.c
 *
 * Time-stamp: <01 Sep 96 12:00:01 HST edo@eosys.com>
 * Time-stamp: <22 Oct 96 09:04:12 MET dent@cosy.sbg.ac.at
 *
 * Version 0.1
 * Version 0.1a
 */


/******************************************************************

Copyright (C) 1996 by Ed Orcutt Systems

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, and/or distribute copies of the
Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall
   be included in all copies or substantial portions of the
   Software.

2. Redistribution for profit requires the express, written
   permission of the author.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.  IN NO EVENT SHALL ED ORCUTT SYSTEMS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcam.h"
#include "qcam_gfx.h"

int *qc_gethistogram (struct qcam *q, scanbuf *scan)
{
  int i, j;
  int width, height;
  int maxpixval;
  int grey_val;
  int *histogram;

  /* actual image width & height after scaling */
  width = q->width / q->transfer_scale;
  height = q->height / q->transfer_scale;

  /* calculate maximum pixel value: 2**bpp */
  maxpixval = (q->bpp > 7) ? 0xff: (1 << q->bpp);

  /* allocate space for histogram array */
  histogram = malloc ((maxpixval + 1) * sizeof (int));

  /* initialize to zero */
  bzero (histogram, sizeof (int) * (maxpixval + 1));
 
  /* if you don't have bzero use this ... */
  /* memset (histogram, 0, sizeof (int) * maxpixel); */

  /* for each pixel value increment it's tally */

  if (qc_getversion (q) == BW_QUICKCAM) {
    for (i = width - 1; i >= 0; i--) {
      for (j = height - 1; j >= 0; j--) {
        grey_val = scan [(j*width)+i];

	histogram [grey_val]++;
      }
    }
  }

  else if (qc_getversion (q) == COLOR_QUICKCAM) {
    for (i = width - 1; i >= 0; i--) {
      for (j = height - 1; j >= 0; j--) {
	grey_val = (scan [(3*j*width)+3*i] + scan [(3*j*width)+3*i+1] + scan [(3*j*width)+3*i+2]) / 3;

	histogram [grey_val]++;
      }
    }
  }

  return histogram;
}

/*
 * Caveat: Assume image height >= 2**bpp :-)
 */

void qc_histogram (struct qcam *q, scanbuf *scan)
{
  int i, j;
  int *histogram;     /* tally of pixel values */
  int maxpixval;      /* greatest pixel value */
  int maxhisval;      /* greatest histogram tally value */
  int hiswidth, hisheight;       /* width of histogram */
  int hispixval;      /* color(?) of histogram bars */
  int amplitude;      /* ampiltude of the histogram-point */
  int width, height;

  /* actual image width & height after scaling */
  width = q->width / q->transfer_scale;
  height = q->height / q->transfer_scale;

  /* calculate maximum pixel value: 2**bpp */
  maxpixval = (q->bpp > 7) ? 0xff: (1 << q->bpp);

  /* use 1/5 of the image width to display the histogram */
  hiswidth = (qc_getversion (q) == COLOR_QUICKCAM) ? width / 5 * 3 : width / 5;

  /* use approximately 1/2 of the image height to display the histogram */
  hisheight = (qc_getversion (q) == COLOR_QUICKCAM) ? height / 2 : height / 2;

  /* color the histogram bars with medium value */
  hispixval = maxpixval / 2 + 1;

  histogram = qc_gethistogram (q, scan);

  /* in order to scale the histogram we need to know the max value */
  for (maxhisval = 1, i = 0; i <= maxpixval; i++) {
    if (histogram[i] > maxhisval)
      maxhisval = histogram[i];
  }

  /* overlay the histogram in the upper-left of the image*/

  if (qc_getversion (q) == BW_QUICKCAM) {
    for (j = 0; j < maxpixval; j++) {
      for (i = (histogram[j]*hiswidth/maxhisval)-1; i >=0; i--)
	scan[(j*width)+i] = hispixval;
      for (i = (histogram[j]*hiswidth/maxhisval); i <= hiswidth; i++)
	scan[(j*width)+i] = 0;
    }
  
    /* let's add a border to the histogram, make it more visible! */
    j = maxpixval * width;
    for (i = 0; i < hiswidth; i++)
      scan[j+i] = 0;
  }

  else if (qc_getversion (q) == COLOR_QUICKCAM) {
    for (j = 0; j < hisheight; j++) {
      for (i = 0, amplitude = 0; i < (maxpixval/hisheight); i++)
	amplitude += histogram[j*maxpixval/hisheight+i];

      amplitude /= (maxpixval/hisheight);
      for (i = 0; i < (amplitude*hiswidth/maxhisval); i++)
	scan [(j*width*3)+i] = scan [(3*j*width)+i+1] = scan [(3*j*width)+i+2] = hispixval;

/* draw lthe black background */
//for (; i <= hiswidth; i++)
//scan [(j*width*3)+i] = scan [(3*j*width)+i+1] = scan [(3*j*width)+i+2] = 0;
      
    }
  }

  free (histogram);
}

