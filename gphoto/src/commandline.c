#include "main.h"
#include "gphoto.h"
#include "commandline.h"

#include <stdlib.h>
#include <stdio.h>

extern struct _Camera *Camera;

char command_prefix[256];

void command_license () {
 	printf("GNU Photo v.0.3\n");
 	printf("Copyright (C) 1998 Scott Fritzinger <scottf@unr.edu>");
 	printf("Copyright (C) 1998 Ole Kristian Aamot <oleaa@ifi.uio.no>");
        printf("Report bugs and details on your camera to ");
 	printf("<gphoto-devel@lists.styx.net>.\n\n");
        printf("GNU Photo is free software; you can redistribute ");
 	printf("it and/or\n");
        printf("modify it under the terms of the ");
 	printf("GNU General Public License \n");
        printf("as published by the Free Software Foundation; ");
 	printf("either version \n");
        printf("2 of the License, or any later version.\n\n");
}

void command_usage () {

 	printf("GNU Photo v.0.3\n");
	printf("Covered by the GNU Public License. See \"man gphoto\" for details.\n");
	printf("Usage: gphoto [-h] [-n] [-s # filename] [-t # filename]\n");
	printf("              [-d #] [-l filename]\n");
	printf("\t-n			display the # of pictures\n");
	printf("\t-s # filename		save image # as filename\n");
	printf("\t-t # filename		save thumbnail # as filename\n");
	printf("\t-d #			delete image # from camera\n");
	printf("\t-l filename		save live preview as filename\n");
	printf("\t-h 			display this help screen\n");
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
				printf("%i\n",
				(*Camera->number_of_pictures)());
				break;
			case 'l':
				im = (*Camera->get_preview)();
				fp = fopen(argv[i+1], "w");
				fwrite(im->image, (size_t)sizeof(char),
				       (size_t)im->image_size, fp);
				fclose(fp);
				i+=1;
				break;
			case 's':
				im = (*Camera->get_picture)
					     (atoi(argv[i+1]), 0);
				fp = fopen(argv[i+2], "w");
				fwrite(im->image, (size_t)sizeof(char),
				       (size_t)im->image_size, fp);
				fclose(fp);
				i+=2;
				break;
			case 't':
				im = (*Camera->get_picture)
					     (atoi(argv[i+1]), 1);
				fp = fopen(argv[i+2], "w");
				fwrite(im->image, (size_t)sizeof(char),
				       (size_t)im->image_size, fp);
				fclose(fp);				
				i+=2;
				break;
			case 'd':
				if ((*Camera->delete_picture)
					(atoi(argv[i+1])) == 0)
					printf("Could not delete image.\n");
				break;
			default:
				break;
		}
		i++;
	}
	_exit(0);
}
