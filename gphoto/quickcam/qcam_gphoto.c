#include <stdio.h>
#include <stdlib.h>

#include "qcam.h"
#include "../src/gphoto.h"
#include "../src/util.h"

#define	qcam_num_pictures_max	12

int 		qcam_initialize ();
void 		qcam_populate();
struct Image   *qcam_get_picture (int picNum, int thumbnail);
struct Image   *qcam_get_preview ();
int 		qcam_take_picture ();
int 		qcam_delete_picture (int picNum);
int 		qcam_number_of_pictures ();
int 		qcam_configure();
char 	       *qcam_summary();
char            *qcam_description();

struct qcam   *qcam_fd;
struct Image  *qcam_pictures[12];
int 	       qcam_num_pictures=0;

int qcam_initialize () {

	int x;

	qcam_fd = qc_init();
	qc_initfile (qcam_fd, NULL);
}

void qcam_populate() {

	int x;

	for (x=0; x<qcam_num_pictures; x++) {
		free_image(qcam_pictures[x]);
	}

	qcam_num_pictures = qcam_num_pictures_max;
	for (x=0; x<qcam_num_pictures; x++) {
		update_progress (100 * x/11);
		qcam_pictures[x] = qcam_get_preview();
	}
	update_progress(0);
}

struct Image *qcam_get_picture (int picNum, int thumbnail) {

	struct Image *im;
	int x;

        im = (struct Image*)malloc(sizeof(struct Image));
        im->image_size = qcam_pictures[picNum-1]->image_size;
	im->image = (char*)malloc(sizeof(char)*im->image_size);
	for (x=0; x<im->image_size; x++) 
		im->image[x] = qcam_pictures[picNum-1]->image[x];
	strcpy(im->image_type, "ppm");
        im->image_info_size = 0;
	return (im);
}

struct Image *qcam_get_preview () {

	scanbuf *buf;
	FILE *tempfile;
	int tempfile_size;
	char filename[1024];
	struct Image *im;

	sprintf(filename, "%s/gphoto-qcam.ppm", gphotoDir);
	tempfile = fopen(filename, "w");

	if (qc_open (qcam_fd)) {
		fprintf (stderr, "An error occured opening QCAM.\n");
		exit (-1);
	}

	qc_set (qcam_fd);
	buf = qc_scan (qcam_fd);

	switch (qc_getversion (qcam_fd)) {
		case COLOR_QUICKCAM:
			buf = qc_scale (buf, 3,
			qc_getwidth(qcam_fd)/qc_gettransfer_scale(qcam_fd),
			qc_getheight(qcam_fd)/qc_gettransfer_scale(qcam_fd),
			qc_getwidth(qcam_fd),qc_getheight(qcam_fd));
			
			qc_writeppm (qcam_fd, tempfile, buf);
			break;

		case BW_QUICKCAM:
			qc_writepgm (qcam_fd, tempfile, buf);
			break;
	}
	fclose(tempfile);
	free (buf);
	qc_close (qcam_fd);

	tempfile = fopen(filename, "r");
        fseek(tempfile, 0, SEEK_END);
        tempfile_size = ftell(tempfile);
        rewind(tempfile);
        im = (struct Image*)malloc(sizeof(struct Image));
        im->image = (char *)malloc(sizeof(char)*tempfile_size);
        fread(im->image,(size_t)sizeof(char),(size_t)tempfile_size,tempfile);
        strcpy(im->image_type, "ppm");
        im->image_size = (int)tempfile_size;
        im->image_info_size = 0;

        fclose(tempfile);
        remove(filename);
        return (im);
}

int qcam_take_picture () {
	return 0;
}

int qcam_delete_picture (int picNum) {
	return 0;
}

int qcam_number_of_pictures () {

	qcam_populate(); /* get some pics */

	return (qcam_num_pictures);
}

int qcam_configure () {
	return 0;
}

char *qcam_summary () {
	return ("Not Supported");
}

char *qcam_description () {
	return (
"QuickCam I/II
Scott Fritzinger <scottf@unr.edu>

Takes concurrent frames from the quickcam
and displays them in the index.");
}

struct _Camera quickcam = {qcam_initialize,
                           qcam_get_picture,
                           qcam_get_preview,
                           qcam_delete_picture,
                           qcam_take_picture,
                           qcam_number_of_pictures,
                           qcam_configure,
                           qcam_summary,
                           qcam_description};
