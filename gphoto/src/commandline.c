
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "main.h"
#include "gphoto.h"

#include "commandline.h"

char command_prefix[256];

void camera_summary()
{
    fprintf(stdout, N_("%s summary:\n%s"), camera_model,
	    (*Camera->summary) ());
    fflush(stdout);
}

void command_usage()
{
    fprintf(stdout,
	    N_("gPhoto %s (%s) - the GNU digital camera application\n"),
	    VERSION, __DATE__);
    fprintf(stdout,
	    N_("Copyright (C) 1998-2000 Scott Fritzinger <scottf@unr.edu>\n\n"));
    fprintf(stdout,
	    N_("Usage: gphoto [-h] [-c] [-n] [-s # filename] [-t # filename]\n"));
    fprintf(stdout, N_("              [-d #] [-p filename] [-l filename]\n"));
    fprintf(stdout, N_("\t-h                    display this help screen\n"));
    fprintf(stdout, N_("\t-c                    display camera summary\n"));
    fprintf(stdout, N_("\t-n                    display the # of pictures\n"));
    fprintf(stdout, N_("\t-s # filename         save image # as filename\n"));
    fprintf(stdout,
	    N_("\t-t # filename         save thumbnail # as filename\n"));
    fprintf(stdout,
	    N_("\t-d #                  delete image # from camera\n"));
    fprintf(stdout,
	    N_("\t-p filename           take picture and save as filename\n"));
    fprintf(stdout,
	    N_("\t-l filename           save live preview as filename\n\n"));
    fprintf(stdout,
	    N_("gPhoto is free GNU software; you can redistribute it and/or\n"));
    fprintf(stdout,
	    N_("modify it under the terms of the GNU General Public License \n"));
    fprintf(stdout,
	    N_("as published by the Free Software Foundation; either version \n"));
    fprintf(stdout,
	    N_("2 of the License, or any later version.  Visit gphoto/COPYING\n"));
    fprintf(stdout,
	    N_("in the source distribution to read the GNU GPL license terms.\n\n"));
    fprintf(stdout,
	    N_("Report bugs and camera success to gphoto-devel@gphoto.org.\n"));
    fprintf(stdout,
	    N_("See http://www.gphoto.org for the latest news and updates.\n"));
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
	    fprintf(stdout, N_("%i\n"), (*Camera->number_of_pictures) ());
	    break;
	case 'l':
	    if (argv[i + 1]) {
		if ((im = (*Camera->get_preview) ()) == 0)
		    fprintf(stderr, N_("ERROR: could not get image.\n"));
		else if ((fp = fopen(argv[i + 1], "w"))) {
		    fwrite(im->image,
			   (size_t) sizeof(char),
			   (size_t) im->image_size, fp);
		    fclose(fp);
		} else
		    fprintf(stderr, N_("ERROR: could not save image.\n"));
	    } else {
		fprintf(stderr, N_("ERROR: 'filename' not specified.\n"));
		command_usage();
	    }
	    i += 1;
	    break;
	case 's':
	    if (argv[i + 1] && argv[i + 2]) {
		fprintf(stderr,
			"Saving image %i from camera as %s: ",
			atoi(argv[i + 1]), argv[i + 2]);
		fflush(stderr);
		if (
		    ((im
		      =
		      (*Camera->get_picture) (atoi(argv[i + 1]),
					      0)) == 0))
		      fprintf(stderr, N_("ERROR: could not get image.\n"));
		else if ((fp = fopen(argv[i + 2], "w"))) {
		    fwrite(im->image,
			   (size_t) sizeof(char),
			   (size_t) im->image_size, fp);
		    fclose(fp);
		} else {
		    fprintf(stderr, N_("ERROR: could not save image.\n"));
		    return;
		}
	    } else {
		fprintf(stderr,
			N_("ERROR: '#' and 'filename' not specified.\n"));
		command_usage();
	    }
	    fprintf(stderr, "\n");
	    fflush(stderr);
	    i += 2;
	    break;
	case 't':
	    if (argv[i + 1] && argv[i + 2]) {
		fprintf(stderr,
			"Saving thumbnail image %i from camera as %s: ",
			atoi(argv[i + 1]), argv[i + 2]);
		fflush(stderr);
		if (
		    (im =
		     (*Camera->get_picture) (atoi(argv[i + 1]),
					     1)) == 0)
		   fprintf(stderr, N_("ERROR: could not get image.\n"));
		else if ((fp = fopen(argv[i + 2], "w"))) {
		    fwrite(im->image,
			   (size_t) sizeof(char),
			   (size_t) im->image_size, fp);
		    fclose(fp);
		} else {
		    fprintf(stderr, N_("ERROR: could not save image.\n"));
		    return;
		}
	    } else {
		fprintf(stderr,
			N_("ERROR: '#' and 'filename' not specified.\n"));
		command_usage();
	    }
	    fprintf(stderr, "\n");
	    i += 2;
	    break;
	case 'd':
	    if (argv[i + 1]) {
		if (((*Camera->delete_picture) (atoi(argv[i + 1]))
		     == 0)) {
		    fprintf(stderr, N_("ERROR: Could not delete image.\n"));
		    return;
		} else {
		    fprintf(stderr,
			    "Deleted image %i from camera.\n",
			    atoi(argv[i + 1]));
		}
	    } else {
		fprintf(stderr, N_("ERROR: '#' not specified.\n"));
		command_usage();
	    }
	    i += 1;
	    break;
	case 'p':
	    if (!argv[i + 1]) {
		fprintf(stderr, N_("ERROR: filename not specified.\n"));
		command_usage();
	    } else {
		fprintf(stderr, N_("Taking picture...\n"));

		picNum = (*Camera->take_picture) ();

		if (picNum == 0) {
		    fprintf(stderr,
			    N_("ERROR: could not take the picture.\n"));
		    return;
		} else {
		    fprintf(stderr,
			    N_("Saving the new image (# %i) as %s: "),
			    picNum, argv[i + 1]);
		}

		if (((im = (*Camera->get_picture) (picNum, 0)) == 0)) {
		    fprintf(stderr, N_("\nERROR: could not get image.\n"));
		    return;
		} else {
		    if ((fp = fopen(argv[i + 1], "w"))) {
			fwrite(im->image, (size_t)
			       sizeof(char), (size_t) im->image_size, fp);
			fclose(fp);
		    } else {
			fprintf(stderr, N_("ERROR: could not save image.\n"));
			return;
		    }
		}
	    }
	    fprintf(stderr, "\n");
	    fflush(stderr);
	    break;
	case 'c':
	    camera_summary();
	    break;
	default:
	    break;
	}
	i++;
    }
    fflush(stderr);
    _exit(0);
}
