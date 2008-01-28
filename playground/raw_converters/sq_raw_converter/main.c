/* main.c 
 *
 * Copyright © 2008 Theodore Kilgore
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
#include "sq_process.h"

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

	if(argc <= 1) {
		fprintf(stderr, "USAGE: show_sq_raw (name of an SQ raw file)\n");
		return 0;
	}
	fprintf (stderr, "Source file is %s\n", argv[1]);
	
	/* input file is raw_(some characters).raw, so, first thing, we find 
	 * the "." in the name.
	 */
	dest = malloc(strlen(argv[1]));
	if (!dest){free(dest);return -1;}

	i = strchr(argv[1], 0x2e) - argv[1];
	/* Throw away the period (to avoid clobbering finished images from
	 * gphoto2 if such are in the same directory) and throw away the 
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
	get_image_info( info, buf, b);
	fprintf(stderr, "i=%i\n",i);
	/* Show the footer bytes */
	for (i = 0; i < HEADER_LEN; i++) {
		fprintf(stderr, " %02X", buf[b-0x10+i]);
	}
	fprintf(stderr, "\n");
	WIDTH = info->width;
	fprintf(stderr, "width is %i\n", WIDTH);
	HEIGHT = info->height;
	gamma_factor = info->gamma;
		fprintf(stderr, "default gamma factor is %f\n", gamma_factor);
	gp_gamma_fill_table(gtable, gamma_factor);

	buf2 = malloc(WIDTH * HEIGHT+256);
	if (!buf2) {
		free (buf2);
		return -1;
	}
	if (info->compression) /* non-zero if image is compressed */
		sq_decompress(buf2, buf, WIDTH,HEIGHT);
	else 
		memcpy(buf2, buf, WIDTH*HEIGHT);
	free(buf);
	ppm = malloc (WIDTH * HEIGHT * 3 + 256); /* Data + header */
	if (!ppm) {
		free (buf2);
		fprintf(stderr, "Exiting. Out of memory\n");
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
	free(buf2);
	sq_postprocess (WIDTH, HEIGHT, ptr);
	if (info-> lighting < 0x40) {
	fprintf(stderr, 
		"Low light condition. Using default gamma. No white balance.\n");
		gp_gamma_correct_single(gtable,ptr,WIDTH*HEIGHT);
	} else 
		white_balance (ptr, WIDTH*HEIGHT, 1.1);

	if ( (fp_dest = fopen(dest, "wb") ) == NULL )
	{
		fprintf (stderr, "Error opening dest file.\n");
		return 0;
	}
	fprintf(stderr, "Successful in creating %s in mode %s.\n", dest, "w");

	fwrite(ppm, 1, size, fp_dest);
	fclose (fp_dest);
	free (info);
	free (ppm);

        gtk_init (&argc, &argv);
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (window), event_box);
	gtk_widget_show(event_box);

	/* This lets the Window Manager to close the window, whereupon the
	 * program exits. */

        g_signal_connect (G_OBJECT (window), "delete_event",
                          G_CALLBACK (delete_event), NULL);
	image = gtk_image_new_from_file(dest);
	gtk_container_add(GTK_CONTAINER(event_box), image);
	gtk_widget_show (image);
        gtk_widget_show (window);
        gtk_main ();

	return 0;
}
