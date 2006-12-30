#include <stdio.h>
#include <stdlib.h>

#include "qcam.h"
#include "qcam_gfx.h"

/******************************************************************/
/* This is a bit of a hack to write pgm files.  I should probably use
 * libpgm, but I don't have the documentation in front of me, and the
 * format is pretty trivial, so I hacked this together with help from
 * a few people. */

void qc_writepgm(struct qcam *q, FILE *f ,scanbuf *scan)
{
  int i, pos, x, y,
    lines = 0,
    pixels_per_line = 0;
  scanbuf *buf_ptr = scan;


  if (q->cam_version == BW_QUICKCAM) {
	//lines = q->height / q->transfer_scale;
	lines = q->height;
	//pixels_per_line = q->width / q->transfer_scale;
	pixels_per_line = q->width;
	fprintf(f,"P5\n");
	fprintf(f,"%d %d\n", pixels_per_line, lines);
	fprintf(f,"%d\n",(q->bpp==4)?15:63);

	for (i = 0; i < pixels_per_line * lines; i++) {
		fputc(scan[i],f);
	}
  } else if (q->cam_version == COLOR_QUICKCAM) {
	fprintf (f, "P3\n");
	fprintf (f, "%d %d 255\n", q->width, q->height);
	pos = 1;

	for (y=0; y<q->height; y++) {
		for (x=0; x<q->width; x++, pos++) {
			for (i =0; i<3; i++, buf_ptr++) {
				fprintf (f, "%d ", *buf_ptr);
			}
		
			fprintf (f, " ");
			if (pos == 5) {
				printf ("\n");
				pos = 0;
			}
		}
	}
  }
}

/******************************************************************/

void qc_writeppm(struct qcam *q, FILE *f, scanbuf *scan)
{
  //int eheight = q->height / q->transfer_scale;
  //int ewidth = q->width / q->transfer_scale;
  int eheight = q->height;
  int ewidth = q->width;
  int bytes = eheight * ewidth * 3;

  fprintf (f, "P6\n%d %d 255\n", ewidth, eheight);

  while (bytes--)
    putc (*scan++, f);    

}
