#include "main.h"
#include "gphoto.h"
#include "callbacks.h"
#include "util.h"
#include "live.h"
#include <stdlib.h>

/* 
	Live Camera! 
*/

extern struct _Camera *Camera;

int live_video_mode = 0;

void live_snapshot(GtkWidget *dialog) {

	int w, h;

	GdkPixmap *pixmap;
	GdkImlibImage *imlibimage;
	struct Image *im;

	GList *child = gtk_container_children(
                        GTK_CONTAINER(GTK_DIALOG(dialog)->vbox));
	GtkWidget *gpixmap = GTK_WIDGET(child->data);

	update_status("Getting live image...");
        if ((im = (*Camera->get_preview)()) == 0) {
		error_dialog("Could not get preview");
		return;
	}
	imlibimage = gdk_imlib_load_image_mem(im->image, im->image_size);
	free_image(im);
        w = imlibimage->rgb_width;
        h = imlibimage->rgb_height;
        gdk_imlib_render(imlibimage, w, h);
        pixmap = gdk_imlib_move_image(imlibimage);
        gtk_pixmap_set(GTK_PIXMAP(gpixmap), pixmap, NULL);
	update_status("Done.");
}

void live_video (GtkWidget *button, GtkWidget *dialog) {

	if (!GTK_TOGGLE_BUTTON(button)->active) {
		live_video_mode = 0;
		return;
	}
	live_video_mode = 1;
	while (live_video_mode) {
		while (gtk_events_pending())
			gtk_main_iteration();
		live_snapshot(dialog);
	}
}

void live_main () {
	int w, h;

	GtkWidget *dialog;
	GtkWidget *button, *ubutton, *cbutton, *tbutton, *hbox;

	GtkWidget *gpixmap;
	GdkPixmap *pixmap;
	GdkImlibImage *imlibimage;
	struct Image *im;

	update_status("Getting live image...");
	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Live Camera!");
	gtk_container_border_width (GTK_CONTAINER(dialog), 10);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_end_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox);

	ubutton = gtk_button_new_with_label("Update Picture");
	gtk_widget_show(ubutton);
	gtk_box_pack_start_defaults(GTK_BOX(hbox),ubutton);
        gtk_signal_connect_object(GTK_OBJECT(ubutton), "clicked",
                           GTK_SIGNAL_FUNC(live_snapshot),
                           GTK_OBJECT(dialog));

        tbutton = gtk_button_new_with_label("Take Picture");
        gtk_widget_show(tbutton);
        gtk_box_pack_start_defaults(GTK_BOX(hbox),tbutton);
        gtk_signal_connect_object(GTK_OBJECT(tbutton), "clicked",
                           GTK_SIGNAL_FUNC(takepicture_call),
                           GTK_OBJECT(dialog));

	button = gtk_check_button_new_with_label(
	"Video Mode (Warning: Heavy processor usage)");
	gtk_widget_show(button);
        gtk_box_pack_end_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		button);
        gtk_signal_connect(GTK_OBJECT(button), "clicked",
        	GTK_SIGNAL_FUNC(live_video), dialog);


	cbutton = gtk_button_new_with_label("Close");
	gtk_widget_show(cbutton);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
				  cbutton);

        gtk_signal_connect_object(GTK_OBJECT(cbutton), "clicked",
                           GTK_SIGNAL_FUNC(gtk_widget_destroy),
                           GTK_OBJECT(dialog)); 
        if ((im = (*Camera->get_preview)()) == 0) {
	        error_dialog("Could not get preview");
		return;
	}
	imlibimage = gdk_imlib_load_image_mem(im->image, im->image_size);
	free(im->image);
	free(im);
	w = imlibimage->rgb_width;
        h = imlibimage->rgb_height;
        gdk_imlib_render(imlibimage, w, h);
        pixmap = gdk_imlib_move_image(imlibimage);
	gpixmap = gtk_pixmap_new(pixmap, NULL);
	gtk_widget_show(gpixmap);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                  gpixmap);
	gtk_widget_show(dialog);
	update_status("Done.");
}
