/* show_sonix_raw.c
 *
 * Copyright  2008 Theodore Kilgore
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gamma.h"
#include "bayer.h"
#include "sonix_process.h"

#include <gtk/gtk.h>


gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
		   gpointer   data )
{
    gtk_main_quit ();
    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete_event". */

    return FALSE;
}

int load_image( GtkWidget *window, GtkWidget *image  )
{

	

	gtk_container_add(GTK_CONTAINER(window), image);
	gtk_widget_show (image);

        gtk_widget_show (window);
        gtk_main ();
        return 0;
}


int main(int argc, char *argv[])
{
	
	image_info *info;


	int i;
	BYTE *bufp;
	BYTE *ppm, *ptr;
	FILE *fp_src, *fp_dest;
	char *dest;
	BYTE invert=0;
	BYTE qvga = 0;
	int size;
	float gamma_factor;
	BYTE gtable[256];
	int b;
	BYTE *buf;
	BYTE *buf2;
	int WIDTH=176, HEIGHT=144;
	BYTE outdoors;
        GtkWidget *window;
	GtkWidget *image;
	GtkWidget *event_box;
	int current = 1;
	int offset_correct = 0;

	if ( !(argc > 1) ){
		printf("Syntax: show_sonix_raw sourcefile, or \n");
		printf("Syntax: show_sonix_raw -useoffset sourcefile, or \n");
		printf("Syntax: show_sonix_raw -useoffset -invert sourcefile\n");
		return 0;
	}

	if ((strcmp(argv[current], "-qvga") == 0) && (current+1 < argc)) 
	{ 
		qvga = 1;
		offset_correct=8;
		current += 1;
		if (argc != 3) {
			printf("Syntax: show_sonix_raw -qvga sourcefile\n");
			return 0;
		}
	}


	if ( !(argc > 2) && (strcmp(argv[current], "-useoffset") == 0) )
	{
		printf("Syntax: show_sonix_raw sourcefile, or \n");
		printf("Syntax: show_sonix_raw -useoffset sourcefile \n");
		printf("Syntax: show_sonix_raw -useoffset -invert sourcefile\n");
		return 0;
	}

		printf("argc=%i\n", argc);


	if ((strcmp(argv[current], "-useoffset") == 0) && (current+1 < argc)) {
		offset_correct = 8;
		current += 1;
		printf("useoffset option used\n");
		printf("current=%i\n", current);
		if ( !(argc > 2) || !(argc < 5) ){
			printf("Syntax: show_sonix_raw sourcefile, or \n");
			printf("Syntax: show_sonix_raw -useoffset sourcefile, or \n");
			printf("Syntax: show_sonix_raw -useoffset -invert sourcefile, or\n");
			printf("Syntax: show_sonix_raw -qvga sourcefile\n");
			return 0;
		}
	}

	if ( !(argc > 3) && (strcmp(argv[current], "-invert") == 0) )
	{
		printf("Syntax: show_sonix_raw sourcefile, or \n");
		printf("Syntax: show_sonix_raw -useoffset sourcefile, or \n");
		printf("Syntax: show_sonix_raw -useoffset -invert sourcefile\n");
		return 0;
	}

	if ((strcmp(argv[current], "-invert") == 0) && (current+1 < argc)) {
		invert = 1;
		current += 1;
		printf("invert option used\n");
		printf("current=%i\n", current);
		if ( !(argc > 3) || !(argc < 5) ){
			printf("Syntax: show_sonix_raw sourcefile, or \n");
			printf("Syntax: show_sonix_raw -useoffset sourcefile, or \n");
			printf("Syntax: show_sonix_raw -useoffset -invert sourcefile \n");
			return 0;
		}

	}

	printf("Options: offset=%i invert=%i\n", offset_correct, invert);


	fprintf (stderr, "Name of source file is %s\n", argv[current]);
	
	/* input file is raw_(some characters).raw, so, first thing, we find 
	 * the "." in the name.
	 */

	dest = malloc(strlen(argv[current]));
	if (!dest){
		free(dest);
		return -1;
	}

	if ((!strchr(argv[current], 0x2e))|| (strncmp(argv[current], "raw", 3))) {
		fprintf (stderr, "Source file appears not to be a gphoto2 raw file.\n");
		fprintf (stderr, "Its name should begin with raw_ and end with .raw\n");		
		fprintf (stderr, "Exiting.\n");
		return 0; 
	}

	i = strchr(argv[current], 0x2e) - argv[current];

	/* and throw away the period (to avoid clobbering any finished images
	 * from gphoto2 which are in the same directory) and throw away the 
	 * suffix.
	 */

	strncpy(dest, argv[current] + 4, i - 4);
	/* then affix "ppm" as a suffix (no preceding period) */
	strcat (dest, "ppm");
	fprintf (stderr, "Destination file will be called %s\n", dest);
	/* now open the raw file */
	if ( (fp_src = fopen(argv[current], "rb") ) == NULL )
	{
		fprintf (stderr, "Error opening source file.\n");
		return 0;
	}
	/* get the size of the raw file and assign that size to "b" */    
	fseek(fp_src, 0, 2);
	b = ftell(fp_src);
	fseek(fp_src, 0, 0);

	buf = malloc(b);
	if (!buf) return -1;

	/* read the raw file to "buf", and close it */
	fread (buf, 1, b, fp_src);
	fclose (fp_src);

	info = malloc(512);
	if (!info) { free(info); return -1; }


	get_image_info( info, buf, b, qvga);

	/* If there is a header, then we print it as debug output, then 
	 * move past it, to the data. 
	 */
	bufp = buf+offset_correct;
	/* show header */
	for (i = 0; i < HEADER_LEN; i++) {
		fprintf(stderr, " %02X", buf[i]);
	}
	fprintf(stderr, "\n");

	WIDTH = info->width;
	HEIGHT = info->height;
	outdoors = info->outdoors;
	gamma_factor = info->gamma;
	gp_gamma_fill_table(gtable, gamma_factor);
	buf2 = malloc(WIDTH * HEIGHT+256);
	if (!buf2) {
		free (buf2);
		return -1;
	}
	if((offset_correct)&&!(qvga)) {
		info->reverse = 1;
		info->bayer = BAYER_TILE_BGGR;
	}
	if (info->compression) { /* non-zero if compression in use */
		sonix_decode(buf2, bufp, WIDTH,HEIGHT);	
	 } else {
		memcpy(buf2, bufp, WIDTH*HEIGHT);
	}
	free(buf);
	if (invert)
		reverse_bytes(buf2, WIDTH*HEIGHT);
	ppm = malloc (WIDTH * HEIGHT * 3 + 256); /* Data + header */
	if (!ppm) {
		free (buf2);
		return 0;
	}

	memset (ppm, 0, WIDTH * HEIGHT * 3 + 256);

	sprintf ((char *)ppm,
        	"P6\n"
        	"#CREATOR: My_decoder\n"
        	"%d %d\n"
        	"255\n", WIDTH, HEIGHT);

        ptr = ppm + strlen ((char *)ppm);
        size = strlen ((char *)ppm) + (WIDTH * HEIGHT * 3);
	gp_bayer_decode(buf2, WIDTH, HEIGHT, ptr, info->bayer);
	free(buf2);
        white_balance(ptr, WIDTH * HEIGHT, 1.2);

	if ( (fp_dest = fopen(dest, "wb") ) == NULL )
	{
		fprintf (stderr, "Error opening dest file.\n");
		return 0;
	}
	fwrite(ppm, 1, size, fp_dest);
	fclose (fp_dest);
	free (ppm);

	/* The file has been saved. The GUI effects follow now. */
        gtk_init (&argc, &argv);
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);


	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (window), event_box);
	gtk_widget_show(event_box);

	/* This lets the Window Manager to close the window _and_ sets up 
	 * this program to exit when the window is closed. 
	 */

        g_signal_connect (G_OBJECT (window), "delete_event",
                          G_CALLBACK (delete_event), NULL);
	/* This displays the image file. */
	image = gtk_image_new_from_file(dest);
	gtk_container_add(GTK_CONTAINER(event_box), image);
	gtk_widget_show (image);
        gtk_widget_show (window);
        gtk_main ();
	free(info);
	free(dest);
	return 0;
}
