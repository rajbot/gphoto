#include <stdio.h>
#include <stdlib.h>

#include "qcam.h"
#include "qcam_gfx.h"

/******************************************************************/

scanbuf *qc_sobel (struct qcam *q, scanbuf *scan, int start)
{ static float conv_kernel [3][3] = {{1, 2, 1},
			          {2, -1, 2},
			          {1, 2, 1}};
  int gap;

  gap = (qc_getversion (q) == COLOR_QUICKCAM) ? 3 : 1;

  return (scan);
//qc_edge (q, scan, start, gap, conv_kernel, 3, 3));
}

/******************************************************************/

scanbuf *qc_edge (struct qcam *q, scanbuf *scan, int start, int gap, float *kernel, int kernel_x, int kernel_y)
{ scanbuf *scantmp;
  int i;
  int s, height, width,
    grad;
  float deltaX, deltaY;

 static float kernel [3][3] = {{1, 2, 1},
			          {2, -1, 2},
			          {1, 2, 1}};
  height = qc_getheight (q);
  width = qc_getwidth (q) * gap;

  scantmp = malloc (height*width);


  for (i = width+start; i < (height-1)*width; i+=gap) {
    deltaX = 
	scan[i-width+gap]*kernel[0][2] + 
	scan[i+gap]*kernel[1][2] + 
	scan[i+width+gap]*kernel[2][2] -
	scan[i-width-gap]*kernel[0][0] - 
	scan[i-gap]*kernel[1][0] - 
	scan[i+width-gap]*kernel[2][0];

    deltaY = 
	scan[i-width-gap]*kernel[0][0] + 
	scan[i-width]*kernel[1][0] + 
	scan[i-width+gap]*kernel[2][0] -
	scan[i+width-gap]*kernel[0][2] - 
	scan[i+width]*kernel[1][2] - 
	scan[i+width+gap]*kernel[2][2];


    grad = (abs(deltaX) + abs(deltaY) + scan[i]*kernel[1][1])/3;

    grad = (grad > 255) ? 255 : grad;

    for (s=0; s<gap; s++) {
      if (s==start) 
	scantmp[i] = grad;
      else
	scantmp[i+s-start] = scan[s+i-start];
    }
  }

  return (scantmp);
}

