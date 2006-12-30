#include "main.h"
#include "gphoto.h"
#include "commandline.h"

#include <stdlib.h>
#include <stdio.h>



char command_prefix[256];

void command_license () {
 	printf("GNU Photo v.0.2\n");
 	printf("Copyright (C) 1998 Scott Fritzinger ");
 	printf("<scottf@scs.unr.edu> (code, generic support)\n");
 	printf("Copyright (C) 1998 Ole Kristian Aamot ");
 	printf("<oleaa@ifi.uio.no> (testing, FAQ/web)\n\n");
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

 	printf("GNU Photo v.0.2\n");
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
	GdkImlibImage *imlibimage;

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
				imlibimage = (*Camera->get_preview)();
				if (gdk_imlib_save_image(
				imlibimage, argv[i+1], NULL)  == 0)
					printf("Could not save image.\n");
				gdk_imlib_kill_image(imlibimage);
				i++;
				break;
			case 's':
				imlibimage = (*Camera->get_picture)
					     (atoi(argv[i+1]), 0);
				if (gdk_imlib_save_image(
				imlibimage, argv[i+2], NULL)  == 0)
					printf("Could not save image.\n");
				gdk_imlib_kill_image(imlibimage);
				i+=2;
				break;
			case 't':
				imlibimage = (*Camera->get_picture)
					     (atoi(argv[i+1]), 1);
				if (gdk_imlib_save_image(
				imlibimage, argv[i+2], NULL)  == 0)
					printf("Could not save image.\n");
				gdk_imlib_kill_image(imlibimage);
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
	exit(0);
}
