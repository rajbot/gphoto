#include "main.h"
#include "gphoto.h"
#include "live.h"
#include "callbacks.h"

/* 
	Live Camera! 
*/

void live_snapshot(GtkWidget *dialog) {

	int w, h;

	GdkPixmap *pixmap;
	GdkImlibImage *imlibimage;

	GList *child = gtk_container_children(
                        GTK_CONTAINER(GTK_DIALOG(dialog)->vbox));
	GtkWidget *gpixmap = GTK_WIDGET(child->data);

	update_status("Getting live image...");
        if ((imlibimage = (*Camera->get_preview)()) == 0)
		return;
        w = imlibimage->rgb_width;
        h = imlibimage->rgb_height;
        gdk_imlib_render(imlibimage, w, h);
        pixmap = gdk_imlib_move_image(imlibimage);
        gtk_pixmap_set(GTK_PIXMAP(gpixmap), pixmap, NULL);
	update_status("Done.");
}

void live_main () {

	int w, h;

	GtkWidget *dialog;
	GtkWidget *button;

	GtkWidget *gpixmap;
	GdkPixmap *pixmap;
	GdkImlibImage *imlibimage;
	
	update_status("Getting live image...");
	dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), "Live Camera!");
	gtk_container_border_width (GTK_CONTAINER(dialog), 10);
        button = gtk_button_new_with_label("Take Picture");
        gtk_widget_show(button);
        gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                  button, FALSE, FALSE, 0);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                           GTK_SIGNAL_FUNC(takepicture_call),
                           GTK_OBJECT(dialog));
	button = gtk_button_new_with_label("Live update");
	gtk_widget_show(button);
	gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->vbox),
				  button, FALSE, FALSE, 0);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                           GTK_SIGNAL_FUNC(live_snapshot),
                           GTK_OBJECT(dialog));
	button = gtk_button_new_with_label("Close");
	gtk_widget_show(button);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->action_area),
				  button);
        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                           GTK_SIGNAL_FUNC(gtk_widget_destroy),
                           GTK_OBJECT(dialog)); 
	if ((imlibimage = (*Camera->get_preview)()) == 0) {
		error_dialog("Could not get preview.");
		return;
	}
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
