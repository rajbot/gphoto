
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "main.h"
#include "gphoto.h"

#include "commandline.h"

char command_prefix[256];

void camera_summary()
{
    fprintf(stdout, "%s summary:\n%s", Camera->name,
	    Camera->ops->summary());
    fflush(stdout);
}

void command_usage()
{
    fprintf(stdout,
	    "gPhoto %s (%s) - the GNU digital camera application\n",
	    VERSION, __DATE__);
    fprintf(stdout,
	    "Copyright (C) 1998-99 Scott Fritzinger <scottf@unr.edu>\n\n");
    fprintf(stdout,
	    "Usage: gphoto [-h] [-c] [-n] [-s # filename] [-t # filename]\n");
    fprintf(stdout, "              [-d #] [-p filename] [-l filename]\n");
    fprintf(stdout, "\t-h                    display this help screen\n");
    fprintf(stdout, "\t-c                    display camera summary\n");
    fprintf(stdout, "\t-n                    display the # of pictures\n");
    fprintf(stdout, "\t-s # filename         save image # as filename\n");
    fprintf(stdout,
	    "\t-t # filename         save thumbnail # as filename\n");
    fprintf(stdout,
	    "\t-d #                  delete image # from camera\n");
    fprintf(stdout,
	    "\t-p filename           take picture and save as filename\n");
    fprintf(stdout,
	    "\t-l filename           save live preview as filename\n\n");
    fprintf(stdout,
	    "gPhoto is free GNU software; you can redistribute it and/or\n");
    fprintf(stdout,
	    "modify it under the terms of the GNU General Public License \n");
    fprintf(stdout,
	    "as published by the Free Software Foundation; either version \n");
    fprintf(stdout,
	    "2 of the License, or any later version.  Visit gphoto/COPYING\n");
    fprintf(stdout,
	    "in the source distribution to read the GNU GPL license terms.\n\n");
    fprintf(stdout,
	    "Report bugs and camera reports to gphoto-devel@gphoto.org.\n");
    fprintf(stdout,
	    "See http://www.gphoto.org for the latest news and updates.\n");
    _exit(0);
}

void command_line(int argc, char *argv[])
{
    int picNum;
    int i = 0;
    struct Image *im;
    FILE *fp;

    if (strcmp(argv[1], "-h") == 0) {
	command_usage();
    }
    i = 1;
    while (i < argc) {
	switch (argv[i][1]) {
	case 'n':
	    fprintf(stdout, "%i\n", Camera->ops->number_of_pictures());
	    break;
	case 'l':
	    if (argv[i + 1]) {
		if ((im = Camera->ops->get_preview()) == 0)
		    fprintf(stdout, "ERROR: could not get image.\n");
		else if ((fp = fopen(argv[i + 1], "w"))) {
		    fwrite(im->image,
			   (size_t) sizeof(char),
			   (size_t) im->image_size, fp);
		    fclose(fp);
		} else
		    fprintf(stdout, "ERROR: could not save image.\n");
	    } else {
		fprintf(stdout, "ERROR: 'filename' not specified.\n");
		command_usage();
	    }
	    i += 1;
	    break;
	case 's':
	    if (argv[i + 1] && argv[i + 2]) {
		fprintf(stdout,
			"Saving image %i from camera as %s: ",
			atoi(argv[i + 1]), argv[i + 2]);
		fflush(stdout);
		if (
		    ((im
		      =
		      Camera->ops->get_picture(atoi(argv[i + 1]),
					      0)) == 0))
		      fprintf(stdout, "ERROR: could not get image.\n");
		else if ((fp = fopen(argv[i + 2], "w"))) {
		    fwrite(im->image,
			   (size_t) sizeof(char),
			   (size_t) im->image_size, fp);
		    fclose(fp);
		} else {
		    fprintf(stdout, "ERROR: could not save image.\n");
		    return;
		}
	    } else {
		fprintf(stdout,
			"ERROR: '#' and 'filename' not specified.\n");
		command_usage();
	    }
	    fprintf(stdout, "\n");
	    fflush(stdout);
	    i += 2;
	    break;
	case 't':
	    if (argv[i + 1] && argv[i + 2]) {
		fprintf(stdout,
			"Saving thumbnail image %i from camera as %s: ",
			atoi(argv[i + 1]), argv[i + 2]);
		fflush(stdout);
		if (
		    (im =
		     Camera->ops->get_picture(atoi(argv[i + 1]),
					     1)) == 0)
		   fprintf(stdout, "ERROR: could not get image.\n");
		else if ((fp = fopen(argv[i + 2], "w"))) {
		    fwrite(im->image,
			   (size_t) sizeof(char),
			   (size_t) im->image_size, fp);
		    fclose(fp);
		} else {
		    fprintf(stdout, "ERROR: could not save image.\n");
		    return;
		}
	    } else {
		fprintf(stdout,
			"ERROR: '#' and 'filename' not specified.\n");
		command_usage();
	    }
	    fprintf(stdout, "\n");
	    i += 2;
	    break;
	case 'd':
	    if (argv[i + 1]) {
		if ((Camera->ops->delete_picture(atoi(argv[i + 1]))
		     == 0)) {
		    fprintf(stdout, "Could not delete image.\n");
		    return;
		} else {
		    fprintf(stdout,
			    "Deleted image %i from camera.\n",
			    atoi(argv[i + 1]));
		}
	    } else {
		fprintf(stdout, "ERROR: '#' not specified.\n");
		command_usage();
	    }
	    i += 1;
	    break;
	case 'p':
	    if (!argv[i + 1]) {
		fprintf(stdout, "ERROR: filename not specified.\n");
		command_usage();
	    } else {
		fprintf(stdout, "Taking picture...\n");

		picNum = Camera->ops->take_picture();

		if (picNum == 0) {
		    fprintf(stdout,
			    "ERROR: could not take the picture.\n");
		    return;
		} else {
		    fprintf(stdout,
			    "Saving the new image (# %i) as %s: ",
			    picNum, argv[i + 1]);
		}

		if (((im = Camera->ops->get_picture(picNum, 0)) == 0)) {
		    fprintf(stdout, "\nERROR: could not get image.\n");
		    return;
		} else {
		    if ((fp = fopen(argv[i + 1], "w"))) {
			fwrite(im->image, (size_t)
			       sizeof(char), (size_t) im->image_size, fp);
			fclose(fp);
		    } else {
			fprintf(stdout, "ERROR: could not save image.\n");
			return;
		    }
		}
	    }
	    fprintf(stdout, "\n");
	    fflush(stdout);
	    break;
	case 'c':
	    camera_summary();
	    break;
	default:
	    break;
	}
	i++;
    }
    fflush(stdout);
    _exit(0);
}
