#include "gphoto.h"
#include "util.h"
#include <stdio.h>    
#include <setjmp.h> 
#include <jpeglib.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* Read in gdk_imlib_load_image_mem for note
struct decom_JPEG_error_mgr {
	struct jpeg_error_mgr pub;
	sigjmp_buf setjmp_buffer;
};

static void jpeg_decomp_FatalErrorHandler(j_common_ptr cinfo);
static void decom_init();
static boolean decom_fill();
static void decom_skip_input_data();
static void decom_term();
*/

int utilcounter = 0; 
/* again, for the load_image_mem part. if you try to load an image
   from a filed named the same as a previously opened image, it will
   pull the other image from the mem cache. so it needs a unique filename
   for each image opened. otherwise, they'll all be the same image.
*/

void update_status(char *newStatus) {

        /*
                displays whatever is in string "newStatus" in the
                status bar at the bottom of the main window
        */

        gtk_label_set(GTK_LABEL(status_bar), newStatus);
        while (gtk_events_pending())
                gtk_main_iteration();
}

void update_progress(float percentage) {

        /*
                sets the progress bar to percentage% at the bottom
                of the main window
        */

        gtk_progress_bar_update(GTK_PROGRESS_BAR(progress), percentage);
        while (gtk_events_pending())
                gtk_main_iteration();
}


void error_dialog(char *Error) {

        /*
           Standard, run-of-the-mill message box
        */
          
        GtkWidget *dialog, *label, *button;

        dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "gPhoto Message");
        gtk_container_border_width(GTK_CONTAINER(dialog), 5);
/*	gtk_widget_set_usize(dialog, 200, 100);*/
        label = gtk_label_new(Error);
        button = gtk_button_new_with_label("OK");
        GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(dialog));
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                           label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           button, TRUE, TRUE, 0);
        gtk_widget_show(label);
        gtk_widget_show(button);
        gtk_widget_show(dialog);
        gtk_widget_grab_default (button);
}

void message_window(char *title, char *message, GtkJustification  jtype )
{
	/*
	 *  message_window
	 *
	 *  Opens a message window dialog box with a single button to
	 *  close it.  Used for displaying information to the user.  
	 *
	 *    Takes a pointer to the title string
	 *          a pointer to the message string
	 *          the gtk justification type:
	 *                 GTK_JUSTIFY_LEFT
	 *                     GTK_JUSTIFY_RIGHT
	 *                     GTK_JUSTIFY_CENTER
	 *                     GTI_JUSTIFY_FILL
	 */

        GtkWidget *dialog, *label, *button;

        dialog = gtk_dialog_new();
        gtk_window_set_title(GTK_WINDOW(dialog), title);
        gtk_container_border_width(GTK_CONTAINER(dialog), 5);
        label = gtk_label_new(message);
        gtk_label_set_justify ( GTK_LABEL(label), jtype );
        button = gtk_button_new_with_label("OK");
        GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy),
                            GTK_OBJECT(dialog));
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                           label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           button, TRUE, TRUE, 0);
        gtk_widget_show(label);
        gtk_widget_show(button);   
        gtk_widget_show(dialog);
        gtk_widget_grab_default (button);
}

void ok_click (GtkWidget *dialog) {

        gtk_object_set_data(GTK_OBJECT(dialog), "button", "OK");
        gtk_widget_hide(dialog);
}
 
int wait_for_hide (GtkWidget *dialog, GtkWidget *ok_button,
                   GtkWidget *cancel_button) {
                   
        gtk_object_set_data(GTK_OBJECT(dialog), "button", "CANCEL");
        gtk_signal_connect_object(
                        GTK_OBJECT(ok_button), "clicked",
                        GTK_SIGNAL_FUNC(ok_click),
                        GTK_OBJECT(dialog));
        gtk_signal_connect_object(GTK_OBJECT(cancel_button),
                        "clicked",
                        GTK_SIGNAL_FUNC(gtk_widget_hide),
                        GTK_OBJECT(dialog));
        gtk_widget_show(dialog);
        while (GTK_WIDGET_VISIBLE(dialog))
                gtk_main_iteration();
        if (strcmp("CANCEL",
           (char*)gtk_object_get_data(GTK_OBJECT(dialog), "button"))==0)
                return 0;
        return 1;
}

void free_image (struct Image *im) {

	/* Note to self: forget to free the image_info here */
	free (im->image);
	free (im);
}

void save_image (char *filename, struct Image *im) {

	FILE *fp;

	fp = fopen (filename, "w");
	fwrite (im->image, (size_t)sizeof(char), (size_t)im->image_size, fp);
	fclose (fp);
}


