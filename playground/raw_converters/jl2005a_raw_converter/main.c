/* main.c
 *
 * Copyright (C) 2007 Theodore Kilgore <kilgota@auburn.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
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
#include "jl2005a_process.h"

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


	int i,j;
	BYTE temp;
	BYTE *bufp;
	BYTE *ppm, *ptr;
	FILE *fp_src, *fp_dest;
	char *dest;
	int size;
	float gamma_factor;
	BYTE gtable[256];
	int b;
	BYTE *buf;
	BYTE *buf2;
	int WIDTH=176, HEIGHT=144;		

        GtkWidget *window;
	GtkWidget *image;
	GtkWidget *event_box;

	info = malloc(512);
	if (!info) { free(info); return -1; }

	fprintf (stderr, "Source file is %s\n", argv[1]);
	
	/* input file is raw_(some characters).raw, so, first thing, we find 
	 * the "." in the name.
	 */
	dest = malloc(strlen(argv[1]));
	if (!dest){free(dest);return -1;}

	i = strchr(argv[1], 0x2e) - argv[1];
	/* and throw away the period (to avoid clobbering any finished images
	 * from gphoto2 which are in the same directory) and throw away the 
	 * suffix.
	 */

	strncpy(dest, argv[1] + 4, i - 4);
	/* then affix "ppm" as a suffix (no preceding period) */
	strcat (dest, "ppm");
	fprintf (stderr, "Destination file will be called %s\n", dest);
	/* now open the raw file */
	if ( (fp_src = fopen(argv[1], "rb") ) == NULL )
	{
		fprintf (stderr, "Error opening src file.\n");
		return 0;
	}
	fprintf(stderr, "Source file %s successfully opened in mode %s.\n", argv[1], "r");

	/* get the size of the raw file and assign that size to "b" */    

	fseek(fp_src, 0, 2);
	b = ftell(fp_src);	
	fseek(fp_src, 0, 0);

	buf = malloc(b);
	if (!buf) return -1;

	/* read the raw file to "buf", and close it */
	fread (buf, 1, b, fp_src);
	fprintf(stderr, "size of %s is 0x%x\n", argv[1], b);
	fclose (fp_src);

	get_image_info( info, b);


	/* If there is a header, then we print it as debug output, then 
	 * move past it, to the data. 
	 */
	bufp = buf;
	/* show header */
	for (i = 0; i < HEADER_LEN; i++) {
		fprintf(stderr, " %02X", buf[i]);
	}
	fprintf(stderr, "\n");

	bufp += HEADER_LEN; /* now move past the header */

	WIDTH = info->width;
	printf("width is %i\n", WIDTH);
	HEIGHT = info->height;
	gamma_factor = info->gamma;
		fprintf(stderr, "gamma_factor = %f\n", gamma_factor);

	gp_gamma_fill_table(gtable, gamma_factor);


        if (WIDTH == 176) {

                for (i=1; i < HEIGHT; i +=4){
                        for (j=0; j< WIDTH; j ++){
                                temp=bufp[i*WIDTH+j];
                                bufp[i*WIDTH+j] = bufp[(i+1)*WIDTH+j];
                                bufp[(i+1)*WIDTH+j] = temp;
                        }
		}
        }

/*
        if (WIDTH == 176) {

                for (i=1; i < HEIGHT; i +=4){
                        for (j=0; j< WIDTH/2; j ++){
                                temp=bufp[i*WIDTH+2*j];
                                bufp[i*WIDTH+2*j] = bufp[(i+1)*WIDTH+2*j+1];
                                bufp[(i+1)*WIDTH+2*j+1] = temp;
                                temp=bufp[i*WIDTH+2*j+1];
                                bufp[i*WIDTH+2*j+1] = bufp[(i+1)*WIDTH+2*j];
                                bufp[(i+1)*WIDTH+2*j] = temp;

                        }
                }
        }
*/

	if (info->compression == 2) 
		HEIGHT *= 2;

	buf2 = malloc(WIDTH * HEIGHT+256);
	if (!buf2) {
		free (buf2);
		return -1;
	}
	
	if (info->compression == 2) /* non-zero if compression in use */
		jl2005a_decompress(bufp, buf2, WIDTH,HEIGHT);
	else 
		memcpy(buf2, bufp, WIDTH*HEIGHT);


	ppm = malloc (WIDTH * HEIGHT * 3 + 256); /* Data + header */
	if (!ppm) {
		free (buf2);
		printf("Exiting. Out of memory\n");
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

	gp_bayer_decode(buf2, WIDTH, HEIGHT, ptr, THIS_CAM_TILE);
	gp_gamma_correct_single(gtable,ptr,WIDTH*HEIGHT);

	if ( (fp_dest = fopen(dest, "wb") ) == NULL )
	{
		fprintf (stderr, "Error opening dest file.\n");
		return 0;
	}
	printf("Successful in creating %s in mode %s.\n", dest, "w");

	fwrite(ppm, 1, size, fp_dest);

	fclose (fp_dest);
	free (ppm);


        gtk_init (&argc, &argv);
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);


	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (window), event_box);
	gtk_widget_show(event_box);

/* This will let the Window Manager to close the window _and_ to exit. */

        g_signal_connect (G_OBJECT (window), "delete_event",
                          G_CALLBACK (delete_event), NULL);

	image = gtk_image_new_from_file(dest);
	gtk_container_add(GTK_CONTAINER(event_box), image);
	gtk_widget_show (image);
        gtk_widget_show (window);
        gtk_main ();

	return 0;
}
