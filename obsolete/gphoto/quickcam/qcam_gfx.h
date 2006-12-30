#ifndef __QCAM__
#include <qcam.h>
#endif

#ifndef __QCAM_GFX__
#define __QCAM_GFX__

void qc_writepgm(struct qcam *q, FILE *f,scanbuf *scan);
void qc_writeppm (struct qcam *q, FILE *f, scanbuf *scan);

int *qc_gethistogram (struct qcam *q, scanbuf *scan);
void qc_histogram (struct qcam *q, scanbuf *scan);

int qc_autobrightness (struct qcam *q, scanbuf *scan);
scanbuf *qc_grey (struct qcam *q, int bpp, scanbuf *scan);
scanbuf *qc_normalize (struct qcam *q, scanbuf *scan);
scanbuf *qc_sobel (struct qcam *q, scanbuf *scan, int color);
scanbuf *qc_edge (struct qcam *q, scanbuf *scan, int color, int gap, float *kernel, int kernel_x, int kernel_y);
scanbuf *qc_scale (scanbuf *src, int planes, int src_width, int src_height, int dst_width, int dst_height);
#endif