GdkImlibImage *gdk_imlib_load_image_mem(char *image, int size) {

	/* Scott was here. just a quick hash until the mem-to-mem
	   copy works on all image types. Bad thing(tm): disk hit.
	   This is being worked on.
	*/

	FILE *fp;
	char c[32], rm[48];
	GdkImlibImage *imlibimage;

	sprintf(c, "/tmp/gphoto_image_%i.jpg", utilcounter);
	utilcounter++;

	fp = fopen(c, "w");
	fwrite (image, (size_t)sizeof(char), (size_t)size, fp);
	fclose(fp);
	imlibimage = gdk_imlib_load_image(c);
	sprintf(rm, "rm %s", c);
	system(rm);

	return (imlibimage);

/*
	struct jpeg_decompress_struct cinfo;
	struct decom_JPEG_error_mgr jerr;
	unsigned char      *data, *line[16], *ptr;
	int                 x, y, i; 
	int w, h;
	GdkImlibImage *imlibimage;

	cinfo.err = jpeg_std_error(&(jerr.pub));
	jerr.pub.error_exit = jpeg_decomp_FatalErrorHandler;

	if (sigsetjmp(jerr.setjmp_buffer, 1)) {
	        jpeg_destroy_decompress(&cinfo);
        	return NULL;
 	}
   
	jpeg_create_decompress(&cinfo);   
	cinfo.src = malloc(sizeof(struct jpeg_source_mgr));
	cinfo.src->next_input_byte = image;
	cinfo.src->bytes_in_buffer = size;
	cinfo.src->init_source = decom_init;
	cinfo.src->fill_input_buffer = decom_fill;
	cinfo.src->skip_input_data = decom_skip_input_data;
	cinfo.src->resync_to_restart = jpeg_resync_to_restart;
	cinfo.src->term_source = decom_term;
	jpeg_read_header(&cinfo, TRUE);
	cinfo.do_fancy_upsampling = FALSE;
	cinfo.do_block_smoothing = FALSE;
	jpeg_start_decompress(&cinfo);
	w = cinfo.output_width;
	h = cinfo.output_height;
	data = (unsigned char *)malloc(w * h * 3);
	if (!data) {
        	jpeg_destroy_decompress(&cinfo);
        	return NULL;
     	}
   	ptr = data;
   	if (cinfo.output_components == 3) {
	        for (y = 0; y < h; y += cinfo.rec_outbuf_height) {
	            for (i = 0; i < cinfo.rec_outbuf_height; i++) {
			line[i] = ptr;
                  	ptr += w * 3;
               	    }
	            jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
          	}
     	} else if (cinfo.output_components == 1) {
	        for (i = 0; i < cinfo.rec_outbuf_height; i++) {
	             if ((line[i] = (unsigned char *)malloc(w)) == NULL) {
                  	int t = 0;
   			for (t = 0; t < i; t++)
                     	free(line[t]);
                  	jpeg_destroy_decompress(&cinfo);
                  	return NULL;
               	     }
          	}
        	for (y = 0; y < h; y += cinfo.rec_outbuf_height) {
	            jpeg_read_scanlines(&cinfo, line,cinfo.rec_outbuf_height);
             	    for (i = 0; i < cinfo.rec_outbuf_height; i++) {
                  	for (x = 0; x < w; x++) {
                       		*ptr++ = line[i][x];
                       		*ptr++ = line[i][x];
                       		*ptr++ = line[i][x];
                     	}
               	    }
          	}
        	for (i = 0; i < cinfo.rec_outbuf_height; i++)
           		free(line[i]);
	}
     	free(cinfo.src);
   	jpeg_finish_decompress(&cinfo);
   	jpeg_destroy_decompress(&cinfo);
      	imlibimage = gdk_imlib_create_image_from_data(data, NULL, w, h);
    	free(data);
    	return imlibimage;
*/
}

/*
void jpeg_decomp_FatalErrorHandler(j_common_ptr cinfo) {
	struct decom_JPEG_error_mgr               *errmgr;
 
	errmgr = (struct decom_JPEG_error_mgr *) cinfo->err;
	cinfo->err->output_message(cinfo);
	siglongjmp(errmgr->setjmp_buffer, 1);
	return;
}

METHODDEF(void) decom_init (j_decompress_ptr cinfo) {
	return;
}

METHODDEF(boolean) decom_fill(struct jpeg_decompress_struct *cinfo) {
    unsigned char *ptr;
    ptr = (unsigned char*)cinfo->src->next_input_byte;
    ptr[0] = (JOCTET) 0xFF;
    ptr[1] = (JOCTET) JPEG_EOI;
    cinfo->src->bytes_in_buffer = 2;

    return TRUE;
}

METHODDEF(void) decom_skip_input_data (j_decompress_ptr cinfo, 
				       long num_bytes) {
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
}

METHODDEF(void) decom_term (j_decompress_ptr cinfo) {
	return;
}
*/
