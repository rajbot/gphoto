#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "main.h"
#include "gphoto.h"

#include "commandline.h"

char command_prefix[256];

void print_camera_models () {

	int i=1;
	int colwidth;

	printf("Supported camera models in gPhoto v.%s: \n", VERSION);

	while (strlen(cameras[i].name) > 0) {

		if (cameras[i].name[0] == cameras[i+1].name[0]
		    && cameras[i].name[3] == cameras[i+1].name[3]) {
			printf("%s, ", cameras[i].name);
		}
		else printf("%s\n", cameras[i].name);
		i++;
	}
	fflush(stdout);
}
	

void command_license () {
 	printf("gPhoto v.%s - the GNU digital camera application\n", VERSION);
 	printf("Copyright (C) 1998-99 Scott Fritzinger <scottf@unr.edu>\n\n");
        printf("gPhoto is free GNU software; you can redistribute it and/or\n");
        printf("modify it under the terms of the GNU General Public License \n");
        printf("as published by the Free Software Foundation; either version \n");
        printf("2 of the License, or any later version.  Visit gphoto/COPYING\n");
	printf("in the source distribution to read the GNU GPL license terms.\n\n");
	print_camera_models ();
        printf("\nReport bugs and camera reports to gphoto-devel@gphoto.org.\n");
	printf("See http://www.gphoto.org for the latest news and updates.\n");
	_exit(0);
}

void command_usage () {

 	printf("gPhoto v.%s - the GNU digital camera application\n", VERSION);
	printf("Covered by the GNU GPL.  Type \"gphoto -v\" for details.\n");
	printf("Usage: gphoto [-h] [-n] [-s # filename] [-t # filename]\n");
	printf("              [-d #] [-l filename]\n");
	printf("\t-n			display the # of pictures\n");
	printf("\t-s # filename		save image # as filename\n");
	printf("\t-t # filename		save thumbnail # as filename\n");
	printf("\t-d #			delete image # from camera\n");
	printf("\t-l filename		save live preview as filename\n");
	printf("\t-h 			display this help screen\n");
	printf("\t-v                      display information about the %s version\n", VERSION);
	_exit(0);
}

void command_line (int argc, char *argv[]) {

	int i=0;
	struct Image *im;
	FILE *fp;

	if (strcmp(argv[1], "-h") == 0)
		command_usage();

	i = 1;
	while (i < argc) {
		switch(argv[i][1]) {
		case 'n':
			printf("%i\n", (*Camera->number_of_pictures)());
			break;
		case 'l':
			if ((im = (*Camera->get_preview)()) == 0)
				printf("Error: could not get image.\n");
			   else
				if (fp = fopen(argv[i+1], "w")) {
					fwrite(im->image, (size_t)sizeof(char),
					       (size_t)im->image_size, fp);
					fclose(fp);}
				   else
					printf("Error: could not save image.\n");
			i+=1;
			break;
		case 's':
			printf("Saving Image: ");
			fflush(stdout);
			if ((im = (*Camera->get_picture)(atoi(argv[i+1]), 0)) 
			   == 0)
				printf("Error: could not get image.\n");
			   else
				if (fp = fopen(argv[i+2], "w")) {
					fwrite(im->image, (size_t)sizeof(char),
					       (size_t)im->image_size,fp);
					fclose(fp);}
				   else
					printf("Error: could not save image.\n");
			i+=2;
			printf("\n");
			break;
		case 't':
			if ((im=(*Camera->get_picture)(atoi(argv[i+1]),1))
			   == 0)
				printf("Error: could not get image.\n");
			   else
				if (fp = fopen(argv[i+2], "w")) {
					fwrite(im->image, (size_t)sizeof(char),
					       (size_t)im->image_size, fp);
					fclose(fp);}
				   else
					printf("Error: could not save image.\n");
			i+=2;
			break;
		case 'd':
			if ((*Camera->delete_picture)(atoi(argv[i+1])) == 0)
				printf("Could not delete image.\n");
			break;
		case 'v':
			command_license ();
			break;
		default:
			break;
		}
		i++;
	}
	_exit(0);
}
